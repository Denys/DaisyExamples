param(
  [string]$Port = "COM3",
  [int]$Seconds = 65,
  [string]$BenchmarkJson = "docs\benchmarks\2026-06-05-dual-stft-results.json",
  [string[]]$Variants = @("fast512", "dafx512"),
  [switch]$Include1024,
  [switch]$Program,
  [int]$PostProgramDelaySeconds = 4,
  [string]$OpenOcdScriptsDir = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$firmwareDir = Join-Path $repoRoot "examples\daisy_dual_stft"
$captureScript = Join-Path $repoRoot "tools\capture_dual_stft_cpu.ps1"

$variantTable = @{
  fast512 = @{
    label = "FastStftBackend 512/128/64"
    defs = "-DSTFT_BOARD_FIELD -DSTFT_BACKEND_FAST -DSTFT_PROFILE_512"
    backend = "FastStftBackend"
    fft = 512
    hop = 128
    block = 64
  }
  dafx512 = @{
    label = "DafxStftEnvBackend 512/128/64"
    defs = "-DSTFT_BOARD_FIELD -DSTFT_BACKEND_DAFX -DSTFT_PROFILE_512"
    backend = "DafxStftEnvBackend"
    fft = 512
    hop = 128
    block = 64
  }
  fast1024 = @{
    label = "FastStftBackend 1024/256/64"
    defs = "-DSTFT_BOARD_FIELD -DSTFT_BACKEND_FAST -DSTFT_PROFILE_1024"
    backend = "FastStftBackend"
    fft = 1024
    hop = 256
    block = 64
  }
  dafx1024 = @{
    label = "DafxStftEnvBackend 1024/256/64"
    defs = "-DSTFT_BOARD_FIELD -DSTFT_BACKEND_DAFX -DSTFT_PROFILE_1024"
    backend = "DafxStftEnvBackend"
    fft = 1024
    hop = 256
    block = 64
  }
}

if ($Include1024) {
  $Variants = @("fast512", "dafx512", "fast1024", "dafx1024")
} else {
  $Variants = @($Variants | ForEach-Object {
      $_ -split ","
    } | ForEach-Object {
      $_.Trim()
    } | Where-Object {
      -not [string]::IsNullOrWhiteSpace($_)
    })
}

foreach ($variant in $Variants) {
  if (-not $variantTable.ContainsKey($variant)) {
    Write-Error "Unknown variant '$variant'. Valid variants: $($variantTable.Keys -join ', ')"
  }
}

function Invoke-CheckedCommand([string]$FilePath, [string[]]$Arguments, [string]$WorkingDirectory) {
  Write-Output ">>> $FilePath $($Arguments -join ' ')"
  Push-Location $WorkingDirectory
  try {
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
      throw "Command failed with exit code ${LASTEXITCODE}: $FilePath $($Arguments -join ' ')"
    }
  } finally {
    Pop-Location
  }
}

function Wait-ForSerialPort([string]$Name, [int]$TimeoutSeconds) {
  $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
  while ((Get-Date) -lt $deadline) {
    $ports = @([System.IO.Ports.SerialPort]::GetPortNames())
    if ($ports -contains $Name) {
      return
    }
    Start-Sleep -Milliseconds 500
  }
  Write-Error "Serial port '$Name' was not visible within $TimeoutSeconds seconds."
}

function Get-SerialPortInfo {
  try {
    return @(Get-CimInstance Win32_SerialPort | Select-Object DeviceID, Name, Description, PNPDeviceID)
  } catch {
    return @()
  }
}

