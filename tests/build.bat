@echo off

set DEF_FLAGS_COMPILER=-std=c89 -pedantic -Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wsign-conversion -Wuninitialized -Winit-self -Wunused -Wunused-macros -Wunused-local-typedefs
set DEF_FLAGS_LINKER=
set SOURCE_NAME=csr_test

rm *.ppm
cc -s -O2 %DEF_FLAGS_COMPILER% -o %SOURCE_NAME%.exe %SOURCE_NAME%.c %DEF_FLAGS_LINKER%
%SOURCE_NAME%.exe

REM RENDER Videos and Gifs from PPM frames
ffmpeg -y -framerate 30 -i stack_%%05d.ppm -c:v libx264 -pix_fmt yuv420p stack.mp4
ffmpeg -y -framerate 30 -i cube_%%05d.ppm -c:v libx264 -pix_fmt yuv420p cube.mp4
ffmpeg -y -framerate 30 -i teddy_%%05d.ppm -c:v libx264 -pix_fmt yuv420p teddy.mp4
ffmpeg -y -framerate 30 -i voxel_teddy_%%05d.ppm -c:v libx264 -pix_fmt yuv420p voxel_teddy.mp4

ffmpeg -y -i stack.mp4 -filter_complex "[0:v] fps=10,scale=360:-1:flags=lanczos,split [a][b];[a] palettegen [p];[b][p] paletteuse" stack.gif
ffmpeg -y -i cube.mp4 -filter_complex "[0:v] fps=10,scale=360:-1:flags=lanczos,split [a][b];[a] palettegen [p];[b][p] paletteuse" cube.gif
ffmpeg -y -i teddy.mp4 -filter_complex "[0:v] fps=10,scale=360:-1:flags=lanczos,split [a][b];[a] palettegen [p];[b][p] paletteuse" teddy.gif
ffmpeg -y -i voxel_teddy.mp4 -filter_complex "[0:v] fps=10,scale=360:-1:flags=lanczos,split [a][b];[a] palettegen [p];[b][p] paletteuse" voxel_teddy.gif

rm *.ppm
