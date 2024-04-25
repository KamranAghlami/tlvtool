cmake_minimum_required(VERSION 3.20)

project(tlvtool LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Werror -Wextra -Wpedantic -Wconversion)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/W4 /WX)
endif()

include(FetchContent)
FetchContent_Declare(
  libtlvcpp
  GIT_REPOSITORY https://github.com/KamranAghlami/libtlvcpp.git
  GIT_TAG origin/main
)
FetchContent_MakeAvailable(libtlvcpp)
target_link_libraries(${PROJECT_NAME} PRIVATE tlvcpp)