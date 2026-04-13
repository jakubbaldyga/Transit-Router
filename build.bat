@echo off
REM build.bat
REM Usage: build.bat [target] [config]
REM target: tests / benchmarks / cli / all
REM config: Debug / Release

SET TARGET=%1
SET CONFIG=%2

IF "%TARGET%"=="" SET TARGET=all
IF "%CONFIG%"=="" SET CONFIG=Debug

if "%TARGET%"=="lint" (
    echo [INFO] Linting C++ sources...
    for /R src %%f in (*.cpp *.hpp) do (
        echo   %%~nxf
        clang-tidy "%%f" -p build --config-file=.clang-tidy -fix -quiet
    )
    echo [INFO] Done.
    pause
    exit /b 0
)

echo [INFO] Building target: %TARGET% with config: %CONFIG%
echo.

REM Configure CMake
cmake -S . -B build -G Ninja ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DBUILD_ALL=ON -DCMAKE_BUILD_TYPE=%CONFIG% ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    -DCMAKE_C_COMPILER_TARGET=x86_64-w64-windows-gnu ^
    -DCMAKE_CXX_COMPILER_TARGET=x86_64-w64-windows-gnu

REM Build targets
IF "%TARGET%"=="all" (
    cmake --build build --target tests --config %CONFIG% -j
    cmake --build build --target benchmarks --config %CONFIG% -j
    cmake --build build --target cli --config %CONFIG% -j
    cmake --build build --target gui --config %CONFIG% -j
) ELSE (
    cmake --build build --target %TARGET% --config %CONFIG% -j
)

REM Run executables
IF "%TARGET%"=="tests" (
    copy /Y .\build\src\core\libcore.dll .\build\src\tests\
    build\src\tests\tests.exe
)
IF "%TARGET%"=="benchmarks" (
    copy /Y .\build\src\core\libcore.dll build\src\benchmark\
    build\src\benchmark\benchmarks.exe
)
IF "%TARGET%"=="cli" (
    copy /Y .\build\src\core\libcore.dll build\src\cli\
    build\src\cli\cli.exe
)
IF "%TARGET%"=="gui" (
     copy /Y .\build\src\core\libcore.dll build\src\gui\
    build\src\gui\gui.exe
)

echo [INFO] Done.
pause