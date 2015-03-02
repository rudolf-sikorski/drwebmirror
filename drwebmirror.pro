TEMPLATE = app
CONFIG += console
CONFIG += warn_on
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/main.c \
    src/common.c \
    src/filesystem.c \
    src/network.c \
    src/decompress.c \
    src/checksum.c \
    src/drwebmirror.c \
    src/avltree/avltree.c \
    src/crc32/crc32.c \
    src/md5/md5c.c \
    src/sha256/sha256.c \
    src/lzma/Alloc.c \
    src/lzma/LzmaDec.c \
    src/lzma/7zFile.c \
    src/lzma/7zStream.c

HEADERS += \
    src/drwebmirror.h \
    src/avltree/avltree.h \
    src/md5/global.h \
    src/md5/md5.h \
    src/lzma/7zFile.h \
    src/lzma/Alloc.h \
    src/lzma/LzmaDec.h \
    src/lzma/Types.h

QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE *= -O3
QMAKE_CFLAGS_WARN_ON *= -Wall
QMAKE_CFLAGS_WARN_ON *= -Wextra
