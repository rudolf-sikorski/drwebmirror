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

/* UserID from license key */
char key_userid[33];
/* MD5 sum of license key */
char key_md5sum[33];

/* Tree for caching checksums in fast mode */
avl_node * tree;
/* Flag of use fast mode */
int8_t use_fast;

/* Get UserID and MD5 sum from keyfile */
int parse_keyfile(char * filename)
{
    char str[255];
    FILE * fp = fopen(filename, "r");
    int8_t flag = 1;

    if(fp == NULL)
    {
        fprintf(ERRFP, "Error with fopen() on %s\n", filename);
        return EXIT_FAILURE;
    }

    while(flag) /* Find "[User]" block */
    {
        if(fscanf(fp, "%[^\r\n]\r\n", str) == -1)
        {
            fprintf(ERRFP, "Unexpected EOF on %s\n", filename);
            fclose(fp);
            return EXIT_FAILURE;
        }
        if(strcmp(str, "[User]") == 0)
            flag = 0;
    }

    flag = 1;
    while(flag) /* Find "Number" field */
    {
        if(fscanf(fp, "%[^\r\n]\r\n", str) == -1)
        {
            fprintf(ERRFP, "Unexpected EOF on %s\n", filename);
            fclose(fp);
            return EXIT_FAILURE;
        }
        if(strstr(str, "Number"))
            flag = 0;
    }

    fclose(fp);

    strcpy(key_userid, strchr(str, '=') + 1);

    if(md5sum(filename, key_md5sum) != EXIT_SUCCESS) /* MD5 sum */
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/* Build caching tree for v4 */
void cache4()
{
    char buf[STRBUFSIZE];
    FILE * fp;
    int8_t flag = 1;
    sprintf(buf, "%s/%s", remotedir, "drweb32.lst");
    fp = fopen(buf, "r");
    if(!fp) return;
    while(flag)
    {
        if(fscanf(fp, "%[^\r\n]\r\n", buf) == -1)
            flag = 0;
        else if(buf[0] == '+' || buf[0] == '=' || buf[0] == '!') /* Need to download this file */
        {
            char filename[STRBUFSIZE];
            char crc_base[9];
            char * beg = buf + 1, * tmp;
            tmp = strchr(beg, '>'); /* if some as "=<w95>spider.vxd, C54AAA37" */
            if(tmp) beg = tmp + 1;
            tmp = strrchr(beg, '\\'); /* if some as "=<wnt>%SYSDIR%\spider.cpl, 871D501E" */
            if(tmp) beg = tmp + 1;
            sprintf(filename, "%s/%s", remotedir, beg);
            * strchr(filename, ',') = '\0';
            tmp = strchr(filename, '|'); /* if some as "!drwreg.exe|-xi, FE7E4B36" */
            if(tmp) * tmp = '\0';
            strcpy(crc_base, strchr(buf, ',') + 2);
            while(crc_base[0] == '0') /* if base crc32 beign with zero */
                memmove(crc_base, crc_base + 1, sizeof(char) * strlen(crc_base));
            tree = avl_insert(tree, filename, crc_base);
            strcat(filename, ".lzma");
            tree = avl_insert(tree, filename, crc_base);
        }
    }
    fclose(fp);
}

/* Update using version 4 of update protocol (flat file, crc32) */
int update4()
{
    char buf[STRBUFSIZE];
    FILE * fp;
    int8_t flag;
    int counter_global = 0, status;
    char main_hash_old[65], main_hash_new[65];

    if(make_path(remotedir) != EXIT_SUCCESS) /* Make all needed directory */
    {
        fprintf(ERRFP, "Error: Can't access to local directory\n");
        return EXIT_FAILURE;
    }
    if(do_lock(remotedir) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if(use_fast)
    {
        sprintf(buf, "%s/%s", remotedir, "drweb32.lst");
        status = sha256sum(buf, main_hash_old);
        if(status != EXIT_SUCCESS)
        {
            use_fast = 0;
            fprintf(ERRFP, "Warning: drweb32.lst was not found\n");
            fprintf(ERRFP, "Warning: Fast mode has been disabled\n");
        }
        else
            cache4();
    }

repeat4: /* Goto here if checksum mismatch */
    if(counter_global > 0 && use_fast) /* Incomplete update will lead to integrity violations */
    {
        use_fast = 0;
        fprintf(ERRFP, "Warning: Fast mode has been disabled\n");
    }

    sprintf(buf, "%s/%s", remotedir, "drweb32.lst");
    status = download(buf);
    if(!DL_SUCCESS(status))
        return EXIT_FAILURE;
    if(use_fast)
    {
        sha256sum(buf, main_hash_new);
        if(strcmp(main_hash_old, main_hash_new) == 0)
            return EXIT_SUCCESS;
    }
    /* Optional files */
    sprintf(buf, "%s/%s", remotedir, "drweb32.lst.lzma");
    download(buf);
    sprintf(buf, "%s/%s", remotedir, "version.lst");
    download(buf);
    sprintf(buf, "%s/%s", remotedir, "version.lst.lzma");
    download(buf);
    sprintf(buf, "%s/%s", remotedir, "drweb32.flg");
    download(buf);
    sprintf(buf, "%s/%s", remotedir, "drweb32.flg.lzma");
    download(buf);

    /* Main file */
    sprintf(buf, "%s/%s", remotedir, "drweb32.lst");
    fp = fopen(buf, "r");
    flag = 1;
    while(flag)
    {
        if(fscanf(fp, "%[^\r\n]\r\n", buf) == -1)
            flag = 0;
        else if(buf[0] == '+' || buf[0] == '=' || buf[0] == '!') /* Need to download this file */
        {
            char filename[STRBUFSIZE];
            char crc_base[9], crc_real[9];
            char * beg = buf + 1, * tmp;
            tmp = strchr(beg, '>'); /* if some as "=<w95>spider.vxd, C54AAA37" */
            if(tmp) beg = tmp + 1;
            tmp = strrchr(beg, '\\'); /* if some as "=<wnt>%SYSDIR%\spider.cpl, 871D501E" */
            if(tmp) beg = tmp + 1;
            sprintf(filename, "%s/%s", remotedir, beg);
            * strchr(filename, ',') = '\0';
            tmp = strchr(filename, '|'); /* if some as "!drwreg.exe|-xi, FE7E4B36" */
            if(tmp) * tmp = '\0';
            strcpy(crc_base, strchr(buf, ',') + 2);
            while(crc_base[0] == '0') /* if base crc32 beign with zero */
                memmove(crc_base, crc_base + 1, sizeof(char) * strlen(crc_base));

            status = download_check(filename, crc_base, crc_real, & crc32sum, "CRC32");
            if(status == DL_TRY_AGAIN && counter_global < MAX_REPEAT) /* Try again */
            {
                counter_global++;
                fclose(fp);
                sleep(REPEAT_SLEEP);
                goto repeat4; /* Yes, it is goto. Sorry, Dijkstra... */
            }
            else if(!DL_SUCCESS(status))
            {
                fclose(fp);
                return EXIT_FAILURE;
            }

            sprintf(buf, "%s.lzma", filename); /* Also get lzma file, if exist */
            if(status == DL_DOWNLOADED || exist(buf))
            {
                status = download_check(buf, crc_base, crc_real, & crc32sum_lzma, "CRC32 LZMA");
                if(status == DL_NOT_FOUND) /* Need for delete lzma file */
                {
                    if(exist(buf))
                    {
                        char * nm = strrchr(buf, '/') + 1;
                        memmove(buf, nm, (strlen(nm) + 1) * sizeof(char));
                        printf("Deleting... %s\n", buf);
                        delete_files(remotedir, buf);
                    }
                }
                else if(status == DL_TRY_AGAIN && counter_global < MAX_REPEAT) /* Try again */
                {
                    counter_global++;
                    fclose(fp);
                    sleep(REPEAT_SLEEP);
                    goto repeat4; /* Yes, it is goto. Sorry, Dijkstra... */
                }
                else if(!DL_SUCCESS(status))
                {
                    fclose(fp);
                    return EXIT_FAILURE;
                }
            }
        }
        else if(buf[0] == '-') /* Need to delete this file */
        {
            char filename[STRBUFSIZE];
            sprintf(filename, "%s", buf + 1);
            * strchr(filename, ',') = '\0';
            delete_files(remotedir, filename);
            strcat(filename, ".lzma");
            delete_files(remotedir, filename);
        }
    }

    fclose(fp);
    return EXIT_SUCCESS;
}

/* Build caching tree for v5 */
void cache5()
{
    char buf[STRBUFSIZE];
    FILE * fp;
    int8_t flag = 1;
    sprintf(buf, "%s/%s", remotedir, "version.lst");
    fp = fopen(buf, "r");
    if(!fp) return;
    while(flag)
    {
        if(fscanf(fp, "%[^\r\n]\r\n", buf) == -1)
            flag = 0;
        else if(buf[0] == '+' || buf[0] == '=' || buf[0] == '!') /* Need to download this file */
        {
            char filename[STRBUFSIZE];
            char sha_base[65];
            char * beg = buf + 1, * tmp;
            tmp = strchr(beg, '>'); /* if some as "=<w95>spider.vxd, ..." */
            if(tmp) beg = tmp + 1;
            tmp = strrchr(beg, '\\'); /* if some as "=<wnt>%SYSDIR%\spider.cpl, ..." */
            if(tmp) beg = tmp + 1;
            sprintf(filename, "%s/%s", remotedir, beg);
            * strchr(filename, ',') = '\0';
            tmp = strchr(filename, '|'); /* if some as "!drwreg.exe|-xi, ..." */
            if(tmp) * tmp = '\0';
            strncpy(sha_base, strchr(buf, ',') + 2, 64);
            sha_base[64] = '\0';
            tree = avl_insert(tree, filename, sha_base);
            strcat(filename, ".lzma");
            tree = avl_insert(tree, filename, sha_base);
        }
    }
    fclose(fp);
}

/* Update using version 5 of update protocol (flat file, sha256) */
int update5()
{
    char buf[STRBUFSIZE];
    FILE * fp;
    int8_t flag;
    int counter_global = 0, status;
    char main_hash_old[65], main_hash_new[65];

    if(make_path(remotedir) != EXIT_SUCCESS) /* Make all needed directory */
    {
        fprintf(ERRFP, "Error: Can't access to local directory\n");
        return EXIT_FAILURE;
    }
    if(do_lock(remotedir) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if(use_fast)
    {
        sprintf(buf, "%s/%s", remotedir, "version.lst");
        status = sha256sum(buf, main_hash_old);
        if(status != EXIT_SUCCESS)
        {
            use_fast = 0;
            fprintf(ERRFP, "Warning: version.lst was not found\n");
            fprintf(ERRFP, "Warning: Fast mode has been disabled\n");
        }
        else
            cache5();
    }

repeat5: /* Goto here if checksum mismatch */
    if(counter_global > 0 && use_fast) /* Incomplete update will lead to integrity violations */
    {
        use_fast = 0;
        fprintf(ERRFP, "Warning: Fast mode has been disabled\n");
    }

    sprintf(buf, "%s/%s", remotedir, "version.lst");
    status = download(buf);
    if(!DL_SUCCESS(status))
        return EXIT_FAILURE;
    if(use_fast)
    {
        sha256sum(buf, main_hash_new);
        if(strcmp(main_hash_old, main_hash_new) == 0)
            return EXIT_SUCCESS;
    }
    /* Optional files */
    sprintf(buf, "%s/%s", remotedir, "version.lst.lzma");
    download(buf);
    /* Usually, these files can be downloaded with version.lst */
    /* Uncomment lines below if something wrong */
    /*
    sprintf(buf, "%s/%s", remotedir, "drweb32.lst");
    download(buf);
    sprintf(buf, "%s/%s", remotedir, "drweb32.lst.lzma");
    download(buf);
    */
    sprintf(buf, "%s/%s", remotedir, "drweb32.flg");
    download(buf);
    sprintf(buf, "%s/%s", remotedir, "drweb32.flg.lzma");
    download(buf);

    /* Main file */
    sprintf(buf, "%s/%s", remotedir, "version.lst");
    fp = fopen(buf, "r");
    flag = 1;
    while(flag)
    {
        if(fscanf(fp, "%[^\r\n]\r\n", buf) == -1)
            flag = 0;
        else if(buf[0] == '+' || buf[0] == '=' || buf[0] == '!') /* Need to download this file */
        {
            char filename[STRBUFSIZE];
            char sha_base[65], sha_real[65];
            off_t filesize = -1;
            char * beg = buf + 1, * tmp;
            tmp = strchr(beg, '>'); /* if some as "=<w95>spider.vxd, ..." */
            if(tmp) beg = tmp + 1;
            tmp = strrchr(beg, '\\'); /* if some as "=<wnt>%SYSDIR%\spider.cpl, ..." */
            if(tmp) beg = tmp + 1;
            sprintf(filename, "%s/%s", remotedir, beg);
            * strchr(filename, ',') = '\0';
            tmp = strchr(filename, '|'); /* if some as "!drwreg.exe|-xi, ..." */
            if(tmp) * tmp = '\0';
            strncpy(sha_base, strchr(buf, ',') + 2, 64);
            sha_base[64] = '\0';
            tmp = strrchr(buf, ',');
            if(tmp)
                sscanf(tmp + 1, "%jd", & filesize);

            status = download_check(filename, sha_base, sha_real, & sha256sum, "SHA256");
            if(status == DL_TRY_AGAIN && counter_global < MAX_REPEAT) /* Try again */
            {
                counter_global++;
                fclose(fp);
                sleep(REPEAT_SLEEP);
                goto repeat5; /* Yes, it is goto. Sorry, Dijkstra... */
            }
            else if(!DL_SUCCESS(status))
            {
                fclose(fp);
                return EXIT_FAILURE;
            }
            if(filesize >= 0 && !check_size(filename, filesize)) /* Wrong size */
            {
                fclose(fp);
                if(counter_global >= MAX_REPEAT)
                    return EXIT_FAILURE;
                counter_global++;
                sleep(REPEAT_SLEEP);
                goto repeat5; /* Yes, it is goto. Sorry, Dijkstra... */
            }

            sprintf(buf, "%s.lzma", filename); /* Also get lzma file, if exist */
            if(status == DL_DOWNLOADED || exist(buf))
            {
                status = download_check(buf, sha_base, sha_real, & sha256sum_lzma, "SHA256 LZMA");
                if(status == DL_NOT_FOUND) /* Need for delete lzma file */
                {
                    if(exist(buf))
                    {
                        char * nm = strrchr(buf, '/') + 1;
                        memmove(buf, nm, (strlen(nm) + 1) * sizeof(char));
                        printf("Deleting... %s\n", buf);
                        delete_files(remotedir, buf);
                    }
                }
                else if(status == DL_TRY_AGAIN && counter_global < MAX_REPEAT) /* Try again */
                {
                    counter_global++;
                    fclose(fp);
                    sleep(REPEAT_SLEEP);
                    goto repeat5; /* Yes, it is goto. Sorry, Dijkstra... */
                }
                else if(!DL_SUCCESS(status))
                {
                    fclose(fp);
                    return EXIT_FAILURE;
                }
                else if(filesize >= 0 && !check_size_lzma(buf, filesize)) /* Wrong size */
                {
                    fclose(fp);
                    if(counter_global >= MAX_REPEAT)
                        return EXIT_FAILURE;
                    counter_global++;
                    sleep(REPEAT_SLEEP);
                    goto repeat5; /* Yes, it is goto. Sorry, Dijkstra... */
                }
            }
        }
        else if(buf[0] == '-') /* Need to delete this file */
        {
            char filename[STRBUFSIZE];
            sprintf(filename, "%s", buf + 1);
            * strchr(filename, ',') = '\0';
            delete_files(remotedir, filename);
            strcat(filename, ".lzma");
            delete_files(remotedir, filename);
        }
    }

    fclose(fp);
    return EXIT_SUCCESS;
}

/* Build caching tree for v7 */
void cache7(const char * file, const char * directory)
{
    char buf[STRBUFSIZE];
    FILE * fp = fopen(file, "r");
    int8_t flag = 1;
    if(!fp) return;
    while(flag)
    {
        if(fscanf(fp, "%[^\r\n]\r\n", buf) == -1)
            flag = 0;
        else if(strstr(buf, "<xml") != NULL || strstr(buf, "<lzma") != NULL) /* file description found */
        {
            char base_hash[65];
            char filename[STRBUFSIZE];
            strncpy(base_hash, strstr(buf, "hash=\"") + 6, 64);
            base_hash[64] = '\0';
            sprintf(filename, "%s/%s", directory, strstr(buf, "name=\"") + 6);
            * strchr(filename, '\"') = '\0';
            tree = avl_insert(tree, filename, base_hash);
        }
    }
    fclose(fp);
}

/* Update using version 7 of update protocol (xml file, sha256) */
int update7()
{
    char buf[STRBUFSIZE];
    FILE * fp;
    int8_t flag;
    int counter_global = 0;
    int status;

    if(make_path(remotedir) != EXIT_SUCCESS)
    {
        fprintf(ERRFP, "Error: Can't access to local directory\n");
        return EXIT_FAILURE;
    }
    if(do_lock(remotedir) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if(use_fast)
    {
        sprintf(buf, "%s/%s", remotedir, "versions.xml");
        cache7(buf, remotedir);
    }

repeat7: /* Goto here if hashsum mismatch */
    if(counter_global > 0 && use_fast) /* Incomplete update will lead to integrity violations */
    {
        use_fast = 0;
        fprintf(ERRFP, "Warning: Fast mode has been disabled\n");
    }

    /* Optional files (WTF???)*/
    /* Uncomment lines below if something wrong */
    /*
    sprintf(buf, "%s/%s", remotedir, "repodb.xml");
    download(buf);
    sprintf(buf, "%s/%s", remotedir, "revisions.xml");
    download(buf);
    */

    /* Get versions.xml */
    sprintf(buf, "%s/%s", remotedir, "versions.xml");
    status = download(buf);
    if(!DL_SUCCESS(status))
        return EXIT_FAILURE;

    /* Parse versions.xml */
    fp = fopen(buf, "r");
    flag = 1;
    while(flag)
    {
        if(fscanf(fp, "%[^\r\n]\r\n", buf) == -1)
            flag = 0;
        else if(strstr(buf, "<xml") != NULL || strstr(buf, "<lzma") != NULL) /* file description found */
        {
            char base_hash[65];
            char real_hash[65];
            char filename[STRBUFSIZE];
            int8_t is_xml = 0;
            off_t filesize = -1;
            char * tmpchr;

            if(strstr(buf, "<xml") != NULL) is_xml = 1;

            strncpy(base_hash, strstr(buf, "hash=\"") + 6, 64);
            base_hash[64] = '\0';
            sprintf(filename, "%s/%s", remotedir, strstr(buf, "name=\"") + 6);
            * strchr(filename, '\"') = '\0';
            if((tmpchr = strstr(buf, "size=\"")) != NULL)
                sscanf(tmpchr + 6, "%jd\"", & filesize);

            if(!exist(filename) && make_path_for(filename) != EXIT_SUCCESS) /* If file not exist, check directories and make it if need */
            {
                fprintf(ERRFP, "Error: Can't access to local directory\n");
                fclose(fp);
                return EXIT_FAILURE;
            }
            else if(tree && counter_global == 0 && is_xml)
            {
                char directory[STRBUFSIZE];
                strncpy(directory, filename, sizeof(directory) - 1);
                * strrchr(directory, '/') = '\0';
                cache7(filename, directory);
            }

            status = download_check(filename, base_hash, real_hash, & sha256sum, "SHA256");
            if(status == DL_TRY_AGAIN && counter_global < MAX_REPEAT) /* Try again */
            {
                counter_global++;
                fclose(fp);
                sleep(REPEAT_SLEEP);
                goto repeat7; /* Yes, it is goto. Sorry, Dijkstra... */
            }
            else if(!DL_SUCCESS(status))
            {
                fclose(fp);
                return EXIT_FAILURE;
            }
            if(filesize >= 0 && !check_size(filename, filesize)) /* Wrong size */
            {
                fclose(fp);
                if(counter_global >= MAX_REPEAT)
                    return EXIT_FAILURE;
                counter_global++;
                sleep(REPEAT_SLEEP);
                goto repeat7; /* Yes, it is goto. Sorry, Dijkstra... */
            }

            if(is_xml) /* Parse this xml file */
            {
                char directory[STRBUFSIZE];
                FILE * xfp = fopen(filename, "r");
                int8_t xflag = 1;
                char * pp = strrchr(filename, '/');
                * pp = '\0';
                strncpy(directory, filename, sizeof(directory) - 1);
                * pp = '/';

                while(xflag)
                {
                    if(fscanf(xfp, "%[^\r\n]\r\n", buf) == -1)
                        xflag = 0;
                    else if(strstr(buf, "<lzma") != NULL) /* lzma file description found */
                    {
                        char xfilename[STRBUFSIZE];
                        int status;
                        off_t xfilesize = -1;

                        strncpy(base_hash, strstr(buf, "hash=\"") + 6, 64);
                        base_hash[64] = '\0';
                        sprintf(xfilename, "%s/%s", directory, strstr(buf, "name=\"") + 6);
                        * strchr(xfilename, '\"') = '\0';
                        if((tmpchr = strstr(buf, "size=\"")) != NULL)
                            sscanf(tmpchr + 6, "%jd\"", & xfilesize);

                        if(!exist(xfilename) && make_path_for(xfilename) != EXIT_SUCCESS) /* If file not exist, check directories and make it if need */
                        {
                            fprintf(ERRFP, "Error: Can't access to local directory\n");
                            fclose(fp);
                            fclose(xfp);
                            return EXIT_FAILURE;
                        }

                        status = download_check(xfilename, base_hash, real_hash, & sha256sum, "SHA256");
                        if(status == DL_TRY_AGAIN && counter_global < MAX_REPEAT) /* Try again */
                        {
                            counter_global++;
                            fclose(fp);
                            fclose(xfp);
                            sleep(REPEAT_SLEEP);
                            goto repeat7; /* Yes, it is goto. Sorry, Dijkstra... */
                        }
                        else if(!DL_SUCCESS(status))
                        {
                            fclose(fp);
                            fclose(xfp);
                            return EXIT_FAILURE;
                        }
                        if(xfilesize >= 0 && !check_size(xfilename, xfilesize)) /* Wrong size */
                        {
                            fclose(fp);
                            fclose(xfp);
                            if(counter_global >= MAX_REPEAT)
                                return EXIT_FAILURE;
                            counter_global++;
                            sleep(REPEAT_SLEEP);
                            goto repeat7; /* Yes, it is goto. Sorry, Dijkstra... */
                        }
                    }
                }
                fclose(xfp);
            }
        }
    }

    fclose(fp);
    return EXIT_SUCCESS;
}

/* Build caching tree for Android */
void cacheA(const char * directory)
{
    char buf[STRBUFSIZE];
    FILE * fp;
    int8_t flag = 1, flag_files = 0;
    fp = fopen(remotedir, "r");
    if(!fp) return;
    while(flag)
    {
        if(fscanf(fp, "%[^\n]\n", buf) == -1)
            flag = 0;
        else if(flag_files)
        {
            if(buf[0] == '[' || strlen(buf) < 84)
                flag = 0;
            else
            {
                char garb[9], md5_base[33];
                char filename_base[STRBUFSIZE], filename[STRBUFSIZE];
                sscanf(buf, "%[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %s",
                       garb, garb, garb, md5_base, garb, garb, filename_base);
                sprintf(filename, "%s/%s", directory, filename_base);
                to_lowercase(md5_base);
                tree = avl_insert(tree, filename, md5_base);
            }
        }
        else
        {
            if(strncmp(buf, "[Files]", strlen("[Files]")) == 0)
                flag_files = 1;
        }
    }
    fclose(fp);
}

/* Update using Android update protocol */
int updateA()
{
    char buf[STRBUFSIZE], real_dir[STRBUFSIZE];
    FILE * fp;
    int8_t flag, flag_files;
    int counter_global = 0;
    int status;

    strncpy(real_dir, remotedir, sizeof(real_dir) - 1);
    real_dir[sizeof(real_dir) - 1] = '\0';
    * (strrchr(real_dir, '/')) = '\0';
    if(make_path(real_dir) != EXIT_SUCCESS) /* Make all needed directory */
    {
        fprintf(ERRFP, "Error: Can't access to local directory\n");
        return EXIT_FAILURE;
    }
    if(do_lock(real_dir) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if(use_fast)
        cacheA(real_dir);

repeatA: /* Goto here if checksum mismatch */
    if(counter_global > 0 && use_fast) /* Incomplete update will lead to integrity violations */
    {
        use_fast = 0;
        fprintf(ERRFP, "Warning: Fast mode has been disabled\n");
    }

    status = download(remotedir);
    if(!DL_SUCCESS(status))
        return EXIT_FAILURE;
    fp = fopen(remotedir, "r");
    flag = 1;
    flag_files = 0;
    while(flag)
    {
        if(fscanf(fp, "%[^\n]\n", buf) == -1)
            flag = 0;
        else if(flag_files)
        {
            if(buf[0] == '[' || strlen(buf) < 84)
                flag = 0;
            else
            {
                char garb[9], md5_base[33], md5_real[33];
                char filename_base[STRBUFSIZE], filename[STRBUFSIZE];
                int status;
                off_t filesize = -1;
                uintmax_t filesize_tmp;

                sscanf(buf, "%[^,], %[^,], %jX, %[^,], %[^,], %[^,], %s",
                       garb, garb, & filesize_tmp, md5_base, garb, garb, filename_base);
                sprintf(filename, "%s/%s", real_dir, filename_base);
                to_lowercase(md5_base);
                filesize = (off_t)filesize_tmp;

                status = download_check(filename, md5_base, md5_real, & md5sum, "MD5");
                if(status == DL_TRY_AGAIN && counter_global < MAX_REPEAT) /* Try again */
                {
                    counter_global++;
                    fclose(fp);
                    sleep(REPEAT_SLEEP);
                    goto repeatA; /* Yes, it is goto. Sorry, Dijkstra... */
                }
                else if(!DL_SUCCESS(status))
                {
                    fclose(fp);
                    return EXIT_FAILURE;
                }
                if(filesize >= 0 && !check_size(filename, filesize)) /* Wrong size */
                {
                    fclose(fp);
                    if(counter_global >= MAX_REPEAT)
                        return EXIT_FAILURE;
                    counter_global++;
                    sleep(REPEAT_SLEEP);
                    goto repeatA; /* Yes, it is goto. Sorry, Dijkstra... */
                }
            }
        }
        else
        {
            if(strncmp(buf, "[Files]", strlen("[Files]")) == 0)
                flag_files = 1;
        }
    }

    fclose(fp);
    return EXIT_SUCCESS;
}
