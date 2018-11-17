/*
   Copyright (C) 2014-2018, Rudolf Sikorski <rudolf.sikorski@freenet.de>

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
#if !defined(_WIN32)
#include <signal.h>
#endif

/* Commandline option types */
typedef enum
{
    OPT_KEYFILE,
    OPT_USER,
    OPT_MD5,
    OPT_SYSHASH,
    OPT_AGENT,
    OPT_SERVER,
    OPT_SERVER_FB,
    OPT_HTTP_USER,
    OPT_HTTP_PASS,
    OPT_HTTP_VER,
    OPT_PORT,
    OPT_PROTO,
    OPT_REMOTE,
    OPT_LOCAL,
    OPT_PROXY,
    OPT_PROXY_USER,
    OPT_PROXY_PASS,
    OPT_FAST,
    OPT_VERBOSE,
    OPT_MORE_VERBOSE,
    OPT_HELP
} option;

/* Flag of use verbose output */
int8_t verbose;
/* Flag of use even more verbose output */
int8_t more_verbose;

/* Show help message */
void show_help()
{
    printf("DrWebMirror " PROG_VERSION ", tool for creating a mirror of DrWeb update server.\n"
           "Homepage: https://fami.codefreak.ru/osp/drwebmirror\n\n"
           "Usage: drwebmirror <options>\n"
           "\n"
           "Options:\n"
           "  -k,  --keyfile=FILE              set key file\n"
           "  -u,  --user=NUMBER               set UserID number from key file\n"
           "  -m,  --md5=STRING                set MD5 sum of key file\n"
           "  -H,  --syshash=STRING            set X-DrWeb-SysHash header\n"
           "  -a,  --agent=STRING              set custom User Agent\n"
           "  -s,  --server=ADDRESS[:PORT]     set update server address and port\n"
           "  -S,  --server-fb=ADDRESS[:PORT]  set fallback update server address and port\n"
           "       --http-user=USER            set username for HTTP connection\n"
           "       --http-password=PASS        set password for HTTP connection\n"
           "       --http-version=VER          set HTTP protocol version (1.0 or 1.1)\n"
           "  -p,  --proto=PROTO               set update protocol (4, 5, 7 or A)\n"
           "  -r,  --remote=PATH               set remote directory or file\n"
           "  -l,  --local=DIR                 set local directory\n"
           "       --proxy=ADDRESS[:PORT]      set HTTP proxy address and port\n"
           "       --proxy-user=USER           set username for HTTP proxy\n"
           "       --proxy-password=PASS       set password for HTTP proxy\n"
           "  -f,  --fast                      use fast checksums checking (dangerous)\n"
           "  -v,  --verbose                   show verbose output\n"
           "  -V,  --verbose-full              show even more verbose output\n"
           "  -h,  --help                      show this help\n"
           "\n"
           "Example:\n"
           "\n"
           "  drwebmirror -k drweb32.key -s update.drweb.com -p 4 -r unix/500 -l .\n"
           "\n"
           "Known update servers:\n"
           "\n"
           "  update.drweb.com         update.msk.drweb.com     update.msk3.drweb.com\n"
           "  update.msk4.drweb.com    update.msk5.drweb.com    update.msk6.drweb.com\n"
           "  update.msk7.drweb.com    update.msk8.drweb.com    update.msk9.drweb.com\n"
           "  update.msk10.drweb.com   update.msk11.drweb.com   update.msk12.drweb.com\n"
           "  update.msk13.drweb.com   update.msk14.drweb.com   update.msk15.drweb.com\n"
           "  update.us.drweb.com      update.us1.drweb.com     update.fr1.drweb.com\n"
           "  update.kz.drweb.com      update.nsk1.drweb.com    update.geo.drweb.com\n"
           "\n"
           "Known remote directories and update protocol versions:\n"
           "\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "|           DrWeb Version           |     Remote directory or file      |  P  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 4.33 for Windows                  | windows                           |  4  |\n"
           "|                                   | 433/windows                       |  4  |\n"
           "| 4.33 for Windows + Antispam       | 433/vr/windows                    |  4  |\n"
           "| 4.33 for Windows Server           | servers/433/windows               |  4  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 4.44 for Windows                  | 444/windows                       |  4  |\n"
           "| 4.44 for Windows + Antispam       | 444/vr/windows                    |  4  |\n"
           "| 4.44 for Windows Server           | 444/servers/windows               |  4  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 5.00 for Novell NetWare           | netware/500                       |  4  |\n"
           "| 5.01 for Novell NetWare           | netware/700                       |  4  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 5.0 for Windows                   | 500/windows                       | 4/5 |\n"
           "|                                   | 500/winold/windows                | 4/5 |\n"
           "| 5.0 Security Space for Windows    | 500/sspace/windows                | 4/5 |\n"
           "| 5.0 for Windows Server            | 500/servers/windows               | 4/5 |\n"
           "|                                   | 500/servers/nt4srv/windows        | 4/5 |\n"
           "| 5.0/6.0 for Microsoft Exchange    | 500/exchange/windows              | 4/5 |\n"
           "| WCL 5.0                           | 500/winconsole/windows            | 4/5 |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 6.0 for Windows                   | x86/600/av/windows                | 4/5 |\n"
           "|                                   | x64/600/av/windows                | 4/5 |\n"
           "| 6.0 Security Space for Windows    | x86/600/sspace/windows            | 4/5 |\n"
           "|                                   | x64/600/sspace/windows            | 4/5 |\n"
           "| 6.0 for Windows Server            | x86/600/servers/windows           | 4/5 |\n"
           "|                                   | x64/600/servers/windows           | 4/5 |\n"
           "| 6.0 for IBM Lotus Domino          | x86/600/lotus/windows             | 4/5 |\n"
           "|                                   | x64/600/lotus/windows             | 4/5 |\n"
           "| 6.0 for Kerio WinRoute            | x86/600/keriowinroute/windows     | 4/5 |\n"
           "|                                   | x64/600/keriowinroute/windows     | 4/5 |\n"
           "| 6.0 for Kerio MailServer          | x86/600/keriomailserver/windows   | 4/5 |\n"
           "|                                   | x64/600/keriomailserver/windows   | 4/5 |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 5.0/6.0 for Unix                  | unix/500                          | 4/5 |\n"
           "| 6.0/8.0 for Unix                  | unix/700                          | 4/5 |\n"
           "| 9.0 for Unix                      | unix/900                          | 4/5 |\n"
           "| 10.0/10.1 for Unix/macOS          | unix/1000/vdb                     | 4/5 |\n"
           "|                                   | unix/1000/dws                     | 4/5 |\n"
           "|                                   | unix/1000/macosx                  | 4/5 |\n"
           "| 11.0/11.1 for Unix/macOS          | unix/1100/version                 |  5  |\n"
           "|                                   | unix/1100/vdb                     |  5  |\n"
           "|                                   | unix/1100/vdb64                   |  5  |\n"
           "|                                   | unix/1100/dws                     |  5  |\n"
           "|                                   | unix/1100/macosx                  |  5  |\n"
           "|                                   | unix/1100/antispam                |  5  |\n"
           "| 11.1 beta for Unix                | unix/1110/version                 | 5(?)|\n"
           "|                                   | unix/1110/vdb                     | 5(?)|\n"
           "|                                   | unix/1110/vdb64                   | 5(?)|\n"
           "|                                   | unix/1110/dws                     | 5(?)|\n"
           "|                                   | unix/1110/antispam                | 5(?)|\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 9.0 LiveDisk                      | livecd/900/windows                | 4/5 |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 7.0 for Windows                   | xmlzone/release/700/av            |  7  |\n"
           "| 7.0 Security Space for Windows    | xmlzone/release/700/sspace        |  7  |\n"
           "| 7.0 for Windows Server            | xmlzone/release/700/servers       |  7  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 8.0 for Windows                   | xmlzone/release/800/av            |  7  |\n"
           "| 8.0 Security Space for Windows    | xmlzone/release/800/sspace        |  7  |\n"
           "| 8.0 for Windows Server            | xmlzone/release/800/servers       |  7  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 9.0/9.1 for Windows               | xmlzone/release/900/windows       |  7  |\n"
           "| 10.0 for Windows                  | xmlzone/release/1000/windows      |  7  |\n"
           "| 11.0 for Windows                  | xmlzone/release/1100/windows      |  7  |\n"
           "| 11.5 for Windows                  | xmlzone/release/1150/windows      |  7  |\n"
           "| 12.0 for Windows                  | xmlzone/release/1200/windows      |  7  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| 11.0 Beta for Windows             | xmlzone/beta/1100/windows         |  7  |\n"
           "| 12.0 Beta for Windows             | xmlzone/beta/1200/windows         |  7  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n"
           "| for Symbian                       | 500/symbian/drwebce.lst           |  A  |\n"
           "| for Windows Mobile                | wince/600/drwebce.lst             |  A  |\n"
           "| 6.0-8.0 for Android               | android/6.1/drwebce.lst           |  A  |\n"
           "| 9.0-9.2 for Android               | android/9/version.lst             |  A  |\n"
           "| 10.0 for Android/BlackBerry       | android/10/version.lst            |  A  |\n"
           "| 10.1 for Android/BlackBerry       | android/10.1/version.lst          |  A  |\n"
           "| 11.0-11.1 for Android             | android/11/version.lst            |  A  |\n"
           "| 12.0 for Android                  | android/12/version.lst            |  A  |\n"
           "|-----------------------------------+-----------------------------------+-----|\n");
}

