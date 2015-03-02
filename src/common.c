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

#include "drwebmirror.h"

/* System timezone */
long tzshift;

/* Get system timezone */
void set_tzshift()
{
    time_t currtime = time(NULL);
    struct tm * ptm;
    long uthr, lthr;
    ptm = gmtime(& currtime); /* Time in GMT */
    uthr = mktime(ptm);
    ptm = localtime(& currtime); /* Time in local timezone */
    lthr = mktime(ptm);
    tzshift = lthr - uthr;
}

/* Set SIGHUP handle */
void sighup_handler(int i)
{
    (void)i;
    fprintf(ERRFP, "Received SIGHUP, ignoring.\n");
}

/* Convert string to lowercase */
void to_lowercase(char * str)
{
    size_t i = 0, len = strlen(str);
    for(; i < len; i++)
        if(str[i] >= 'A' && str[i] <= 'Z')
            str[i] -= 'A' - 'a';
}
