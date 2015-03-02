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
#include <signal.h>

#define OPT_KEYFILE      0x01
#define OPT_USER         0x02
#define OPT_MD5          0x03
#define OPT_SYSHASH      0x04
#define OPT_AGENT        0x05
#define OPT_SERVER       0x06
#define OPT_PORT         0x07
#define OPT_PROTO        0x08
#define OPT_REMOTE       0x09
#define OPT_LOCAL        0x0A
#define OPT_FAST         0x0B
#define OPT_VERBOSE      0x0C
#define OPT_MORE_VERBOSE 0x0D
#define OPT_HELP         0x0E

/* Flag of use verbose output */
int8_t verbose;
/* Flag of use even more verbose output */
int8_t more_verbose;

/* Show help message */
void show_help()
{
    printf("DrWebMirror %s, tool for create mirror of DrWeb update server.\n", PROG_VERSION);
    printf("Copyright (C) 2014-2015, Rudolf Sikorski <rudolf.sikorski@freenet.de>\n");
    printf("Homepage: http://fami-net.dlinkddns.com/osp/drwebmirror\n\n");
    printf("Usage: drwebmirror <options>\n");
    printf("\n");
    printf("Options:\n");
    printf("  -k FILE,    --keyfile=FILE       set key file\n");
    printf("  -u NUMBER,  --user=NUMBER        set UserID number from key file\n");
    printf("  -m STRING,  --md5=STRING         set MD5 sum of key file\n");
    printf("  -H STRING,  --syshash=STRING     set X-DrWeb-SysHash header\n");
    printf("  -a STRING,  --agent=STRING       set custom User Agent\n");
    printf("  -s ADDRESS, --server=ADDRESS     set update server\n");
    printf("  -P NUMBER,  --port=NUMBER        set update server port\n");
    printf("  -p PROTO,   --proto=PROTO        set update protocol (4, 5, 7 or A)\n");
    printf("  -r PATH,    --remote=PATH        set remote directory or file\n");
    printf("  -l DIR,     --local=DIR          set local directory\n");
    printf("  -f,         --fast               use fast checksums checking (dangerous)\n");
    printf("  -v,         --verbose            show verbose output\n");
    printf("  -V,         --verbose-full       show even more verbose output\n");
    printf("  -h,         --help               show this help\n");
    printf("\n");
    printf("Example:\n");
    printf("\n");
    printf("  drwebmirror -k drweb32.key -s update.drweb.com -p 4 -r unix/500 -l .\n");
    printf("\n");
    printf("Known remote directories and update protocol versions:\n");
    printf("\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("|           DrWeb Version              |   Remote directory or file   |  P  |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 4.33 for Windows               | windows                      |  4  |\n");
    printf("|                                      | 433/windows                  |  4  |\n");
    printf("| DrWeb 4.33 for Windows + Antispam    | 433/vr/windows               |  4  |\n");
    printf("| DrWeb 4.33 for Windows Server        | servers/433/windows          |  4  |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 4.44 for Windows               | 444/windows                  |  4  |\n");
    printf("| DrWeb 4.44 for Windows + Antispam    | 444/vr/windows               |  4  |\n");
    printf("| DrWeb 4.44 for Windows Server        | 444/servers/windows          |  4  |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 5.0 for Windows                | 500/windows                  | 4/5 |\n");
    printf("|                                      | 500/winold/windows           | 4/5 |\n");
    printf("| DrWeb 5.0 Security Space for Windows | 500/sspace/windows           | 4/5 |\n");
    printf("| DrWeb 5.0 for Windows Server         | 500/servers/windows          | 4/5 |\n");
    printf("|                                      | 500/servers/nt4srv/windows   | 4/5 |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 6.0 for Windows                | x86/600/av/windows           | 4/5 |\n");
    printf("|                                      | x64/600/av/windows           | 4/5 |\n");
    printf("| DrWeb 6.0 Security Space for Windows | x86/600/sspace/windows       | 4/5 |\n");
    printf("|                                      | x64/600/sspace/windows       | 4/5 |\n");
    printf("| DrWeb 6.0 for Windows Server         | x86/600/servers/windows      | 4/5 |\n");
    printf("|                                      | x64/600/servers/windows      | 4/5 |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 5.0/6.0 for Unix               | unix/500                     | 4/5 |\n");
    printf("| DrWeb 6.0/8.0 for Unix               | unix/700                     | 4/5 |\n");
    printf("| DrWeb 9.0 for Unix                   | unix/900                     | 4/5 |\n");
    printf("| DrWeb 10.0 for Unix                  | unix/1000/vdb                | 4/5 |\n");
    printf("|                                      | unix/1000/dws                | 4/5 |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 9.0 LiveDisk                   | livecd/900/windows           | 4/5 |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 7.0 for Windows                | xmlzone/release/700/av       |  7  |\n");
    printf("| DrWeb 7.0 Security Space for Windows | xmlzone/release/700/sspace   |  7  |\n");
    printf("| DrWeb 7.0 for Windows Server         | xmlzone/release/700/servers  |  7  |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 8.0 for Windows                | xmlzone/release/800/av       |  7  |\n");
    printf("| DrWeb 8.0 Security Space for Windows | xmlzone/release/800/sspace   |  7  |\n");
    printf("| DrWeb 8.0 for Windows Server         | xmlzone/release/800/servers  |  7  |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 9.0 for Windows                | xmlzone/release/900/windows  |  7  |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 10.0 for Windows               | xmlzone/release/1000/windows |  7  |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
    printf("| DrWeb 6.0-8.0 for Android            | android/6.1/drwebce.lst      |  A  |\n");
    printf("| DrWeb 9.0 for Android                | android/9/version.lst        |  A  |\n");
    printf("|--------------------------------------|------------------------------|-----|\n");
}

/* Show help hint message */
inline void show_hint()
{
    fprintf(ERRFP, "Try `drwebmirror --help' or `drwebmirror -h' for view help.\n");
}

/* Autodetect User Agent */
void detect_useragent(const char * dir)
{
    useragent[0] = '\0';
    /* TODO: More User Agents ? */
    if(strncmp(dir, "android/", strlen("android/")) == 0)
    {
        /* DrWeb Light 7.00.0, Android 2.3.7 */
        if(strcmp(dir, "android/6.1/drwebce.lst") == 0)
            strncpy(useragent, "Dr.Web Updater Symbian/1.0", sizeof(useragent) - 1);
        /* DrWeb Light 9.00.2(0), Android 2.3.7 */
        else if(strcmp(dir, "android/9/version.lst") == 0)
            strncpy(useragent, "Dr.Web anti-virus Light Version: 9.00.2.7039 Device model: LG-P500 Firmware version: 2.3.7", sizeof(useragent) - 1);
    }
    if(useragent[0] == '\0')
        strncpy(useragent, DEF_USERAGENT, sizeof(useragent) - 1);
}

/* Autodetect update protocol */
char detect_proto(const char * dir)
{
    if(strncmp(dir, "xmlzone/", strlen("xmlzone/")) == 0)
        return '7';
    if(strncmp(dir, "android/", strlen("android/")) == 0)
        return 'A';
    return '4';
}

/* Main function */
int main(int argc, char * argv[])
{
    int opt = 0, i;
    int8_t o_k = 0, o_a = 0, o_s = 0, o_p = 0, o_r = 0, o_l = 0, o_v = 0, o_h = 0;
    int8_t o_u = 0, o_m = 0, o_H = 0, o_P = 0, o_V = 0, o_f = 0;
    char * optval = NULL;
    char proto = '\0';
    char * workdir = NULL;
    char cwd[STRBUFSIZE];
    time_t time1;
    struct tm * time2;
    char time3[48];
    struct sigaction sigact;
    double time_exiec;
    char * inp_user = NULL, * inp_md5 = NULL;
    int status = EXIT_FAILURE;

    memset(& sigact, 0, sizeof(struct sigaction));
    sigemptyset(& sigact.sa_mask);
    sigact.sa_handler = sighup_handler;
    sigaction(SIGHUP, & sigact, 0);

    for(i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            if(argv[i][1] == '-') /* long option */
            {
                if(strstr(argv[i] + 2, "keyfile") == argv[i] + 2)
                    opt = OPT_KEYFILE;
                else if(strstr(argv[i] + 2, "user") == argv[i] + 2)
                    opt = OPT_USER;
                else if(strstr(argv[i] + 2, "md5") == argv[i] + 2)
                    opt = OPT_MD5;
                else if(strstr(argv[i] + 2, "syshash") == argv[i] + 2)
                    opt = OPT_SYSHASH;
                else if(strstr(argv[i] + 2, "agent") == argv[i] + 2)
                    opt = OPT_AGENT;
                else if(strstr(argv[i] + 2, "server") == argv[i] + 2)
                    opt = OPT_SERVER;
                else if(strstr(argv[i] + 2, "port") == argv[i] + 2)
                    opt = OPT_PORT;
                else if(strstr(argv[i] + 2, "proto") == argv[i] + 2)
                    opt = OPT_PROTO;
                else if(strstr(argv[i] + 2, "remote") == argv[i] + 2)
                    opt = OPT_REMOTE;
                else if(strstr(argv[i] + 2, "local") == argv[i] + 2)
                    opt = OPT_LOCAL;
                else if(strstr(argv[i] + 2, "fast") == argv[i] + 2)
                    opt = OPT_FAST;
                else if(strstr(argv[i] + 2, "verbose-full") == argv[i] + 2)
                    opt = OPT_MORE_VERBOSE;
                else if(strstr(argv[i] + 2, "verbose") == argv[i] + 2)
                    opt = OPT_VERBOSE;
                else if(strstr(argv[i] + 2, "help") == argv[i] + 2)
                    opt = OPT_HELP;
                else
                {
                    fprintf(ERRFP, "Unknown option %s.\n\n", argv[i]);
                    show_hint();
                    return EXIT_FAILURE;
                }

                if(opt == OPT_KEYFILE || opt == OPT_USER || opt == OPT_MD5 || opt == OPT_SYSHASH ||
                   opt == OPT_AGENT || opt == OPT_SERVER || opt == OPT_PORT || opt == OPT_PROTO ||
                   opt == OPT_REMOTE || opt == OPT_LOCAL)
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
                else if(argv[i][1] == 'P')
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
                   opt == OPT_REMOTE || opt == OPT_LOCAL)
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
            strncpy(syshash, optval, sizeof(syshash) - 1);
            break;
        case OPT_AGENT:
            o_a++;
            strncpy(useragent, optval, sizeof(useragent) - 1);
            break;
        case OPT_SERVER:
            o_s++;
            strncpy(servername, optval, sizeof(servername) - 1);
            break;
        case OPT_PORT:
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
            strncpy(remotedir, optval, sizeof(remotedir) - 1);
            break;
        case OPT_LOCAL:
            o_l++;
            workdir = optval;
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
        strncpy(servername, DEF_SERVER, sizeof(servername) - 1);

    if(!o_k)
    {
        if(o_u && o_m)
        {
            int8_t flag;
            size_t i;
            for(i = 0, flag = 0; i < strlen(inp_user); i++)
                if(inp_user[i] < '0' || inp_user[i] > '9')
                    flag++;
            if(flag || strlen(inp_user) > 32)
            {
                fprintf(ERRFP, "Error: Incorrect UserID from key file.\n\n");
                show_hint();
                return EXIT_FAILURE;
            }

            to_lowercase(inp_md5);
            for(i = 0, flag = 0; i < strlen(inp_md5); i++)
                if(!((inp_md5[i] >= '0' && inp_md5[i] <= '9') || (inp_md5[i] >= 'a' && inp_md5[i] <= 'f')))
                    flag++;
            if(flag || strlen(inp_md5) != 32)
            {
                fprintf(ERRFP, "Error: Incorrect MD5 sum of key file.\n\n");
                show_hint();
                return EXIT_FAILURE;
            }

            strncpy(key_userid, inp_user, sizeof(key_userid) - 1);
            strncpy(key_md5sum, inp_md5, sizeof(key_md5sum) - 1);
        }
        else if(use_android == 0)
        {
#if defined(DEF_USERID) && defined(DEF_MD5SUM)
            fprintf(ERRFP, "Warning: No key file or UserID & MD5 specified, using default key value.\n");
            strncpy(key_userid, DEF_USERID, sizeof(key_userid) - 1);
            strncpy(key_md5sum, DEF_MD5SUM, sizeof(key_md5sum) - 1);
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
        size_t i;
        for(i = 0; i < strlen(syshash); i++)
        {
            if(syshash[i] >= 'a' && syshash[i] <= 'f')
                syshash[i] += 'A' - 'a';
            if(!((syshash[i] >= '0' && syshash[i] <= '9') || (syshash[i] >= 'A' && syshash[i] <= 'F')))
                flag++;
        }
        if(flag || strlen(syshash) != 32)
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

    printf("---------- Update bases (v%c) ----------\n", proto);
    printf("Date:  %s\n", time3);
    printf("From:  http://%s:%u/%s\n", servername, (unsigned)serverport, remotedir);
    getcwd(cwd, sizeof(cwd));
    printf("To:    %s\n", cwd);
    if(verbose)
    {
        if(use_android == 0)
        {
            printf("User:  %s\n", key_userid);
            printf("MD5:   %s\n", key_md5sum);
        }
        if(use_syshash)
            printf("Hash:  %s\n", syshash);
        printf("Agent: %s\n", useragent);
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

    if(tree) avl_dealloc(tree);
    if(status != EXIT_SUCCESS)
    {
        printf("FAILED.\n");
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

    return EXIT_SUCCESS;
}
