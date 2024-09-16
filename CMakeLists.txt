cmake_minimum_required(VERSION 3.13)

project(redis-server)
# add dependencies
include(cmake/CPM.cmake)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
include_directories(include)

set(CMAKE_CXX_STANDARD 20) # Enable the C++23 standard
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
add_subdirectory(3rdparty/quill)
add_subdirectory(3rdparty/quill_wrapper)

# build RTTR
CPMAddPackage(
  NAME rttr # link against RTTR::Core_Lib
  VERSION 0.9.6
  GITHUB_REPOSITORY rttrorg/rttr
  OPTIONS
    "BUILD_RTTR_DYNAMIC Off"
    "BUILD_UNIT_TESTS Off"
    "BUILD_STATIC On"
    "BUILD_PACKAGE Off"
    "BUILD_WITH_RTTI On"
    "BUILD_EXAMPLES Off"
    "BUILD_DOCUMENTATION Off"
    "BUILD_INSTALLER Off"
    "USE_PCH Off"
    "CUSTOM_DOXYGEN_STYLE Off"
)

# Force the script to use FetchContent and not CPM internally to fetch asio
option(ASIO_USE_CPM "Download Asio with CPM instead of FetchContent" OFF)
option(
  ASIO_CPM_FETCHCONTENT_COMPAT
  "Should asio be declared with FetchContent functions to be compatible. This doesn't not allow CPM cache to work."
  ON
)
# Download this repository
include(FetchContent)
FetchContent_Declare(
  asiocmake
  GIT_REPOSITORY "https://github.com/OlivierLDff/asio.cmake"
  GIT_TAG        "main"
)
FetchContent_MakeAvailable(asiocmake)


CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.8.1
  VERSION 1.8.1
  OPTIONS
      "INSTALL_GTEST OFF"
      "gtest_force_shared_crt ON"
)


add_executable(server ${SOURCE_FILES})

target_link_libraries(server PRIVATE asio asio::asio)
target_link_libraries(server PRIVATE Threads::Threads quill_wrapper_recommended RTTR::Core_Lib)

add_subdirectory(tests)