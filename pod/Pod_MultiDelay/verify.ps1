param([string]$DaisyExamples='C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples')
$ErrorActionPreference='Stop'
Set-Location -LiteralPath $PSScriptRoot
py -3 -B .\verify_dependencies.py
if($LASTEXITCODE -ne 0){throw 'Pinned dependency validation failed'}
$hostCompiler='C:\msys64\ucrt64\bin\g++.exe'
$env:PATH='C:\msys64\ucrt64\bin;C:\Program Files\DaisyToolchain\bin;'+$env:PATH
$engine=Join-Path $DaisyExamples 'DaisyHost\src\PedalDelayEngine.cpp'
$pinnedEngine=(Get-Content -LiteralPath '.\dependencies.json' -Raw | ConvertFrom-Json).files | Where-Object { $_.path.EndsWith('/DaisyHost/src/PedalDelayEngine.cpp') }
if ([IO.Path]::GetFullPath($engine) -ine [IO.Path]::GetFullPath($pinnedEngine.path)) { throw 'The selected source root differs from the frozen dependency manifest' }
$include='-I'+(Join-Path $DaisyExamples 'DaisyHost\include')
foreach($name in @('test_digi','test_demo','test_controls')) {
 $arguments=@('-std=c++17','-O2','-Wall','-Wextra','-Werror',$include,($name+'.cpp'))
 if($name -ne 'test_digi'){$arguments+=$engine}
 $arguments+=@('-o',($name+'.exe'))
 & $hostCompiler @arguments
 if($LASTEXITCODE -ne 0){throw "Compilation failed: $name"}
 & (Join-Path $PSScriptRoot ($name+'.exe'))
 if($LASTEXITCODE -ne 0){throw "Behavior check failed: $name"}
}
& 'C:\Program Files\DaisyToolchain\bin\make.exe' -j2 ('DAISY_EXAMPLES_DIR='+$DaisyExamples.Replace('\','/'))
if($LASTEXITCODE -ne 0){throw 'ARM build failed'}
