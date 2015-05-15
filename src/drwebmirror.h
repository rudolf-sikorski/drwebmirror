/*
   Copyright (C) 2014-2015, Rudolf Sikorski <rudolf.sikorski@freenet.de>

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
#define  PROG_VERSION   "1.7"
#define  LOCKFILENAME   "drwebmirror.lock"
#define  DEF_USERID     "0117974833"
#define  DEF_MD5SUM     "c41fca5271008eb5f94d308b99a896a7"
#define  DEF_SERVER     "update.drweb.com"
#define  DEF_USERAGENT  "DrWebUpdate-6.00.12.03291 (windows: 6.01.7601)"
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include "avltree/avltree.h"

/* Flag of use verbose output */
extern int8_t verbose;
/* Flag of use even more verbose output */
extern int8_t more_verbose;

/* System timezone */
extern long tzshift;
/* UserID from license key */
extern char key_userid[33];
/* MD5 sum of license key */
extern char key_md5sum[33];
/* Server name */
extern char servername[256];
/* Server port */
extern uint16_t serverport;
/* Remote directory */
extern char remotedir[256];
/* User Agent */
extern char useragent[256];
/* SysHash */
extern char syshash[33];
extern int8_t use_syshash;
/* Android */
extern int8_t use_android;

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
void sighup_handler(int i);
/* Convert string to lowercase */
void to_lowercase(char * str);

/* Drwebmirror */
/* Get UserID and MD5 sum from keyfile */
int parse_keyfile(char * filename);
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

#endif
