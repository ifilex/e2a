@echo off
setlocal

REM Comprobar si se han proporcionado suficientes argumentos
if "%~1"=="" (
    echo Error: Missing argument.
    echo.
    echo Usage: winecpp [OPTIONS] zip_pre_compiled.zip android_name android_icon
    exit /b
)

if "%~1"=="--help" (
    echo Usage: winecpp [OPTIONS] zip_pre_compiled.zip android_name android_icon
    echo This script compile executables pre-compiled in ZIP format for wine.
    echo.
    echo Options:
    echo     --help               Display this help and exit
    echo     --version            Output version information and exit
    exit /b
)

REM Manejar la opción --version
if "%~1"=="--version" (
    echo winecpp-1.0.1 
    exit /b
)

REM Verificar si el archivo ZIP existe
if not exist "%~1" (
    echo Error: File %~1 not found.
    exit /b
)

REM Obtener el nombre del archivo zip sin la extensión
set "archivo=%~n1"
set "nombre=%~2"

REM Crear directorio si no existe
if not exist "pak\res\values" (
    mkdir "pak\res\values"
)
if not exist "pak\assets" (
    mkdir "pak\assets"
)

REM Crear AndroidManifest.xml con el namespace
echo Creando archivo AndroidManifest.xml con el nombre de la aplicación %archivo%...
(
    echo ^<?xml version="1.0" encoding="utf-8" standalone="no"?^>^<manifest xmlns:android="http://schemas.android.com/apk/res/android" android:compileSdkVersion="30" android:compileSdkVersionCodename="11" package="com.%archivo%.box" platformBuildVersionCode="23" platformBuildVersionName="6.0-2438415"^>
    echo   ^<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE"/^>
    echo   ^<uses-permission android:name="android.permission.INTERNET"/^>
    echo     ^<application android:allowBackup="true" android:fullBackupContent="true" android:icon="@drawable/ic_launcher" android:label="@string/app_name" android:theme="@style/MaterialAppTheme" android:usesCleartextTraffic="@bool/usesCleartextTraffic"^>
    echo        ^<activity android:configChanges="keyboardHidden|orientation|screenSize" android:hardwareAccelerated="true" android:label="@string/app_name" android:name="com.wine.box.MainActivity" android:screenOrientation="user" android:windowSoftInputMode="adjustResize"^>
    echo            ^<intent-filter^>
    echo               ^<action android:name="android.intent.action.VIEW"/^>
    echo               ^<action android:name="android.intent.action.MAIN"/^>
    echo                ^<category android:name="android.intent.category.LAUNCHER"/^>
    echo            ^</intent-filter^>
    echo            ^<intent-filter^>
    echo                ^<action android:name="android.intent.action.VIEW"/^>
    echo                ^<category android:name="android.intent.category.DEFAULT"/^>
    echo                ^<category android:name="android.intent.category.BROWSABLE"/^>
    echo                ^<data android:host="null" android:pathPrefix="/" android:scheme="https"/^>
    echo            ^</intent-filter^>
    echo        ^</activity^>
    echo        ^<meta-data android:name="android.support.VERSION" android:value="26.1.0"/^>
    echo        ^<meta-data android:name="android.arch.lifecycle.VERSION" android:value="27.0.0-SNAPSHOT"/^>
    echo    ^</application^>
    echo  ^</manifest^>
) > "pak\AndroidManifest.xml"

