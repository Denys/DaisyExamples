[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$HelperPath,
    [Parameter(Mandatory = $true)]
    [string]$PluginPath,
    [Parameter(Mandatory = $true)]
    [string]$OutputPath,
    [Parameter(Mandatory = $true)]
    [string]$Version
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$resolvedHelperPath = (Resolve-Path -LiteralPath $HelperPath).Path
$resolvedPluginPath = (Resolve-Path -LiteralPath $PluginPath).Path
$outputDirectory = Split-Path -Parent $OutputPath

New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null

Write-Host "creating $OutputPath"
& $resolvedHelperPath -create -version $Version -path $resolvedPluginPath -output $OutputPath

if ($LASTEXITCODE -ne 0) {
    throw "juce_vst3_helper failed with exit code ${LASTEXITCODE}"
}
