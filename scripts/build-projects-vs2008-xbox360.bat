@call %~dp0config.bat

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2008-xbox360.xml ^
	%TRAKTOR_HOME%\resources\build\ExternXbox360.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2008-xbox360.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorXbox360.xms
