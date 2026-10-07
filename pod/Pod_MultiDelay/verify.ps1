param([string]$DaisyExamples='C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples',
      [string]$Dafx='C:\Users\denko\Claude\DAFX_2_Daisy_lib2',
      [string]$DafxPin='73976da36b9599fd838583f8e3d519035423bd1d')
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
# DAFX DigitalDelayNode headers at the contract pin, for the bit-exact parity test.
$dafxOut=Join-Path $PSScriptRoot 'build\dafx\pedal_harness'
New-Item -ItemType Directory -Force $dafxOut | Out-Null
foreach($h in @('dsp_contract.hpp','digital_delay_node.hpp')) {
 $text = git -C $Dafx show ($DafxPin+':src/pedal_harness/'+$h)
 if($LASTEXITCODE -ne 0){throw "Cannot read $h at DAFX $DafxPin"}
 Set-Content -LiteralPath (Join-Path $dafxOut $h) -Value $text -Encoding utf8
}
foreach($name in @('test_digi','test_parity','test_demo','test_controls')) {
 $arguments=@('-std=c++17','-O2','-Wall','-Wextra','-Werror',$include,('-I'+(Join-Path $PSScriptRoot 'build\dafx')),($name+'.cpp'))
 if($name -eq 'test_demo' -or $name -eq 'test_controls'){$arguments+=$engine}
 $arguments+=@('-o',($name+'.exe'))
 & $hostCompiler @arguments
 if($LASTEXITCODE -ne 0){throw "Compilation failed: $name"}
 & (Join-Path $PSScriptRoot ($name+'.exe'))
 if($LASTEXITCODE -ne 0){throw "Behavior check failed: $name"}
}
& 'C:\Program Files\DaisyToolchain\bin\make.exe' -j2 ('DAISY_EXAMPLES_DIR='+$DaisyExamples.Replace('\','/'))
if($LASTEXITCODE -ne 0){throw 'ARM build failed'}
