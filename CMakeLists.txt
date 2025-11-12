################################################################################
# MIT License

# Copyright (c) 2023 NuHepMC/cpputils

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
################################################################################

cmake_minimum_required (VERSION 3.17 FATAL_ERROR)

#Use the compilers found in the path
find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

SET(NuHepMC_CPPUtils_VERSION 0.9.9)
project(NuHepMC_CPPUtils VERSION ${NuHepMC_CPPUtils_VERSION} LANGUAGES CXX)

# The specification version used in this repo
SET(NuHepMC_VERSION_MAJOR  1)
SET(NuHepMC_VERSION_MINOR  0)
SET(NuHepMC_VERSION_PATCH  0)

set(CMAKE_CXX_STANDARD 17)

option(NuHepMC_CPPUtils_BUILTIN_HEPMC3 "Whether to build our own version of HepMC3" OFF)
option(NuHepMC_CPPUtils_PYTHON_ENABLED "Whether to build python bindings" OFF)
option(NuHepMC_CPPUtils_PROTOBUF_INTERFACE "Whether to build the protobuf interface" OFF)
option(NuHepMC_CPPUtils_ENABLE_TESTS "Whether to enable test suite" OFF)
option(NuHepMC_CPPUtils_ENABLE_SANITIZERS_CLI "Whether to enable ASAN LSAN and UBSAN" OFF)
option(NuHepMC_CPPUtils_ENABLE_GCOV_CLI "Whether to enable GCOV" OFF)

if(NuHepMC_CPPUtils_ENABLE_TESTS)
    SET(NuHepMC_CPPUtils_ENABLE_SANITIZERS ON)
    SET(NuHepMC_CPPUtils_ENABLE_GCOV ON)
else()
  if(NuHepMC_CPPUtils_ENABLE_SANITIZERS_CLI)
    SET(NuHepMC_CPPUtils_ENABLE_SANITIZERS ON)
  endif()
  if(NuHepMC_CPPUtils_ENABLE_GCOV_CLI)
    SET(NuHepMC_CPPUtils_ENABLE_GCOV ON)
  endif()
endif()

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/"
             CACHE PATH "default install path" FORCE)
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

######################### Dependencies

add_library(nuhepmc_private_compile_options INTERFACE)
target_compile_options(nuhepmc_private_compile_options INTERFACE -Wall -Werror -Wextra)
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$")
    target_compile_options(nuhepmc_private_compile_options INTERFACE -Wno-class-memaccess)
endif()

add_library(nuhepmc_options INTERFACE)

if(NuHepMC_CPPUtils_ENABLE_SANITIZERS)
  target_compile_options(nuhepmc_options BEFORE INTERFACE -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined)
  target_link_options(nuhepmc_options BEFORE INTERFACE -fsanitize=address -fsanitize=leak -fsanitize=undefined)
endif()

if(NuHepMC_CPPUtils_ENABLE_GCOV)
  target_compile_options(nuhepmc_options BEFORE INTERFACE -fprofile-arcs -ftest-coverage)
  target_link_options(nuhepmc_options BEFORE INTERFACE -fprofile-arcs -ftest-coverage)
endif()

target_include_directories(nuhepmc_options INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>)

set_target_properties(nuhepmc_options PROPERTIES EXPORT_NAME Options)
install(TARGETS nuhepmc_options
    EXPORT nuhepmc_cpputils-targets)

add_library(NuHepMC::Options ALIAS nuhepmc_options)

SET(NuHepMC_HEPMC3_USE_COMPRESSION)
SET(NuHepMC_HEPMC3_Z_SUPPORT)
SET(NuHepMC_HEPMC3_ProtobufIO_SUPPORT)

SET(NuHepMC_HEPMC3_MIN_VERSION 3.3.1)

include(get_cpm)

set(NuHepMC_FMT_VERSION "10.2.1")
set(NuHepMC_FMT_VERSION_RANGE "10...<11")
find_package(fmt ${NuHepMC_FMT_VERSION} QUIET NO_CMAKE_ENVIRONMENT_PATH)

set(NuHepMC_SPDLOG_VERSION "1.14.1")
set(NuHepMC_SPDLOG_VERSION_RANGE "1.14...<1.15")
find_package(spdlog ${NuHepMC_SPDLOG_VERSION} QUIET NO_CMAKE_ENVIRONMENT_PATH)

if(NOT spdlog_FOUND OR NOT fmt_FOUND)
  CPMAddPackage(
      NAME fmt
      VERSION "${NuHepMC_FMT_VERSION}"
      GIT_TAG "${NuHepMC_FMT_VERSION}"
      GITHUB_REPOSITORY fmtlib/fmt
      OPTIONS
          "FMT_INSTALL ON"
          "BUILD_SHARED_LIBS ON"
  )

  CPMAddPackage(
      NAME spdlog
      VERSION "${NuHepMC_SPDLOG_VERSION}"
      GIT_TAG "v${NuHepMC_SPDLOG_VERSION}"
      GITHUB_REPOSITORY gabime/spdlog
      OPTIONS
          "SPDLOG_COMPILED_LIB ON"
          "SPDLOG_BUILD_SHARED ON"
          "SPDLOG_FMT_EXTERNAL ON"
          "SPDLOG_INSTALL ON"
  )
