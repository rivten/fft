@echo off

set SDLPath=C:\SDL2-2.0.5\
set SDLBinPath=%SDLPath%\lib\x64\
set RivtenPath=..\..\rivten\
set STBPath=..\..\stb\
set ImguiPath=..\..\imgui\

set CommonCompilerFlags=-Od -EHsc -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4800 -wd4996 -wd4244 -FC -Z7 /I %SDLPath%\include\ /I %SDLNetPath%\include\ /I %STBPath% /I %RivtenPath% /I %ImguiPath%

rem set CommonCompilerFlags=-DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 %CommonCompilerFlags% 
set CommonLinkerFlags= -incremental:no -opt:ref -subsystem:console user32.lib gdi32.lib winmm.lib opengl32.lib %SDLBinPath%\SDL2.lib %SDLBinPath%\SDL2main.lib

REM TODO - can we just build both with one exe?

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 64-bit build
REM Optimization switches /wO2
cl %CommonCompilerFlags% ..\code\fft.cpp /link %CommonLinkerFlags%
popd
