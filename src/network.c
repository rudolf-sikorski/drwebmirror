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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

extern char * strptime(const char * buf, const char * format, struct tm * tm);
extern const char * hstrerror(int err);

/* Server name */
char servername[256];
/* Server port */
uint16_t serverport;
/* Remote directory */
char remotedir[256];
/* User Agent */
char useragent[256];
/* SysHash */
char syshash[33];
int8_t use_syshash;
/* Android */
int8_t use_android;

/* Open connection */
int conn_open(int * sock_fd, const char * server, uint16_t port)
{
    struct sockaddr_in sock_addr;
    struct hostent * host_info = gethostbyname(server);
    struct timeval tv;
    fd_set fdset;
    int sock_opts;

    if(host_info == NULL)
    {
        fprintf(ERRFP, "Error %d with gethostbyname(): %s\n", h_errno, hstrerror(h_errno));
        return EXIT_FAILURE;
    }

    * sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(* sock_fd == -1)
    {
        fprintf(ERRFP, "Error %d with socket(): %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    /* Change to non-blocking mode */
    sock_opts = fcntl(* sock_fd, F_GETFL);
    sock_opts |= O_NONBLOCK;
    fcntl(* sock_fd, F_SETFL, sock_opts);

    memset(& sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = host_info->h_addrtype;
    sock_addr.sin_port = htons(port);
    memcpy(& sock_addr.sin_addr.s_addr, host_info->h_addr_list[0], host_info->h_length);

    if(connect(* sock_fd, (const struct sockaddr *)(& sock_addr), sizeof(sock_addr)) != 0 && errno != EINPROGRESS)
    {
        fprintf(ERRFP, "Error %d with connect(): %s\n", errno, strerror(errno));
        close(* sock_fd);
        return EXIT_FAILURE;
    }

    /* Set timeout value */
    memset(& tv, 0, sizeof(struct timeval));
    tv.tv_sec = TIMEOUT;
    FD_ZERO(& fdset);
    FD_SET(* sock_fd, & fdset);

    if(select((* sock_fd) + 1, NULL, & fdset, NULL, & tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof so_error;
        getsockopt(* sock_fd, SOL_SOCKET, SO_ERROR, & so_error, & len);

        if(so_error != 0)
        {
            fprintf(ERRFP, "Error %d with select(): %s\n", so_error, strerror(so_error));
            close(* sock_fd);
            return EXIT_FAILURE;
        }
    }
    else
    {
        fprintf(ERRFP, "Error with select(): Connection timeout\n");
        close(* sock_fd);
        return EXIT_FAILURE;
    }

    /* Change to blocking mode */
    sock_opts = fcntl(* sock_fd, F_GETFL);
    sock_opts ^= O_NONBLOCK;
    fcntl(* sock_fd, F_SETFL, sock_opts);

    /* Set recv timeout value */
    setsockopt(* sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)(& tv), sizeof(struct timeval));
    /* Set send timeout value */
    setsockopt(* sock_fd, SOL_SOCKET, SO_SNDTIMEO, (const void *)(& tv), sizeof(struct timeval));

    return EXIT_SUCCESS;
}

/* Close connection */
void conn_close(int * sock_fd)
{
    shutdown(* sock_fd, SHUT_RDWR);
    close(* sock_fd);
}

/* Get file <filename> from server */
int conn_get(const char * filename)
{
    int sock_fd;

    char * buffer, * bufpos, * bufend;
    unsigned long msgsize = 0;
    int status;
    int8_t msgbegin;
    unsigned long msgcurr = 0;
    size_t redirect_num = 0;

    time_t lastmod = 0;
    FILE * fp;
    const char request_fmt[] = "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Encoding: identity\r\nAccept-Ranges: bytes\r\nHost: %s\r\nX-DrWeb-Validate: %s\r\nX-DrWeb-KeyNumber: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n\r\n";
    const char request_fmt_syshash[] = "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Encoding: identity\r\nAccept-Ranges: bytes\r\nHost: %s\r\nX-DrWeb-Validate: %s\r\nX-DrWeb-KeyNumber: %s\r\nX-DrWeb-SysHash: %s\r\nUser-Agent: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n\r\n";
    const char request_fmt_android[] = "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Encoding: identity\r\nAccept-Ranges: bytes\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\n\r\n";
    const char request_fmt_android_syshash[] = "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Encoding: identity\r\nAccept-Ranges: bytes\r\nHost: %s\r\nUser-Agent: %s\r\nX-DrWeb-SysHash: %s\r\nConnection: close\r\n\r\n";
    char * request;
    char filename_dl[STRBUFSIZE];
    char servername_dl[256];
    uint16_t serverport_dl = serverport;

    buffer = (char *)malloc(NETBUFSIZE);

    strncpy(filename_dl, filename, sizeof(filename_dl) - 1);
    strncpy(servername_dl, servername, sizeof(servername_dl) - 1);

    printf("Downloading %s\n", filename);

redirect: /* Goto here if 30x received */
    status = -1;
    msgbegin = 0;
    bufpos = bufend = buffer;

    if(conn_open(& sock_fd, servername_dl, serverport_dl) != EXIT_SUCCESS) /* Open connection */
        return EXIT_FAILURE;

    if(use_android == 0)
    {
        if(use_syshash == 0)
        {
            request = (char *)calloc(strlen(request_fmt) + strlen(filename_dl) +
                                     strlen(servername_dl) + strlen(key_md5sum) +
                                     strlen(key_userid) + strlen(useragent), sizeof(char));
            sprintf(request, request_fmt, filename_dl, servername_dl, key_md5sum, key_userid, useragent);
        }
        else
        {
            request = (char *)calloc(strlen(request_fmt_syshash) + strlen(filename_dl) +
                                     strlen(servername_dl) + strlen(key_md5sum) +
                                     strlen(key_userid) + strlen(useragent) +
                                     strlen(syshash), sizeof(char));
            sprintf(request, request_fmt_syshash, filename_dl, servername_dl, key_md5sum, key_userid, syshash, useragent);
        }
    }
    else
    {
        if(use_syshash == 0)
        {
            request = (char *)calloc(strlen(request_fmt) + strlen(filename_dl) +
                                     strlen(servername_dl) + strlen(useragent), sizeof(char));
            sprintf(request, request_fmt_android, filename_dl, servername_dl, useragent);
        }
        else
        {
            request = (char *)calloc(strlen(request_fmt_syshash) + strlen(filename_dl) +
                                     strlen(servername_dl) + strlen(key_md5sum) +
                                     strlen(key_userid) + strlen(useragent) +
                                     strlen(syshash), sizeof(char));
            sprintf(request, request_fmt_android_syshash, filename_dl, servername_dl, useragent, syshash);
        }
    }

    if(more_verbose)
    {
        printf("\n");
        size_t i;
        for(i = 0; i < strlen(request); i++)
            if(request[i] != '\r')
                printf("%c", request[i]);
    }

    if(send(sock_fd, request, strlen(request), 0) < 0) /* Send request */
    {
        fprintf(ERRFP, "Error %d with send(): %s\n", errno, strerror(errno));
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
        free(request);
        free(buffer);
        return EXIT_FAILURE;
    }
    free(request);

    buffer[0] = '\0';
    while(!msgbegin) /* Parse header of response */
    {
        char * tmp;
        char field_name[255];
        char field_content[STRBUFSIZE];
        size_t old_buf = (size_t)bufend - (size_t)buffer;
        ssize_t recv_count;

        bufpos = bufend;
        recv_count = recv(sock_fd, bufpos, NETBUFSIZE - old_buf, 0);
        bufend = bufpos + recv_count;
        if(recv_count <= 0)
        {
            fprintf(ERRFP, "Error %d with recv(): %s\n", errno, strerror(errno));
            shutdown(sock_fd, SHUT_RDWR);
            close(sock_fd);
            free(buffer);
            return EXIT_FAILURE;
        }

        if(more_verbose)
        {
            char * smth;
            for(smth = bufpos; smth < bufend; smth++)
                if(* smth != '\r')
                    printf("%c", * smth);
                else if(smth + 2 < bufend && * (smth + 2) == '\r')
                {
                    printf("\n\n");
                    break;
                }
        }

        if(status < 0)
        {
            bufpos = strchr(buffer, ' ');
            if(bufpos != NULL)
            {
                bufpos++;
                sscanf(bufpos, "%d", & status);
                bufpos = strchr(bufpos, '\n');
                if(bufpos != NULL)
                    bufpos++;
                else
                    status = EXIT_FAILURE;
            }
            else
                status = EXIT_FAILURE;
            if(status == EXIT_FAILURE)
            {
                fprintf(ERRFP, "Error with recv(): Can't parse response\n");
                shutdown(sock_fd, SHUT_RDWR);
                close(sock_fd);
                free(buffer);
                return status;
            }

            /* Redirect */
            /* Warning: 300 work only if server set Location field */
            if(status >= 300 && status <= 303 && redirect_num < MAX_REDIRECT)
            {
                shutdown(sock_fd, SHUT_RDWR);
                close(sock_fd);
                redirect_num++;
                while((tmp = strchr(bufpos, '\r')) != NULL && tmp - 2 != strstr(bufpos - 2, "\r\n\r\n"))
                {
                    sscanf(bufpos, "%[^:]", field_name);
                    bufpos += strlen(field_name) + 2;
                    sscanf(bufpos, "%[^\r]", field_content);
                    bufpos += strlen(field_content) + 2;
                    if(strcmp(field_name, "Location") == 0) /* Parse new location */
                    {
                        char * servername_beg, * serverport_beg, * filename_beg;
                        servername_beg = strstr(field_content, "://");
                        if(servername_beg)
                        {
                            servername_beg += 3;
                            serverport_beg = strchr(servername_beg, ':');
                            filename_beg = strchr(servername_beg, '/');

                            if(* (filename_beg + 1) != '\0')
                                strncpy(filename_dl, filename_beg + 1, sizeof(filename_dl) - 1);
                            else
                                strcpy(filename_dl, "/");
                            * filename_beg = '\0';
                            if(serverport_beg && serverport_beg < filename_beg) /* Non-default port */
                            {
                                serverport_dl = atoi(serverport_beg + 1);
                                * serverport_beg = '\0';
                            }
                            else
                                serverport_dl = 80;
                            strncpy(servername_dl, servername_beg, sizeof(servername_dl) - 1);
                        }
                    }
                }
                if(verbose)
                    printf("Redirected (%d) to http://%s:%u/%s\n", status, servername_dl, (unsigned)serverport_dl, filename_dl);
                goto redirect;
            }

            /*
            Message in DrWebUpW:
            Your license key file has not been found in the database! Please contact technical support: http://support.drweb.com.
            */
            if(status == 451)
                fprintf(ERRFP, "Error: License key file has not been found in the database.\n");

            /*
            Message in DrWebUpW:
            License key file is blocked!
            */
            if(status == 452)
                fprintf(ERRFP, "Error: License key file is blocked or incorrect UserID/MD5.\n");

            /*
            Message in DrWebUpW:
            You are using an unregistered version of Dr.Web. To receive updates, please register.
            */
            if(status == 600)
                fprintf(ERRFP, "Error: License key file is key from an unregistered version.\n");

            /* All good */
            if(status != 200)
            {
                shutdown(sock_fd, SHUT_RDWR);
                close(sock_fd);
                free(buffer);
                return status;
            }
        }

        while((tmp = strchr(bufpos, '\r')) != NULL && tmp - 2 != strstr(bufpos - 2, "\r\n\r\n"))
        {
            sscanf(bufpos, "%[^:]", field_name);
            bufpos += strlen(field_name) + 2;
            sscanf(bufpos, "%[^\r]", field_content);
            bufpos += strlen(field_content) + 2;

            if(strcmp(field_name, "Content-Length") == 0)
            {
                sscanf(field_content, "%lu", & msgsize);
            }
            else if(strcmp(field_name, "Last-Modified") == 0) /* Only RFC 822, sorry */
            {
                char * tmz = strrchr(field_content, ' ') + 1;
                long tzshift_loc = 0;
                struct tm raw_time;
                memset(&raw_time, 0, sizeof(struct tm));

                if(strcmp(tmz, "GMT") != 0)
                    sscanf(tmz, "%ld", & tzshift_loc);
                tzshift_loc *= 60 * 60 / 100;

                strptime(field_content, "%a, %e %b %Y %H:%M:%S %z", & raw_time);

                /* Fixes buggy strptime */
                raw_time.tm_wday = -1;
                raw_time.tm_yday = -1;
                raw_time.tm_isdst = -1;

                lastmod = mktime(& raw_time);
                if(lastmod > 0)
                    lastmod -= tzshift_loc - tzshift;
            }
        }

        if(tmp != NULL && tmp - 2 == strstr(bufpos - 2, "\r\n\r\n"))
        {
            msgbegin = 1;
            if((ssize_t)(bufend - buffer) - (ssize_t)(tmp + 2 - buffer) > 0)
                bufpos = tmp + 2;
            else
            {
                bufpos = buffer;
                bufend = buffer;
            }
        }
        else
        {
            memmove(buffer, bufpos, (size_t)(bufend - bufpos));
            bufend -= (bufpos - buffer);
            bufpos = buffer;
        }
    }

    fp = fopen(filename, "wb"); /* Open result file */
    if(!fp)
    {
        fprintf(ERRFP, "Error with fopen() on %s\n", filename);
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
        free(buffer);
        return EXIT_FAILURE;
    }

    if(bufpos != bufend) /* Write content */
    {
        msgcurr = bufend - bufpos;
        if(msgcurr > msgsize)
            msgcurr = msgsize;
        fwrite(bufpos, sizeof(char), msgcurr, fp);
    }

    memset(buffer, 0, sizeof(char) * NETBUFSIZE);
    while(msgcurr < msgsize)
    {
        ssize_t recv_count = recv(sock_fd, buffer, NETBUFSIZE, 0);
        if(recv_count <= 0)
        {
            fprintf(ERRFP, "Error %d with recv(): %s\n", errno, strerror(errno));
            shutdown(sock_fd, SHUT_RDWR);
            close(sock_fd);
            free(buffer);
            return EXIT_FAILURE;
        }

        fwrite(buffer, sizeof(char), recv_count, fp);
        msgcurr += recv_count;
    }

    conn_close(&sock_fd); /* Close connection */
    fclose(fp);
    free(buffer);

    if(lastmod && set_mtime(filename, lastmod) != EXIT_SUCCESS) /* Set last modification time */
        return EXIT_FAILURE;
    chmod(filename, MODE_FILE); /* Change access permissions */

    return EXIT_SUCCESS;
}

/* Download file <filename> */
int download(const char * filename)
{
    int counter = 0, status;
    do
    {
        status = conn_get(filename);
        counter++;
    }
    while(counter < MAX_REPEAT && status != EXIT_SUCCESS && status != 404);
    if(status == 404) /* Not found */
        return DL_NOT_FOUND;
    if(counter >= MAX_REPEAT)
    {
        if(status != EXIT_FAILURE)
            fprintf(ERRFP, "Error: Server response %d\n", status);
        return DL_FAILED;
    }
    return DL_DOWNLOADED;
}

/* Download file <filename> and compare checksum <checksum_base>
 * with <checksum_real> using <checksum_func> function */
int download_check(const char * filename, const char * checksum_base, char * checksum_real,
                   int (* checksum_func)(const char *, char *), const char * checksum_desc)
{
    if(tree && exist(filename)) /* Using fast check */
    {
        const char * checksum_tree = avl_hash(tree, filename);
        if(checksum_tree)
        {
            strcpy(checksum_real, checksum_tree);
            if(verbose)
                printf("%s exist, fast checking %s ", filename, checksum_desc);
            if(strcmp(checksum_base, checksum_real) == 0)
            {
                if(verbose)
                    printf("[OK]\n");
                return DL_EXIST;
            }
            else if(verbose)
                printf("[NOT OK]\n");
        }
    }

    int status = checksum_func(filename, checksum_real);
    if(status == EXIT_SUCCESS) /* File exist */
    {
        if(verbose)
            printf("%s exist, checking %s ", filename, checksum_desc);
        if(strcmp(checksum_base, checksum_real) != 0) /* Sum mismatched */
        {
            status = EXIT_FAILURE;
            if(verbose)
                printf("[NOT OK]\n");
        }
        else
        {
            if(verbose)
                printf("[OK]\n");
            return DL_EXIST;
        }
    }

    status = download(filename);
    if(status != DL_DOWNLOADED)
        return status;
    if(checksum_func(filename, checksum_real) != EXIT_SUCCESS)
        return DL_FAILED;

    if(verbose)
        printf("%s downloaded, checking %s ", filename, checksum_desc);
    if(strcmp(checksum_base, checksum_real) != 0)
    {
        if(verbose)
            printf("[NOT OK]\n");
        fprintf(ERRFP, "Warning: %s mismatch (real=\"%s\", base=\"%s\")\n", checksum_desc, checksum_real, checksum_base);
        return DL_TRY_AGAIN;
    }
    else if(verbose)
        printf("[OK]\n");
    return DL_DOWNLOADED;
}