else()
  message(STATUS "Found system fmt: ${fmt_DIR}")
  message(STATUS "Found system spdlog: ${spdlog_DIR}")
endif()

if(NuHepMC_CPPUtils_PYTHON_ENABLED)
  find_package(Python3 REQUIRED COMPONENTS Development Interpreter)

  CPMFindPackage(
    NAME pybind11
    VERSION 2.13.6
    GITHUB_REPOSITORY pybind/pybind11
    GIT_TAG "v2.13.6"
  )
endif()

if(NuHepMC_CPPUtils_BUILTIN_HEPMC3)

  set(NuHepMC_CPPUtils_BUILTIN_HEPMC3_ENABLE_PROTOBUFIO OFF)
  if(NuHepMC_CPPUtils_PROTOBUF_INTERFACE)
    find_package(Protobuf 2.4)
    if(Protobuf_FOUND)
        set(NuHepMC_CPPUtils_BUILTIN_HEPMC3_ENABLE_PROTOBUFIO ON)
    endif()
  endif()

  CPMAddPackage(
      NAME HepMC3
      VERSION ${NuHepMC_HEPMC3_MIN_VERSION}
      GIT_REPOSITORY
        https://gitlab.cern.ch/hepmc/HepMC3.git
        https://github.com/hep-mirrors/hepmc3.git
      GIT_TAG master
      OPTIONS
        "HEPMC3_CXX_STANDARD ${CMAKE_CXX_STANDARD}"
        "HEPMC3_ENABLE_SEARCH OFF"
        "HEPMC3_ENABLE_ROOTIO OFF"
        "HEPMC3_ENABLE_PROTOBUFIO ${NuHepMC_CPPUtils_BUILTIN_HEPMC3_ENABLE_PROTOBUFIO}"
        "HEPMC3_ENABLE_PYTHON ${NuHepMC_CPPUtils_PYTHON_ENABLED}"
        "HEPMC3_Python_SITEARCH${Python_VERSION_MAJOR}${Python_VERSION_MINOR} ${CMAKE_INSTALL_PREFIX}/python/${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}"
        "HEPMC3_BUILD_DOCS OFF"
        "HEPMC3_BUILD_EXAMPLES OFF"
        "HEPMC3_INSTALL_EXAMPLES OFF"
        "HEPMC3_ENABLE_TEST OFF"
        "HEPMC3_INSTALL_INTERFACES ON"
        "HEPMC3_BUILD_STATIC_LIBS OFF"
  )
else() #using a prebuilt HepMC3
  message(STATUS "Using prebuilt HepMC3")
  find_package(HepMC3 ${NuHepMC_HEPMC3_MIN_VERSION} REQUIRED)
  if(TARGET HepMC3::rootIO)
    find_package(ROOT REQUIRED)
    target_link_libraries(nuhepmc_options INTERFACE HepMC3::rootIO)
  endif()
  if(TARGET HepMC3::protobufIO)
    find_package(Protobuf 2.4 REQUIRED)
    target_link_libraries(nuhepmc_options INTERFACE HepMC3::protobufIO)
  endif()
endif()