/* Show help hint message */
void show_hint()
{
    fprintf(ERRFP, "Try `drwebmirror --help' or `drwebmirror -h' for view help.\n");
}

/* Autodetect User Agent */
void detect_useragent(const char * dir)
{
    useragent[0] = '\0';
    /* TODO: More User Agents ? */
    if(strncmp(dir, "android/", 8) == 0)
    {
        /* DrWeb Light 7.00.0, Android 2.3.7 */
        if(strcmp(dir, "android/6.1/drwebce.lst") == 0)
            bsd_strlcpy(useragent, "Dr.Web Updater Symbian/1.0", sizeof(useragent));
        /* DrWeb Light 9.00.2(0), Android 2.3.7 */
        else if(strcmp(dir, "android/9/version.lst") == 0)
            bsd_strlcpy(useragent, "Dr.Web anti-virus Light Version: 9.00.2.7039 "
                                   "Device model: LG-P500 Firmware version: 2.3.7", sizeof(useragent));
        /* DrWeb Security Space 10.0.3(1), Android 4.4.2 */
        else if(strcmp(dir, "android/10/version.lst") == 0)
            bsd_strlcpy(useragent, "Dr.Web anti-virus License:DRWEB KEY (Support ES) "
                                   "LicenseState:LICENSE KEY Version: 10.0.3.14009 "
                                   "Device model: SM-N9005 Firmware version: 4.4.2", sizeof(useragent));
        /* DrWeb Security Space 10.1.1(1), Android 4.4.2 */
        else if(strcmp(dir, "android/10.1/version.lst") == 0)
            bsd_strlcpy(useragent, "Dr.Web anti-virus License:DRWEB KEY (Support ES) "
                                   "LicenseState:LICENSE KEY Version: 10.1.1.14011 "
                                   "Device model: SM-N9005 Firmware version: 4.4.2", sizeof(useragent));
    }
    /*
    if(useragent[0] == '\0')
        bsd_strlcpy(useragent, "DrWebUpdate-6.00.12.03291 (windows: 6.01.7601)", sizeof(useragent));
    */
    /*
       If User Agent is unknown, we will emulate a bad proxy server (for example, Squid with
       "header_access User-Agent deny all"). Maybe this idea is better than mimic a Windows.
    */
}

