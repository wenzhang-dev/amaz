cmake_minimum_required(VERSION 3.12)

project(zmaz VERSION 0.0.1 LANGUAGES CXX)

# export the compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CO "enable coreoutine" ON)

if (ENABLE_CO)
  set(CMAKE_CXX_STANDARD 20)
  add_compile_definitions(ENABLE_CO)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
else()
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -fPIC -fPIE -Wall -Werror -rdynamic -lcrypto -lssl")

# boost lib
set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/third-party/boost/install)
set(BOOST_NO_SYSTEM_PATHS ON)
find_package(Boost REQUIRED)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/third-party
  ${PROJECT_SOURCE_DIR}//third-party/fmt/include
)

# fmt lib
add_subdirectory("third-party/fmt")

# ada lib
set(ADA_BENCHMARKS OFF)
set(ADA_TESTING OFF)
set(ADA_TOOLS OFF)
add_subdirectory("third-party/ada")

set(CMAKE_BUILD_TYPE None)
option(BUILD_TESTS "build unit tests" ON)

if (BUILD_TESTS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g3")
endif()

# unit test
if (BUILD_TESTS)
  enable_testing()

  add_subdirectory("third-party/catch2")
  include(CTest)
  include(Catch)

  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAMAZ_DEBUG=1")

  function(add_tc)
    cmake_parse_arguments(
      TC "" "NAME" "LIBS"
      ${ARGN}
    )

    get_filename_component(test_target_name "${TC_NAME}" NAME_WE)

    add_executable("${test_target_name}" "")
    target_sources("${test_target_name}" PRIVATE "${TC_NAME}")
    
    target_link_libraries("${test_target_name}" ${TC_LIBS} Catch2::Catch2 dl pthread)

    add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
  endfunction()
endif()

# base lib
include(${CMAKE_SOURCE_DIR}/base/sources.cmake)
add_library(base STATIC ${BASE_SRC})

