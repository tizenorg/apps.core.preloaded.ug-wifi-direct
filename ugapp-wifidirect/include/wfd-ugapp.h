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
 * This file declares wifi direct application functions.
 *
 * @file    wfd-app.h
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */


#ifndef __WFD_UG_APP_MAIN_H__
#define __WFD_UG_APP_MAIN_H__

#include <appcore-efl.h>
#include <Ecore_X.h>
#include <Elementary.h>
#include <appsvc.h>
#include <aul.h>


#define PACKAGE "org.tizen.wifi-direct-ugapp"
#define LOCALEDIR "/usr/apps/org.tizen.wifi-direct-ugapp"
#define DESKTOP_ICON "/usr/apps/org.tizen.setting/res/icons/A01-1_icon_Wi-Fi_direct.png"

typedef struct {
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *conform;
	Evas_Object *top_layout;
	Evas_Object *icon;
	ui_gadget_h wifi_direct_ug;
} wfd_appdata_t;

#endif  /* __WFD_UG_APP_MAIN_H__ */
