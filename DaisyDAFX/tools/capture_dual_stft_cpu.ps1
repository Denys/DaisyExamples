param(
  [string]$Port,
  [int]$BaudRate = 115200,
  [int]$Seconds = 65,
  [string]$BenchmarkJson = "docs\benchmarks\2026-06-05-dual-stft-results.json",
  [string]$OutputPath,
  [string]$InputLog,
  [string]$BackendName,
  [int]$FftSize = 0,
  [int]$HopSize = 0,
  [int]$BlockSize = 0,
  [switch]$UpdateBenchmark
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-DefaultOutputPath {
  $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
  return ".tmp\dual-stft-cpu-capture-$stamp.json"
}

function Convert-ToDoubleInvariant([string]$Value) {
  return [double]::Parse($Value, [System.Globalization.CultureInfo]::InvariantCulture)
}

if ($Seconds -lt 5) {
  Write-Error "-Seconds must be at least 5 so the firmware can emit multiple CPU lines."
}

if ([string]::IsNullOrWhiteSpace($OutputPath)) {
  $OutputPath = Get-DefaultOutputPath
}

$lines = New-Object System.Collections.Generic.List[object]
$samples = New-Object System.Collections.Generic.List[object]
$detectedBackend = $BackendName
$detectedFft = $FftSize
$detectedHop = $HopSize
$detectedBlock = $BlockSize
$startedAt = Get-Date
$deadline = $startedAt.AddSeconds($Seconds)

function Add-TelemetryLine([string]$Line) {
  $line = $Line.Trim()
  if ([string]::IsNullOrWhiteSpace($line)) {
    return
  }

  $timestamp = (Get-Date).ToString("o")
  $lines.Add([pscustomobject]@{
    timestamp = $timestamp
    text = $line
  }) | Out-Null
  Write-Output "[$timestamp] $line"

  if ($line -match '^Backend:\s*(?<backend>.+)$') {
    $script:detectedBackend = $Matches.backend.Trim()
    return
  }

  if ($line -match '^FFT/Hop/Block:\s*(?<fft>\d+)\s*/\s*(?<hop>\d+)\s*/\s*(?<block>\d+)') {
    $script:detectedFft = [int]$Matches.fft
    $script:detectedHop = [int]$Matches.hop
    $script:detectedBlock = [int]$Matches.block
    return
  }

  if ($line -match '^CPU max/avg/min %:\s*(?<max>[-+]?\d+(?:\.\d+)?)\s*/\s*(?<avg>[-+]?\d+(?:\.\d+)?)\s*/\s*(?<min>[-+]?\d+(?:\.\d+)?)') {
    $samples.Add([pscustomobject]@{
      timestamp = $timestamp
      max_cpu_percent = Convert-ToDoubleInvariant $Matches.max
      average_cpu_percent = Convert-ToDoubleInvariant $Matches.avg
      min_cpu_percent = Convert-ToDoubleInvariant $Matches.min
    }) | Out-Null
  }
}

$sourceLabel = $Port
if (-not [string]::IsNullOrWhiteSpace($InputLog)) {
  if (-not (Test-Path -LiteralPath $InputLog)) {
    Write-Error "Input log not found: $InputLog"
  }

  $sourceLabel = $InputLog
  Write-Output "Parsing saved telemetry log: $InputLog"
  foreach ($line in Get-Content -LiteralPath $InputLog) {
    Add-TelemetryLine $line
  }
} else {
  $availablePorts = @([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object)
  if ([string]::IsNullOrWhiteSpace($Port)) {
    if ($availablePorts.Count -eq 0) {
      Write-Error "No serial ports are visible. Connect the Daisy Seed and pass -Port COMx, or pass -InputLog with a saved serial log."
    }

    Write-Output "Available serial ports: $($availablePorts -join ', ')"
    Write-Error "Pass -Port with the Daisy serial port to capture CPU telemetry, or pass -InputLog with a saved serial log."
  }

  if ($availablePorts.Count -gt 0 -and $availablePorts -notcontains $Port) {
    Write-Warning "Port '$Port' is not in the currently visible port list: $($availablePorts -join ', ')"
  }

  $serial = [System.IO.Ports.SerialPort]::new($Port, $BaudRate)
  $serial.ReadTimeout = 1000
  $serial.NewLine = "`n"
  $serial.DtrEnable = $true
  $serial.RtsEnable = $true

  try {
    $serial.Open()
    Write-Output "Capturing $Seconds seconds from $Port at $BaudRate baud..."

    while ((Get-Date) -lt $deadline) {
      try {
        Add-TelemetryLine $serial.ReadLine()
      } catch [System.TimeoutException] {
        continue
      }
    }
  } finally {
    if ($serial.IsOpen) {
      $serial.Close()
    }
    $serial.Dispose()
  }
}

if ($samples.Count -eq 0) {
  $capture = [pscustomobject]@{
    measured_at = $startedAt.ToString("o")
    port = $Port
    baud_rate = $BaudRate
    duration_seconds = $Seconds
    source = $sourceLabel
    backend_name = $detectedBackend
    fft_size = $detectedFft
    hop_size = $detectedHop
    block_size = $detectedBlock
    sample_count = 0
    raw_lines = $lines
  }
  $dir = Split-Path -Parent $OutputPath
  if (-not [string]::IsNullOrWhiteSpace($dir)) {
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
  }
  $capture | ConvertTo-Json -Depth 8 | Set-Content -Path $OutputPath -Encoding utf8
  Write-Error "No CPU samples were captured. Wrote raw capture to $OutputPath."
}

$maxCpu = ($samples | Measure-Object -Property max_cpu_percent -Maximum).Maximum
$avgCpu = ($samples | Measure-Object -Property average_cpu_percent -Average).Average
$minCpu = ($samples | Measure-Object -Property min_cpu_percent -Minimum).Minimum

$result = [pscustomobject]@{
  measured_at = $startedAt.ToString("o")
  port = $Port
  baud_rate = $BaudRate
  duration_seconds = $Seconds
  source = $sourceLabel
  backend_name = $detectedBackend
  fft_size = $detectedFft
  hop_size = $detectedHop
  block_size = $detectedBlock
  sample_count = $samples.Count
  max_cpu_percent = [Math]::Round($maxCpu, 3)
  average_cpu_percent = [Math]::Round($avgCpu, 3)
  min_cpu_percent = [Math]::Round($minCpu, 3)
  samples = $samples
  raw_lines = $lines
}

$outputDir = Split-Path -Parent $OutputPath
if (-not [string]::IsNullOrWhiteSpace($outputDir)) {
  New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}
$result | ConvertTo-Json -Depth 8 | Set-Content -Path $OutputPath -Encoding utf8
Write-Output "Wrote capture JSON: $OutputPath"
Write-Output ("Summary: backend={0}, profile={1}/{2}/{3}, max={4}%, avg={5}%, samples={6}" -f `
  $detectedBackend, $detectedFft, $detectedHop, $detectedBlock, `
  $result.max_cpu_percent, $result.average_cpu_percent, $samples.Count)

if ($UpdateBenchmark) {
  if ([string]::IsNullOrWhiteSpace($detectedBackend) -or $detectedFft -eq 0 -or $detectedHop -eq 0 -or $detectedBlock -eq 0) {
    Write-Error "Cannot update benchmark JSON because backend/profile metadata was not detected. Pass -BackendName, -FftSize, -HopSize, and -BlockSize."
  }

  if (-not (Test-Path -LiteralPath $BenchmarkJson)) {
    Write-Error "Benchmark JSON not found: $BenchmarkJson"
  }

  $benchmark = Get-Content -LiteralPath $BenchmarkJson -Raw | ConvertFrom-Json
  $matches = @($benchmark.results | Where-Object {
      $_.backend_name -eq $detectedBackend -and
      [int]$_.fft_size -eq $detectedFft -and
      [int]$_.hop_size -eq $detectedHop -and
      [int]$_.block_size -eq $detectedBlock
    })

  if ($matches.Count -ne 1) {
    Write-Error "Expected one benchmark entry for $detectedBackend $detectedFft/$detectedHop/$detectedBlock, found $($matches.Count)."
  }

  $entry = $matches[0]
  $entry.max_cpu_percent = $result.max_cpu_percent
  $entry.average_cpu_percent = $result.average_cpu_percent
  $entry.cpu_evidence = "measured Daisy runtime via tools/capture_dual_stft_cpu.ps1 on $($result.measured_at) from $sourceLabel for $Seconds seconds"
  $entry | Add-Member -NotePropertyName cpu_evidence_label -NotePropertyValue "measured Daisy runtime" -Force
  $entry | Add-Member -NotePropertyName cpu_measurement -NotePropertyValue ([pscustomobject]@{
      tool = "tools/capture_dual_stft_cpu.ps1"
      measured_at = $result.measured_at
      port = $Port
      source = $sourceLabel
      baud_rate = $BaudRate
      duration_seconds = $Seconds
      sample_count = $samples.Count
      capture_json = $OutputPath
      min_cpu_percent = $result.min_cpu_percent
    }) -Force

  if ($entry.known_limitations) {
    $entry.known_limitations = @($entry.known_limitations | Where-Object {
        $_ -notmatch 'Actual Daisy CPU max/avg'
      })
  }

  $benchmark | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $BenchmarkJson -Encoding utf8
  Write-Output "Updated benchmark JSON: $BenchmarkJson"
}
