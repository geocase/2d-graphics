cmake_minimum_required(VERSION 3.1.0)
project(platformer)
set(EXE_NAME platformer)

include_directories(lib/SDL2-2.0.18/include)
link_directories(lib/SDL2-2.0.18/lib/x64)

include_directories(lib/cglm-0.8.3/include)
link_directories(lib/cglm-0.8.3/build)

include_directories(src)

set(SOURCES src/main.c src/image.c src/shader.c src/fio.c)

include_directories(lib/glad/include/)
set(GLAD_SOURCES lib/glad/src/glad.c)

add_executable(${EXE_NAME} ${SOURCES} ${GLAD_SOURCES})
set_property(TARGET ${EXE_NAME} PROPERTY C_STANDARD 11)

if(WIN32)
    target_compile_options(${EXE_NAME} PRIVATE)
    target_link_libraries(${EXE_NAME} SDL2)
endif(WIN32)