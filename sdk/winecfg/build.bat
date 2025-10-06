@echo off

echo Compilando wconfig.exe ..
python -m PyInstaller --noconfirm --log-level=WARN ^
    --onefile --noconsole ^
    --hidden-import=PySide2 ^
    --hidden-import=shiboken2 ^
    --add-data  ./resources;resources ^
    --icon=./resources/app.ico ^
    wconfig.py
