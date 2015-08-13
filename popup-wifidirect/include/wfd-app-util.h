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

#ifdef VITA_FEATURE
#include <dlog.h>

#define WIFI_DIRECT_APP_MID		"wfd-app"

#define WFD_APP_LOG_LOW	LOG_INFO
#define WFD_APP_LOG_HIGH	LOG_INFO
#define WFD_APP_LOG_ERROR	LOG_ERROR
#define WFD_APP_LOG_WARN	LOG_WARN
#define WFD_APP_LOG_ASSERT	LOG_FATAL
#define WFD_APP_LOG_EXCEPTION	LOG_FATAL
#define WFD_MAX_SIZE            128
#define WFD_MAC_ADDRESS_SIZE    18

char *wfd_app_trim_path(const char *filewithpath);
int wfd_app_gettid();
/* TODO:: To change the log level as LOG_INFO */
#define WFD_APP_LOG(log_level, format, args...) \
	LOG(LOG_ERROR, WIFI_DIRECT_APP_MID, "[%s:%04d,%d] " format, wfd_app_trim_path(__FILE__),  __LINE__, wfd_app_gettid(), ##args)
#define WFD_APP_LOGSECURE(log_level, format, args...) \
	SECURE_LOG(LOG_ERROR, WIFI_DIRECT_APP_MID, "[%s:%04d,%d] " format, wfd_app_trim_path(__FILE__),  __LINE__, wfd_app_gettid(), ##args)

#if 0
#define __WFD_APP_FUNC_ENTER__	LOG(LOG_VERBOSE,  WIFI_DIRECT_APP_MID, "[%s:%04d,%d] Enter: %s()\n", wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), __func__)
#define __WFD_APP_FUNC_EXIT__	LOG(LOG_VERBOSE,  WIFI_DIRECT_APP_MID, "[%s:%04d,%d] Quit: %s()\n", wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), __func__)
#else
#define __WFD_APP_FUNC_ENTER__
#define __WFD_APP_FUNC_EXIT__
#endif

#else /** _DLOG_UTIL */

#define WFD_APP_LOG(log_level, format, args...) printf("[%s:%04d,%d] " format, wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), ##args)
#if 0
#define __WFD_APP_FUNC_ENTER__	printf("[%s:%04d,%d] Entering: %s()\n", wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), __func__)
#define __WFD_APP_FUNC_EXIT__	printf("[%s:%04d,%d] Quit: %s()\n", wfd_app_trim_path(__FILE__), __LINE__, wfd_app_gettid(), __func__)
#else
#define __WFD_APP_FUNC_ENTER__
#define __WFD_APP_FUNC_EXIT__
#endif

#endif /** _USE_DLOG_UTIL */

#define WFD_RET_IF(expr, fmt, args...) \
	do { \
		if(expr) { \
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "[%s] Return, message "fmt, #expr, ##args );\
			return; \
		} \
	} while (0)

#define WFD_IF_FREE_MEM(mem) \
	do { \
		if(mem) { \
			free(mem); \
			mem = NULL; \
		} \
	} while (0)

#define WFD_RETV_IF(expr, val, fmt, args...) \
	do { \
		if(expr) { \
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "[%s] Return value, message "fmt, #expr, ##args );\
			return (val); \
		} \
	} while (0)

#define assertm_if(expr, fmt, arg...) do { \
	if (expr) { \
	  WFD_APP_LOG(WFD_APP_LOG_ASSERT, " ##(%s) -> %s() assert!!## "fmt, #expr, __FUNCTION__, ##arg); \
		 exit(1); \
	} \
} while (0)

int wfd_app_util_register_hard_key_down_cb(void *data);
int wfd_app_util_register_vconf_callbacks(void *data);
int wfd_app_util_deregister_vconf_callbacks(void *data);

void wfd_app_util_del_notification(wfd_appdata_t *ad);
#ifdef NOT_CONNECTED_INDICATOR_ICON
void wfd_app_util_add_indicator_icon(void *user_data);
#endif
void wfd_app_util_add_wfd_turn_off_notification(void *user_data);

#ifdef WFD_SCREEN_MIRRORING_ENABLED
void wfd_app_util_set_screen_mirroring_deactivated(wfd_appdata_t *ad);
#endif
void wfd_app_util_del_wfd_connected_notification(wfd_appdata_t *ad);
#endif /* __WFD_APP_UTIL_H__ */
