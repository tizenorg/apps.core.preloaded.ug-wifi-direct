/*
 * Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of org.tizen.wifi-direct-popup
 * Written by Sungsik Jang <sungsik.jang@samsung.com>
 *            Dongwook Lee <dwmax.lee@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with SAMSUNG ELECTRONICS. 
 *
 * SAMSUNG make no representations or warranties about the suitability
 * of the software, either express or implied, including but not limited
 * to the implied warranties of merchantability, fitness for a particular
 * purpose, or non-infringement. SAMSUNG shall not be liable for any
 * damages suffered by licensee as a result of using, modifying or
 * distributing this software or its derivatives.
 *
 */

/**
 * This file implements wifi direct application utils functions.
 *
 * @file    wfd-app-util.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */


#include <stdio.h>
#include <string.h>
#include "wfd-app-util.h"


char *wfd_app_trim_path(const char *filewithpath)
{
    static char *filename[100];
    char *strptr = NULL;
    int start = 0;
    const char *space = "                                        ";
    int len = strlen(filewithpath);

    if (len > 20)
    {
        strptr = (char *) filewithpath + (len - 20);
        start = 0;
    }
    else if (len < 20)
    {
        strptr = (char *) filewithpath;
        start = 20 - len;
    }
    strncpy((char *) filename, space, strlen(space));
    strncpy((char *) filename + start, strptr, 50);

    return (char *) filename;
}


int wfd_app_gettid()
{
#ifdef __NR_gettid
    return syscall(__NR_gettid);
#else
    fprintf(stderr,
            "__NR_gettid is not defined, please include linux/unistd.h ");
    return -1;
#endif
}
