/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This file declares functions for view of Wi-Fi direct UI Gadget.
 *
 * @file    wfd_ug_view.h
 * @author  Gibyoung Kim (lastkgb.kim@samsung.com)
 * @version 0.1
 */


#ifndef __WFD_UG_VIEW_H__
#define __WFD_UG_VIEW_H__

#define EDJDIR "/opt/ug/res/edje/ug-setting-wifidirect-efl"
#define WFD_UG_EDJ_PATH  EDJDIR"/wfd_ug.edj"
#define WFD_IMG_DIR "/opt/ug/res/images/ug-setting-wifidirect-efl"

#define WFD_ICON_DEVICE_PC 		WFD_IMG_DIR"/A09_device_computer.png"
#define WFD_ICON_DEVICE_KEYBOARD 	WFD_IMG_DIR"/31_BT_device_keyboard.png"
#define WFD_ICON_DEVICE_PRINTER 	WFD_IMG_DIR"/31_BT_device_printer.png"
#define WFD_ICON_DEVICE_UNKNOWN 	WFD_IMG_DIR"/31_BT_device_unknown.png"
#define WFD_ICON_DEVICE_PHONE		WFD_IMG_DIR"/A09_device_mobile.png"
#define WFD_ICON_DEVICE_HEADSET 	WFD_IMG_DIR"/31_BT_device_headset.png"

#define WFD_ICON_DEVICE_MOUSE	WFD_IMG_DIR"/31_BT_device_mouse.png"

#define WFD_ICON_CONNECTED		WFD_IMG_DIR"/A09_Connect.png"


enum
{
    HEAD_TEXT_TYPE_DIRECT,
    HEAD_TEXT_TYPE_DEACTIVATING,
    HEAD_TEXT_TYPE_ACTIVATING,
    HEAD_TEXT_TYPE_ACTIVATED,
    HEAD_TEXT_TYPE_SCANING,
};

enum
{
    POPUP_TYPE_WIFI_OFF,
    POPUP_TYPE_HOTSPOT_OFF,

    POPUP_TYPE_ACTIVATE_FAIL,
    POPUP_TYPE_DEACTIVATE_FAIL,

    POPUP_TYPE_LINK_TIMEOUT,
    POPUP_TYPE_AUTH_FAIL,
    POPUP_TYPE_LINK_FAIL,
    POPUP_TYPE_UNKNOWN_ERROR,

    POPUP_TYPE_TERMINATE,
};

void create_wfd_ug_view(void *data);
void destroy_wfd_ug_view(void *data);
void wfd_ug_view_refresh_glitem(void *obj);
void wfd_ug_view_refresh_button(void *obj, int enable);
void wfd_ug_view_update_peers(void *data);
void wfd_ug_view_free_peers(void *data);
void wfd_ug_act_popup(void *data, const char *message, int popup_type);
void wfg_ug_act_popup_remove(void *data);
void wfd_ug_warn_popup(void *data, const char *message, int popup_type);
void wfg_ug_warn_popup_remove(void *data);

#endif                          /* __WFD_UG_VIEW_H__ */