REM Crear strings.xml con el nombre de la aplicación
echo Creando archivo strings.xml con el nombre de la aplicación %nombre%...
(
    echo ^<?xml version="1.0" encoding="utf-8"?^>
    echo ^<resources^>
    echo     ^<string name="CacheMode"^>DefaultCache^</string^>
    echo     ^<string name="abc_action_bar_home_description"^>Navigate home^</string^>
    echo     ^<string name="abc_action_bar_home_description_format"^>%1$s, %2$s^</string^>
    echo     ^<string name="abc_action_bar_home_subtitle_description_format"^>%1$s, %2$s, %3$s^</string^>
    echo     ^<string name="abc_action_bar_up_description"^>Navigate up^</string^>
    echo     ^<string name="abc_action_menu_overflow_description"^>More options^</string^>
    echo     ^<string name="abc_action_mode_done"^>Done^</string^>
    echo     ^<string name="abc_activity_chooser_view_see_all"^>See all^</string^>
    echo     ^<string name="abc_activitychooserview_choose_application"^>Choose an app^</string^>
    echo     ^<string name="abc_capital_off"^>OFF^</string^>
    echo     ^<string name="abc_capital_on"^>ON^</string^>
    echo     ^<string name="abc_font_family_body_1_material"^>sans-serif^</string^>
    echo     ^<string name="abc_font_family_body_2_material"^>sans-serif-medium^</string^>
    echo     ^<string name="abc_font_family_button_material"^>sans-serif-medium^</string^>
    echo     ^<string name="abc_font_family_caption_material"^>sans-serif^</string^>
    echo     ^<string name="abc_font_family_display_1_material"^>sans-serif^</string^>
    echo     ^<string name="abc_font_family_display_2_material"^>sans-serif^</string^>
    echo     ^<string name="abc_font_family_display_3_material"^>sans-serif^</string^>
    echo     ^<string name="abc_font_family_display_4_material"^>sans-serif-light^</string^>
    echo     ^<string name="abc_font_family_headline_material"^>sans-serif^</string^>
    echo     ^<string name="abc_font_family_menu_material"^>sans-serif^</string^>
    echo     ^<string name="abc_font_family_subhead_material"^>sans-serif^</string^>
    echo     ^<string name="abc_font_family_title_material"^>sans-serif-medium^</string^>
    echo     ^<string name="abc_search_hint">Search?^</string^>
    echo     ^<string name="abc_searchview_description_clear"^>Clear query^</string^>
    echo     ^<string name="abc_searchview_description_query"^>Search query^</string^>
    echo     ^<string name="abc_searchview_description_search"^>Search^</string^>
    echo     ^<string name="abc_searchview_description_submit"^>Submit query^</string^>
    echo     ^<string name="abc_searchview_description_voice"^>Voice search^</string^>
    echo     ^<string name="abc_shareactionprovider_share_with"^>Share with^</string^>
    echo     ^<string name="abc_shareactionprovider_share_with_application"^>Share with ^%s^</string^>
    echo     ^<string name="abc_toolbar_collapse_description"^>Collapse^</string^>
    echo     ^<string name="aboutTitle"^>About^</string^>
    echo     ^<string name="action_exit"^>Exit^</string^>
    echo     ^<string name="action_tag"^>About^</string^>
    echo     ^<string name="app_name"^>%nombre%^</string^>
    echo     ^<string name="appbar_scrolling_view_behavior"^>android.support.design.widget.AppBarLayout$ScrollingViewBehavior^</string^>
    echo     ^<string name="bottom_sheet_behavior"^>android.support.design.widget.BottomSheetBehavior^</string^>
    echo     ^<string name="character_counter_pattern"^>%1$d / %2$d^</string^>
    echo     ^<string name="cnfExit"^>Are you sure you want to exit?^</string^>
    echo     ^<string name="csum"^>83DC66979E0EACCAA5763B73AD9B2194^</string^>
    echo     ^<string name="devid"^>B7ECD79E7BC6D765C669729C381DDE951C0DB5C5^</string^>
    echo     ^<string name="password_toggle_content_description"^>Toggle password visibility^</string^>
    echo     ^<string name="path_password_eye"^>M12,4.5C7,4.5 2.73,7.61 1,12c1.73,4.39 6,7.5 11,7.5s9.27,-3.11 11,-7.5c-1.73,-4.39 -6,-7.5 -11,-7.5zM12,17c-2.76,0 -5,-2.24 -5,-5s2.24,-5 5,-5 5,2.24 5,5 -2.24,5 -5,5zM12,9c-1.66,0 -3,1.34 -3,3s1.34,3 3,3 3,-1.34 3,-3 -1.34,-3 -3,-3z^</string^>
    echo     ^<string name="path_password_eye_mask_strike_through"^>M2,4.27 L19.73,22 L22.27,19.46 L4.54,1.73 L4.54,1 L23,1 L23,23 L1,23 L1,4.27 Z^</string^>
    echo     ^<string name="path_password_eye_mask_visible"^>M2,4.27 L2,4.27 L4.54,1.73 L4.54,1.73 L4.54,1 L23,1 L23,23 L1,23 L1,4.27 Z^</string^>
    echo     ^<string name="path_password_strike_through"^>M3.27,4.27 L19.74,20.74^</string^>
    echo     ^<string name="search_menu_title"^>Search^</string^>
    echo     ^<string name="sharesubject"^>%nombre%^</string^>
    echo     ^<string name="sharetext"^>Hi there, Give this app a try.^</string^>
    echo     ^<string name="status_bar_notification_info_overflow"^>999+^</string^>
    echo     ^<string name="tag"^>%nombre%^</string^>
    echo ^</resources^>
) > "pak\res\values\strings.xml"

REM Crear el archivo index.html
echo Creando archivo index.html...
(
    echo ^<!DOCTYPE html^>
    echo ^<html^>
    echo ^<head^>
    echo     ^<meta http-equiv="x-ua-compatible" content="IE=edge"^>
    echo     ^<META HTTP-EQUIV="REFRESH" CONTENT="0;URL=wine.html?app=%archivo%&p=%archivo%.exe"^>
    echo     ^<title^>Loading...^</title^>
    echo ^</head^>
    echo ^<body^>
    echo ^</body^>
    echo ^</html^>
) > "pak\assets\index.html"

REM Copiar archivo ZIP a pak\assets
echo Copiando archivo %~1 a pak\assets...
copy "%~1" "pak\assets\%archivo%.zip"

REM Verificar si hay un icono
if "%~3"=="" (
        echo Copiando icono generico a pak\res\drawable...
        copy "tools\ic_launcher.png" "pak\res\drawable"
        goto cc
)

echo Copiando icono "%~3" a pak\res\drawable...
copy "%~3" "pak\res\drawable\ic_launcher.png"

:cc
REM Compilar APK
echo Compilando APK...
JDK\bin\java -jar apktool.jar b pak -o "%archivo%.apk"
if errorlevel 1 (
    echo Error: Fallo en la compilación del APK.
    goto :cleanup
)

REM Verificar si el APK fue creado exitosamente
if not exist "%archivo%.apk" (
    echo Error: No se pudo generar el archivo APK.
    goto :cleanup
)

echo APK generado correctamente: %archivo%.apk

REM Firmar el APK
echo Firmando el archivo APK...
JDK\bin\java -jar signer.jar -a "%archivo%.apk"
if errorlevel 1 (
    echo Error: Fallo al firmar el APK.
    goto :cleanup
)

REM Limpieza de archivos temporales
:cleanup
echo Limpiando archivos temporales...
del "pak\assets\index.html"
del "pak\assets\%archivo%.zip"
del "pak\res\values\strings.xml"
del "pak\res\drawable\ic_launcher.png"
del "pak\AndroidManifest.xml"
del "%archivo%.apk"
cd pak
rd /s /q build
cd ..

echo Proceso completado.

exit /b
