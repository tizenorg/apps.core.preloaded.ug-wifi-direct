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

#ifndef __WFD_APP_STRING_H__
#define __WFD_APP_STRING_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define WFD_STR_HEADER					_("WiFi Direct")
#define WFD_STR_HEADER_CONNECTED		_("WiFi Direct connected")
#define WFD_STR_HEADER_DEACTIVATING		_("Deactivating WiFi Direct...")
#define WFD_STR_HEADER_CANCEL_CONNECT	_("Canceling connection...")
#define WFD_STR_HEADER_DEFAULT			_("Wi-Fi")
#define WFD_STR_BUTN_YES					_("Yes")
#define WFD_STR_BUTN_NO					_("No")
#define WFD_STR_BTN_OK					_("OK")
#define WFD_STR_BTN_CLOSE				_("Close")
#define WFD_STR_BTN_CANCEL				_("Cancel")

#define WFD_STR_POP_APRV_CONNECT4		"Connect with %s in %d sec?"
#define WFD_STR_POP_ENTER_PIN			"Enter PIN"
#define WFD_STR_POP_INVALID_PIN		"PIN is not valid."


#define WFD_STR_POP_PROG_CONNECT		_("Connecting...")
#define WFD_STR_POP_PROG_CONNECT_WITH_PIN	"Connecting...<br>"\
										"PIN:"

#define WFD_STR_POP_PROG_CANCEL			_("Canceling...")
#define WFD_STR_POP_NOTI_CONNECTED		_("Connected")
#define WFD_STR_POP_NOTI_DISCONNECTED	_("Disconnected")
#define WFD_STR_POP_FAIL_CONNECT		"An error has occurred during<br>"\
   										"connecting"
#define WFD_STR_POP_FAIL_TIMEOUT		"Connection timeout"

#define WFD_STR_TITLE_MAIN         _("WiFi Direct")


#ifdef __cplusplus
}
#endif

#endif                          /* __WFD_APP_STRING_H__ */
