@call %~dp0config.bat
@%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2013-win64.xml %TRAKTOR_HOME%\TraktorWin64.xms
