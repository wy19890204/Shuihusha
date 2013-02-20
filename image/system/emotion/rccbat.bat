:MAIN
@echo off
for /R %%s in (*.qrc) do (
call :RCC %%s
)
pause
GOTO :EOF

:RCC
set str=%1
@echo Creat %str:~0,-4%.rcc, please wait...
if exist "F:\QT\qt\bin\rcc.exe" goto branch
E:\QT\Desktop\Qt\4.7.4\mingw\bin\rcc.exe -binary %1 -o %str:~0,-4%.rcc
:branch
F:\QT\qt\bin\rcc.exe -binary %1 -o %str:~0,-4%.rcc
GOTO :EOF
