@echo off
setlocal
set "ROOT=%~dp0"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "__QWEN_PATH=%Path%"
set "PATH="
set "Path=%__QWEN_PATH%"

if not exist "%VSWHERE%" (
  echo Could not find vswhere.exe. Open QwensTtsBuddy.sln in Visual Studio and build Release x64.
  exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set "VSINSTALL=%%i"
)

if not defined VSINSTALL (
  echo Could not find a Visual Studio install with MSBuild. Open QwensTtsBuddy.sln in Visual Studio and build Release x64.
  exit /b 1
)

call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
msbuild "%ROOT%QwensTtsBuddy.sln" /m /p:Configuration=Release /p:Platform=x64
exit /b %ERRORLEVEL%
endlocal
