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


#include <libintl.h>

#include <assert.h>
#include <glib.h>

#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"


Elm_Gen_Item_Class sep_itc;
Elm_Gen_Item_Class head_itc;
Elm_Gen_Item_Class name_itc;
Elm_Gen_Item_Class title_itc;
Elm_Gen_Item_Class peer_itc;
Elm_Gen_Item_Class noitem_itc;
Elm_Gen_Item_Class help_itc;


void _back_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    if (!ugd)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

    wfd_ug_view_free_peers(ugd);

    ug_destroy_me(ugd->ug);

    __FUNC_EXIT__;
    return;
}

void _scan_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;
    int res;

    res = wfd_client_start_discovery(ugd);

    __FUNC_EXIT__;
    return;
}

Eina_Bool _is_wifi_on()
{
    __FUNC_ENTER__;
    int wifi_state;

    vconf_get_int(VCONFKEY_WIFI_STATE, &wifi_state);

    if (wifi_state >= VCONFKEY_WIFI_CONNECTED)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    __FUNC_EXIT__;
}

static void _wfd_onoff_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    ugd->wfd_status = wfd_client_get_link_status();
    DBG(LOG_VERBOSE, "WFD state is [%d]", ugd->wfd_status);
    if (ugd->wfd_status < 0)
    {
        DBG(LOG_VERBOSE, "bad wfd status\n");
        wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_ACTIVATE_FAIL"), POPUP_TYPE_ACTIVATE_FAIL);

        ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
        wfd_ug_view_refresh_glitem(ugd->head);
        return;
    }

    if (!ugd->wfd_onoff)
    {
        ugd->head_text_mode = HEAD_TEXT_TYPE_ACTIVATING;
        wfd_client_switch_on(ugd);
    }
    else
    {
        ugd->head_text_mode = HEAD_TEXT_TYPE_DEACTIVATING;
        wfd_client_switch_off(ugd);
    }
    wfd_ug_view_refresh_glitem(ugd->head);

    __FUNC_EXIT__;
}

