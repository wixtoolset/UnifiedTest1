@setlocal
@pushd %~dp0

@set _C=Debug
@set _N=nuget_dev.config
@:parse_args
@if /i "%1"=="release" set _C=Release& shift
@if /i "%1"=="official" set _N=nuget_official.config& shift
@if not "%1"=="" shift & goto parse_args

@echo Building burn %_C% using %_N%

:: burn

nuget restore -ConfigFile ..\%_N% || exit /b

msbuild -t:Test -p:Configuration=%_C% test\BurnUnitTest || exit /b

msbuild -p:Configuration=%_C%;Platform=x86 || exit /b
msbuild -p:Configuration=%_C%;Platform=x64 || exit /b
msbuild -p:Configuration=%_C%;Platform=arm64 || exit /b

msbuild -p:Configuration=%_C% -t:PackNative stub\stub.vcxproj || exit /b

@popd
@endlocal
