/***************************************************************************
 *   Copyright (C) 2005, 2006 by Dmitry Morozhnikov   *
 *   dmiceman@mail.ru   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/param.h>
#include <pwd.h>

#include <linux/iso_fs.h>

#define FUSE_USE_VERSION 25
#include <fuse.h>

#include <zlib.h>
#include <locale.h>
#include <langinfo.h>

#include "isofs.h"

#ifdef __GNUC__
# define UNUSED(x) x __attribute__((unused))
#else
# define UNUSED(x) x
#endif

#define VERSION __DATE__

static char *imagefile = NULL;
static char *mount_point = NULL;
static int image_fd = -1;

char* iocharset = NULL;

static char* normalize_name(const char* fname)
{
    char* abs_fname = (char*) malloc(PATH_MAX);
    realpath(fname, abs_fname);
    // ignore errors from realpath()
    return abs_fname;
}

static int check_mount_point()
{
    struct stat st;
    int rc = lstat(mount_point, &st);

    if (-1 == rc && ENOENT == errno)
    {
        // directory does not exists, createcontext
        rc = mkdir(mount_point, 0777); // let`s underlying filesystem manage permissions

        if (0 != rc)
        {
            perror("Can't create mount point");
            return -EIO;
        }
    }
    else if (-1 == rc)
    {
        perror("Can't check mount point");
        return -1;
    }

    return 0;
}

static void del_mount_point()
{
    // Do not check rmdir() return value because mount point directory
    // can be already remove if unmounted from OS X UI, i.e. from Finder
    rmdir(mount_point);
}

static int isofs_getattr(const char *path, struct stat *stbuf)
{
    return isofs_real_getattr(path, stbuf);
}

static int isofs_readlink(const char *path, char *target, size_t size)
{
    return isofs_real_readlink(path, target, size);
}

static int isofs_open(const char *path, struct fuse_file_info *UNUSED(fi))
{
    return isofs_real_open(path);
}

static int isofs_read(const char *path, char *buf, size_t size,
    off_t offset, struct fuse_file_info *UNUSED(fi))
{
    return isofs_real_read(path, buf, size, offset);
}

static int isofs_flush(const char *UNUSED(path), struct fuse_file_info *UNUSED(fi))
{
    return 0;
}

static void* isofs_init()
{
    return isofs_real_init();
}

static int isofs_opendir(const char *path, struct fuse_file_info *UNUSED(fi))
{
    return isofs_real_opendir(path);
}

static int isofs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t UNUSED(offset), struct fuse_file_info *UNUSED(fi))
{
    return isofs_real_readdir(path, buf, filler);
}

static int isofs_statfs(const char *UNUSED(path), struct statvfs *stbuf)
{
    return isofs_real_statfs(stbuf);
}

static struct fuse_operations isofs_oper =
{
    .getattr    = isofs_getattr,
    .readlink   = isofs_readlink,
    .open       = isofs_open,
    .read       = isofs_read,
    .flush      = isofs_flush,
    .init       = isofs_init,
    .opendir    = isofs_opendir,
    .readdir    = isofs_readdir,
    .statfs     = isofs_statfs,
};

static void usage(const char* prog)
{
    printf("Version: %s\nUsage: %s [-c <iocharset>] [-h] <isofs_image_file> [<FUSE library options>]\n"
        "Where options are:\n"
        "    -c <iocharset>     -- specify iocharset for Joliet filesystem\n"
        "    -h                 -- print this screen\n"
        "\nCommon FUSE library options are:\n"
        "    -f                 -- run in foreground, do not daemonize\n"
        "    -d                 -- run in foreground and print debug information\n"
        "    -s                 -- run single-threaded\n"
        "\nPlease consult with FUSE ducumentation for more information\n",
        VERSION, 
        prog);
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, ""); // set current locale for proper iocharset
    
    char c;
    
    while ((c = (char) getopt(argc, argv, "+npc:h")) > 0)
    {
        switch (c)
        {
            case 'c':
                if (optarg)
                {
                    iocharset = optarg;
                }
                break;

            case 'h':
                usage(argv[0]);
                exit(0);
                break;

            case '?':
            case ':':
                usage(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }
    
    if ((argc - optind) < 1)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    imagefile = normalize_name(argv[optind]);
    
    image_fd = open(imagefile, O_RDONLY);

    if (-1 == image_fd)
    {
        fprintf(stderr, "Supplied image file name: \"%s\"\n", imagefile);
        perror("Can't open image file");
        exit(EXIT_FAILURE);
    }
    
    // with space for possible -o use_ino arguments
    char** nargv = (char**) malloc( (size_t)(argc + 2) * sizeof(char*) );
    int nargc = argc - optind;
    
    nargv[0] = argv[0];
    
    int i;
    int next_opt = 0;
    int use_ino_found = 0;
    
    for (i = 0; i < nargc - 1; ++i)
    {
        if (next_opt && !use_ino_found)
        {
            if (strstr(argv[i + optind + 1], "use_ino"))
            {
                // ok, already there
                use_ino_found = 1;
                nargv[i + 1] = argv[i + optind + 1];
            }
            else
            {
                // add it
                char* str = (char*) malloc(strlen(argv[i + optind + 1]) + 10);
                strcpy(str, argv[i + optind + 1]);
                strcat(str, ",use_ino");
                nargv[i + 1] = str;
                use_ino_found = 1;
            }
        }
        else
        {
            nargv[i + 1] = argv[i + optind + 1];
        }

        // check if this is -o string mean that next argument should be options string
        if(i > 1 && nargv[i + 1][0] == '-' && nargv[i + 1][1] == 'o')
        {
            next_opt = 1;
        }
    }

    if (NULL == iocharset)
    {
        char* nlcharset = nl_langinfo(CODESET);

        if (NULL != nlcharset)
        {
            iocharset = (char*) malloc(strlen(nlcharset) + 9);
            strcpy(iocharset, nlcharset);
            strcat(iocharset, "//IGNORE");
        }

        if (NULL == iocharset)
        {
            iocharset = "UTF-8//IGNORE";
        }
    }

    // Prepare volume name

    char* volumePart = strrchr(imagefile, '/');
    if (NULL == volumePart)
    {
        volumePart = imagefile;
    }
    else
    {
        ++volumePart; // skip leading path separator
    }

    char* volumeName = strdup(volumePart);

    // Remove file extension from volume name

    char* extensionPosition = strrchr(volumeName, '.');
    if (NULL != extensionPosition)
    {
        *extensionPosition = '\0';
    }

    // Combine volume name with other options

    char optionsBuffer[PATH_MAX + 128] = {0};
    snprintf(optionsBuffer, sizeof(optionsBuffer), "-o%sallow_other,ro,volname=%s",
        (use_ino_found ? "" : "use_ino,"), volumeName);

    char mountPointBuffer[PATH_MAX] = {0};
    snprintf(mountPointBuffer, sizeof(mountPointBuffer), "/Volumes/%s", volumeName);

    free(volumeName);

    nargv[nargc++] = mountPointBuffer;
    nargv[nargc++] = optionsBuffer;

    // Setup mount point

    mount_point = mountPointBuffer;

    if (0 != check_mount_point())
    {
        exit(EXIT_FAILURE);
    }

    if (0 != atexit(del_mount_point))
    {
        fprintf(stderr, "Can't set exit function\n");
        exit(EXIT_FAILURE);
    }

    // will exit in case of failure
    isofs_real_preinit(imagefile, image_fd);
    
    return fuse_main(nargc, nargv, &isofs_oper);
}