static char *_gl_header_label_get(void *data, Evas_Object * obj,
                                  const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    DBG(LOG_VERBOSE, "%s", part);

    if (data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.text"))
    {
        DBG(LOG_VERBOSE, "Current text mode [%d]\n", ugd->head_text_mode);
        switch (ugd->head_text_mode)
        {
        case HEAD_TEXT_TYPE_DIRECT:
            return strdup(dgettext("sys_string", "IDS_COM_OPT1_WI_FI_DIRECT"));
            break;
        case HEAD_TEXT_TYPE_DEACTIVATING:
            return strdup(_("IDS_WFD_BODY_DEACTIVATING"));
            break;
        case HEAD_TEXT_TYPE_ACTIVATING:
            return strdup(_("IDS_WFD_BODY_ACTIVATING"));
            break;
        case HEAD_TEXT_TYPE_ACTIVATED:
            return strdup(_("IDS_WFD_BODY_ACTIVATED"));
            break;
        case HEAD_TEXT_TYPE_SCANING:
            return strdup(_("IDS_WFD_BODY_SCANNING"));
            break;
        default:
            break;
        }
    }

    __FUNC_EXIT__;
    return NULL;
}

static Evas_Object *_gl_header_icon_get(void *data, Evas_Object * obj,
                                        const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    Evas_Object *onoff = NULL;

    if (data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (ugd->head_text_mode == HEAD_TEXT_TYPE_ACTIVATING ||
        ugd->head_text_mode == HEAD_TEXT_TYPE_DEACTIVATING)
        return NULL;

    DBG(LOG_VERBOSE, "%s", part);
    onoff = elm_check_add(obj);
    elm_object_style_set(onoff, "on&off");
    elm_check_state_set(onoff, ugd->wfd_onoff);
    evas_object_smart_callback_add(onoff, "changed", _wfd_onoff_btn_cb, ugd);
    evas_object_show(onoff);

    __FUNC_EXIT__;

    return onoff;
}

static char *_gl_name_label_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;

    if (data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    DBG(LOG_VERBOSE, "%s", part);

    if (!strcmp(part, "elm.text.1"))
    {
        return strdup(_("IDS_WFD_BODY_DEVICE_NAME"));
    }
    else if (!strcmp(part, "elm.text.2"))
    {
        return strdup(ugd->dev_name);
    }

    __FUNC_EXIT__;

    return NULL;
}

static char *_gl_title_label_get(void *data, Evas_Object * obj,
                                 const char *part)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;

    if (data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.text"))
    {
        if (ugd->peer_cnt)
            return strdup(_("IDS_WFD_BODY_AVAILABLE_DEVICES"));
        else
            return strdup(_("IDS_WFD_BODY_WIFI_DIRECT_DEVICES"));
    }

    __FUNC_EXIT__;

    return NULL;
}

static Evas_Object *_gl_noitem_icon_get(void *data, Evas_Object * obj,
                                        const char *part)
{
    __FUNC_ENTER__;
    Evas_Object *nocontent;

    if (data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    nocontent = elm_layout_add(obj);
    if (nocontent == NULL)
    {
        DBG(LOG_ERROR, "Failed to add nocontent");
        return NULL;
    }
    elm_layout_theme_set(nocontent, "layout", "nocontents", "unnamed");
    elm_object_part_text_set(nocontent, "elm.text",
                             _("IDS_WFD_BODY_NO_DEVICES"));
    evas_object_size_hint_min_set(nocontent, 400, 200);
    evas_object_size_hint_max_set(nocontent, 400, 200);
    evas_object_resize(nocontent, 400, 200);

    __FUNC_EXIT__;

    return nocontent;
}

static void _gl_noitem_del(void *data, Evas_Object * obj)
{
    __FUNC_ENTER__;

    __FUNC_EXIT__;
    return;
}

static char *_gl_peer_label_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    assertm_if(NULL == obj, "NULL!!");
    assertm_if(NULL == part, "NULL!!");

    device_type_s *peer = (device_type_s *) data;

    if (data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    __FUNC_EXIT__;
    return strdup(peer->ssid);
}

static Evas_Object *_gl_peer_icon_get(void *data, Evas_Object * obj,
                                      const char *part)
{
    __FUNC_ENTER__;
    assertm_if(NULL == obj, "NULL!!");
    assertm_if(NULL == part, "NULL!!");

    device_type_s *peer = (device_type_s *) data;
    Evas_Object *icon = NULL;

    if (data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return NULL;
    }

    if (!strcmp(part, "elm.icon.2"))
    {
        DBG(LOG_VERBOSE, "elm.icon.2 - connection status [%d]\n",
            peer->conn_status);
        if (peer->conn_status == PEER_CONN_STATUS_CONNECTING)
        {
            icon = elm_progressbar_add(obj);
            elm_object_style_set(icon, "list_process");
            elm_progressbar_pulse(icon, EINA_TRUE);
        }
        else if (peer->conn_status == PEER_CONN_STATUS_CONNECTED)
        {
            icon = elm_icon_add(obj);
            elm_icon_file_set(icon, WFD_ICON_CONNECTED, NULL);
        }
        else
        {
            ;
        }
        evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1,
                                         1);
        elm_icon_resizable_set(icon, 1, 1);
        evas_object_show(icon);
    }
    else if (!strcmp(part, "elm.icon.1"))
    {
        DBG(LOG_VERBOSE, "elm.icon.1 - category [%d]\n", peer->category);
        char *img_path = NULL;
        switch (peer->category)
        {
        case WFD_DEVICE_TYPE_COMPUTER:
            img_path = WFD_ICON_DEVICE_PC;
            break;
        case WFD_DEVICE_TYPE_INPUT_DEVICE:
            img_path = WFD_ICON_DEVICE_KEYBOARD;
            break;
        case WFD_DEVICE_TYPE_PRINTER:
            img_path = WFD_ICON_DEVICE_PRINTER;
            break;
        case WFD_DEVICE_TYPE_CAMERA:
            img_path = WFD_ICON_DEVICE_UNKNOWN;
            break;
        case WFD_DEVICE_TYPE_STORAGE:
        case WFD_DEVICE_TYPE_NW_INFRA:
        case WFD_DEVICE_TYPE_DISPLAYS:
        case WFD_DEVICE_TYPE_MM_DEVICES:
        case WFD_DEVICE_TYPE_GAME_DEVICES:
        case WFD_DEVICE_TYPE_OTHER:
            img_path = WFD_ICON_DEVICE_UNKNOWN;
            break;
        case WFD_DEVICE_TYPE_TELEPHONE:
            img_path = WFD_ICON_DEVICE_PHONE;
            break;
        case WFD_DEVICE_TYPE_AUDIO:
            img_path = WFD_ICON_DEVICE_HEADSET;
            break;
        default:
            break;
        }

        if (img_path != NULL)
        {
            icon = elm_icon_add(obj);
            elm_icon_file_set(icon, img_path, NULL);
            evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL,
                                             1, 1);
            elm_icon_resizable_set(icon, 1, 1);
            evas_object_show(icon);
        }
    }

    __FUNC_EXIT__;
    return icon;
}

static void _gl_peer_del(void *data, Evas_Object * obj)
{
    __FUNC_ENTER__;
    assertm_if(NULL == obj, "NULL!!");
    assertm_if(NULL == data, "NULL!!");

    __FUNC_EXIT__;
    return;
}

static void _gl_peer_sel(void *data, Evas_Object * obj, void *event_info)
{
    __FUNC_ENTER__;
    assertm_if(NULL == obj, "NULL!!");
    assertm_if(NULL == data, "NULL!!");
    device_type_s *peer = (device_type_s *) data;
    Elm_Object_Item *item = (Elm_Object_Item *) event_info;
    int res;

    if (data == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)\n");
        return;
    }

    if (item != NULL)
        elm_genlist_item_selected_set(item, EINA_FALSE);

    if (peer->conn_status == PEER_CONN_STATUS_DISCONNECTED)
    {
        DBG(LOG_VERBOSE, "Connect with peer [%s]\n", peer->mac_addr);
        res = wfd_client_connect((const char *) peer->mac_addr);
        if (res != 0)
        {
            DBG(LOG_ERROR, "Failed to send connection request. [%d]\n", res);
            return;
        }
        peer->conn_status = PEER_CONN_STATUS_CONNECTING;
    }
    else
    {
        res = wfd_client_disconnect((const char *) peer->mac_addr);
        if (res != 0)
        {
            DBG(LOG_ERROR, "Failed to send disconnection request. [%d]\n", res);
            return;
        }
        peer->conn_status = PEER_CONN_STATUS_DISCONNECTED;
    }

    wfd_ug_view_refresh_glitem(peer->gl_item);

    __FUNC_EXIT__;
    return;
}

