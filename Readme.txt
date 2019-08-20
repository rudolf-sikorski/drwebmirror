*** DrWebMirror ***

Project mission:
    To develop a simple tool for creating a mirror of DrWeb update server.

Programming language:
    Plain C (with POSIX or Windows API).

License:
    GNU GPL v3 (main program), various (3rd party components).

Homepage:
    https://github.com/rudolf-sikorski/drwebmirror

Compilation:
    * Unix-like systems:
      $ mkdir build
      $ cd build
      $ cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
      $ make
      $ sudo make install
    * Windows (MinGW):
      > md build
      > cd build
      > set PATH=C:\MinGW\bin;%PATH%
      > cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
      > mingw32-make
    * Windows (Visual Studio 2003+):
      > md build
      > cd build
      > cmake -G "Visual Studio 7 .NET 2003" ..
      > cmake --build . --config Release
    * Windows (OpenWatcom):
      > md build
      > cd build
      > cmake -G "Watcom WMake" -DCMAKE_BUILD_TYPE=Release ..
      > wmake

Submitting Bugs:
    * GitHub issues tracker:
      https://github.com/rudolf-sikorski/drwebmirror/issues
    * Email:
      Rudolf Sikorski <rudolf.sikorski@freenet.de>



*** Annex 1. Tricks for Apache ***

# DrWebUpW clients should receive files from the mirrors/drweb/
RewriteCond %{HTTP_USER_AGENT} ^Dr\.?Web(.*) [NC,OR]
RewriteCond %{HTTP:X-DrWeb-KeyNumber} ^[0-9]+ [OR]
RewriteCond %{HTTP:X-DrWeb-Validate} ^[0-9a-f]+ [NC]
RewriteRule ^([^(mirrors)](.*))$ mirrors/drweb/$1 [L]


*** Annex 2. Tricks for Nginx ***

# DrWebUpW clients should receive files from the /mirrors/drweb/
if ($http_user_agent ~* "^Dr\.?Web(.*)")
{ rewrite ^/([^(mirrors)](.*))$ /mirrors/drweb/$1 break; }
if ($http_x_drweb_keynumber)
{ rewrite ^/([^(mirrors)](.*))$ /mirrors/drweb/$1 break; }
if ($http_x_drweb_validate)
{ rewrite ^/([^(mirrors)](.*))$ /mirrors/drweb/$1 break; }
