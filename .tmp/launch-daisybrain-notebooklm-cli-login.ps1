$ErrorActionPreference = "Stop"

$NotebookHome = "$HOME\.notebooklm-daisybrain"
New-Item -ItemType Directory -Force -Path $NotebookHome | Out-Null

$PyScripts = "$HOME\AppData\Local\Programs\Python\Python314\Scripts"
$NotebookExe = Join-Path $PyScripts "notebooklm.exe"

if(-not (Test-Path $NotebookExe))
{
    throw "notebooklm.exe not found at $NotebookExe"
}

$command = @"
`$env:NOTEBOOKLM_HOME = '$NotebookHome'
& '$NotebookExe' login
"@

Start-Process powershell.exe -ArgumentList @(
    "-NoExit",
    "-Command",
    $command
)

Write-Host "Launched NotebookLM CLI login window using NOTEBOOKLM_HOME=$NotebookHome"