/* Autodetect update protocol */
char detect_proto(const char * dir)
{
    const char * proto_4[] =
    {
        "444/",
        "433/",
        "windows",
        "servers/433/windows",
        "netware/"
    };
    const char * proto_7[] =
    {
        "xmlzone/"
    };
    const char * proto_A[] =
    {
        "android/",
        "wince/",
        "500/symbian/"
    };
    size_t i;
    for(i = 0; i < sizeof(proto_4) / sizeof(char *); i++)
        if(strncmp(dir, proto_4[i], strlen(proto_4[i])) == 0)
            return '4';
    for(i = 0; i < sizeof(proto_7) / sizeof(char *); i++)
        if(strncmp(dir, proto_7[i], strlen(proto_7[i])) == 0)
            return '7';
    for(i = 0; i < sizeof(proto_A) / sizeof(char *); i++)
        if(strncmp(dir, proto_A[i], strlen(proto_A[i])) == 0)
            return 'A';
    return '5';
}

/* Autodetect update server */
void detect_server(const char * dir)
{
    const char * not_geo[] =
    {
        "444/",
        "433/",
        "500/",
        "x64/600",
        "x86/600",
        "windows",
        "servers/433/windows",
        "netware/",
        "wince/",
        "android/",
        "livecd/",
        "unix/500",
        "unix/700",
        "unix/900"
    };
    size_t i;
    for(i = 0; i < sizeof(not_geo) / sizeof(char *); i++)
        if(strncmp(dir, not_geo[i], strlen(not_geo[i])) == 0)
        {
            bsd_strlcpy(servername, "update.drweb.com", sizeof(servername));
            return;
        }
    bsd_strlcpy(servername, "update.geo.drweb.com", sizeof(servername));
}

