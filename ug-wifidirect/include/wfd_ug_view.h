/*
 * Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of ug-setting-wifidirect-efl
 * Written by Gibyoung Kim <lastkgb.kim@samsung.com>
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
