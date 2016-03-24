@echo off

if "%2" == "" (
    echo Usage: minimal_win_git ^<git-install-path^> ^<minimal-git-path^>
    exit /b
)

set SRCDIR=%1
set DSTDIR=%2

if %SRCDIR% == %DSTDIR% (
    echo Minimal Git path must be different from Git installation path
    exit /b
)

set GIT_FILES=

if exist %SRCDIR%\mingw32 (
    set MINGW=mingw32
    echo 32-bit version of Git detected
    set GIT_FILES=bin\libgcc_s_dw2-1.dll
) else (
    if exist %SRCDIR%\mingw64 (
        set MINGW=mingw64
        echo 64-bit version of Git detected
    )
)

if not defined MINGW (
    echo Git installation is not found in %SRCDIR%
    exit /b
)

echo Copying...
md %DSTDIR%\bin
md %DSTDIR%\libexec\git-core
md %DSTDIR%\libexec\ssl\certs
md %DSTDIR%\share\git-core\templates\hooks

set GIT_FILES=%GIT_FILES% ^
    bin\git.exe ^
    bin\libcurl-4.dll ^
    bin\libeay32.dll ^
    bin\libffi-6.dll ^
    bin\libgmp-10.dll ^
    bin\libgnutls-30.dll ^
    bin\libhogweed-4-1.dll ^
    bin\libiconv-2.dll ^
    bin\libidn-11.dll ^
    bin\libintl-8.dll ^
    bin\libnettle-6-1.dll ^
    bin\libp11-kit-0.dll ^
    bin\libpcre-1.dll ^
    bin\librtmp-1.dll ^
    bin\libssh2-1.dll ^
    bin\libssp-0.dll ^
    bin\libtasn1-6.dll ^
    bin\libwinpthread-1.dll ^
    bin\ssleay32.dll ^
    bin\zlib1.dll ^
    libexec\git-core\git-remote-https.exe ^
    share\git-core\templates\hooks\applypatch-msg.sample ^
    share\git-core\templates\hooks\commit-msg.sample ^
    share\git-core\templates\hooks\post-update.sample ^
    share\git-core\templates\hooks\pre-applypatch.sample ^
    share\git-core\templates\hooks\pre-commit.sample ^
    share\git-core\templates\hooks\pre-push.sample ^
    share\git-core\templates\hooks\pre-rebase.sample ^
    share\git-core\templates\hooks\prepare-commit-msg.sample ^
    share\git-core\templates\hooks\update.sample

for %%f in (%GIT_FILES%) do (
    copy %SRCDIR%\%MINGW%\%%f %DSTDIR%\%%f
)

copy %SRCDIR%\%MINGW%\ssl\certs\ca-bundle.crt %DSTDIR%\libexec\ssl\certs\ca-bundle.crt
copy %SRCDIR%\%MINGW%\ssl\certs\ca-bundle.trust.crt %DSTDIR%\libexec\ssl\certs\ca-bundle.trust.crt

echo Testing...
%DSTDIR%\bin\git.exe --version
