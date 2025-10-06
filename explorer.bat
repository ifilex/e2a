@echo off
rem script para lanzar notepad
copy dock.zip explorer.zip >nul
wine explorer.zip
del explorer.zip >nul
