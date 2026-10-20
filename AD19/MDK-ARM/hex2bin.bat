@echo off

REM 处理小时是一位数的情况，前面补0
if %time:~0,2% leq 9 (set hour=0%time:~1,1%) else (set hour=%time:~0,2%)

REM 粒度 秒
REM set current=%date:~0,4%%date:~5,2%%date:~8,2%_%hour%%time:~3,2%%time:~6,2%
REM 粒度 分
REM set current=%date:~0,4%%date:~5,2%%date:~8,2%_%hour%%time:~3,2%
REM 粒度 天
set current=%date:~0,4%%date:~5,2%%date:~8,2%

REM set current=%date:~0,4%%date:~5,2%%date:~8,2%

REM 调用bat文件时输入的第1个参数，.afx 文件所在目录
set objs=%1

REM 调用bat文件时输入的第2个参数，生成的目标文件名称
set target=%2

F:\MDK5.36\ARM\ARMCC\bin\fromelf.exe --bin -o .\bin\%target%_%current%.bin .\%objs%\%target%.axf
.\EncodeFirmware.exe -i .\bin\%target%_%current%.bin -o ad19_firmware.bin -t 0x009B