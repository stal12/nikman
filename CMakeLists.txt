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

project(Nikman VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED YES) # Force the following command to generate an error when it fails
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS NO)

#==============================================================================
# External dependencies
#==============================================================================
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SFML COMPONENTS audio CONFIG REQUIRED)

add_executable(Nikman)
# Set nikman as the startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Nikman)

target_link_libraries(Nikman PRIVATE glfw OpenGL sfml-audio glad::glad)
target_include_directories(Nikman PRIVATE include "3rdparty/include")

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(3rdparty)

file(GLOB VERTEX_FILES "shaders/*.vert")
file(GLOB FRAGMENT_FILES "shaders/*.frag")
target_sources(Nikman PRIVATE "${VERTEX_FILES};${FRAGMENT_FILES}")

source_group("Vertex Shaders" REGULAR_EXPRESSION "vert$")
source_group("Fragment Shaders" REGULAR_EXPRESSION "frag$")

add_executable(Launcher src/launcher.cpp src/Nikman.rc)

# Install program
if(MSVC)
  install(DIRECTORY shaders DESTINATION .)
  install(DIRECTORY resources DESTINATION .)
  install(FILES "installer/comandi.bat" DESTINATION .)
  install(FILES "installer/NikmanInstaller.wxs" DESTINATION .)
  install(FILES "credits.txt" DESTINATION .)
  install(TARGETS Nikman RUNTIME DESTINATION bin)
  install(TARGETS Nikman RUNTIME DESTINATION .)
endif()
