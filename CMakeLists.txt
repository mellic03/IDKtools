cmake_minimum_required(VERSION 3.9)

add_compile_options("-g" "-fno-ident" "-fno-asynchronous-unwind-tables")
add_definitions(-DIDK_DEBUG)

include_directories(${CMAKE_SOURCE_DIR}/external/include)


# MapIBL
# ----------------------------------------------------------------------------------------------
project (diffuseIBL)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_image_INCLUDE_DIRS})
include_directories(${SDL2_mixer_INCLUDE_DIRS})

set(GRAPHICS_LIBRARIES
    GLEW
    SDL2
    SDL2_image
    SDL2_mixer
    ${OPENGL_LIBRARIES}
)


add_executable(
    ${PROJECT_NAME} "src/MapIBL/diffuseIBL.cpp"
)

target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    ${GRAPHICS_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/external/lib/libIDKengine.so
)


project (specularIBL)

add_executable(
    ${PROJECT_NAME} "src/MapIBL/specularIBL.cpp"
)

target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    ${GRAPHICS_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/external/lib/libIDKengine.so
)
# ----------------------------------------------------------------------------------------------


# ModelConvert
# ----------------------------------------------------------------------------------------------
project (modelconvert)

add_executable(
    ${PROJECT_NAME} "src/ModelConvert/modelconvert.cpp;src/ModelConvert/animation.cpp"
)

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath=../IDKGE/shipping/")

target_link_libraries(
    ${PROJECT_NAME}
    ${GRAPHICS_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/external/lib/libIDKengine.so
    ${CMAKE_SOURCE_DIR}/external/lib/libassimp.so
)
# ----------------------------------------------------------------------------------------------


# TextureConvert
# ----------------------------------------------------------------------------------------------
project (textureconvert)

add_executable(
    ${PROJECT_NAME} "src/TextureConvert/textureconvert.cpp"
)

target_link_libraries(
    ${PROJECT_NAME}
    ${GRAPHICS_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/external/lib/libIDKengine.so
)
# ----------------------------------------------------------------------------------------------