static char *_gl_help_label_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;
    DBG(LOG_VERBOSE, "%s", part);
    __FUNC_ENTER__;
    return strdup("Help");
}

static Evas_Object *_create_basic_genlist(void *data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data *) data;
    Evas_Object *genlist;

    genlist = elm_genlist_add(ugd->naviframe);

    sep_itc.item_style = "grouptitle.dialogue.seperator";
    sep_itc.func.text_get = NULL;
    sep_itc.func.content_get = NULL;
    sep_itc.func.state_get = NULL;
    sep_itc.func.del = NULL;

    head_itc.item_style = "dialogue/1text.1icon";
    head_itc.func.text_get = _gl_header_label_get;
    head_itc.func.content_get = _gl_header_icon_get;
    head_itc.func.state_get = NULL;

    name_itc.item_style = "dialogue/2text.3";
    name_itc.func.text_get = _gl_name_label_get;
    name_itc.func.content_get = NULL;
    name_itc.func.state_get = NULL;
    name_itc.func.del = NULL;

    Elm_Object_Item *item;
    elm_genlist_item_append(genlist, &sep_itc, NULL, NULL,
                            ELM_GENLIST_ITEM_NONE, NULL, NULL);
    ugd->head =
        elm_genlist_item_append(genlist, &head_itc, ugd, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, NULL);
    elm_genlist_item_select_mode_set(ugd->head,
                                     ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
    item =
        elm_genlist_item_append(genlist, &name_itc, ugd, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, NULL);
    elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    __FUNC_EXIT__;

    return genlist;
}

