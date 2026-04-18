@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "PROJECT_ROOT=%%~fI"
set "DOXYFILE=%PROJECT_ROOT%\Doxyfile"
set "DOC_BUILD_DIR=%PROJECT_ROOT%\build\docs\daisypedal"
set "LATEX_DIR=%DOC_BUILD_DIR%\latex"
set "PDF_OUT=%PROJECT_ROOT%\docs\daisypedal_reference.pdf"

where doxygen >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Required tool not found in PATH: doxygen
    exit /b 1
)

set "LATEX_ENGINE="
where pdflatex >nul 2>nul
if not errorlevel 1 (
    where makeindex >nul 2>nul
    if not errorlevel 1 set "LATEX_ENGINE=pdflatex"
)
if not defined LATEX_ENGINE (
    where tectonic >nul 2>nul
    if not errorlevel 1 set "LATEX_ENGINE=tectonic"
)
if not defined LATEX_ENGINE (
    echo [ERROR] Required LaTeX toolchain not found in PATH. Install pdflatex+makeindex or tectonic.
    exit /b 1
)

if exist "%DOC_BUILD_DIR%" rmdir /s /q "%DOC_BUILD_DIR%"

pushd "%PROJECT_ROOT%"
echo [1/4] Running Doxygen
doxygen "%DOXYFILE%"
if errorlevel 1 (
    popd
    exit /b 1
)
popd

if not exist "%LATEX_DIR%\refman.tex" (
    echo [ERROR] Doxygen did not generate "%LATEX_DIR%\refman.tex"
    exit /b 1
)

pushd "%LATEX_DIR%"
echo [2/4] Building LaTeX reference
if "%LATEX_ENGINE%"=="pdflatex" (
    pdflatex -interaction=nonstopmode refman.tex >nul
    if errorlevel 1 exit /b 1
    makeindex refman.idx >nul
    if errorlevel 1 exit /b 1
    echo [3/4] Finalizing PDF
    pdflatex -interaction=nonstopmode refman.tex >nul
    if errorlevel 1 exit /b 1
    pdflatex -interaction=nonstopmode refman.tex >nul
    if errorlevel 1 exit /b 1
) else (
    type nul > refman.ind
    tectonic refman.tex >nul
    if errorlevel 1 exit /b 1
)
popd

copy /y "%LATEX_DIR%\refman.pdf" "%PDF_OUT%" >nul
if errorlevel 1 exit /b 1

echo [4/4] Wrote "%PDF_OUT%"
exit /b 0
