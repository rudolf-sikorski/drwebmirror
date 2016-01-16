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

#include "drwebmirror.h"
#include "md5/global.h"
#include "md5/md5.h"
uint32_t crc32(uint32_t crc, const void * buf, size_t size);
int sha_file(const char * filename, unsigned char * hash);
typedef struct {
    uint32_t state[8], length, curlen;
    unsigned char buf[64];
} sha_state;
void sha_init(sha_state * md);
void sha_process(sha_state * md, unsigned char * buf, int len);
void sha_done(sha_state * md, unsigned char * hash);

/* Calculate MD5 sum of file <filename> */
int md5sum(const char * filename, char str[33])
{
    FILE * file = fopen(filename, "rb");
    MD5_CTX context;
    size_t len;
    unsigned char buffer[1024], digest[16];
    unsigned int i;

    if(file == NULL)
        return EXIT_FAILURE;

    MD5Init(& context);
    while((len = fread(buffer, 1, 1024, file)))
        MD5Update(& context, buffer, len);
    MD5Final(digest, & context);
    fclose(file);

    if(str)
    {
        for(i = 0; i < 16; i++)
            sprintf(str + i * 2, "%02x", digest[i]);
    }

    return EXIT_SUCCESS;
}

/* Calculate CRC32 sum of file <filename> */
int crc32sum(const char * filename, char str[9])
{
    FILE * file = fopen(filename, "rb");
    size_t len;
    unsigned char buffer[1024];
    uint32_t crc = 0;

    if(file == NULL)
        return EXIT_FAILURE;

    while((len = fread(buffer, 1, 1024, file)))
        crc = crc32(crc, buffer, len);
    fclose(file);

    if(str)
        sprintf(str, "%X", crc);

    return EXIT_SUCCESS;
}

/* Calculate SHA256 sum of file <filename> */
int sha256sum(const char * filename, char str[65])
{
    unsigned char buf[33];
    int i;

    if(!sha_file(filename, buf))
        return EXIT_FAILURE;

    if(str)
    {
        for(i = 0; i < 32; i++)
            sprintf(str + i * 2, "%02x", buf[i]);
    }

    return EXIT_SUCCESS;
}

/* Calculate CRC32 sum of contains LZMA <filename> */
int crc32sum_lzma(const char * filename, char str[9])
{
    FILE * file = fopen(filename, "rb");
    FILE * tmpf;
    size_t len;
    unsigned char buffer[1024];
    uint32_t crc = 0;
    char name[STRBUFSIZE] = "\0";

    if(file == NULL)
        return EXIT_FAILURE;

    if((tmpf = fopen_temp(name)) == NULL)
    {
        fclose(file);
        return EXIT_FAILURE;
    }

    if(decompress_lzma(file, tmpf) != EXIT_SUCCESS)
    {
        fclose(file);
        fclose(tmpf);
        return EXIT_FAILURE;
    }
    fclose(file);
    rewind(tmpf);

    while((len = fread(buffer, 1, 1024, tmpf)))
        crc = crc32(crc, buffer, len);
    fclose(tmpf);
    if(name[0] != '\0') remove(name);

    if(str)
        sprintf(str, "%X", crc);

    return EXIT_SUCCESS;
}

/* Calculate SHA256 sum of contains LZMA <filename> */
int sha256sum_lzma(const char * filename, char str[65])
{
    FILE * file = fopen(filename, "rb");
    FILE * tmpf;
    int i;
    unsigned char buffer[512];
    char name[STRBUFSIZE] = "\0";
    sha_state md;
    unsigned char hash[32];

    if(file == NULL)
        return EXIT_FAILURE;
    if((tmpf = fopen_temp(name)) == NULL)
    {
        fclose(file);
        return EXIT_FAILURE;
    }

    if(decompress_lzma(file, tmpf) != EXIT_SUCCESS)
    {
        fclose(file);
        fclose(tmpf);
        return EXIT_FAILURE;
    }
    fclose(file);
    rewind(tmpf);

    sha_init(& md);
    do
    {
        i = (int)fread(buffer, 1, 512, tmpf);
        sha_process(& md, buffer, i);
    }
    while(i == 512);
    sha_done(& md, hash);
    fclose(tmpf);
    if(name[0] != '\0') remove(name);

    if(str)
    {
        for(i = 0; i < 32; i++)
            sprintf(str + i * 2, "%02x", hash[i]);
    }

    return EXIT_SUCCESS;
}
