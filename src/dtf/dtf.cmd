@setlocal
@pushd %~dp0

@set _C=Debug
@set _N=nuget_dev.config
@:parse_args
@if /i "%1"=="release" set _C=Release& shift
@if /i "%1"=="official" set _N=nuget_official.config& shift
@if not "%1"=="" shift & goto parse_args

@echo Building dtf %_C% using %_N%

:: dtf

nuget restore -ConfigFile ..\%_N% || exit /b

msbuild -p:Configuration=%_C% || exit /b

msbuild -t:Pack -p:Configuration=%_C% WixToolset.Dtf.Compression || exit /b
msbuild -t:Pack -p:Configuration=%_C% WixToolset.Dtf.Compression.Cab || exit /b
msbuild -t:Pack -p:Configuration=%_C% WixToolset.Dtf.Compression.Zip || exit /b
msbuild -t:Pack -p:Configuration=%_C% WixToolset.Dtf.Resources || exit /b
msbuild -t:Pack -p:Configuration=%_C% WixToolset.Dtf.WindowsInstaller || exit /b
msbuild -t:Pack -p:Configuration=%_C% WixToolset.Dtf.WindowsInstaller.Linq || exit /b
msbuild -t:Pack -p:Configuration=%_C% WixToolset.Dtf.WindowsInstaller.Package || exit /b

@popd
@endlocal
