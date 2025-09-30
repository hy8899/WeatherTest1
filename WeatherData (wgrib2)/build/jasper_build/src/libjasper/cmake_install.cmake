# Install script for directory: /home/admin1/build/jasper/src/libjasper

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/admin1/local/jasper")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libjasper.so.7.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libjasper.so.7"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "/home/admin1/local/jasper/lib")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/admin1/build/jasper_build/src/libjasper/libjasper.so.7.0.0"
    "/home/admin1/build/jasper_build/src/libjasper/libjasper.so.7"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libjasper.so.7.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libjasper.so.7"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH ":::::::::::::::::::::::::::::"
           NEW_RPATH "/home/admin1/local/jasper/lib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/admin1/build/jasper_build/src/libjasper/libjasper.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/jasper" TYPE FILE FILES
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_cm.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_compiler.h"
    "/home/admin1/build/jasper_build/src/libjasper/include/jasper/jas_config.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_debug.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_dll.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_fix.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_getopt.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_icc.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_image.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_init.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_log.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_malloc.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_math.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jasper.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_seq.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_stream.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_string.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_thread.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_tmr.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_tvp.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_types.h"
    "/home/admin1/build/jasper/src/libjasper/include/jasper/jas_version.h"
    "/home/admin1/build/jasper_build/src/libjasper/include/jasper/jas_export_cmake.h"
    )
endif()

