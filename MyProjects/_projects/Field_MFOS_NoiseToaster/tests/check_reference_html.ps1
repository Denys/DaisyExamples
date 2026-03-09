$ErrorActionPreference = 'Stop'

$htmlPath = Join-Path $PSScriptRoot '..\Field_MFOS_NoiseToaster_reference.html'
$htmlPath = [System.IO.Path]::GetFullPath($htmlPath)

if (-not (Test-Path $htmlPath)) {
    throw "Missing artifact: $htmlPath"
}

$content = Get-Content -Raw $htmlPath

$requiredPatterns = @(
    'Firmware Block Diagram',
    'Front Panel Mapping',
    'Interactive Brainstorm Mode',
    'K1 -> VCO Frequency',
    'B3 -> VCF Mod Source',
    'SW1 -> Manual Gate',
    'LFO Rate = 2.2 Hz',
    'Noise Blend = 18%',
    'Output Level = 72%'
)

foreach ($pattern in $requiredPatterns) {
    if ($content -notmatch [regex]::Escape($pattern)) {
        throw "Missing required content: $pattern"
    }
}

Write-Output "PASS: HTML reference artifact contains required sections and mappings."
