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

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(3rdparty)

file(GLOB VERTEX_FILES "shaders/*.vert")
file(GLOB FRAGMENT_FILES "shaders/*.frag")
target_sources(${ProjectName} PRIVATE "${VERTEX_FILES};${FRAGMENT_FILES}")

source_group("Vertex Shaders" REGULAR_EXPRESSION "vert$")
source_group("Fragment Shaders" REGULAR_EXPRESSION "frag$")

add_executable(Nikman src/launcher.cpp)
set_property(TARGET Nikman PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Install program

install(DIRECTORY shaders DESTINATION .)
install(DIRECTORY resources DESTINATION .)
#install(FILES "scripts/Nikman.bat" DESTINATION .)
install(FILES "3rdparty/OpenAL/openal32.dll" DESTINATION bin)
install(TARGETS ${ProjectName} RUNTIME DESTINATION bin)
install(TARGETS Nikman RUNTIME DESTINATION .)