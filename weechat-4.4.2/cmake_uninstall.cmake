#
# Copyright (C) 2003-2024 Sébastien Helleu <flashcode@flashtux.org>
#
# This file is part of WeeChat, the extensible chat client.
#
# WeeChat is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# WeeChat is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with WeeChat.  If not, see <https://www.gnu.org/licenses/>.
#

if(NOT EXISTS "/Users/anastasia/Downloads/weechat-4.4.2/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: \"/Users/anastasia/Downloads/weechat-4.4.2/install_manifest.txt\"")
endif()

file(READ "/Users/anastasia/Downloads/weechat-4.4.2/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
string(REGEX REPLACE ";$" "" files "${files}")
list(REVERSE files)
foreach(file ${files})
  message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
  if(EXISTS "$ENV{DESTDIR}${file}")
    execute_process(COMMAND "/opt/homebrew/Cellar/cmake/3.29.2/bin/cmake" -E remove "$ENV{DESTDIR}${file}" OUTPUT_VARIABLE rm_out RESULT_VARIABLE rm_retval)
    if("${rm_retval}" GREATER 0)
      message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif()
  else()
    message(STATUS "File \"$ENV{DESTDIR}${file}\" does not exist.")
  endif()
endforeach()
