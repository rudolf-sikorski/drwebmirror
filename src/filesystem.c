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
#include <sys/stat.h>
#include <fcntl.h>
#if !defined (NO_POSIX_API)
#include <utime.h>
#include <dirent.h>
#else
#include <sys/utime.h>
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
#endif

/* Lock file descriptor */
int lockfd;
/* Lokfile name */
char lockfile[384];

/* Set modification time <mtime> to file <filename> */
int set_mtime(const char * filename, const time_t mtime)
{
    struct stat f_stat;
    struct utimbuf new_times;

    if(stat(filename, & f_stat) < 0)
    {
        fprintf(ERRFP, "Error %d with stat() on %s: %s\n", errno, filename, strerror(errno));
        return EXIT_FAILURE;
    }

    new_times.actime = f_stat.st_atime;
    new_times.modtime = mtime;

    if(utime(filename, & new_times) < 0)
    {
        fprintf(ERRFP, "Error %d with utime() on %s: %s\n", errno, filename, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* Make directory <name> */
int make_dir(const char * name)
{
    struct stat st;
    if(stat(name, & st) != 0) /* Don't exist */
    {
#if defined(_WIN32)
        if(mkdir(name) != 0)
#else
        if(mkdir(name, MODE_DIR) != 0)
#endif
        {
            fprintf(ERRFP, "Error %d with mkdir(): %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
    }
    else if(!S_ISDIR(st.st_mode)) /* Not directory */
    {
        errno = ENOTDIR;
        fprintf(ERRFP, "Error %d with mkdir(): %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    chmod(name, MODE_DIR); /* Change access permissions */
    return EXIT_SUCCESS;
}

/* Recursive make directory <path> */
int make_path(const char * path)
{
    int8_t flag = 1;
    int status = EXIT_SUCCESS;
    char tmppath[STRBUFSIZE];
    char * curr = tmppath;
    bsd_strlcpy(tmppath, path, sizeof(tmppath));

    while(flag)
    {
        curr = strchr(curr, '/');
        if(curr)
        {
            * curr = '\0';
            status = make_dir(tmppath);
            * curr = '/';
            curr = curr + 1;
        }
        else if(status == EXIT_SUCCESS)
        {
            status = make_dir(tmppath);
            flag = 0;
        }
    }
    return status;
}

/* Recursive make directoty for file <filename> */
int make_path_for(char * filename)
{
    char * pp = strrchr(filename, '/');
    int status = EXIT_SUCCESS;
    if(pp)
    {
        * pp = '\0';
        status = make_path(filename);
        * pp = '/';
    }
    return status;
}

/* Delete files by mask <mask> in directory <directory> */
int delete_files(const char * directory, const char * mask)
{
#if !defined (NO_POSIX_API)
    DIR * dfd = opendir(directory);
    struct dirent * dp;

    if(dfd == NULL)
    {
        fprintf(ERRFP, "Error: No such directory %s\n", directory);
        return EXIT_FAILURE;
    }

    while((dp = readdir(dfd)) != NULL)
    {
        const char * curr_name = dp->d_name;
        const char * curr_mask = mask;
        int8_t flag = 1;
        while(flag)
        {
            if(* curr_mask == '*')
            {
                while(* curr_mask == '*')
                    curr_mask++;
                if(* curr_mask == '\0')
                    flag = 0;
                else
                {
                    while(* curr_name != '\0' && * curr_name != * curr_mask)
                        curr_name++;
                    if(* curr_name == '\0')
                        break;
                    else
                    {
                        curr_name++;
                        curr_mask++;
                        if(* curr_name == '\0' && * curr_mask == '\0')
                            flag = 0;
                    }
                }
            }
            else if(* curr_mask == '?' || * curr_name == * curr_mask)
            {
                curr_name++;
                curr_mask++;
                if(* curr_name == '\0' && * curr_mask == '\0')
                    flag = 0;
            }
            else
                break;
        }
        if(!flag)
        {
            char buf[STRBUFSIZE];
            sprintf(buf, "%s/%s", directory, dp->d_name);
            if(remove(buf) != 0)
                fprintf(ERRFP, "Error: Can't delete file %s/%s\n", directory, dp->d_name);
        }
    }

    closedir(dfd);
    return EXIT_SUCCESS;
#else
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA ffd;
    char szDir[MAX_PATH];
    DWORD dwError = 0;
    size_t dir_len = bsd_strlcpy(szDir, directory, MAX_PATH);
    char * curr = szDir + dir_len++;
    *(curr++) = '/';
    bsd_strlcpy(curr, mask, MAX_PATH - dir_len - 1);
    hFind = FindFirstFileA(szDir, &ffd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                char buf[STRBUFSIZE];
                sprintf(buf, "%s/%s", directory, ffd.cFileName);
                if(remove(buf) != 0)
                    fprintf(ERRFP, "Error: Can't delete file %s/%s\n", directory, ffd.cFileName);
            }
        }
        while(FindNextFileA(hFind, &ffd) != 0);
    }
    dwError = GetLastError();
    if(dwError != ERROR_FILE_NOT_FOUND && dwError != ERROR_NO_MORE_FILES)
    {
        char * lpMsgBuf;
        FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dwError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPSTR) &lpMsgBuf,
                    0, NULL );
        fprintf(ERRFP, "Error: Can't delete file %s/%s (%s)\n", directory, mask, lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    FindClose(hFind);
    return EXIT_SUCCESS;
#endif
}

/* Check <filename> exist */
int exist(const char * filename)
{
    struct stat st;
    if(stat(filename, & st) != 0)
        return 0;
    return 1;
}

/* Get <filename> size */
off_t get_size(const char * filename)
{
    struct stat st;
    if(stat(filename, & st) != 0)
    {
        fprintf(ERRFP, "Error %d with stat(): %s\n", errno, strerror(errno));
        return -1;
    }
    return st.st_size;
}

/* Compare size of <filename> with <filesize> */
int check_size(const char * filename, off_t filesize)
{
    if(verbose)
        printf("Checking size of %s ", filename);
    if(filesize == get_size(filename))
    {
        if(verbose)
            printf("[OK]\n");
        return 1;
    }
    else if(verbose)
        printf("[NOT OK]\n");
    return 0;
}

/* Open temp file */
FILE * fopen_temp(char * filename)
{
    FILE * tmpf = tmpfile();
    if(tmpf == NULL && filename != NULL) /* In some strange cases, tmpfile() does not work */
    {
        tmpnam(filename);
        tmpf = fopen(filename, "wb+");

#if defined(__CYGWIN__) || defined(_WIN32)
        if(tmpf == NULL) /* In some strange cases with cygwin, tmpnam() return broken path */
        {
            char sb[L_tmpnam];
            char * nm = strrchr(filename, '/');
            if(nm != NULL)
                bsd_strlcpy(sb, nm, sizeof(sb));
            else
                bsd_strlcpy(sb, filename, sizeof(sb));
            sprintf(filename, "%s%s", getenv("TEMP"), sb);
            tmpf = fopen(filename, "wb+");

            if(tmpf == NULL) /* Hmm, maybe TMP instead of TEMP will work? */
            {
                if(nm != NULL)
                    bsd_strlcpy(sb, nm, sizeof(sb));
                else
                    bsd_strlcpy(sb, filename, sizeof(sb));
                sprintf(filename, "%s%s", getenv("TMP"), sb);
                tmpf = fopen(filename, "wb+");

                if(tmpf == NULL) /* So sad... */
                {
                    fprintf(ERRFP, "Error: Can't create temporary file.\n");
                    return NULL;
                }
            }
        }
#else
        if(tmpf == NULL) /* So sad... */
        {
            fprintf(ERRFP, "Error: Can't create temporary file.\n");
            return NULL;
        }
#endif
    }
    else if(filename != NULL)
        filename[0] = '\0';
    return tmpf;
}

/* Lock file */
int do_lock(const char * directory)
{
    size_t dir_len = bsd_strlcpy(lockfile, directory, sizeof(lockfile));
    char * lockfile_curr = lockfile + dir_len;
#if !defined(__CYGWIN__) && !defined(_WIN32)
    struct flock fl;
    memset(& fl, 0, sizeof(struct flock));
    fl.l_whence = SEEK_SET;
    fl.l_type = F_WRLCK | F_RDLCK;
#endif
    if(dir_len > sizeof(lockfile))
    {
        lockfile_curr -= strlen(LOCKFILENAME) + 2;
#if defined(_WIN32)
        while(((* lockfile_curr) != '/' || (* lockfile_curr) != '\\') &&
#else
        while((* lockfile_curr) != '/' &&
#endif
              lockfile_curr != lockfile)
            lockfile_curr--;
    }
    *(lockfile_curr++) = '/';
    strcpy(lockfile_curr, LOCKFILENAME);
    lockfd = 0;

    /* Open */
    if(verbose) printf("Opening lock file\n");
    if(exist(lockfile) || (lockfd = open(lockfile, O_RDWR | O_CREAT | O_EXCL, MODE_LOCKFILE)) < 0)
    {
        if(lockfd < 0)
            fprintf(ERRFP, "Warning: Error %d with open(): %s\n", errno, strerror(errno));
        else
            fprintf(ERRFP, "Warning: Lock file already exists\n");
        if(use_fast)
        {
            use_fast = 0;
            fprintf(ERRFP, "Warning: Fast mode has been disabled\n");
        }
        if((lockfd = open(lockfile, O_RDWR | O_CREAT, MODE_LOCKFILE)) < 0)
        {
            fprintf(ERRFP, "Error: Error %d with open(): %s\n", errno, strerror(errno));
            return EXIT_FAILURE;
        }
    }

/* Cygwin implementation of fcntl() can't work */
#if !defined(__CYGWIN__) && !defined(_WIN32)
    /* Lock */
    if(verbose) printf("Locking lock file\n");
    if(fcntl(lockfd, F_SETLK, & fl) < 0)
    {
        if(errno == EAGAIN || errno == EACCES)
        {
            fprintf(ERRFP, "Error: Error %d with fcntl(): %s\n", errno, strerror(errno));
            close(lockfd);
            return EXIT_FAILURE;
        }
        else
        {
            fprintf(ERRFP, "Warning: Error %d with fcntl(): %s\n", errno, strerror(errno));
            close(lockfd);
            lockfd = -1;
        }
    }
#endif

    /* All OK */
    return EXIT_SUCCESS;
}

/* Unlock file */
int do_unlock()
{
#if !defined(__CYGWIN__) && !defined(_WIN32)
    struct flock fl;
    memset(& fl, 0, sizeof(struct flock));
    fl.l_whence = SEEK_SET;
    fl.l_type = F_UNLCK;
#endif

    if(lockfd < 0)
        return EXIT_SUCCESS;

/* Cygwin implementation of fcntl() can't work */
#if !defined(__CYGWIN__) && !defined(_WIN32)
    /* Unlock */
    if(verbose) printf("Unlocking lock file\n");
    if(fcntl(lockfd, F_SETLK, & fl) < 0)
    {
        fprintf(ERRFP, "Error: Error %d with fcntl(): %s\n", errno, strerror(errno));
        close(lockfd);
        return EXIT_FAILURE;
    }
#endif

    /* All OK */
    close(lockfd);
    return EXIT_SUCCESS;
}
