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
 * This file declares wifi direct application util functions.
 *
 * @file    wfd-app-util.h
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */


#ifndef __WFD_APP_UTIL_H__
#define __WFD_APP_UTIL_H__


#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#ifdef VITA_FEATURE
#include <dlog.h>

#define WIFI_DIRECT_APP_MID		"wfd-app"

#define WFD_APP_LOG_LOW 	LOG_VERBOSE
#define WFD_APP_LOG_HIGH 	LOG_INFO
#define WFD_APP_LOG_ERROR 	LOG_ERROR
#define WFD_APP_LOG_WARN 	LOG_WARN
#define WFD_APP_LOG_ASSERT 	LOG_FATAL
#define WFD_APP_LOG_EXCEPTION 	LOG_FATAL

char *wfd_app_trim_path(const char *filewithpath);
int wfd_app_gettid();

#define WFD_APP_LOG(log_level, format, args...) \
	LOG(log_level, WIFI_DIRECT_APP_MID, "[%s:%04d,%d] " format, wfd_app_trim_path(__FILE__), __LINE__,wfd_app_gettid(),##args)
#define __WFD_APP_FUNC_ENTER__	LOG(LOG_VERBOSE,  WIFI_DIRECT_APP_MID, "[%s:%04d,%d] Enter: %s()\n", wfd_app_trim_path(__FILE__), __LINE__,wfd_app_gettid(),__func__)
#define __WFD_APP_FUNC_EXIT__	LOG(LOG_VERBOSE,  WIFI_DIRECT_APP_MID, "[%s:%04d,%d] Quit: %s()\n", wfd_app_trim_path(__FILE__), __LINE__,wfd_app_gettid(),__func__)

#else /** _DLOG_UTIL */

#define WFD_APP_LOG(log_level, format, args...) printf("[%s:%04d,%d] " format, wfd_app_trim_path(__FILE__), __LINE__,wfd_app_gettid(), ##args)
#define __WFD_APP_FUNC_ENTER__	printf("[%s:%04d,%d] Entering: %s()\n", wfd_app_trim_path(__FILE__), __LINE__,wfd_app_gettid(),__func__)
#define __WFD_APP_FUNC_EXIT__	printf("[%s:%04d,%d] Quit: %s()\n", wfd_app_trim_path(__FILE__), __LINE__,wfd_app_gettid(),__func__)

#endif /** _USE_DLOG_UTIL */



#define assertm_if(expr, fmt, arg...) do { \
   if(expr) { \
	  WFD_APP_LOG(WFD_APP_LOG_ASSERT, " ##(%s) -> %s() assert!!## "fmt, #expr, __FUNCTION__, ##arg); \
		 exit(1); \
   } \
} while (0)



#endif                          /* __WFD_APP_UTIL_H__ */
