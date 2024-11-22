#!/bin/bash
clear
echo "EXE2APK Installer"
echo "==========================================================="
echo ""
echo "- Installing linux packs.."
echo ""
apt-get update &>/dev/null
apt-get -y --with-new-pkgs -o Dpkg::Options::="--force-confdef" upgrade &>/dev/null
apt install python --no-install-recommends -y &>/dev/null
clear
echo "EXE2APK Installer"
echo "==========================================================="
echo ""
echo "- Download the online installer.."
    
curl -o e2a.py https://raw.githubusercontent.com/ifilex/e2a/main/e2a.py && python3 e2a.py
