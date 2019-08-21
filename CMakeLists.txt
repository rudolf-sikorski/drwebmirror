#
#   Copyright (C) 2019, Rudolf Sikorski <rudolf.sikorski@freenet.de>
#
#   This file is part of the `drwebmirror' program.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(DrWebMirror C)

INCLUDE(CheckCCompilerFlag)

FUNCTION(ADD_SUPPORTED_COMPILER_FLAG BUILD_TYPE FLAG)
  CHECK_C_COMPILER_FLAG(${FLAG} SUPPORTED_${FLAG})
  IF(SUPPORTED_${FLAG})
    IF("${BUILD_TYPE}" STREQUAL "RELEASE")
      SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${FLAG}" PARENT_SCOPE)
    ELSEIF("${BUILD_TYPE}" STREQUAL "DEBUG")
      SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${FLAG}" PARENT_SCOPE)
    ELSE()
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}" PARENT_SCOPE)
    ENDIF()
  ENDIF()
ENDFUNCTION()

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
ADD_EXECUTABLE(drwebmirror
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/common.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/network.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/decompress.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/checksum.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/drwebmirror.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/drwebmirror.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/avltree/avltree.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/avltree/avltree.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/strlcpy/strlcpy.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/crc32/crc32.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md5/global.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md5/md5.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md5/md5c.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/sha256/sha256.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/7zFile.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/7zFile.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/7zStream.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/7zTypes.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/Alloc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/Alloc.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/Compiler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/LzmaDec.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/LzmaDec.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lzma/Precomp.h"
  )

IF(WIN32 AND NOT CYGWIN)
  TARGET_LINK_LIBRARIES(drwebmirror wsock32)
ENDIF()

IF("id_${CMAKE_C_COMPILER_ID}" STREQUAL "id_GNU" OR
   "id_${CMAKE_C_COMPILER_ID}" STREQUAL "id_Clang" OR
   "id_${CMAKE_C_COMPILER_ID}" STREQUAL "id_AppleClang")
  ADD_SUPPORTED_COMPILER_FLAG(ANY "-Wall")
  ADD_SUPPORTED_COMPILER_FLAG(ANY "-Wextra")
  ADD_SUPPORTED_COMPILER_FLAG(ANY "-Wno-format-overflow")
  ADD_SUPPORTED_COMPILER_FLAG(RELEASE "-O3")
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
  IF(WIN32 AND NOT CYGWIN)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
  ENDIF()
ENDIF()

IF("id_${CMAKE_C_COMPILER_ID}" STREQUAL "id_MSVC")
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/src/msinttypes")
  FOREACH(VAR CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
    FOREACH(FLAG "/MDd" "/MD" "/MTd" "/MT" "/W1" "/W2" "/W3" "/W4")
      IF(${VAR} MATCHES ${FLAG})
        STRING(REGEX REPLACE ${FLAG} "" ${VAR} "${${VAR}}")
      ENDIF()
    ENDFOREACH()
  ENDFOREACH()
  FOREACH(VAR CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE)
    FOREACH(FLAG "/O0" "/O1" "/O2")
      IF(${VAR} MATCHES ${FLAG})
        STRING(REGEX REPLACE ${FLAG} "" ${VAR} "${${VAR}}")
      ENDIF()
    ENDFOREACH()
  ENDFOREACH()
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CRT_SECURE_NO_WARNINGS /W3")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
  SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Ox /MT /GS-")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SAFESEH:NO")
ENDIF()

IF("id_${CMAKE_C_COMPILER_ID}" STREQUAL "id_Watcom" OR
   "id_${CMAKE_C_COMPILER_ID}" STREQUAL "id_OpenWatcom")
  ADD_SUPPORTED_COMPILER_FLAG(ANY "/MT")
  ADD_SUPPORTED_COMPILER_FLAG(ANY "/W3")
  ADD_SUPPORTED_COMPILER_FLAG(RELEASE "/Ox")
ENDIF()

INSTALL(TARGETS drwebmirror RUNTIME DESTINATION bin)
