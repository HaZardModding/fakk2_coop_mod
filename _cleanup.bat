@echo off
cd fgame
rd  /s /q Debug
cd ../
cd cgame
rd  /s /q Debug
cd ../
rd  /s /q Debug
cd ../
del CTF.sdf

del /s /q /f *.sbr
del /s /q /f *.o
echo Cleanup COMPLETE
pause>NUL