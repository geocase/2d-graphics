cmake_minimum_required(VERSION 3.1.0)
project(platformer)
set(EXE_NAME platformer)

include_directories(lib/SDL2-2.0.18/include)
link_directories(lib/SDL2-2.0.18/lib/x64)

include_directories(lib/SDL2_image-2.0.5/include)
link_directories(lib/SDL2_image-2.0.5/lib/x64)

include_directories(lib/cglm-0.8.3/include)
link_directories(lib/cglm-0.8.3/build)

include_directories(lib/cute)

include_directories(src)

set(SOURCES src/main.c src/image.c src/shader.c src/fio.c src/renderer.c src/render_objects.c src/common.c src/game/input_state.c src/game/actor.c)

include_directories(lib/glad/include/)
set(GLAD_SOURCES lib/glad/src/glad.c)

add_executable(${EXE_NAME} ${SOURCES} ${GLAD_SOURCES})
set_property(TARGET ${EXE_NAME} PROPERTY C_STANDARD 11)

if(WIN32)
    add_compile_options(/IGNORE:C4996)
    target_compile_options(${EXE_NAME} PRIVATE)
    target_link_libraries(${EXE_NAME} SDL2 SDL2_image)
endif(WIN32)