/* Main function */
int main(int argc, char * argv[])
{
    int opt = 0, i;
    int8_t o_k = 0, o_a = 0, o_s = 0, o_p = 0, o_r = 0, o_l = 0, o_v = 0, o_h = 0;
    int8_t o_u = 0, o_m = 0, o_H = 0, o_P = 0, o_V = 0, o_f = 0, o_pr = 0, o_pru = 0, o_prp = 0;
    int8_t o_htu = 0, o_htp = 0, o_htv = 0, o_sfb = 0;
    char * optval = NULL;
    char proto = '\0';
    char * workdir = NULL;
    char cwd[STRBUFSIZE];
    time_t time1;
    struct tm * time2;
    char time3[48];
#if !defined(_WIN32)
    struct sigaction sigact;
#endif
    double time_exiec;
    char * inp_user = NULL, * inp_md5 = NULL;
    int status = EXIT_FAILURE;
    char * proxy_user = NULL, * proxy_pass = NULL;
    char * http_user = NULL, * http_pass = NULL, * http_ver = NULL;
    char * servername_fb = NULL;

#if !defined(_WIN32)
    memset(& sigact, 0, sizeof(struct sigaction));
    sigemptyset(& sigact.sa_mask);
    sigact.sa_handler = sighup_handler;
    sigaction(SIGHUP, & sigact, 0);
#endif

    for(i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            if(argv[i][1] == '-') /* long option */
            {
                if(strstr(argv[i] + 2, "keyfile=") == argv[i] + 2)
                    opt = OPT_KEYFILE;
                else if(strstr(argv[i] + 2, "user=") == argv[i] + 2)
                    opt = OPT_USER;
                else if(strstr(argv[i] + 2, "md5=") == argv[i] + 2)
                    opt = OPT_MD5;
                else if(strstr(argv[i] + 2, "syshash=") == argv[i] + 2)
                    opt = OPT_SYSHASH;
                else if(strstr(argv[i] + 2, "agent=") == argv[i] + 2)
                    opt = OPT_AGENT;
                else if(strstr(argv[i] + 2, "server=") == argv[i] + 2)
                    opt = OPT_SERVER;
                else if(strstr(argv[i] + 2, "server-fb=") == argv[i] + 2)
                    opt = OPT_SERVER_FB;
                else if(strstr(argv[i] + 2, "http-user=") == argv[i] + 2)
                    opt = OPT_HTTP_USER;
                else if(strstr(argv[i] + 2, "http-password=") == argv[i] + 2)
                    opt = OPT_HTTP_PASS;
                else if(strstr(argv[i] + 2, "http-version=") == argv[i] + 2)
                    opt = OPT_HTTP_VER;
                else if(strstr(argv[i] + 2, "port=") == argv[i] + 2) /* Deprecated */
                    opt = OPT_PORT;
                else if(strstr(argv[i] + 2, "proto=") == argv[i] + 2)
                    opt = OPT_PROTO;
                else if(strstr(argv[i] + 2, "remote=") == argv[i] + 2)
                    opt = OPT_REMOTE;
                else if(strstr(argv[i] + 2, "local=") == argv[i] + 2)
                    opt = OPT_LOCAL;
                else if(strstr(argv[i] + 2, "proxy-user=") == argv[i] + 2)
                    opt = OPT_PROXY_USER;
                else if(strstr(argv[i] + 2, "proxy-password=") == argv[i] + 2)
                    opt = OPT_PROXY_PASS;
                else if(strstr(argv[i] + 2, "proxy=") == argv[i] + 2)
                    opt = OPT_PROXY;
                else if(strcmp(argv[i] + 2, "fast") == 0)
                    opt = OPT_FAST;
                else if(strcmp(argv[i] + 2, "verbose-full") == 0)
                    opt = OPT_MORE_VERBOSE;
                else if(strcmp(argv[i] + 2, "verbose") == 0)
                    opt = OPT_VERBOSE;
                else if(strcmp(argv[i] + 2, "help") == 0)
                    opt = OPT_HELP;
                else
                {
                    fprintf(ERRFP, "Unknown option %s.\n\n", argv[i]);
                    show_hint();
                    return EXIT_FAILURE;
                }

                if(opt == OPT_KEYFILE || opt == OPT_USER || opt == OPT_MD5 || opt == OPT_SYSHASH ||
                   opt == OPT_AGENT || opt == OPT_SERVER || opt == OPT_PORT || opt == OPT_PROTO ||
                   opt == OPT_REMOTE || opt == OPT_LOCAL || opt == OPT_PROXY || opt == OPT_PROXY_USER ||
                   opt == OPT_PROXY_PASS || opt == OPT_HTTP_USER || opt == OPT_HTTP_PASS ||
                   opt == OPT_HTTP_VER || opt == OPT_SERVER_FB)
                {
                    optval = strchr(argv[i], '=');
                    if(optval)
                    {
                        optval++;
                    }
                    else
                    {
                        fprintf(ERRFP, "Incorrect usage of option %s.\n\n", argv[i]);
                        show_hint();
                        return EXIT_FAILURE;
                    }
                }
                else
                {
                    optval = NULL;
                }
            }
            else /* short option */
            {
                if(argv[i][1] == 'k')
                    opt = OPT_KEYFILE;
                else if(argv[i][1] == 'u')
                    opt = OPT_USER;
                else if(argv[i][1] == 'm')
                    opt = OPT_MD5;
                else if(argv[i][1] == 'H')
                    opt = OPT_SYSHASH;
                else if(argv[i][1] == 'a')
                    opt = OPT_AGENT;
                else if(argv[i][1] == 's')
                    opt = OPT_SERVER;
                else if(argv[i][1] == 'S')
                    opt = OPT_SERVER_FB;
                else if(argv[i][1] == 'P') /* Deprecated */
                    opt = OPT_PORT;
                else if(argv[i][1] == 'p')
                    opt = OPT_PROTO;
                else if(argv[i][1] == 'r')
                    opt = OPT_REMOTE;
                else if(argv[i][1] == 'l')
                    opt = OPT_LOCAL;
                else if(argv[i][1] == 'f')
                    opt = OPT_FAST;
                else if(argv[i][1] == 'V')
                    opt = OPT_MORE_VERBOSE;
                else if(argv[i][1] == 'v')
                    opt = OPT_VERBOSE;
                else if(argv[i][1] == 'h')
                    opt = OPT_HELP;
                else
                {
                    fprintf(ERRFP, "Unknown option %s.\n\n", argv[i]);
                    show_hint();
                    return EXIT_FAILURE;
                }

                if(opt == OPT_KEYFILE || opt == OPT_USER || opt == OPT_MD5 || opt == OPT_SYSHASH ||
                   opt == OPT_AGENT || opt == OPT_SERVER || opt == OPT_PORT || opt == OPT_PROTO ||
                   opt == OPT_REMOTE || opt == OPT_LOCAL || opt == OPT_SERVER_FB)
                {
                    i++;
                    if(i < argc)
                    {
                        optval = argv[i];
                    }
                    else
                    {
                        fprintf(ERRFP, "Incorrect usage of option %s.\n\n", argv[i]);
                        show_hint();
                        return EXIT_FAILURE;
                    }
                }
                else
                {
                    optval = NULL;
                }
            }
        }

        switch(opt)
        {
        case OPT_KEYFILE:
            o_k++;
            parse_keyfile(optval);
            break;
        case OPT_USER:
            o_u++;
            inp_user = optval;
            break;
        case OPT_MD5:
            o_m++;
            inp_md5 = optval;
            break;
        case OPT_SYSHASH:
            o_H++;
            bsd_strlcpy(syshash, optval, sizeof(syshash));
            break;
        case OPT_AGENT:
            o_a++;
            bsd_strlcpy(useragent, optval, sizeof(useragent));
            break;
        case OPT_SERVER:
            o_s++;
            bsd_strlcpy(servername, optval, sizeof(servername));
            break;
        case OPT_SERVER_FB:
            o_sfb++;
            servername_fb = optval;
            break;
        case OPT_HTTP_USER:
            o_htu++;
            http_user = optval;
            break;
        case OPT_HTTP_PASS:
            o_htp++;
            http_pass = optval;
            break;
        case OPT_HTTP_VER:
            o_htv++;
            http_ver = optval;
            break;
        case OPT_PORT: /* Deprecated */
            o_P++;
            serverport = atoi(optval);
            break;
        case OPT_PROTO:
            o_p++;
            proto = optval[0];
            if(proto == 'a') proto = 'A';
            break;
        case OPT_REMOTE:
            o_r++;
            bsd_strlcpy(remotedir, optval, sizeof(remotedir));
            break;
        case OPT_LOCAL:
            o_l++;
            workdir = optval;
            break;
        case OPT_PROXY:
            o_pr++;
            bsd_strlcpy(proxy_address, optval, sizeof(proxy_address));
            break;
        case OPT_PROXY_USER:
            o_pru++;
            proxy_user = optval;
            break;
        case OPT_PROXY_PASS:
            o_prp++;
            proxy_pass = optval;
            break;
        case OPT_FAST:
            o_f++;
            break;
        case OPT_VERBOSE:
            o_v++;
            break;
        case OPT_MORE_VERBOSE:
            o_v++;
            o_V++;
            break;
        case OPT_HELP:
            o_h++;
            break;
        }
    }

    if(o_h)
    {
        show_help();
        return EXIT_SUCCESS;
    }

    if(!o_r)
    {
        fprintf(ERRFP, "Error: No remote dir specified.\n\n");
        show_hint();
        return EXIT_FAILURE;
    }

    if(o_p && proto != '4' && proto != '5' && proto != '7' && proto != 'A')
    {
        fprintf(ERRFP, "Error: Incorrect version of protocol.\n\n");
        show_hint();
        return EXIT_FAILURE;
    }

    if(!o_p)
        proto = detect_proto(remotedir);

    if(proto == 'A')
        use_android = 1;
    else
        use_android = 0;

    if(o_l)
    {
        if(chdir(workdir) < 0)
        {
            fprintf(ERRFP, "Error: Incorrect local directory.\n\n");
            show_hint();
            return EXIT_FAILURE;
        }
    }

    if(!o_s)
        detect_server(remotedir);
    else
    {
        char * delim = strchr(servername, ':');
        if(delim)
        {
            * delim = '\0';
            serverport = atoi(++delim);
            o_P++;
        }
    }

    if(o_htu && o_htp)
    {
        char http_auth_text[64];
        size_t orig_len = strlen(http_user) + strlen(http_pass) + 1;
        size_t base64_len = ((orig_len + 2) / 3 * 4) + 1;
        if(base64_len > 76)
        {
            fprintf(ERRFP, "HTTP username or password is too long.\n\n");
            return EXIT_FAILURE;
        }
        sprintf(http_auth_text, "%s:%s", http_user, http_pass);
        http_auth_text[sizeof(http_auth_text) - 1] = '\0';
        base64_encode(http_auth_text, http_auth);
        use_http_auth = 1;
    }
    else
        use_http_auth = 0;

    if(o_htv)
    {
        if(strcmp(http_ver, "1.0") != 0 && strcmp(http_ver, "1.1") != 0)
        {
            fprintf(ERRFP, "Error: Incorrect HTTP protocol version.\n\n");
            show_hint();
            return EXIT_FAILURE;
        }
        bsd_strlcpy(http_version, http_ver, sizeof(http_version));
    }
    else
    {
        bsd_strlcpy(http_version, "1.1", sizeof(http_version));
    }

    if(!o_k)
    {
        if(o_u && o_m)
        {
            int8_t flag;
            size_t i, len = strlen(inp_user);
            for(i = 0, flag = 0; i < len; i++)
                if(inp_user[i] < '0' || inp_user[i] > '9')
                    flag++;
            if(flag || len > 32)
            {
                fprintf(ERRFP, "Error: Incorrect UserID from key file.\n\n");
                show_hint();
                return EXIT_FAILURE;
            }

            len = strlen(inp_md5);
            for(i = 0, flag = 0; i < len; i++)
            {
                if(inp_md5[i] >= 'A' && inp_md5[i] <= 'Z')
                    inp_md5[i] -= 'A' - 'a';
                if(!((inp_md5[i] >= '0' && inp_md5[i] <= '9') || (inp_md5[i] >= 'a' && inp_md5[i] <= 'f')))
                    flag++;
            }
            if(flag || len != 32)
            {
                fprintf(ERRFP, "Error: Incorrect MD5 sum of key file.\n\n");
                show_hint();
                return EXIT_FAILURE;
            }

            bsd_strlcpy(key_userid, inp_user, sizeof(key_userid));
            bsd_strlcpy(key_md5sum, inp_md5, sizeof(key_md5sum));
        }
        else if(use_android == 0)
        {
#if defined(DEF_USERID) && defined(DEF_MD5SUM)
            fprintf(ERRFP, "Warning: No key file or UserID & MD5 specified, default key value will be used.\n");
            bsd_strlcpy(key_userid, DEF_USERID, sizeof(key_userid));
            bsd_strlcpy(key_md5sum, DEF_MD5SUM, sizeof(key_md5sum));
#else
            fprintf(ERRFP, "Error: No key file or UserID & MD5 specified.\n\n");
            show_hint();
            return EXIT_FAILURE;
#endif
        }
    }

    if(!o_a)
        detect_useragent(remotedir);

    if(o_H)
    {
        int8_t flag = 0;
        size_t i, len = strlen(syshash);
        for(i = 0; i < len; i++)
        {
            if(syshash[i] >= 'a' && syshash[i] <= 'f')
                syshash[i] += 'A' - 'a';
            if(!((syshash[i] >= '0' && syshash[i] <= '9') || (syshash[i] >= 'A' && syshash[i] <= 'F')))
                flag++;
        }
        if(flag || len != 32)
        {
            fprintf(ERRFP, "Incorrecr X-DrWeb-SysHash header.\n\n");
            show_hint();
            return EXIT_FAILURE;
        }
        use_syshash = 1;
    }
    else
        use_syshash = 0;

    if(o_P)
    {
        if(serverport == 0)
        {
            fprintf(ERRFP, "Incorrecr update server port.\n\n");
            show_hint();
            return EXIT_FAILURE;
        }
    }
    else
        serverport = 80;

    if(!o_pr)
    {
        char * http_proxy_env = getenv("http_proxy");
        if(http_proxy_env)
        {
            bsd_strlcpy(proxy_address, http_proxy_env, sizeof(proxy_address));
            o_pr++;
        }
    }

    if(o_pr)
    {
        unsigned tmp = 3128;
        char * delim = strrchr(proxy_address, ':');
        if(delim != NULL)
        {
            sscanf(delim + 1, "%u", & tmp);
            * delim = '\0';
        }
        proxy_port = (uint16_t)tmp;
        use_proxy = 1;
        if(o_pru && o_prp)
        {
            char proxy_auth_text[64];
            size_t orig_len = strlen(proxy_user) + strlen(proxy_pass) + 1;
            size_t base64_len = ((orig_len + 2) / 3 * 4) + 1;
            if(base64_len > 76)
            {
                fprintf(ERRFP, "Proxy username or password is too long.\n\n");
                return EXIT_FAILURE;
            }
            sprintf(proxy_auth_text, "%s:%s", proxy_user, proxy_pass);
            proxy_auth_text[sizeof(proxy_auth_text) - 1] = '\0';
            base64_encode(proxy_auth_text, proxy_auth);
            use_proxy_auth = 1;
        }
    }
    else
    {
        use_proxy = 0;
        use_proxy_auth = 0;
    }

    if(o_v)
        verbose = 1;
    else
        verbose = 0;

    if(o_V)
        more_verbose = 1;
    else
        more_verbose = 0;

    if(o_f)
        use_fast = 1;
    else
        use_fast = 0;
    tree = NULL;

    set_tzshift();

    time1 = time(NULL);
    time2 = localtime(& time1);
    if(time2 == NULL)
    {
        fprintf(ERRFP, "Error: Can't get localtime.\n\n");
        return EXIT_FAILURE;
    }
    if(strftime(time3, sizeof(time3), "%c", time2) == 0)
    {
        fprintf(ERRFP, "Error: Can't get localtime.\n\n");
        return EXIT_FAILURE;
    }

