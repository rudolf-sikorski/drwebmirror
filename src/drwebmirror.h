/*
   Copyright (C) 2014-2016, Rudolf Sikorski <rudolf.sikorski@freenet.de>

   This file is part of the `drwebmirror' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DRWEBMIRROR_H_INCLUDED
#define DRWEBMIRROR_H_INCLUDED

/* Begin of custom defines block */
#define  PROG_VERSION   "1.12"
#define  LOCKFILENAME   "drwebmirror.lock"
#define  DEF_USERID     "0125742880"
#define  DEF_MD5SUM     "8e66ca120e1a786b191da63524fa719b"
#define  MAX_REPEAT     5
#define  MAX_REDIRECT   5   /* RFC 2068 */
#define  TIMEOUT        10
#define  REPEAT_SLEEP   10
#define  NETBUFSIZE     32768
#define  STRBUFSIZE     1024
#define  MODE_DIR       0755
#define  MODE_FILE      0644
#define  MODE_LOCKFILE  0600
#define  ERRFP          stdout
/* End of custom defines block */

/* Cygwin implementation of file mode bits is ugly */
#if defined(__CYGWIN__)
#undef   MODE_DIR
#define  MODE_DIR       0777
#undef   MODE_FILE
#define  MODE_FILE      0777
#undef   MODE_LOCKFILE
#define  MODE_LOCKFILE  0777
#endif

/* Windows-specific file mode bits */
#if defined(_WIN32)
#include <sys/stat.h>
#if !defined (S_IRUSR)
#define S_IRUSR (_S_IREAD)
#endif
#if !defined (S_IWUSR)
#define S_IWUSR (_S_IWRITE)
#endif
#if !defined (S_IRWXU)
#define S_IRWXU ((_S_IREAD) | (_S_IWRITE))
#endif
#undef   MODE_DIR
#define  MODE_DIR       (S_IRWXU)
#undef   MODE_FILE
#define  MODE_FILE      (S_IRWXU)
#undef   MODE_LOCKFILE
#define  MODE_LOCKFILE  ((S_IRUSR) | (S_IWUSR))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include "avltree/avltree.h"

#if !(defined(_WIN32) && defined (_MSC_VER))
#include <unistd.h>
#else
#include <direct.h>
#include <io.h>
#include <windows.h>
#define sleep(s) Sleep((s)*1000)
#define chmod(name, mode)       (_chmod((name), (mode)))
#define chdir(path)             (_chdir((path)))
#define getcwd(buf, size)       (_getcwd((buf), (size)))
#define mkdir(path)             (_mkdir(path))
#define open(name, flag, mode)  (_open((name), (flag), (mode)))
#define close(fd)               (_close((fd)))
#define NO_POSIX_API
#endif

/* Flag of use verbose output */
extern int8_t verbose;
/* Flag of use even more verbose output */
extern int8_t more_verbose;

/* System timezone */
extern time_t tzshift;
/* UserID from license key */
extern char key_userid[33];
/* MD5 sum of license key */
extern char key_md5sum[33];
/* Server name */
extern char servername[256];
/* Server port */
extern uint16_t serverport;
/* Server auth */
extern int8_t use_http_auth;
extern char http_auth[77];
/* Remote directory */
extern char remotedir[256];
/* User Agent */
extern char useragent[256];
/* SysHash */
extern char syshash[33];
extern int8_t use_syshash;
/* Android */
extern int8_t use_android;
/* Proxy parameters */
extern int8_t use_proxy;
extern int8_t use_proxy_auth;
extern char proxy_address[256];
extern uint16_t proxy_port;
extern char proxy_auth[77];

/* Tree for caching checksums in fast mode */
extern avl_node * tree;
/* Flag of use fast mode */
extern int8_t use_fast;

/* Lokfile name */
extern char lockfile[384];

/* Common */
/* Get system timezone */
void set_tzshift();
/* Set SIGHUP handle */
#if !defined(_WIN32)
void sighup_handler(int i);
#endif
/* Convert string to lowercase */
void to_lowercase(char * str);
/* Base64 encoding (RFC 2045) */
size_t base64_encode(const char * in, char * out);
/* Size-bounded string copying */
size_t bsd_strlcpy(char * dst, const char * src, size_t dsize);

/* Drwebmirror */
/* Get UserID and MD5 sum from keyfile */
int parse_keyfile(const char * filename);
/* Update using version 4 of update protocol (flat file, crc32) */
int update4();
/* Update using version 5 of update protocol (flat file, sha256) */
int update5();
/* Update using version 7 of update protocol (xml file, sha256) */
int update7();
/* Update using Android update protocol */
int updateA();

/* Network */
/* Return values for download() and download_check() functions */
#define DL_EXIST        0x00
#define DL_DOWNLOADED   0x01
#define DL_FAILED       0x02
#define DL_TRY_AGAIN    0x03
#define DL_NOT_FOUND    0x04
#define DL_SUCCESS(st) ((st)==(DL_EXIST)||(st)==(DL_DOWNLOADED))
/* Get file <filename> from server */
int conn_get(const char * filename);
/* Startup network */
void conn_startup();
/* Cleanup network */
void conn_cleanup();
/* Download file <filename> */
int download(const char * filename);
/* Download file <filename> and compare checksum <checksum_base>
 * with <checksum_real> using <checksum_func> function */
int download_check(const char * filename, const char * checksum_base, char * checksum_real,
                   int (* checksum_func)(const char *, char *), const char * checksum_desc);

/* Filesystem */
/* Set modification time <mtime> to file <filename> */
int set_mtime(const char * filename, const time_t mtime);
/* Recursive make directory <path> */
int make_path(const char * path);
/* Recursive make directoty for file <filename> */
int make_path_for(char * filename);
/* Delete files by mask <mask> in directory <directory> */
int delete_files(const char * directory, const char * mask);
/* Check <filename> exist */
int exist(const char * filename);
/* Get <filename> size */
off_t get_size(const char * filename);
/* Compare size of <filename> with <filesize> */
int check_size(const char * filename, off_t filesize);
/* Open temp file */
FILE * fopen_temp(char * filename);
/* Lock file */
int do_lock(const char * directory);
/* Unlock file */
int do_unlock();

/* Checksum */
/* Calculate MD5 sum of file <filename> */
int md5sum(const char * filename, char str[33]);
/* Calculate CRC32 sum of file <filename> */
int crc32sum(const char * filename, char str[9]);
/* Calculate SHA256 sum of file <filename> */
int sha256sum(const char * filename, char str[65]);
/* Calculate CRC32 sum of contains LZMA <filename> */
int crc32sum_lzma(const char * filename, char str[9]);
/* Calculate SHA256 sum of contains LZMA <filename> */
int sha256sum_lzma(const char * filename, char str[65]);

/* Decompress */
/* Decompress LZMA archive <input> to file <output> */
int decompress_lzma(FILE * input, FILE * output);
/* Compare size of LZMA archive <filename> content with <filesize> */
int check_size_lzma(const char * filename, off_t filesize);

#endif /* DRWEBMIRROR_H_INCLUDED */