static int _create_device_genlist(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;

    title_itc.item_style = "dialogue/title";
    title_itc.func.text_get = _gl_title_label_get;
    title_itc.func.content_get = NULL;
    title_itc.func.state_get = NULL;
    title_itc.func.del = NULL;

    peer_itc.item_style = "dialogue/1text.2icon.2";
    peer_itc.func.text_get = _gl_peer_label_get;
    peer_itc.func.content_get = _gl_peer_icon_get;
    peer_itc.func.state_get = NULL;
    peer_itc.func.del = _gl_peer_del;

    noitem_itc.item_style = "dialogue/bg/1icon";
    noitem_itc.func.text_get = NULL;
    noitem_itc.func.content_get = _gl_noitem_icon_get;
    noitem_itc.func.state_get = NULL;
    noitem_itc.func.del = _gl_noitem_del;

    sep_itc.item_style = "grouptitle.dialogue.seperator";
    sep_itc.func.text_get = NULL;
    sep_itc.func.content_get = NULL;
    sep_itc.func.state_get = NULL;
    sep_itc.func.del = NULL;

    help_itc.item_style = "dialogue/1text";
    help_itc.func.text_get = _gl_help_label_get;
    help_itc.func.content_get = NULL;
    help_itc.func.state_get = NULL;
    help_itc.func.del = NULL;

    Elm_Object_Item *item;
    item =
        elm_genlist_item_append(ugd->genlist, &title_itc, (void *) ugd, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, NULL);
    elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
    ugd->noitem =
        elm_genlist_item_append(ugd->genlist, &noitem_itc, (void *) ugd, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, NULL);
    elm_genlist_item_select_mode_set(ugd->noitem,
                                     ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    __FUNC_EXIT__;
    return 0;
}

void create_wfd_ug_view(void *data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data *) data;
    Evas_Object *back_btn = NULL;
    Elm_Object_Item *navi_item = NULL;
    Evas_Object *control_bar = NULL;

    if (ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)");
        return;
    }

    ugd->naviframe = elm_naviframe_add(ugd->base);
    elm_object_part_content_set(ugd->base, "elm.swallow.content",
                                ugd->naviframe);
    evas_object_show(ugd->naviframe);

    back_btn = elm_button_add(ugd->naviframe);
    elm_object_style_set(back_btn, "naviframe/back_btn/default");
    evas_object_smart_callback_add(back_btn, "clicked", _back_btn_cb,
                                   (void *) ugd);
    elm_object_focus_allow_set(back_btn, EINA_FALSE);

    elm_theme_extension_add(NULL, WFD_UG_EDJ_PATH);

    ugd->genlist = _create_basic_genlist(ugd);
    if (ugd->genlist == NULL)
    {
        DBG(LOG_ERROR, "Failed to create basic genlist");
        return;
    }
    evas_object_show(ugd->genlist);
    _create_device_genlist(ugd);

    back_btn = elm_button_add(ugd->naviframe);
    elm_object_style_set(back_btn, "naviframe/back_btn/default");
    evas_object_smart_callback_add(back_btn, "clicked", _back_btn_cb,
                                   (void *) ugd);
    elm_object_focus_allow_set(back_btn, EINA_FALSE);

    navi_item =
        elm_naviframe_item_push(ugd->naviframe, _("IDS_WFD_HEADER_WIFI_DIRECT"),
                                back_btn, NULL, ugd->genlist, NULL);

    control_bar = elm_toolbar_add(ugd->naviframe);
    elm_toolbar_shrink_mode_set(control_bar, ELM_TOOLBAR_SHRINK_EXPAND);
    evas_object_show(control_bar);

    ugd->scan_btn =
        elm_toolbar_item_append(control_bar, NULL, _("IDS_WFD_BUTTON_SCAN"),
                                _scan_btn_cb, (void *) ugd);
    elm_object_item_disabled_set(ugd->scan_btn, !ugd->wfd_onoff);

    elm_object_item_part_content_set(navi_item, "controlbar", control_bar);

    __FUNC_EXIT__;
}

void destroy_wfd_ug_view(void *data)
{
    __FUNC_ENTER__;

    struct ug_data *ugd = (struct ug_data *) data;

    if (ugd->genlist)
    {
        evas_object_del(ugd->genlist);
        ugd->genlist = NULL;
    }

    if (ugd->naviframe)
    {
        evas_object_del(ugd->naviframe);
        ugd->naviframe = NULL;
    }

    __FUNC_EXIT__;
}

void wfd_ug_view_refresh_glitem(void *obj)
{
    __FUNC_ENTER__;
    elm_genlist_item_update(obj);
    __FUNC_EXIT__;
}

void wfd_ug_view_refresh_button(void *obj, int enable)
{
    __FUNC_ENTER__;
    DBG(LOG_VERBOSE, "scan button is enabling. [%d]\n", enable);
    elm_object_item_disabled_set(obj, !enable);
    __FUNC_EXIT__;
}

void wfd_ug_view_update_peers(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    int i;

    DBG(LOG_VERBOSE, "peer count [%d], peer instance [%x]\n", ugd->peer_cnt,
        ugd->peers);

    if (ugd->peer_cnt == 0)
    {
        DBG(LOG_ERROR, "There are No peers\n");
        if (ugd->noitem == NULL)
            ugd->noitem =
                elm_genlist_item_append(ugd->genlist, &noitem_itc, (void *) ugd,
                                        NULL, ELM_GENLIST_ITEM_NONE, NULL,
                                        NULL);
        return;
    }
    else if (ugd->peer_cnt > 0)
    {
        if (ugd->noitem)
        {
            elm_object_item_del(ugd->noitem);
            ugd->noitem = NULL;
            DBG(LOG_VERBOSE, "Noitem list is removed\n");
        }

        for (i = 0; i < ugd->peer_cnt; i++)
        {
            DBG(LOG_VERBOSE, "%dth peer being added on genlist\n", i);
            ugd->peers[i].gl_item =
                elm_genlist_item_append(ugd->genlist, &peer_itc,
                                        (void *) &(ugd->peers[i]), NULL,
                                        ELM_GENLIST_ITEM_NONE, _gl_peer_sel,
                                        (void *) &(ugd->peers[i]));
        }
    }

    __FUNC_EXIT__;
}

