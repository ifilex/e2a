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
::Zh4grVQjdCyDJGyX8VAjFBZGWwWGAE+1BaAR7ebv/Najp1sYWN4ocYGJ26SFHPca5ECqcI4otg==
::YB416Ek+ZG8=
::
::
::978f952a14a936cc963da21a135fa983
@echo off
setlocal

REM Mostrar ayuda si no se pasa ningún argumento o si se pasa una opción incorrecta
if "%~1"=="" (
    echo Usage: wine [OPTIONS] archivo.zip
    echo This script runs executables pre-compiled in ZIP format for wine.
    echo.
    echo Options:
    echo     --help               Display this help and exit
    echo     --version            Output version information and exit
    echo     --debug              Run wine with debug messages
    exit /b
)

REM Manejar la opción --help
if "%~1"=="--help" (
    echo Usage: wine [OPTIONS] archivo.zip
    echo This script runs executables pre-compiled in ZIP format for wine.
    echo.
    echo Options:
    echo     --help               Display this help and exit
    echo     --version            Output version information and exit
    echo     --debug              Run wine with debug messages
    exit /b
)

REM Manejar la opción --version
if "%~1"=="--version" (
    echo wine-5.0 
    exit /b
)

REM Verificar si es --debug y que se haya pasado un archivo .zip
if "%~1"=="--debug" (
    if "%~2"=="" (
        echo Error: You must specify a .zip file when using --debug.
        exit /b
    )
    
    REM Obtener el nombre del archivo sin la extensión
    setlocal enabledelayedexpansion
    set "archivo=%~n2"
    echo Debug: Archivo definido como !archivo!


    REM Verificar que el archivo sea .zip
    if /I "%~x2" NEQ ".zip" (
        echo Error: Only ZIP files are supported.
        exit /b
    )

    REM Imprimir la versión del script
    echo Wine version 1.75
    echo.
    echo Global Variables
    echo.
    echo app=!archivo!
    echo prg=!archivo!.exe	
    echo.

    REM Mostrar mensajes de depuración y creación del archivo index.html
    echo Debug: Creating index.html for !archivo!.
    echo ^<!DOCTYPE html^> > tools\boot\resources\app\index.html
    echo ^<html^> >> tools\boot\resources\app\index.html
    echo ^<head^> >> tools\boot\resources\app\index.html
    echo ^    ^<meta http-equiv="x-ua-compatible" content="IE=edge"^> >> tools\boot\resources\app\index.html
    echo ^    ^<META HTTP-EQUIV="REFRESH" CONTENT="0;URL=winex.html?app=!archivo!&p=!archivo!.exe"^> >> tools\boot\resources\app\index.html
    echo ^    ^<title^>Loading...^</title^> >> tools\boot\resources\app\index.html
    echo ^</head^> >> tools\boot\resources\app\index.html
    echo ^<body^> >> tools\boot\resources\app\index.html
    echo ^</body^> >> tools\boot\resources\app\index.html
    echo ^</html^> >> tools\boot\resources\app\index.html

    REM Copiar el archivo zip
    echo Debug: Copying %~2 to tools\boot\resources\app
    copy "%~2" tools\boot\resources\app

    REM Ejecutar el comando wine
    echo Debug: Running wine.
    tools\boot\wine

    REM Eliminar los archivos después de que el proceso termine
    echo Debug: Cleaning up files.
    del tools\boot\resources\app\%~nx2
    del tools\boot\resources\app\index.html

    echo Debug: Process completed.
    exit /b
)

REM Verificar si es un archivo .zip (modo silencioso)
set "archivo=%~n1"
if /I "%~x1"==".zip" (
    REM Crear el archivo index.html sin mensajes
    echo ^<!DOCTYPE html^> > tools\boot\resources\app\index.html
    echo ^<html^> >> tools\boot\resources\app\index.html
    echo ^<head^> >> tools\boot\resources\app\index.html
    echo ^    ^<meta http-equiv="x-ua-compatible" content="IE=edge"^> >> tools\boot\resources\app\index.html
    echo ^    ^<META HTTP-EQUIV="REFRESH" CONTENT="0;URL=wine.html?app=%archivo%&p=%archivo%.exe"^> >> tools\boot\resources\app\index.html
    echo ^    ^<title^>Loading...^</title^> >> tools\boot\resources\app\index.html
    echo ^</head^> >> tools\boot\resources\app\index.html
    echo ^<body^> >> tools\boot\resources\app\index.html
    echo ^</body^> >> tools\boot\resources\app\index.html
    echo ^</html^> >> tools\boot\resources\app\index.html

    REM Copiar el archivo ZIP
    copy "%~1" tools\boot\resources\app >nul

    REM Ejecutar wine
    tools\boot\wine >nul

    REM Eliminar archivos generados
    del tools\boot\resources\app\%~nx1
    del tools\boot\resources\app\index.html
    exit /b
)

REM Si el argumento no es válido o no es un archivo ZIP
echo Error: Only ZIP files are supported.
exit /b
