@echo off

echo Compilando winezip.exe ..
python -m PyInstaller --noconfirm --log-level=WARN ^
    --onefile --noconsole ^
    --hidden-import=PySide2 ^
    --hidden-import=shiboken2 ^
    --add-data  ./resources;resources ^
    --icon=./resources/app.ico ^
    winezip.py
