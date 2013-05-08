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

#ifdef USE_DLOG
#include <dlog.h>

#undef LOG_TAG
#define LOG_TAG "WIFI_DIRECT_UGAPP"

#define WDUA_LOGV(format, args...) LOGV(format, ##args)
#define WDUA_LOGD(format, args...) LOGD(format, ##args)
#define WDUA_LOGI(format, args...) LOGI(format, ##args)
#define WDUA_LOGW(format, args...) LOGW(format, ##args)
#define WDUA_LOGE(format, args...) LOGE(format, ##args)
#define WDUA_LOGF(format, args...) LOGF(format, ##args)

#define __WDUA_LOG_FUNC_ENTER__ LOGV("Enter")
#define __WDUA_LOG_FUNC_EXIT__ LOGV("Quit")

#define assertm_if(expr, fmt, args...) do { \
	if (expr) { \
		WDUA_LOGF(" ##(%s) -> assert!!## "fmt, #expr, ##args); \
		exit(1); \
	} \
} while (0)

#else /** _DLOG_UTIL */
#define WDUA_LOGV(format, args...)
#define WDUA_LOGD(format, args...)
#define WDUA_LOGI(format, args...)
#define WDUA_LOGW(format, args...)
#define WDUA_LOGE(format, args...)
#define WDUA_LOGF(format, args...)

#define __WDUA_LOG_FUNC_ENTER__
#define __WDUA_LOG_FUNC_EXIT__

#endif /** _DLOG_UTIL */

#endif /* __WFD_APP_UTIL_H__ */
