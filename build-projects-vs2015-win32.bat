@call %~dp0config.bat
@%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2015-win32.xml %TRAKTOR_HOME%\TraktorWin32.xms
