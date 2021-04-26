@setlocal
@pushd %~dp0

@set _C=Debug
@set _N=nuget_dev.config
@:parse_args
@if /i "%1"=="release" set _C=Release& shift
@if /i "%1"=="official" set _O=official&set _N=nuget_official.config& shift
@if not "%1"=="" shift & goto parse_args

@echo build %_C% using %_N%

md ..\build\package-cache

:: DTF

call dtf\dtf.cmd %_C_% %_O% || exit /b


:: internal

call internal\internal.cmd %_C_% %_O% || exit /b


:: libs

call libs\libs.cmd %_C_% %_O% || exit /b


:: api

call api\api.cmd %_C_% %_O% || exit /b


:: burn

call burn\burn.cmd %_C_% %_O% || exit /b


:: wix

call wix\wix.cmd %_C_% %_O% || exit /b


:: ext

:: ext\ext.cmd %_C_% %_O% || exit /b


:: samples

:: samples\samples.cmd %_C_% %_O% || exit /b


@popd
@endlocal