void wfd_ug_view_free_peers(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    int i;

    for (i = 0; i < ugd->peer_cnt; i++)
    {
        DBG(LOG_VERBOSE, "%dth peer is deleted\n", i);
        elm_object_item_del(ugd->peers[i].gl_item);
    }

    if (ugd->peer_cnt > 0 && ugd->peers != NULL)
    {
        DBG(LOG_VERBOSE, "peers will be destroyed\n");
        free(ugd->peers);
        ugd->peers = NULL;
        ugd->peer_cnt = 0;
    }

    __FUNC_EXIT__;
}

static void _wfd_ug_act_popup_wifi_ok_cb(void *data, Evas_Object * obj,
                                         void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;

    ugd->wfd_status = WFD_LINK_STATUS_DEACTIVATED;
    wfd_wifi_off();

    evas_object_del(ugd->act_popup);
    ugd->act_popup = NULL;
    __FUNC_EXIT__;
}

static void _wfd_ug_act_popup_wifi_cancel_cb(void *data, Evas_Object * obj,
                                             void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;

    ugd->head_text_mode = HEAD_TEXT_TYPE_DIRECT;
    wfd_ug_view_refresh_glitem(ugd->head);

    evas_object_del(ugd->act_popup);
    ugd->act_popup = NULL;
    __FUNC_EXIT__;
}

void wfd_ug_act_popup(void *data, const char *message, int popup_type)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    Evas_Object *popup = NULL;
    Evas_Object *btn1 = NULL, *btn2 = NULL;

    popup = elm_popup_add(ugd->base);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, message);

    btn1 = elm_button_add(popup);
    elm_object_text_set(btn1, S_("IDS_COM_SK_YES"));
    elm_object_part_content_set(popup, "button1", btn1);
    evas_object_smart_callback_add(btn1, "clicked",
                                   _wfd_ug_act_popup_wifi_ok_cb, (void *) ugd);

    btn2 = elm_button_add(popup);
    elm_object_text_set(btn2, S_("IDS_COM_SK_NO"));
    elm_object_part_content_set(popup, "button2", btn2);
    evas_object_smart_callback_add(btn2, "clicked",
                                   _wfd_ug_act_popup_wifi_cancel_cb,
                                   (void *) ugd);

    evas_object_show(popup);
    ugd->act_popup = popup;
    __FUNC_EXIT__;
}

void wfg_ug_act_popup_remove(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;

    if (ugd->act_popup)
    {
        evas_object_del(ugd->act_popup);
        ugd->act_popup = NULL;
    }
    __FUNC_EXIT__;
}

static void _wfd_ug_terminate_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    evas_object_del(ugd->warn_popup);
    ugd->warn_popup = NULL;

    wfd_ug_view_free_peers(ugd);

    ug_destroy_me(ugd->ug);
    __FUNC_EXIT__;
}

static void _wfd_ug_warn_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    evas_object_del(ugd->warn_popup);
    ugd->warn_popup = NULL;
    __FUNC_EXIT__;
}

void wfd_ug_warn_popup(void *data, const char *message, int popup_type)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;
    Evas_Object *popup = NULL;
    Evas_Object *btn = NULL;

    popup = elm_popup_add(ugd->base);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, message);

    btn = elm_button_add(popup);
    elm_object_text_set(btn, S_("IDS_COM_SK_OK"));
    elm_object_part_content_set(popup, "button1", btn);
    if(popup_type == POPUP_TYPE_TERMINATE)
        evas_object_smart_callback_add(btn, "clicked", _wfd_ug_terminate_popup_cb, (void*) ugd);
    else
        evas_object_smart_callback_add(btn, "clicked", _wfd_ug_warn_popup_cb, (void*) ugd);

    evas_object_show(popup);
    ugd->warn_popup = popup;
    __FUNC_EXIT__;
}

void wfg_ug_warn_popup_remove(void *data)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data *) data;

    if (ugd->warn_popup)
    {
        evas_object_del(ugd->warn_popup);
        ugd->warn_popup = NULL;
    }
    __FUNC_EXIT__;
}
