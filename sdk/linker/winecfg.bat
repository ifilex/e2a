::[Bat To Exe Converter]
::
::YAwzoRdxOk+EWAnk
::fBw5plQjdG8=
::YAwzuBVtJxjWCl3EqQJgSA==
::ZR4luwNxJguZRRnk
::Yhs/ulQjdF+5
::cxAkpRVqdFKZSDk=
::cBs/ulQjdF+5
::ZR41oxFsdFKZSDk=
::eBoioBt6dFKZSDk=
::cRo6pxp7LAbNWATEpCI=
::egkzugNsPRvcWATEpCI=
::dAsiuh18IRvcCxnZtBJQ
::cRYluBh/LU+EWAnk
::YxY4rhs+aU+JeA==
::cxY6rQJ7JhzQF1fEqQJQ
::ZQ05rAF9IBncCkqN+0xwdVs0
::ZQ05rAF9IAHYFVzEqQJQ
::eg0/rx1wNQPfEVWB+kM9LVsJDGQ=
::fBEirQZwNQPfEVWB+kM9LVsJDGQ=
::cRolqwZ3JBvQF1fEqQJQ
::dhA7uBVwLU+EWDk=
::YQ03rBFzNR3SWATElA==
::dhAmsQZ3MwfNWATElA==
::ZQ0/vhVqMQ3MEVWAtB9wSA==
::Zg8zqx1/OA3MEVWAtB9wSA==
::dhA7pRFwIByZRRnk
::Zh4grVQjdCyDJGyX8VAjFBZGWwWGAE+1BaAR7ebv/Najp1sYWN4ocYGJ26SFHPca5EDnc5Fj02Jf+A==
::YB416Ek+ZG8=
::
::
::978f952a14a936cc963da21a135fa983
@echo off
setlocal

REM Mostrar ayuda si no se pasa ningún argumento o si se pasa una opción incorrecta
if "%~1"=="" (
    echo Usage: winecfg [OPTIONS] 
    echo This script changes the wine version in your system.
    echo.
    echo Options:
    echo     --help               Display this help and exit
    echo     --settings, --set    Launch winecfg.zip
    echo     --version            Change the wine version in your system
    exit /b
)

REM Manejar la opción --help
if "%~1"=="--help" (
    echo Usage: wine [OPTIONS] archivo.zip
    echo This script runs executables pre-compiled in ZIP format for wine.
    echo.
    echo Options:
    echo     --help               Display this help and exit
    echo     --version            [Version]     
    echo.  
    echo    Change the version: 
    echo                wine-17  
    echo                wine-50  
    exit /b
)

REM Manejar la opción --version
if "%~1"=="--version" (
    setlocal enabledelayedexpansion
    set "archivo=%~n2"
rem    echo Archivo !archivo!.zip
    
    REM Verificar si el archivo ZIP de la versión existe en tools\roms
    if exist "tools\roms\!archivo!.zip" (
        del tools\boot\resources\app\dlls.zip >nul
        del pak\assets\dlls.zip >nul
        copy "tools\roms\!archivo!.zip" tools\boot\resources\app\dlls.zip >nul
        copy "tools\roms\!archivo!.zip" pak\assets\dlls.zip >nul
        echo System changed to version !archivo!
    ) else (
        echo The version '%~2' is not available in tools\roms.
    )
    exit /b
)

REM Manejar las opciones --settings y --set para iniciar winecfg.zip
if "%~1"=="--settings" (
    wine winecfg.zip
    exit /b
)

if "%~1"=="--set" (
    wine winecfg.zip
    exit /b
)

REM Ejecutar wine winecfg.zip si no se usa ninguna opción válida
wine winecfg.zip
