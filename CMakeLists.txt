# MIT License
#
# Copyright (c) 2021-2023 Stefano Allegretti, Davide Papazzoni, Nicola Baldini, Lorenzo Governatori e Simone Gemelli
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.17.1)

set (ProjectName "Game")

project (${ProjectName} C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED YES) # Force the following command to generate an error when it fails
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS NO)

source_group("Vertex Shaders" REGULAR_EXPRESSION "vert$")
source_group("Fragment Shaders" REGULAR_EXPRESSION "frag$")

add_executable(${ProjectName})
# A check on glfw runtime can be added
set_property(TARGET ${ProjectName} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName})

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML COMPONENTS audio REQUIRED)

target_link_libraries(${ProjectName} glfw)
target_link_libraries(${ProjectName} OpenGL::GL)
target_link_libraries(${ProjectName} sfml-audio)

target_include_directories(${ProjectName} PUBLIC include)
target_include_directories(${ProjectName} PUBLIC "3rdparty/glad/include")
target_include_directories(${ProjectName} PUBLIC "3rdparty/include")

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(3rdparty)


file(GLOB VERTEX_FILES "shaders/*.vert")
file(GLOB FRAGMENT_FILES "shaders/*.frag")
target_sources(${ProjectName} PRIVATE "${VERTEX_FILES};${FRAGMENT_FILES}")

source_group("Vertex Shaders" REGULAR_EXPRESSION "vert$")
source_group("Fragment Shaders" REGULAR_EXPRESSION "frag$")

add_executable(Nikman src/launcher.cpp src/Nikman.rc)
set_property(TARGET Nikman PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(Maze src/maze.cpp "3rdparty/glad/src/glad.c")
set_property(TARGET Maze PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_include_directories(Maze PUBLIC include)
target_include_directories(Maze PUBLIC "3rdparty/glad/include")
target_include_directories(Maze PUBLIC "3rdparty/include")
target_link_libraries(Maze glfw)
target_link_libraries(Maze OpenGL::GL)
target_link_libraries(Maze sfml-audio)

if(WIN32)
  configure_file("3rdparty/OpenAL/openal32.dll" "${CMAKE_BINARY_DIR}/openal32.dll" COPYONLY)
endif()

# Install program
if(MSVC)
  install(DIRECTORY shaders DESTINATION .)
  install(DIRECTORY resources DESTINATION .)
  #install(FILES "scripts/Nikman.bat" DESTINATION .)
  install(FILES "3rdparty/OpenAL/openal32.dll" DESTINATION bin)
  install(FILES "installer/comandi.bat" DESTINATION .)
  install(FILES "installer/NikmanInstaller.wxs" DESTINATION .)
  install(FILES "credits.txt" DESTINATION .)
  install(TARGETS ${ProjectName} RUNTIME DESTINATION bin)
  install(TARGETS Nikman RUNTIME DESTINATION .)
endif()
