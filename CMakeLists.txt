cmake_minimum_required(VERSION 3.10)
project(BallCatcherGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/Camera.cpp
    src/Vector3.cpp
    src/Texture.cpp
    src/Text.cpp
    src/Fruit.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
)