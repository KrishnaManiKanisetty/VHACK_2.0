@echo off
set "VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

if not exist "%VCVARSALL%" (
    echo Error: vcvarsall.bat not found at %VCVARSALL%
    exit /b 1
)

call "%VCVARSALL%" x64

echo Compiling...
cl /EHsc /O2 /I"lib/llama.cpp/include" /I"lib/whisper.cpp/include" /I"lib/llama.cpp/ggml/include" /I"lib/whisper.cpp/ggml/include" ^
    src/main.cpp src/audio_engine.cpp src/assistant_brain.cpp ^
    /Fe:assistant.exe ^
    ole32.lib user32.lib winmm.lib ^
    /link /LIBPATH:"lib/llama.cpp" /LIBPATH:"lib/whisper.cpp"

if %ERRORLEVEL% neq 0 (
    echo Compilation failed.
    exit /b 1
)

echo Build successful! Run assistant.exe to start.
