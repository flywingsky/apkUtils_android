@echo off
color 0f
cd /d %~dp0
call jni\\build.cmd
cd /d %~dp0
echo==========================================================================================================
echo Push ...
adb push %~dp0libs\armeabi\search /data/local/tmp

echo Chmod ...
adb shell su -c "chmod 777 /data/local/tmp/search"

echo Execute ...
echo.
echo==========================================================================================================
echo.
rem adb shell "ps | grep com.tomagoyaky"
adb shell "ps | grep com.tomagoyaky"
set /p pid=������pid:
adb shell "cat /proc/%pid%/maps | grep com.tomagoyaky"
set /p moudleName=������moudleName:
adb shell su -c "/data/local/tmp/search" %pid% %moudleName%"
echo.
for /f "tokens=2,*" %%i in ('reg query "HKCU\Software\microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v "Desktop"') do (set desk=%%j)
adb pull /sdcard/dump.out %desk%
echo dump�ļ��ѱ�����{%desk%\dump.out}
echo.
echo==========================================================================================================
echo �ű��������
echo==========================================================================================================
echo.
pause