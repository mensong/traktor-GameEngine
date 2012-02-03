@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	pushd %DEPLOY_PROJECTROOT:/=\%\bin\latest\osx\releaseshared
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% * > %DEPLOY_PROJECTROOT%\deploy.log
	popd

) else if "%1"=="launch" (

	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% Application.config > %DEPLOY_PROJECTROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteLaunch %DEPLOY_TARGET_HOST% %DEPLOY_EXECUTABLE% "\-s Application.config" >> %DEPLOY_PROJECTROOT%\deploy.log

) else if "%1"=="migrate" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MODULES% > %DEPLOY_PROJECTROOT%\migrate.log
	
)

