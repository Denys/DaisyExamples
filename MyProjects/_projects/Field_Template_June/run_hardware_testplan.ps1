param(
    [switch]$Flash,
    [switch]$SkipBuild,
    [switch]$NonInteractive,
    [string]$Port,
    [int]$Baud = 115200,
    [int]$CaptureSeconds = 20,
    [switch]$NoSerial,
    [string]$EvidenceDir = ".\hardware_evidence"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location -LiteralPath $ProjectRoot

if([System.IO.Path]::IsPathRooted($EvidenceDir)) {
    $EvidenceRoot = $EvidenceDir
}
else {
    $EvidenceRoot = Join-Path $ProjectRoot $EvidenceDir
}

$RunStamp = Get-Date -Format "yyyyMMdd-HHmmss"
$RunDir = Join-Path $EvidenceRoot $RunStamp
New-Item -ItemType Directory -Force -Path $RunDir | Out-Null

$Summary = [ordered]@{
    project = "Field_Template_June"
    started = (Get-Date).ToString("o")
    evidence_dir = $RunDir
    non_interactive = [bool]$NonInteractive
    overall_status = "UNKNOWN"
    build = [ordered]@{ status = "SKIPPED"; exit_code = $null; log = $null; reason = $null }
    qae = [ordered]@{ status = "SKIPPED"; exit_code = $null; log = $null; reason = $null }
    flash = [ordered]@{ status = "SKIPPED"; exit_code = $null; log = $null; reason = "Run with -Flash to call make program." }
    serial = [ordered]@{ status = "SKIPPED"; port = $Port; baud = $Baud; log = $null; commands_log = $null; reason = $null }
    checks = [ordered]@{}
    limitations = @(
        "Serial telemetry cannot physically move knobs, press Field keys, or prove OLED visibility.",
        "Audio output requires an audio interface or measurement rig for unattended pass/fail validation.",
        "MIDI input can be observed if an external MIDI source or scripted MIDI rig is connected during capture."
    )
    error = $null
}

function Save-Summary {
    $Summary.finished = (Get-Date).ToString("o")
    $SummaryPath = Join-Path $RunDir "summary.json"
    $Summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $SummaryPath -Encoding UTF8
    Write-Host ""
    Write-Host "Summary: $SummaryPath"
}

function Invoke-LoggedCommand {
    param(
        [string]$Label,
        [string]$LogFile,
        [scriptblock]$Command
    )

    Write-Host ""
    Write-Host "== $Label =="
    $LogPath = Join-Path $RunDir $LogFile
    $global:LASTEXITCODE = 0
    $Output = & $Command 2>&1
    $ExitCode = $global:LASTEXITCODE
    if($null -eq $ExitCode) {
        $ExitCode = 0
    }
    foreach($Item in $Output) {
        $Line = $Item.ToString()
        Add-Content -LiteralPath $LogPath -Value $Line
        Write-Host $Line
    }

    if($ExitCode -ne 0) {
        throw "$Label failed with exit code $ExitCode. See $LogPath"
    }

    return [ordered]@{ status = "PASS"; exit_code = $ExitCode; log = $LogPath; reason = $null }
}

function New-SerialPort {
    param([string]$PortName)

    $Serial = New-Object System.IO.Ports.SerialPort
    $Serial.PortName = $PortName
    $Serial.BaudRate = $Baud
    $Serial.Parity = [System.IO.Ports.Parity]::None
    $Serial.DataBits = 8
    $Serial.StopBits = [System.IO.Ports.StopBits]::One
    $Serial.ReadTimeout = 500
    $Serial.WriteTimeout = 500
    $Serial.DtrEnable = $true
    $Serial.RtsEnable = $true
    $Serial.NewLine = "`n"
    $Serial.Open()
    return $Serial
}

function Read-SerialUntil {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [datetime]$Until,
        [string]$LogPath
    )

    while((Get-Date) -lt $Until) {
        try {
            $Line = $Serial.ReadLine().TrimEnd("`r", "`n")
            if($Line.Length -gt 0) {
                Add-Content -LiteralPath $LogPath -Value $Line
                Write-Host $Line
            }
        }
        catch [System.TimeoutException] {
        }
    }
}

function Test-PortForMarker {
    param([string]$Candidate)

    $Serial = $null
    try {
        $Serial = New-SerialPort -PortName $Candidate
        $Serial.DiscardInBuffer()
        $Deadline = (Get-Date).AddSeconds(4)
        while((Get-Date) -lt $Deadline) {
            try {
                $Line = $Serial.ReadLine()
                if($Line -match "\[FTJUNE\]") {
                    return $true
                }
            }
            catch [System.TimeoutException] {
            }
        }
    }
    catch {
        return $false
    }
    finally {
        if($null -ne $Serial -and $Serial.IsOpen) {
            $Serial.Close()
        }
    }

    return $false
}

function Find-FtjunePort {
    if($Port) {
        return $Port
    }

    $Ports = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
    if($Ports.Count -eq 0) {
        return $null
    }

    foreach($Candidate in $Ports) {
        if(Test-PortForMarker -Candidate $Candidate) {
            return $Candidate
        }
    }

    if($Ports.Count -eq 1) {
        Write-Host "No FTJUNE marker seen yet; using the only serial port: $($Ports[0])"
        return $Ports[0]
    }

    return $null
}

