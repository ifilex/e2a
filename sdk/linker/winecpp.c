// WineCPP compiler

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PATH 1024
#define VERSION "1.0.2"

void show_help() {
    printf("Usage: winecpp [OPTIONS] zip_pre_compiled.zip android_name android_icon\n");
    printf("This program compiles executables pre-compiled in ZIP format for wine.\n\n");
    printf("Options:\n");
    printf("    --help               Display this help and exit\n");
    printf("    --version            Output version information and exit\n");
}

void show_version() {
    printf("winecpp-%s\n", VERSION);
}

void create_directory(const char* path) {
    char tmp[MAX_PATH];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            #ifdef _WIN32
            mkdir(tmp);
            #else
            mkdir(tmp, 0755);
            #endif
            *p = '/';
        }
    }
    #ifdef _WIN32
    mkdir(tmp);
    #else
    mkdir(tmp, 0755);
    #endif
}

void create_android_manifest(const char* archivo) {
    FILE* fp = fopen("pak/AndroidManifest.xml", "w");
    if (!fp) {
        printf("Error creating AndroidManifest.xml\n");
        return;
    }

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n");
    fprintf(fp, "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\" ");
    fprintf(fp, "android:compileSdkVersion=\"30\" android:compileSdkVersionCodename=\"11\" ");
    fprintf(fp, "package=\"com.%s.box\" platformBuildVersionCode=\"23\" ", archivo);
    fprintf(fp, "platformBuildVersionName=\"6.0-2438415\">\n");
    fprintf(fp, "  <uses-permission android:name=\"android.permission.ACCESS_NETWORK_STATE\"/>\n");
    fprintf(fp, "  <uses-permission android:name=\"android.permission.INTERNET\"/>\n");
    fprintf(fp, "    <application android:allowBackup=\"true\" android:fullBackupContent=\"true\" ");
    fprintf(fp, "android:icon=\"@drawable/ic_launcher\" android:label=\"@string/app_name\" ");
    fprintf(fp, "android:theme=\"@style/MaterialAppTheme\" android:usesCleartextTraffic=\"@bool/usesCleartextTraffic\">\n");
    fprintf(fp, "       <activity android:configChanges=\"keyboardHidden|orientation|screenSize\" ");
    fprintf(fp, "android:hardwareAccelerated=\"true\" android:label=\"@string/app_name\" ");
    fprintf(fp, "android:name=\"com.wine.box.MainActivity\" android:screenOrientation=\"user\" ");
    fprintf(fp, "android:windowSoftInputMode=\"adjustResize\">\n");
    fprintf(fp, "           <intent-filter>\n");
    fprintf(fp, "              <action android:name=\"android.intent.action.VIEW\"/>\n");
    fprintf(fp, "              <action android:name=\"android.intent.action.MAIN\"/>\n");
    fprintf(fp, "               <category android:name=\"android.intent.category.LAUNCHER\"/>\n");
    fprintf(fp, "           </intent-filter>\n");
    fprintf(fp, "           <intent-filter>\n");
    fprintf(fp, "               <action android:name=\"android.intent.action.VIEW\"/>\n");
    fprintf(fp, "               <category android:name=\"android.intent.category.DEFAULT\"/>\n");
    fprintf(fp, "               <category android:name=\"android.intent.category.BROWSABLE\"/>\n");
    fprintf(fp, "               <data android:host=\"null\" android:pathPrefix=\"/\" android:scheme=\"https\"/>\n");
    fprintf(fp, "           </intent-filter>\n");
    fprintf(fp, "       </activity>\n");
    fprintf(fp, "       <meta-data android:name=\"android.support.VERSION\" android:value=\"26.1.0\"/>\n");
    fprintf(fp, "       <meta-data android:name=\"android.arch.lifecycle.VERSION\" android:value=\"27.0.0-SNAPSHOT\"/>\n");
    fprintf(fp, "   </application>\n");
    fprintf(fp, "</manifest>\n");
    
    fclose(fp);
}

