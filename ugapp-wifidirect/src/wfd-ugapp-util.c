/*
*  WiFi-Direct UG
*
* Copyright 2012 Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://floralicense.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
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
#include "wfd-ugapp-util.h"


char *wfd_app_trim_path(const char *filewithpath)
{
#if 0
	char *filename = NULL;
	if ((filename = strrchr(filewithpath, '/')) == NULL)
	    return (char *) filewithpath;
	else
	    return (filename + 1);
#else
	static char *filename[100];
	char *strptr = NULL;
	int start = 0;
	const char *space = "                                        ";
	int len = strlen(filewithpath);

	if (len > 20) {
		strptr = (char *) filewithpath + (len - 20);
		start = 0;
	} else if (len < 20) {
		strptr = (char *) filewithpath;
		start = 20 - len;
	}

	strncpy((char *) filename, space, strlen(space));
	strncpy((char *) filename + start, strptr, 50);

	return (char *) filename;
#endif
}


int wfd_app_gettid()
{
#ifdef __NR_gettid
	return syscall(__NR_gettid);
#else
	WDUA_LOGE("__NR_gettid is not defined, please include linux/unistd.h ");
	return -1;
#endif
}
