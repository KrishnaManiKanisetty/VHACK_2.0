cmake_minimum_required(VERSION 3.10)
project(LaptopAssistant CXX)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(lib/llama.cpp/include)
include_directories(lib/whisper.cpp/include)
include_directories(lib/llama.cpp/ggml/include)
include_directories(lib/whisper.cpp/ggml/include)

# Add our source files
add_executable(assistant 
    src/main.cpp 
    src/audio_engine.cpp 
    src/assistant_brain.cpp
)

# Link against the built libraries
# Note: These paths assume CMake Release build in build/llama and build/whisper
target_link_libraries(assistant PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/build/llama/src/Release/llama.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/build/llama/ggml/src/Release/ggml.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/build/whisper/src/Release/whisper.lib
)

# Windows libraries
target_link_libraries(assistant PRIVATE
    ole32.lib
    user32.lib
    winmm.lib
)