void create_strings_xml(const char* nombre) {
    FILE* fp = fopen("pak/res/values/strings.xml", "w");
    if (!fp) {
        printf("Error creating strings.xml\n");
        return;
    }

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fprintf(fp, "<resources>\n");
    fprintf(fp, "    <string name=\"CacheMode\">DefaultCache</string>\n");
    fprintf(fp, "    <string name=\"abc_action_bar_home_description\">Navigate home</string>\n");
    fprintf(fp, "    <string name=\"abc_action_bar_home_description_format\">%%1$s, %%2$s</string>\n");
    fprintf(fp, "    <string name=\"abc_action_bar_home_subtitle_description_format\">%%1$s, %%2$s, %%3$s</string>\n");
    fprintf(fp, "    <string name=\"abc_action_bar_up_description\">Navigate up</string>\n");
    fprintf(fp, "    <string name=\"abc_action_menu_overflow_description\">More options</string>\n");
    fprintf(fp, "    <string name=\"abc_action_mode_done\">Done</string>\n");
    fprintf(fp, "    <string name=\"abc_activity_chooser_view_see_all\">See all</string>\n");
    fprintf(fp, "    <string name=\"abc_activitychooserview_choose_application\">Choose an app</string>\n");
    fprintf(fp, "    <string name=\"abc_capital_off\">OFF</string>\n");
    fprintf(fp, "    <string name=\"abc_capital_on\">ON</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_body_1_material\">sans-serif</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_body_2_material\">sans-serif-medium</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_button_material\">sans-serif-medium</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_caption_material\">sans-serif</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_display_1_material\">sans-serif</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_display_2_material\">sans-serif</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_display_3_material\">sans-serif</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_display_4_material\">sans-serif-light</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_headline_material\">sans-serif</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_menu_material\">sans-serif</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_subhead_material\">sans-serif</string>\n");
    fprintf(fp, "    <string name=\"abc_font_family_title_material\">sans-serif-medium</string>\n");
    fprintf(fp, "    <string name=\"abc_search_hint\">Search?</string>\n");
    fprintf(fp, "    <string name=\"abc_searchview_description_clear\">Clear query</string>\n");
    fprintf(fp, "    <string name=\"abc_searchview_description_query\">Search query</string>\n");
    fprintf(fp, "    <string name=\"abc_searchview_description_search\">Search</string>\n");
    fprintf(fp, "    <string name=\"abc_searchview_description_submit\">Submit query</string>\n");
    fprintf(fp, "    <string name=\"abc_searchview_description_voice\">Voice search</string>\n");
    fprintf(fp, "    <string name=\"abc_shareactionprovider_share_with\">Share with</string>\n");
    fprintf(fp, "    <string name=\"abc_shareactionprovider_share_with_application\">Share with %%s</string>\n");
    fprintf(fp, "    <string name=\"abc_toolbar_collapse_description\">Collapse</string>\n");
    fprintf(fp, "    <string name=\"aboutTitle\">About</string>\n");
    fprintf(fp, "    <string name=\"action_exit\">Exit</string>\n");
    fprintf(fp, "    <string name=\"action_tag\">About</string>\n");
    fprintf(fp, "    <string name=\"app_name\">%s</string>\n", nombre);
    fprintf(fp, "    <string name=\"appbar_scrolling_view_behavior\">android.support.design.widget.AppBarLayout$ScrollingViewBehavior</string>\n");
    fprintf(fp, "    <string name=\"bottom_sheet_behavior\">android.support.design.widget.BottomSheetBehavior</string>\n");
    fprintf(fp, "    <string name=\"character_counter_pattern\">%%1$d / %%2$d</string>\n");
    fprintf(fp, "    <string name=\"cnfExit\">Are you sure you want to exit?</string>\n");
    fprintf(fp, "    <string name=\"csum\">83DC66979E0EACCAA5763B73AD9B2194</string>\n");
    fprintf(fp, "    <string name=\"devid\">B7ECD79E7BC6D765C669729C381DDE951C0DB5C5</string>\n");
    fprintf(fp, "    <string name=\"password_toggle_content_description\">Toggle password visibility</string>\n");
    fprintf(fp, "    <string name=\"path_password_eye\">M12,4.5C7,4.5 2.73,7.61 1,12c1.73,4.39 6,7.5 11,7.5s9.27,-3.11 11,-7.5c-1.73,-4.39 -6,-7.5 -11,-7.5zM12,17c-2.76,0 -5,-2.24 -5,-5s2.24,-5 5,-5 5,2.24 5,5 -2.24,5 -5,5zM12,9c-1.66,0 -3,1.34 -3,3s1.34,3 3,3 3,-1.34 3,-3 -1.34,-3 -3,-3z</string>\n");
    fprintf(fp, "    <string name=\"path_password_eye_mask_strike_through\">M2,4.27 L19.73,22 L22.27,19.46 L4.54,1.73 L4.54,1 L23,1 L23,23 L1,23 L1,4.27 Z</string>\n");
    fprintf(fp, "    <string name=\"path_password_eye_mask_visible\">M2,4.27 L2,4.27 L4.54,1.73 L4.54,1.73 L4.54,1 L23,1 L23,23 L1,23 L1,4.27 Z</string>\n");
    fprintf(fp, "    <string name=\"path_password_strike_through\">M3.27,4.27 L19.74,20.74</string>\n");
    fprintf(fp, "    <string name=\"search_menu_title\">Search</string>\n");
    fprintf(fp, "    <string name=\"sharesubject\">%s</string>\n", nombre);
    fprintf(fp, "    <string name=\"sharetext\">Hi there, Give this app a try.</string>\n");
    fprintf(fp, "    <string name=\"status_bar_notification_info_overflow\">999+</string>\n");
    fprintf(fp, "    <string name=\"tag\">%s</string>\n", nombre);
    fprintf(fp, "</resources>\n");

    fclose(fp);
}

