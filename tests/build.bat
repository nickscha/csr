@echo off

set DEF_FLAGS_COMPILER=-std=c89 -pedantic -Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wsign-conversion -Wuninitialized -Winit-self -Wunused -Wunused-macros -Wunused-local-typedefs
set DEF_FLAGS_LINKER=
set SOURCE_NAME=csr_test

rm *.ppm
cc -s -O2 %DEF_FLAGS_COMPILER% -o %SOURCE_NAME%.exe %SOURCE_NAME%.c %DEF_FLAGS_LINKER%
%SOURCE_NAME%.exe

REM RENDER Video and Gif
REM ffmpeg -y -framerate 30 -i output_%%05d.ppm -c:v libx264 -pix_fmt yuv420p %SOURCE_NAME%.mp4
REM ffmpeg -y -i %SOURCE_NAME%.mp4 -filter_complex "[0:v] fps=10,scale=360:-1:flags=lanczos,split [a][b];[a] palettegen [p];[b][p] paletteuse" %SOURCE_NAME%.gif
