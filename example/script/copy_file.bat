@echo off
setlocal enabledelayedexpansion

set current_path=%~dp0
echo "copy dll file"

set src_path=%current_path%..\..\third\lib\
set release_dest_path=%current_path%..\..\output\bin\Release\
set debug_dest_path=%current_path%..\..\output\bin\Debug\
echo %src_path%
echo %release_dest_path%
echo %debug_dest_path%

if exist %release_dest_path% (
    copy %src_path%sdl2\SDL2.dll %release_dest_path%
    copy %src_path%opengl\glew32.dll %release_dest_path%
    copy %src_path%ffmpeg\Release\avcodec-58.dll %release_dest_path%
    copy %src_path%ffmpeg\Release\avformat-58.dll %release_dest_path%
    copy %src_path%ffmpeg\Release\avutil-56.dll %release_dest_path%
    copy %src_path%ffmpeg\Release\swscale-5.dll %release_dest_path%
    copy %src_path%ffmpeg\Release\swresample-1.dll %release_dest_path%
) else (
    echo "not exist Release"
)

if exist %debug_dest_path% (
    copy %src_path%sdl2\SDL2.dll %debug_dest_path%
    copy %src_path%opengl\glew32.dll %debug_dest_path%
    copy %src_path%ffmpeg\Debug\avcodec-56.dll %debug_dest_path%
    copy %src_path%ffmpeg\Debug\avformat-56.dll %debug_dest_path%
    copy %src_path%ffmpeg\Debug\avutil-54.dll %debug_dest_path%
    copy %src_path%ffmpeg\Debug\swscale-3.dll %debug_dest_path%
    copy %src_path%ffmpeg\Debug\swresample-1.dll %debug_dest_path%
) else (
    echo "not exist Debug"
)

