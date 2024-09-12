# Install script for directory: /Users/anastasia/Downloads/weechat-4.4.2/src/plugins

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/weechat/weechat-plugin.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/include/weechat" TYPE FILE FILES "/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/weechat-plugin.h")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/alias/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/buflist/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/charset/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/exec/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/fifo/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/fset/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/irc/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/logger/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/relay/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/script/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/perl/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/python/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/lua/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/tcl/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/spell/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/trigger/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/typing/cmake_install.cmake")
  include("/Users/anastasia/Downloads/weechat-4.4.2/src/plugins/xfer/cmake_install.cmake")

endif()