void create_index_html(const char* archivo) {
    FILE* fp = fopen("pak/assets/index.html", "w");
    if (!fp) {
        printf("Error creating index.html\n");
        return;
    }

    fprintf(fp, "<!DOCTYPE html>\n");
    fprintf(fp, "<html>\n");
    fprintf(fp, "<head>\n");
    fprintf(fp, "    <meta http-equiv=\"x-ua-compatible\" content=\"IE=edge\">\n");
    fprintf(fp, "    <META HTTP-EQUIV=\"REFRESH\" CONTENT=\"0;URL=wine.html?app=%s&p=%s.exe\">\n", 
            archivo, archivo);
    fprintf(fp, "    <title>Loading...</title>\n");
    fprintf(fp, "</head>\n");
    fprintf(fp, "<body>\n");
    fprintf(fp, "</body>\n");
    fprintf(fp, "</html>\n");

    fclose(fp);
}

void copy_file(const char* source, const char* destination) {
    FILE *src, *dst;
    char buffer[4096];
    size_t bytes;

    src = fopen(source, "rb");
    if (!src) {
        printf("Error opening source file: %s\n", source);
        return;
    }

    dst = fopen(destination, "wb");
    if (!dst) {
        fclose(src);
        printf("Error opening destination file: %s\n", destination);
        return;
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
}

void compile_apk(const char* archivo) {
    char command[MAX_PATH];
    snprintf(command, sizeof(command), 
             "JDK/bin/java -jar apktool.jar b pak -o \"%s.apk\"", archivo);
    system(command);
}

void sign_apk(const char* archivo) {
    char command[MAX_PATH];
    snprintf(command, sizeof(command), 
             "JDK/bin/java -jar signer.jar -a \"%s.apk\"", archivo);
    system(command);
}

void cleanup(const char* archivo) {
    remove("pak/assets/index.html");
    char zip_path[MAX_PATH];
    snprintf(zip_path, sizeof(zip_path), "pak/assets/%s.zip", archivo);
    remove(zip_path);
    remove("pak/res/values/strings.xml");
       remove("pak/res/drawable/ic_launcher.png");
    remove("pak/AndroidManifest.xml");
    char apk_path[MAX_PATH];
    snprintf(apk_path, sizeof(apk_path), "%s.apk", archivo);
    remove(apk_path);
    system("cd pak && rm -rf build && cd ..");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Error: Missing argument.\n\n");
        show_help();
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        show_help();
        return 0;
    }

    if (strcmp(argv[1], "--version") == 0) {
        show_version();
        return 0;
    }

    // Verificar si el archivo ZIP existe
    if (access(argv[1], F_OK) == -1) {
        printf("Error: File %s not found.\n", argv[1]);
        return 1;
    }

    // Extraer el nombre del archivo sin extensión
    char archivo[MAX_PATH];
    strncpy(archivo, argv[1], strlen(argv[1]) - 4);
    archivo[strlen(argv[1]) - 4] = '\0';

    char* nombre = argv[2];

    // Crear directorios necesarios
    create_directory("pak/res/values");
    create_directory("pak/assets");

    // Crear los archivos necesarios
    create_android_manifest(archivo);
    create_strings_xml(nombre);
    create_index_html(archivo);

    // Copiar archivo ZIP
    char zip_dest[MAX_PATH];
    snprintf(zip_dest, sizeof(zip_dest), "pak/assets/%s.zip", archivo);
    copy_file(argv[1], zip_dest);

    // Manejar el icono
    if (argc < 4) {
        copy_file("tools/ic_launcher.png", "pak/res/drawable/ic_launcher.png");
    } else {
        copy_file(argv[3], "pak/res/drawable/ic_launcher.png");
    }

    // Compilar y firmar APK
    compile_apk(archivo);
    sign_apk(archivo);

    // Limpiar archivos temporales
    cleanup(archivo);

    printf("Process completed.\n");
    return 0;
}