function Invoke-SerialCapture {
    $LogPath = Join-Path $RunDir "serial.log"
    $CommandsPath = Join-Path $RunDir "serial_commands.log"
    Set-Content -LiteralPath $LogPath -Value ""
    Set-Content -LiteralPath $CommandsPath -Value ""

    $PortName = Find-FtjunePort
    if(-not $PortName) {
        $Summary.serial.status = "BLOCKED"
        $Summary.serial.reason = "No unique FTJUNE serial port found. Pass -Port COMx after checking Device Manager."
        $Summary.serial.log = $LogPath
        $Summary.serial.commands_log = $CommandsPath
        return
    }

    $Summary.serial.port = $PortName
    $Summary.serial.log = $LogPath
    $Summary.serial.commands_log = $CommandsPath

    Write-Host ""
    Write-Host "== Serial capture on $PortName @ $Baud =="

    $Serial = $null
    try {
        $Serial = New-SerialPort -PortName $PortName
        $Serial.DiscardInBuffer()

        $WarmupUntil = (Get-Date).AddSeconds(2)
        Read-SerialUntil -Serial $Serial -Until $WarmupUntil -LogPath $LogPath

        foreach($Command in @("HELP", "SELFTEST", "STATUS", "SNAP")) {
            Add-Content -LiteralPath $CommandsPath -Value ("SEND " + $Command)
            $Serial.WriteLine($Command)
            Read-SerialUntil -Serial $Serial -Until (Get-Date).AddSeconds(2) -LogPath $LogPath
        }

        $RemainingUntil = (Get-Date).AddSeconds([Math]::Max(1, $CaptureSeconds))
        Read-SerialUntil -Serial $Serial -Until $RemainingUntil -LogPath $LogPath
        $Summary.serial.status = "PASS"
    }
    catch {
        $Summary.serial.status = "FAIL"
        $Summary.serial.reason = $_.Exception.Message
    }
    finally {
        if($null -ne $Serial -and $Serial.IsOpen) {
            $Serial.Close()
        }
    }
}

function Add-SerialChecks {
    if($Summary.serial.status -ne "PASS") {
        $Summary.checks.boot_marker = "BLOCKED"
        $Summary.checks.selftest_pass = "BLOCKED"
        $Summary.checks.status_seen = "BLOCKED"
        $Summary.checks.snapshot_seen = "BLOCKED"
        return
    }

    $Text = ""
    if(Test-Path -LiteralPath $Summary.serial.log) {
        $Text = Get-Content -Raw -LiteralPath $Summary.serial.log
    }

    $Summary.checks.boot_marker = if($Text -match "\[FTJUNE\] BOOT") { "PASS" } else { "FAIL" }
    $Summary.checks.selftest_pass = if($Text -match "\[FTJUNE\] SELFTEST .*result=PASS") { "PASS" } else { "FAIL" }
    $Summary.checks.status_seen = if($Text -match "\[FTJUNE\] STATUS") { "PASS" } else { "FAIL" }
    $Summary.checks.snapshot_seen = if($Text -match "\[FTJUNE\] SNAP") { "PASS" } else { "FAIL" }
}

function Update-OverallStatus {
    if($null -ne $Summary.error) {
        $Summary.overall_status = "FAIL"
        return
    }

    if($NoSerial) {
        $Summary.overall_status = "PARTIAL_NO_SERIAL"
        return
    }

    $ChecksPass = $true
    foreach($Name in @("boot_marker", "selftest_pass", "status_seen", "snapshot_seen")) {
        if($Summary.checks[$Name] -ne "PASS") {
            $ChecksPass = $false
        }
    }

    if($Summary.serial.status -eq "PASS" -and $ChecksPass) {
        $Summary.overall_status = "PASS"
    }
    else {
        $Summary.overall_status = "FAIL_OR_BLOCKED"
    }
}

$ExitCode = 0
try {
    if(-not $SkipBuild) {
        $Summary.build = Invoke-LoggedCommand "Build Field_Template_June" "build.log" {
            make
        }

        $Summary.qae = Invoke-LoggedCommand "Run Daisy QAE validator" "qae.log" {
            $env:PYTHONIOENCODING = "utf-8"
            py -3 ../../../DAISY_QAE/validate_daisy_code.py .
        }
    }
    else {
        $Summary.build.reason = "Skipped by -SkipBuild."
        $Summary.qae.reason = "Skipped by -SkipBuild."
    }

    if($Flash) {
        $Summary.flash = Invoke-LoggedCommand "Flash with ST-Link" "flash.log" {
            make program
        }
    }

    if($NoSerial) {
        $Summary.serial.reason = "Skipped by -NoSerial."
    }
    else {
        Invoke-SerialCapture
    }

    Add-SerialChecks
    Update-OverallStatus

    if($Summary.overall_status -eq "FAIL_OR_BLOCKED") {
        $ExitCode = 1
    }
}
catch {
    $Summary.error = $_.Exception.Message
    $ExitCode = 1
    Write-Host ""
    Write-Host "ERROR: $($Summary.error)"
    Update-OverallStatus
}
finally {
    Save-Summary
}

exit $ExitCode
