/*
   Copyright (C) 2014-2019, Rudolf Sikorski <rudolf.sikorski@freenet.de>

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

/* System timezone */
time_t tzshift;

/* Get system timezone */
void set_tzshift(void)
{
    time_t currtime = time(NULL);
    struct tm * ptm;
    time_t uthr, lthr;
    ptm = gmtime(& currtime); /* Time in GMT */
    uthr = mktime(ptm);
    ptm = localtime(& currtime); /* Time in local timezone */
    lthr = mktime(ptm);
    tzshift = lthr - uthr;
}

/* Set SIGHUP handle */
#if !defined(_WIN32)
void sighup_handler(int i)
{
    (void)i;
    fprintf(ERRFP, "Received SIGHUP, ignoring.\n");
}
#endif

/* Convert string to lowercase */
void to_lowercase(char * str)
{
    size_t i = 0, len = strlen(str);
    for(; i < len; i++)
        if(str[i] >= 'A' && str[i] <= 'Z')
            str[i] -= 'A' - 'a';
}

/* Base64 encoding (RFC 2045) */
size_t base64_encode(const char * in, char * out)
{
    static const char dict[] =
    {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
        'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
        'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '+', '/'
    };
    char * curr = out;
    size_t i, len = strlen(in);
    for(i = 0; i < len; i += 3)
    {
        int b = (in[i] & 0xFC) >> 2;
        * curr++ = dict[b];
        b = (in[i] & 0x03) << 4;
        if(i + 1 < len)
        {
            b |= (in[i + 1] & 0xF0) >> 4;
            * curr++ = dict[b];
            b = (in[i + 1] & 0x0F) << 2;
            if(i + 2 < len)
            {
                b |= (in[i + 2] & 0xC0) >> 6;
                * curr++ = dict[b];
                b = in[i + 2] & 0x3F;
                * curr++ = dict[b];
            }
            else
            {
                * curr++ = dict[b];
                * curr++ = '=';
            }
        }
        else
        {
            * curr++ = dict[b];
            * curr++ = '=';
            * curr++ = '=';
        }
    }
    * curr++ = '\0';
    return curr - out;
}