function Resolve-CapturePort([string]$RequestedPort, [int]$TimeoutSeconds) {
  $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
  while ((Get-Date) -lt $deadline) {
    $ports = @(Get-SerialPortInfo)
    $requested = @($ports | Where-Object { $_.DeviceID -eq $RequestedPort })
    $daisyUsbPorts = @($ports | Where-Object {
        $_.PNPDeviceID -match "VID_0483&PID_5740"
      })

    if ($requested.Count -eq 1 -and $requested[0].PNPDeviceID -match "VID_0483&PID_5740") {
      return $RequestedPort
    }

    if ($daisyUsbPorts.Count -eq 1) {
      $resolved = $daisyUsbPorts[0].DeviceID
      if ($resolved -ne $RequestedPort) {
        Write-Host "Resolved capture port from $RequestedPort to $resolved ($($daisyUsbPorts[0].Name))."
      }
      return $resolved
    }

    if ($requested.Count -eq 1 -and $requested[0].PNPDeviceID -notmatch "VID_0483&PID_374E") {
      return $RequestedPort
    }

    Start-Sleep -Milliseconds 500
  }

  $visible = @([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object)
  Write-Error "Could not resolve a Daisy USB serial logger port from '$RequestedPort' within $TimeoutSeconds seconds. Visible ports: $($visible -join ', ')"
}

function Get-ShortDirectoryPath([string]$DirectoryPath) {
  $resolved = (Resolve-Path -LiteralPath $DirectoryPath).Path
  try {
    $fso = New-Object -ComObject Scripting.FileSystemObject
    $shortPath = $fso.GetFolder($resolved).ShortPath
    if (-not [string]::IsNullOrWhiteSpace($shortPath) -and (Test-Path -LiteralPath $shortPath)) {
      return $shortPath.Replace("\", "/")
    }
  } catch {
    # Fall back to the resolved path below.
  }
  return $resolved.Replace("\", "/")
}

function Resolve-OpenOcdScriptsDir([string]$ConfiguredPath) {
  $candidates = @()
  if (-not [string]::IsNullOrWhiteSpace($ConfiguredPath)) {
    $candidates += $ConfiguredPath
  }
  $candidates += @(
    "C:\Program Files\DaisyToolchain\openocd\scripts",
    "C:\DaisyToolchain\openocd\scripts"
  )

  foreach ($candidate in $candidates) {
    if ([string]::IsNullOrWhiteSpace($candidate) -or -not (Test-Path -LiteralPath $candidate)) {
      continue
    }
    $stlinkCfg = Join-Path $candidate "interface\stlink.cfg"
    $stm32h7Cfg = Join-Path $candidate "target\stm32h7x.cfg"
    if ((Test-Path -LiteralPath $stlinkCfg) -and (Test-Path -LiteralPath $stm32h7Cfg)) {
      $makePath = Get-ShortDirectoryPath $candidate
      if ($makePath -match "\s") {
        Write-Error "OpenOCD scripts path contains whitespace and no usable short path was available: $makePath. Pass -OpenOcdScriptsDir with a path that Make can use without quoting."
      }
      return $makePath
    }
  }

  Write-Error "OpenOCD ST-Link scripts were not found. Pass -OpenOcdScriptsDir pointing to a directory containing interface\stlink.cfg and target\stm32h7x.cfg."
}

if (-not (Test-Path -LiteralPath $firmwareDir)) {
  Write-Error "Firmware directory not found: $firmwareDir"
}

if (-not (Test-Path -LiteralPath $captureScript)) {
  Write-Error "Capture script not found: $captureScript"
}

Write-Output "Daisy Field dual STFT measurement runner"
Write-Output "Variants: $($Variants -join ', ')"
Write-Output "Port: $Port"
Write-Output "Program mode: $($Program.IsPresent)"

$programOpenOcdArg = $null
if ($Program) {
  $resolvedOpenOcdScriptsDir = Resolve-OpenOcdScriptsDir $OpenOcdScriptsDir
  $programOpenOcdArg = "OCD_DIR=$resolvedOpenOcdScriptsDir"
  Write-Output "OpenOCD scripts: $resolvedOpenOcdScriptsDir"
}

foreach ($variantName in $Variants) {
  $variant = $variantTable[$variantName]
  $stftArg = "STFT_DEFS=$($variant.defs)"

  Write-Output ""
  Write-Output "=== $($variant.label) ==="
  Invoke-CheckedCommand "make" @("clean") $firmwareDir
  Invoke-CheckedCommand "make" @($stftArg) $firmwareDir

  if (-not $Program) {
    Write-Output "Build verified for $($variant.label). Re-run with -Program to flash and capture CPU telemetry."
    continue
  }

  Invoke-CheckedCommand "make" @($stftArg, $programOpenOcdArg, "program") $firmwareDir
  Start-Sleep -Seconds $PostProgramDelaySeconds
  $capturePort = Resolve-CapturePort $Port 20
  Wait-ForSerialPort $capturePort 15

  $captureArgs = @(
    "-NoProfile",
    "-ExecutionPolicy",
    "Bypass",
    "-File",
    $captureScript,
    "-Port",
    $capturePort,
    "-Seconds",
    "$Seconds",
    "-BenchmarkJson",
    (Join-Path $repoRoot $BenchmarkJson),
    "-BackendName",
    $variant.backend,
    "-FftSize",
    "$($variant.fft)",
    "-HopSize",
    "$($variant.hop)",
    "-BlockSize",
    "$($variant.block)",
    "-UpdateBenchmark"
  )
  Invoke-CheckedCommand "powershell" $captureArgs $repoRoot
}

Write-Output ""
if ($Program) {
  Write-Output "Measurement run finished. Inspect $BenchmarkJson for measured CPU values."
} else {
  Write-Output "Build-only run finished. No flashing or benchmark update was attempted."
}