if(TARGET HepMC3::HepMC3)
  target_link_libraries(nuhepmc_options INTERFACE HepMC3::HepMC3)

  SET(NuHepMC_Protobuf_FOUND FALSE)
  if(Protobuf_FOUND AND NuHepMC_CPPUtils_PROTOBUF_INTERFACE)
    target_link_libraries(nuhepmc_options INTERFACE HepMC3::protobufIO)
    message(STATUS "PROTOBUF_INCLUDE_DIRS: ${PROTOBUF_INCLUDE_DIRS}")
    message(STATUS "PROTOBUF_LIBRARIES: ${PROTOBUF_LIBRARIES}")
    message(STATUS "PROTOBUF_FOUND: ${Protobuf_FOUND}")
    SET(NuHepMC_Protobuf_FOUND TRUE)
    SET(NuHepMC_HEPMC3_ProtobufIO_SUPPORT "#define HEPMC3_ProtobufIO_SUPPORT 1")
  endif()

  if(TARGET HepMC3::rootIO)
    target_link_libraries(nuhepmc_options INTERFACE HepMC3::rootIO)
  endif()

  SET(NuHepMC_ZLIB_FOUND FALSE)
  find_package(ZLIB)
  if(ZLIB_FOUND)
    message(STATUS "ZLIB_INCLUDE_DIRS: ${ZLIB_INCLUDE_DIRS}")
    message(STATUS "ZLIB_LIBRARIES: ${ZLIB_LIBRARIES}")
    target_link_libraries(nuhepmc_options INTERFACE ZLIB::ZLIB)
    SET(NuHepMC_HEPMC3_USE_COMPRESSION "#define HEPMC3_USE_COMPRESSION 1")
    SET(NuHepMC_HEPMC3_Z_SUPPORT "#define HEPMC3_Z_SUPPORT 1")
    SET(NuHepMC_ZLIB_FOUND TRUE)
  endif()

  SET(NuHepMC_LibLZMA_FOUND FALSE)
  find_package(LibLZMA)
  if(LibLZMA_FOUND)
    message(STATUS "LIBLZMA_INCLUDE_DIRS: ${LIBLZMA_INCLUDE_DIRS}")
    message(STATUS "LIBLZMA_LIBRARIES: ${LIBLZMA_LIBRARIES}")
    target_link_libraries(nuhepmc_options INTERFACE LibLZMA::LibLZMA)
    SET(NuHepMC_HEPMC3_USE_COMPRESSION "#define HEPMC3_USE_COMPRESSION 1")
    SET(NuHepMC_HEPMC3_LZMA_SUPPORT "#define HEPMC3_LZMA_SUPPORT 1")
    SET(NuHepMC_LibLZMA_FOUND TRUE)
  endif()

  SET(NuHepMC_BZip2_FOUND FALSE)
  find_package(BZip2)
  if(BZip2_FOUND)
    message(STATUS "BZIP2_INCLUDE_DIRS: ${BZIP2_INCLUDE_DIRS}")
    message(STATUS "BZIP2_LIBRARIES: ${BZIP2_LIBRARIES}")
    target_link_libraries(nuhepmc_options INTERFACE BZip2::BZip2)
    SET(NuHepMC_HEPMC3_USE_COMPRESSION "#define HEPMC3_USE_COMPRESSION 1")
    SET(NuHepMC_HEPMC3_BZ2_SUPPORT "#define HEPMC3_BZ2_SUPPORT 1")
    SET(NuHepMC_BZip2_FOUND TRUE)
  endif()

else()
  message(FATAL_ERROR "HepMC3::HepMC3 target is not defined. HepMC3_FOUND=${HepMC3}")
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/HepMC3Features.hxx.in
  "${PROJECT_BINARY_DIR}/include/NuHepMC/HepMC3Features.hxx" @ONLY)
install(FILES
  "${PROJECT_BINARY_DIR}/include/NuHepMC/HepMC3Features.hxx" DESTINATION
  include/NuHepMC)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/NuHepMCVersion.hxx.in
  "${PROJECT_BINARY_DIR}/include/NuHepMC/NuHepMCVersion.hxx" @ONLY)
install(FILES
  "${PROJECT_BINARY_DIR}/include/NuHepMC/NuHepMCVersion.hxx" DESTINATION
  include/NuHepMC)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/NuHepMC-config.in
  "${PROJECT_BINARY_DIR}/NuHepMC-config" @ONLY)
install(PROGRAMS
  "${PROJECT_BINARY_DIR}/NuHepMC-config" DESTINATION bin)

install(FILES
  examples/ana_skeleton.cxx DESTINATION share/NuHepMC/examples)

add_subdirectory(src/NuHepMC)

if(NuHepMC_CPPUtils_PYTHON_ENABLED)
  # PYTHON PATHS
  set(NuHepMC_CPPUtils_PYSITEARCH "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
  set(NuHepMC_CPPUtils_PYTHONPATH "python/${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")

  add_subdirectory(python)
endif()

install(EXPORT nuhepmc_cpputils-targets
  FILE NuHepMC_CPPUtilsTargets.cmake
  NAMESPACE NuHepMC::
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/NuHepMC_CPPUtils
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/NuHepMC_CPPUtilsConfig.cmake.in
  ${CMAKE_BINARY_DIR}/NuHepMC_CPPUtilsConfig.cmake
  INSTALL_DESTINATION
    /this/is/ignored/for/some/reason/thanks/kitware
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
  ${CMAKE_BINARY_DIR}/NuHepMC_CPPUtilsConfigVersion.cmake
  VERSION ${NuHepMC_CPPUtils_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES
    ${CMAKE_BINARY_DIR}/NuHepMC_CPPUtilsConfig.cmake
    ${CMAKE_BINARY_DIR}/NuHepMC_CPPUtilsConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/NuHepMC_CPPUtils)

if(NuHepMC_CPPUtils_ENABLE_TESTS)

  CPMFindPackage(
      NAME Catch2
      GITHUB_REPOSITORY catchorg/Catch2
      VERSION 3.3.2
  )

  if(NOT "${Catch2_SOURCE_DIR}" STREQUAL "")
    LIST(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
  elseif(NOT "${Catch2_DIR}" STREQUAL "")
    LIST(APPEND CMAKE_MODULE_PATH ${Catch2_DIR})
  endif()

  include(CTest)
  include(Catch)

  add_subdirectory(tests)
endif()
