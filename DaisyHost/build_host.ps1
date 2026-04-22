[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Configuration = "Release",
    [string]$BuildDir = "build",
    [switch]$SkipConfigure,
    [switch]$SkipBuild,
    [switch]$SkipTests
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Normalize-PathEnvironment {
    # Mirror the known-good manual fix already used in this checkout.
    if (Test-Path Env:PATH) {
        $env:Path = $env:PATH
        Remove-Item Env:PATH -ErrorAction SilentlyContinue
    }

    if (-not $env:Path) {
        throw "Env:Path is not set after PATH normalization."
    }
}

function Invoke-Step {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Command
    )

    Write-Host ("+ " + ($Command -join " "))

    $executable = $Command[0]
    $arguments = @()
    if ($Command.Length -gt 1) {
        $arguments = $Command[1..($Command.Length - 1)]
    }

    & $executable @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $($Command -join ' ')"
    }
}

$workspaceRoot = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$resolvedBuildDir = if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    $BuildDir
} else {
    Join-Path $workspaceRoot $BuildDir
}

Normalize-PathEnvironment

if (-not $SkipConfigure) {
    Invoke-Step @("cmake", "-S", $workspaceRoot, "-B", $resolvedBuildDir)
}

if (-not $SkipBuild) {
    Invoke-Step @(
        "cmake",
        "--build",
        $resolvedBuildDir,
        "--config",
        $Configuration,
        "--target",
        "unit_tests",
        "DaisyHostHub",
        "DaisyHostRender",
        "DaisyHostPatch_VST3",
        "DaisyHostPatch_Standalone"
    )
}

if (-not $SkipTests) {
    Invoke-Step @(
        "ctest",
        "--test-dir",
        $resolvedBuildDir,
        "-C",
        $Configuration,
        "--output-on-failure"
    )
}