update_begin:

    conn_startup();

    printf("---------- Update bases (v%c) ----------\n", proto);
    printf("Date:  %s\n", time3);
    printf("From:  http://%s:%u/%s\n", servername, (unsigned)serverport, remotedir);
    if(getcwd(cwd, sizeof(cwd)) == NULL)
    {
        fprintf(ERRFP, "Error: Can't get current working directory.\n\n");
        return EXIT_FAILURE;
    }
    printf("To:    %s\n", cwd);
    if(use_proxy)
        printf("Proxy: %s:%u\n", proxy_address, (unsigned)proxy_port);
    if(verbose == 1)
    {
        if(use_android == 0)
        {
            printf("User:  %s\n", key_userid);
            printf("MD5:   %s\n", key_md5sum);
        }
        if(use_syshash)
            printf("Hash:  %s\n", syshash);
        if(useragent[0] != '\0')
            printf("Agent: %s\n", useragent);
    }
    if(more_verbose == 1 && use_http_auth == 1)
    {
        printf("HTTP User:    %s\n", http_user);
        printf("HTTP Pass:    %s\n", http_pass);
        printf("HTTP base64:  %s\n", http_auth);
    }
    if(more_verbose == 1 && use_proxy_auth == 1)
    {
        printf("Proxy User:   %s\n", proxy_user);
        printf("Proxy Pass:   %s\n", proxy_pass);
        printf("Proxy base64: %s\n", proxy_auth);
    }
    printf("---------------------------------------\n");

    switch(proto)
    {
    case '4':
        status = update4();
        break;
    case '5':
        status = update5();
        break;
    case '7':
        status = update7();
        break;
    case 'A':
        status = updateA();
        break;
    }

    conn_cleanup();

    if(tree) avl_dealloc(tree);
    tree = NULL;

    if(status != EXIT_SUCCESS)
    {
        printf("FAILED.\n");
        do_unlock();
        if(o_sfb != 0)
        {
            char * delim = strchr(servername_fb, ':');
            serverport = 80;
            if(delim)
            {
                * delim = '\0';
                serverport = atoi(++delim);
                if(serverport == 0)
                {
                    fprintf(ERRFP, "Incorrecr fallback update server port.\n\n");
                    show_hint();
                    return EXIT_FAILURE;
                }
            }
            bsd_strlcpy(servername, servername_fb, sizeof(servername));
            o_sfb = 0;
            fprintf(ERRFP, "Warning: Trying with fallback server...\n");
            use_fast = 0;
            fprintf(ERRFP, "Warning: Fast mode has been disabled\n");
            goto update_begin;
        }
        return status;
    }

    time_exiec = difftime(time(NULL), time1);
    printf("SUCCESS, time = ");
    if(time_exiec > 3600)
    {
        unsigned int h = (unsigned int)time_exiec / 3600;
        unsigned int m = (unsigned int)(time_exiec - h * 3600.0) / 60;
        unsigned int s = (unsigned int)(time_exiec - h * 3600.0 - m * 60.0);
        printf("%u hr %u min %u sec.\n", h, m, s);
    }
    else if(time_exiec > 60)
    {
        unsigned int m = (unsigned int)(time_exiec) / 60;
        unsigned int s = (unsigned int)(time_exiec - m * 60.0);
        printf("%u min %u sec.\n", m, s);
    }
    else
    {
        unsigned int s = (unsigned int)time_exiec;
        printf("%u sec.\n", s);
    }

    if(do_unlock() != EXIT_SUCCESS)
        return EXIT_FAILURE;
    if(verbose) printf("Removing lock file\n");
    if(remove(lockfile) != 0)
    {
        fprintf(ERRFP, "Error: Error %d with remove() %s: %s\n", errno, lockfile, strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
