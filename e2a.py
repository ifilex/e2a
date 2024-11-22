import os, shutil, time
def packages():
    os.system("pkg install x11-repo -y &>/dev/null")
    os.system("pkg install pulseaudio wget xkeyboard-config virglrenderer-android proot-distro termux-x11-nightly termux-am -y &>/dev/null")
def check_prev_version():
    config = "/sdcard/E2A"
    if os.path.exists(config):
        shutil.rmtree(config)
    os.system("proot-distro remove ubuntu-e2a &>/dev/null")
def install_rootfs():
    os.makedirs("/data/data/com.termux/files/usr/var/lib/proot-distro", exist_ok=True)
    os.makedirs("/data/data/com.termux/files/usr/var/lib/proot-distro/installed-rootfs", exist_ok=True)
    os.makedirs("/data/data/com.termux/files/usr/var/lib/proot-distro/installed-rootfs/ubuntu", exist_ok=True)
    os.system("wget -q --show-progress https://github.com/ifilex/e2a/releases/download/Develkit/e2a.tar.xz")
    os.system("proot-distro restore e2a.tar.xz &>/dev/null")
def scripts():
    os.system("wget https://raw.githubusercontent.com/ifilex/e2a/main/e2a &>/dev/null")
    os.system("chmod +x e2a")
    os.system("mv e2a $PREFIX/bin/")
def clear_waste():
    os.system("rm e2a.tar.xz install e2a.py")
    os.system("clear")
def storage():
    if not os.path.exists("/data/data/com.termux/files/home/storage"):
        os.system("termux-setup-storage")
        time.sleep(2)
os.system("clear")
print("EXE2APK Installer")
print("===========================================================")
print("")
print(" [-] Please allow storage permission")
storage()
print(" [-] Installing packages...")
packages()
print(" [-] Checking for older EXE2APK versions and removing them if any...")
print("")
check_prev_version()
print(" [-] Downloading and installing rootfs, please wait...")
print("")
install_rootfs()
print("")
print(" [-] Downloading starting scripts...")
print("")
scripts()
print(" [-] Removing the installation temp files...")
clear_waste()
print("")
print(" Installation finished. To start EXE2APK run 'e2a'")
print("")

