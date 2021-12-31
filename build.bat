@echo off

if "%1"=="-release" (set optimization=-O2) else (set optimization=-Od)

set defines=-DBUILD_WIN32=1
set common_compiler_flags= -MTd -GR -EHa-  %optimization% -Oi -W4 -wd4201 -wd4100 -wd4211 -wd4189 -nologo -FC -Z7
set common_linker_flags=-incremental:no -opt:ref user32.lib gdi32.lib opengl32.lib

if not exist run_tree mkdir run_tree
pushd run_tree
cl  %defines% %common_compiler_flags% ..\src\minesweeper.cpp /link %common_linker_flags% /out:minesweeper.exe
popd
