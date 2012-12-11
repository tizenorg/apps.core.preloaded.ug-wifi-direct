/*
*  WiFi-Direct UG
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.tizenopensource.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
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

#define WFD_APP_LOG_LOW		LOG_VERBOSE
#define WFD_APP_LOG_HIGH		LOG_INFO
#define WFD_APP_LOG_ERROR		LOG_ERROR
#define WFD_APP_LOG_WARN		LOG_WARN
#define WFD_APP_LOG_ASSERT		LOG_FATAL
#define WFD_APP_LOG_EXCEPTION	LOG_FATAL
#define WFD_MAX_SIZE            128
#define WFD_MAC_ADDRESS_SIZE    18

char *wfd_app_trim_path(const char *filewithpath);
int wfd_app_gettid();

#define WFD_APP_LOG(log_level, format, args...) \
	LOG(log_level, WIFI_DIRECT_APP_MID, "[%s:%04d,%d] " format, wfd_app_trim_path(__FILE__),  __LINE__, wfd_app_gettid(), ##args)
#define __WFD_APP_FUNC_ENTER__	LOG(LOG_VERBOSE,  WIFI_DIRECT_APP_MID, "[%s:%04d,%d] Enter: %s()\n", wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), __func__)
#define __WFD_APP_FUNC_EXIT__	LOG(LOG_VERBOSE,  WIFI_DIRECT_APP_MID, "[%s:%04d,%d] Quit: %s()\n", wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), __func__)

#else /** _DLOG_UTIL */

#define WFD_APP_LOG(log_level, format, args...) printf("[%s:%04d,%d] " format, wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), ##args)
#define __WFD_APP_FUNC_ENTER__	printf("[%s:%04d,%d] Entering: %s()\n", wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), __func__)
#define __WFD_APP_FUNC_EXIT__	printf("[%s:%04d,%d] Quit: %s()\n", wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), __func__)

#endif /** _USE_DLOG_UTIL */



#define assertm_if(expr, fmt, arg...) do { \
	if (expr) { \
	  WFD_APP_LOG(WFD_APP_LOG_ASSERT, " ##(%s) -> %s() assert!!## "fmt, #expr, __FUNCTION__, ##arg); \
		 exit(1); \
	} \
} while (0)



#endif /* __WFD_APP_UTIL_H__ */
