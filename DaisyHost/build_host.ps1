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
    # The PowerShell Env: provider can throw before cleanup when the process
    # contains both Path and PATH, so normalize via .NET instead.
    $existingPath = [System.Environment]::GetEnvironmentVariable("PATH", "Process")
    if (-not $existingPath) {
        $existingPath = [System.Environment]::GetEnvironmentVariable("Path", "Process")
    }

    if ($existingPath) {
        [System.Environment]::SetEnvironmentVariable("PATH", $null, "Process")
        [System.Environment]::SetEnvironmentVariable("Path", $existingPath, "Process")
    }

    if (-not [System.Environment]::GetEnvironmentVariable("Path", "Process")) {
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
$unitTestRunTag = (Get-Date).ToUniversalTime().ToString("yyyyMMddHHmmssfff")

Normalize-PathEnvironment

if (-not $SkipConfigure) {
    Invoke-Step @(
        "cmake",
        "-S",
        $workspaceRoot,
        "-B",
        $resolvedBuildDir,
        "-DDAISYHOST_UNIT_TEST_RUN_TAG=$unitTestRunTag"
    )
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
        "DaisyHostCLI",
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
