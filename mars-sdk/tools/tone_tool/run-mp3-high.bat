@echo off

SET FMT_PARAM=-r -s 16000 -m m --bitwidth 16

cd /d %~dp0
rd /s /q output
md output\amp
md output\ring
.\ToneTool\ToneTool.exe .\tts\ 200 200


for /f "delims=" %%i in ('"dir .\output\amp\ /a/s/b/on"') do (
    .\ToneTool\lame.exe -V0 -b 8 -B 32 -T %FMT_PARAM% %%~fi .\output\ring\%%~ni.mp3
)

for /f "delims=" %%i in ('"dir .\sound\ /a/s/b/on"') do (
    copy /V %%~fi .\output\ring\
)

rd /s /q output\amp

cd /d %~dp0\output

..\ToneTool\tone_tool_header.exe .\ring