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

cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

#Use the compilers found in the path
find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

project(NuHepMC_cpputils VERSION 0.9.0)

# The specification version used in this repo
SET(NuHepMC_VERSION_MAJOR  0)
SET(NuHepMC_VERSION_MINOR  9)
SET(NuHepMC_VERSION_PATCH  0)

set(CMAKE_CXX_STANDARD 17)

option(BUILTIN_HEPMC3 "Whether to use build our own version of HepMC3" FALSE)

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if(CMAKE_INSTALL_PREFIX STREQUAL "" OR CMAKE_INSTALL_PREFIX STREQUAL
  "/usr/local")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
elseif(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
endif()

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

#### RPATH handling #####

set(CMAKE_MACOSX_RPATH 1)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")


######################### Dependencies

add_library(nuhepmc_cpputils INTERFACE)
target_include_directories(nuhepmc_cpputils INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include> 
  $<INSTALL_INTERFACE:include>)

SET(NuHepMC_HEPMC3_USE_COMPRESSION)
SET(NuHepMC_HEPMC3_Z_SUPPORT)
SET(NuHepMC_HEPMC3_ProtobufIO_SUPPORT)

if(BUILTIN_HEPMC3)
  include(CPM)

  find_package(Protobuf 2.4 REQUIRED)

  set(BUILTIN_HEPMC3_ENABLE_PROTOBUFIO OFF)
  if(Protobuf_FOUND)
    set(BUILTIN_HEPMC3_ENABLE_PROTOBUFIO ON)
    SET(NuHepMC_HEPMC3_ProtobufIO_SUPPORT "#define HEPMC3_ProtobufIO_SUPPORT 1")
  endif()

  CPMAddPackage(
      NAME HepMC3
      VERSION 3.2.6
      GIT_REPOSITORY "https://gitlab.cern.ch/hepmc/HepMC3.git"
      GIT_TAG 3.2.6
      OPTIONS
        "HEPMC3_CXX_STANDARD ${CMAKE_CXX_STANDARD}"
        "HEPMC3_ENABLE_SEARCH OFF"
        "HEPMC3_ENABLE_ROOTIO OFF"
        "HEPMC3_ENABLE_PROTOBUFIO ${BUILTIN_HEPMC3_ENABLE_PROTOBUFIO}"
        "HEPMC3_ENABLE_PYTHON OFF"
        "HEPMC3_BUILD_DOCS OFF"
        "HEPMC3_BUILD_EXAMPLES OFF"
        "HEPMC3_INSTALL_EXAMPLES OFF"
        "HEPMC3_ENABLE_TEST OFF"
        "HEPMC3_INSTALL_INTERFACES OFF"
        "HEPMC3_BUILD_STATIC_LIBS OFF"
  )

endif()

if(TARGET HepMC3::All)
  target_link_libraries(nuhepmc_cpputils INTERFACE HepMC3::All)

  find_package(ZLIB)
  if(ZLIB_FOUND)
    target_link_libraries(nuhepmc_cpputils INTERFACE ZLIB::ZLIB)
    SET(NuHepMC_HEPMC3_USE_COMPRESSION "#define HEPMC3_USE_COMPRESSION 1")
    SET(NuHepMC_HEPMC3_Z_SUPPORT "#define HEPMC3_Z_SUPPORT 1")
  endif()

  find_package(LibLZMA)
  if(LibLZMA_FOUND)
    target_link_libraries(nuhepmc_cpputils INTERFACE LibLZMA::LibLZMA)
    SET(NuHepMC_HEPMC3_USE_COMPRESSION "#define HEPMC3_USE_COMPRESSION 1")
    SET(NuHepMC_HEPMC3_LZMA_SUPPORT "#define HEPMC3_LZMA_SUPPORT 1")
  endif()

  find_package(BZip2)
  if(BZip2_FOUND)
    target_link_libraries(nuhepmc_cpputils INTERFACE BZip2::BZip2)
    SET(NuHepMC_HEPMC3_USE_COMPRESSION "#define HEPMC3_USE_COMPRESSION 1")
    SET(NuHepMC_HEPMC3_BZ2_SUPPORT "#define HEPMC3_BZ2_SUPPORT 1")
  endif()

else()
  if(BUILTIN_HEPMC3)
    message(FATAL_ERROR "BUILTIN_HEPMC3 enabled but HepMC3::All was not defined after CPM call completed.")
  endif()
endif()

add_library(NuHepMC::CPPUtils ALIAS nuhepmc_cpputils)

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

install(DIRECTORY include/NuHepMC DESTINATION include)