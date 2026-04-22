[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$Action = "help",

    [Parameter(Position = 1, ValueFromRemainingArguments = $true)]
    [string[]]$Arguments
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Show-Usage {
    @"
Usage:
  .\docs\notebooklm\daisybrain.ps1 info
  .\docs\notebooklm\daisybrain.ps1 id
  .\docs\notebooklm\daisybrain.ps1 use
  .\docs\notebooklm\daisybrain.ps1 ask [notebooklm ask args...]
  .\docs\notebooklm\daisybrain.ps1 source-list
  .\docs\notebooklm\daisybrain.ps1 source-add [notebooklm source add args...]
  .\docs\notebooklm\daisybrain.ps1 source-wait [notebooklm source wait args...]
  .\docs\notebooklm\daisybrain.ps1 list-notebooks

Notes:
  - The wrapper always targets the checked-in DaisyBrain notebook id.
  - NOTEBOOKLM_HOME is set to the repo-preferred local NotebookLM home.
  - Forward extra NotebookLM flags after the action, for example:
      .\docs\notebooklm\daisybrain.ps1 ask --json "What changed in DaisyHost?"
      .\docs\notebooklm\daisybrain.ps1 source-add .\docs\notebooklm\context\05-current-state-2026-04.md
"@
}

function Expand-ConfigPath([string]$value) {
    return [Environment]::ExpandEnvironmentVariables($value)
}

function Resolve-NotebookLmCli([object]$config) {
    $command = Get-Command notebooklm -ErrorAction SilentlyContinue
    if($null -ne $command -and -not [string]::IsNullOrWhiteSpace($command.Source)) {
        return $command.Source
    }

    foreach($candidate in $config.cliCandidates) {
        $expanded = Expand-ConfigPath([string]$candidate)
        if(Test-Path -LiteralPath $expanded) {
            return $expanded
        }
    }

    throw "NotebookLM CLI not found. Repair the local NotebookLM install first."
}

$scriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Path
$configPath  = Join-Path $scriptDir "daisybrain.config.json"
$config      = Get-Content -LiteralPath $configPath -Raw | ConvertFrom-Json
$notebookId  = [string]$config.notebookId
$notebook    = [string]$config.notebookTitle
$homePath    = Expand-ConfigPath([string]$config.preferredHome)
$env:NOTEBOOKLM_HOME = $homePath
$cliPath     = Resolve-NotebookLmCli $config

switch($Action) {
    "help" {
        Show-Usage
    }
    "id" {
        Write-Output $notebookId
    }
    "home" {
        Write-Output $homePath
    }
    "info" {
        [pscustomobject]@{
            notebookTitle = $notebook
            notebookId    = $notebookId
            notebookHome  = $homePath
            cliPath       = $cliPath
        } | ConvertTo-Json -Depth 3
    }
    "use" {
        & $cliPath use $notebookId
    }
    "ask" {
        if($Arguments.Count -eq 0) {
            throw "ask requires a NotebookLM question or flags."
        }
        & $cliPath ask -n $notebookId @Arguments
    }
    "source-list" {
        & $cliPath source list -n $notebookId --json
    }
    "source-add" {
        if($Arguments.Count -eq 0) {
            throw "source-add requires a file path, URL, or inline text."
        }
        & $cliPath source add -n $notebookId @Arguments
    }
    "source-wait" {
        if($Arguments.Count -eq 0) {
            throw "source-wait requires a source id."
        }
        & $cliPath source wait -n $notebookId @Arguments
    }
    "list-notebooks" {
        & $cliPath list --json
    }
    default {
        throw "Unknown action '$Action'. Run '.\docs\notebooklm\daisybrain.ps1 help'."
    }
}
