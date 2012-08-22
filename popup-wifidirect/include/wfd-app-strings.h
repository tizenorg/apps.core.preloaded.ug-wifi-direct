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



#ifndef __WFD_APP_STRING_H__
#define __WFD_APP_STRING_H__

#ifdef __cplusplus
extern "C" {
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

/* Not included in excel file, but it exist in STMS */
#define WFD_STR_TITLE_MAIN         _("WiFi Direct")


#ifdef __cplusplus
}
#endif

#endif /* __WFD_APP_STRING_H__ */
