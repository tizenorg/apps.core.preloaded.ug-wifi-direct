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
 * This file implements wifi direct system popup view  functions.
 *
 * @file    wfd-app-popup-view.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <libintl.h>
#include <glib.h>

#include "wifi-direct.h"
#include "wfd-app.h"
#include "wfd-app-strings.h"
#include "wfd-app-util.h"

extern wfd_appdata_t *g_wfd_ad;
extern wfd_popup_t *g_wfd_pop;
extern unsigned char g_wfd_peer_mac[6];
extern unsigned char g_wfd_peer_name[32];

static void __popup_resp_cb(void *data, Evas_Object * obj, void *event_info)
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = wfd_get_appdata();
    int result = -1;
    int resp = (int) data;
    Evas_Object *btn = obj;

    WFD_APP_LOG(WFD_APP_LOG_HIGH, "popup resp : %d\n", resp);

    switch (resp)
    {
    case WFD_POP_RESP_APRV_CONNECT_PBC_YES:
        {
            WFD_APP_LOG(WFD_APP_LOG_HIGH,
                        "WFD_POP_RESP_APRV_CONNECT_PBC_YES\n");

            result = wifi_direct_accept_connection(ad->peer_mac);
            WFD_APP_LOG(WFD_APP_LOG_LOW,
                        "wifi_direct_accept_connection() result=[%d]\n",
                        result);
            if (result == WIFI_DIRECT_ERROR_NONE)
            {
                wfd_prepare_popup(WFD_POP_PROG_CONNECT, NULL);
            }
            else
            {
                WFD_APP_LOG(WFD_APP_LOG_ERROR,
                            "wifi_direct_accept_connection() FAILED!!\n");
                evas_object_hide(ad->win);
            }
        }
        break;

    case WFD_POP_RESP_APRV_CONNECT_DISPLAY_YES:
        {
            WFD_APP_LOG(WFD_APP_LOG_HIGH,
                        "WFD_POP_RESP_APRV_CONNECT_DISPLAY_YES\n");

            result = wifi_direct_accept_connection(ad->peer_mac);
            WFD_APP_LOG(WFD_APP_LOG_LOW,
                        "wifi_direct_accept_connection() result=[%d]\n",
                        result);
            if (result == WIFI_DIRECT_ERROR_NONE)
            {
                wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN,
                                  ad->pin_number);
            }
            else
            {
                WFD_APP_LOG(WFD_APP_LOG_ERROR,
                            "wifi_direct_client_send_connect_request() FAILED!!\n");
                evas_object_hide(ad->win);
            }
        }
        break;

    case WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK:
    case WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES:
        {
            WFD_APP_LOG(WFD_APP_LOG_HIGH,
                        "WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES\n");

            int len = strlen(ad->pin_number);
            WFD_APP_LOG(WFD_APP_LOG_LOW, "button ok: pin [%s]", ad->pin_number);

            if (len > 7 && len < 64)
            {
                int result = 0;
                WFD_APP_LOG(WFD_APP_LOG_LOW, "pin=[%s]\n", ad->pin_number);

                result = wifi_direct_set_wps_pin(ad->pin_number);

                if (result != WIFI_DIRECT_ERROR_NONE)
                {
                    wfd_prepare_popup(WFD_POP_FAIL_CONNECT, NULL);
                    return;
                }

                result = wifi_direct_activate_pushbutton();
                result = wifi_direct_accept_connection(ad->peer_mac);
                WFD_APP_LOG(WFD_APP_LOG_LOW,
                            "wifi_direct_accept_connection(%s) result=[%d]\n",
                            ad->peer_mac, result);
                if (result == WIFI_DIRECT_ERROR_NONE)
                {
                    wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN,
                                      ad->pin_number);
                }
                else
                {
                    WFD_APP_LOG(WFD_APP_LOG_ERROR,
                                "wifi_direct_accept_connection() FAILED!!\n");
                    evas_object_hide(ad->win);
                }
            }
            else
            {
                WFD_APP_LOG(WFD_APP_LOG_ERROR, "Error, Incorrect PIN!!\n");
                wfd_prepare_popup(WFD_POP_INCORRECT_PIN, NULL);
                return;
            }
        }
        break;

    case WFD_POP_RESP_APRV_CONNECT_NO:
        {
            WFD_APP_LOG(WFD_APP_LOG_HIGH,
                        "WFD_POP_RESP_APRV_CONNECT_NO: destroy_popup...\n");

            wfd_destroy_popup();
        }
        break;

    case WFD_POP_RESP_PROG_CONNECT_CANCEL:
        {
            WFD_APP_LOG(WFD_APP_LOG_HIGH, "WFD_POP_RESP_PROG_CONNECT_CANCEL\n");
            result = wifi_direct_disconnect(ad->peer_mac);
            WFD_APP_LOG(WFD_APP_LOG_LOW,
                        "wifi_direct_disconnect[%s] result=[%d]\n",
                        ad->peer_mac, result);

            if (result == WIFI_DIRECT_ERROR_NONE)
            {
                wfd_prepare_popup(WFD_POP_PROG_CONNECT_CANCEL, NULL);
            }
            else
            {
                WFD_APP_LOG(WFD_APP_LOG_ERROR,
                            "wifi_direct_disconnect() FAILED!!\n");
                wfd_prepare_popup(WFD_POP_FAIL_CONNECT, NULL);
            }
        }
        break;

    default:
        {
            WFD_APP_LOG(WFD_APP_LOG_ERROR, "Unknown respone\n");
            evas_object_hide(ad->win);
        }
        break;
    }

    __WFD_APP_FUNC_EXIT__;
}

static Evas_Object *__create_progress_layout(Evas_Object * parent,
                                             const char *text)
{
    __WFD_APP_FUNC_ENTER__;

    if (parent == NULL || text == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "param is NULL\n");
        return NULL;
    }

    Evas_Object *progressbar = NULL, *layout = NULL;
    Evas_Object *label = NULL;
    int w = 0, h = 0;

    layout = elm_layout_add(parent);
    if (layout == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "layout is NULL\n");
        return NULL;
    }

    elm_layout_file_set(layout, EDJ_NAME, "progress_popup");
    evas_object_size_hint_weight_set(layout,
                                     EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    progressbar = elm_progressbar_add(layout);
    if (progressbar == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "progressbar is NULL\n");
        evas_object_del(layout);
        return NULL;
    }
    elm_object_style_set(progressbar, "list_process");
    evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
    evas_object_size_hint_weight_set(progressbar,
                                     EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(layout, "popup_pb", progressbar);
    elm_progressbar_pulse(progressbar, EINA_TRUE);

    label = elm_label_add(layout);
    if (label == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "label is NULL\n");
        evas_object_del(layout);
        evas_object_del(progressbar);
        return NULL;
    }
    elm_object_style_set(label, "popup_description/default");
    elm_object_part_content_set(layout, "popup_progress_text", label);
    edje_object_part_geometry_get(layout, "popup_progress_text", NULL, NULL, &w,
                                  &h);
    elm_label_line_wrap_set(label, ELM_WRAP_WORD);
    elm_label_wrap_width_set(label, w);
    elm_object_text_set(label, text);

    evas_object_show(layout);

    __WFD_APP_FUNC_EXIT__;

    return layout;
}


void wfd_destroy_popup()
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = wfd_get_appdata();

    if (ad == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "ad is NULL\n");
        return;
    }

    if (ad->popup)
    {
        evas_object_smart_callback_del(ad->popup, "response", __popup_resp_cb);
        evas_object_del(ad->popup);
        ad->popup = NULL;
    }

    if (ad->popup_timeout_handle > 0)
    {
        g_source_remove(ad->popup_timeout_handle);
        ad->popup_timeout_handle = 0;
    }

    evas_object_hide(ad->win);

    __WFD_APP_FUNC_EXIT__;
    return;
}

static Evas_Object *wfd_draw_pop_type_a(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, pop->text);
    elm_popup_timeout_set(popup, pop->timeout);
    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

static Evas_Object *wfd_draw_pop_type_b(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup = NULL;
    Evas_Object *btn = NULL;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, pop->text);

    btn = elm_button_add(popup);
    elm_object_text_set(btn, pop->label1);
    elm_object_part_content_set(popup, "button1", btn);
    evas_object_smart_callback_add(btn, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data1);

    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

static Evas_Object *wfd_draw_pop_type_c(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup = NULL;
    Evas_Object *btn1 = NULL, *btn2 = NULL;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, pop->text);

    btn1 = elm_button_add(popup);
    elm_object_text_set(btn1, pop->label1);
    elm_object_part_content_set(popup, "button1", btn1);
    evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data1);

    btn2 = elm_button_add(popup);
    elm_object_text_set(btn2, pop->label2);
    elm_object_part_content_set(popup, "button2", btn2);
    evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data2);

    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

static Evas_Object *wfd_draw_pop_type_d(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;

    Evas_Object *popup;
    Evas_Object *layout;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_style_set(popup, "customstyle");
    layout = __create_progress_layout(popup, pop->text);
    elm_popup_timeout_set(popup, pop->timeout);
    elm_object_content_set(popup, layout);
    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}

static void __popup_block_clicked_cb(void *data, Evas_Object * obj,
                                     void *event_info)
{
    wfd_appdata_t *ad = wfd_get_appdata();
    wfd_popup_t *pop = NULL;

    if (ad != NULL)
        pop = ad->popup;

    if (ad->win != NULL)
        evas_object_hide(ad->win);

    if (ad->popup_timeout_handle > 0)
    {
        g_source_remove(ad->popup_timeout_handle);
        ad->popup_timeout_handle = 0;
    }
}

gboolean __popup_remove_timeout_cb(gpointer user_data)
{
    wfd_appdata_t *ad = wfd_get_appdata();
    wfd_popup_t *pop = NULL;

    if (ad != NULL)
        pop = ad->popup;
    else
        return false;

    if (pop != user_data)
        return false;

    if (ad->win != NULL)
        evas_object_hide(ad->win);

    ad->popup_timeout_handle = 0;
    return false;
}

static Evas_Object *wfd_draw_pop_type_e(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = wfd_get_appdata();
    Evas_Object *popup;

    if (ad == NULL)
        return NULL;

    popup = elm_popup_add(win);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, pop->text);
    evas_object_smart_callback_add(popup, "block,clicked",
                                   __popup_block_clicked_cb, NULL);
    ad->popup_timeout_handle =
        g_timeout_add(3000, __popup_remove_timeout_cb, popup);
    evas_object_show(popup);
    evas_object_show(win);

    __WFD_APP_FUNC_EXIT__;
    return popup;
}


static void _smart_ime_cb(void *data, Evas_Object * obj, void *event_info)
{
    __WFD_APP_FUNC_ENTER__;
    wfd_appdata_t *ad = wfd_get_appdata();

    Ecore_IMF_Context *imf_context = NULL;
    imf_context = (Ecore_IMF_Context *) ad->pin_entry;

    if (NULL == imf_context)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "Error!!! Ecore_IMF_Context is NULL!!");
        return;
    }

    const char *txt =
        elm_entry_markup_to_utf8(elm_entry_entry_get
                                 ((const Evas_Object *) imf_context));

    if (NULL != txt)
    {
        WFD_APP_LOG(WFD_APP_LOG_LOW, "* text [%s], len=[%d]", txt, strlen(txt));
        strncpy(ad->pin_number, txt, sizeof(ad->pin_number));
    }
    else
    {
        WFD_APP_LOG(WFD_APP_LOG_LOW, "Err!");
    }

    __WFD_APP_FUNC_EXIT__;
}

Evas_Object *wfd_draw_pop_type_keypad(Evas_Object * win, wfd_popup_t * pop)
{
    __WFD_APP_FUNC_ENTER__;
    wfd_appdata_t *ad = wfd_get_appdata();

    Evas_Object *conformant = NULL;
    Evas_Object *layout = NULL;
    Evas_Object *pinpopup = NULL;
    Evas_Object *btn1 = NULL, *btn2 = NULL;

    conformant = elm_conformant_add(win);
    assertm_if(NULL == conformant, "conformant is NULL!!");
    elm_win_conformant_set(win, EINA_TRUE);
    elm_win_resize_object_add(win, conformant);
    evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND,
                                     EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(conformant);

    pinpopup = NULL;
    layout = elm_layout_add(conformant);
    elm_object_content_set(conformant, layout);
    pinpopup = elm_popup_add(layout);
    assertm_if(NULL == pinpopup, "pinpopup is NULL!!");
    elm_object_part_text_set(pinpopup, "title,text", pop->text);

    Evas_Object *box = elm_box_add(pinpopup);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(box);

    Evas_Object *editfield = elm_layout_add(box);
    elm_layout_theme_set(editfield, "layout", "editfield", "default");
    Evas_Object *editfield_entry = elm_layout_add(box);
    elm_object_part_content_set(editfield, "elm.swallow.content",
                                editfield_entry);
    elm_object_part_text_set(editfield, "elm.text", _("Enter PIN"));
    elm_entry_single_line_set(editfield_entry, EINA_TRUE);
    elm_entry_scrollable_set(editfield_entry, EINA_TRUE);
    elm_object_signal_emit(editfield_entry, "elm,state,eraser,show", "elm");
    evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND,
                                     EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, EVAS_HINT_FILL);
    ad->pin_entry = editfield_entry;
    evas_object_smart_callback_add(ad->pin_entry, "changed", _smart_ime_cb,
                                   NULL);
    evas_object_show(editfield);
    elm_box_pack_end(box, editfield);

    elm_object_content_set(pinpopup, box);

    btn1 = elm_button_add(pinpopup);
    elm_object_text_set(pinpopup, pop->label1);
    elm_object_part_content_set(pinpopup, "button1", btn1);
    evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data1);

    btn2 = elm_button_add(pinpopup);
    elm_object_text_set(btn2, pop->label2);
    elm_object_part_content_set(pinpopup, "button2", btn2);
    evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
                                   (void *) pop->resp_data2);

    evas_object_show(pinpopup);
    evas_object_show(win);
    elm_object_focus_set(ad->pin_entry, EINA_TRUE);

    __WFD_APP_FUNC_EXIT__;

    return pinpopup;
}

void wfd_prepare_popup(int type, void *userdata)
{
    __WFD_APP_FUNC_ENTER__;
    wfd_appdata_t *ad = wfd_get_appdata();
    wfd_popup_t *pop = ad->popup_data;

    wfd_destroy_popup();

    memset(pop, 0, sizeof(wfd_popup_t));

    pop->type = type;

    switch (pop->type)
    {

    case WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ:
        {
            snprintf(pop->text, sizeof(pop->text), _("IDS_WFD_POP_CONNECT_Q"),
                     ad->peer_name, 120);
            snprintf(pop->label1, sizeof(pop->label1), "%s",
                     dgettext("sys_string", "IDS_COM_SK_YES"));
            snprintf(pop->label2, sizeof(pop->label2), "%s",
                     dgettext("sys_string", "IDS_COM_SK_NO"));
            pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_PBC_YES;
            pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

            ad->popup = wfd_draw_pop_type_c(ad->win, pop);
        }
        break;

    case WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ:
        {
            char *pin = (char *) userdata;
            snprintf(pop->text, sizeof(pop->text),
                     _("IDS_WFD_POP_CONNECT_WITH_PIN_Q"), ad->peer_name, 120,
                     ad->pin_number);
            snprintf(pop->label1, sizeof(pop->label1), "%s",
                     dgettext("sys_string", "IDS_COM_SK_YES"));
            snprintf(pop->label2, sizeof(pop->label2), "%s",
                     dgettext("sys_string", "IDS_COM_SK_NO"));

            pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_DISPLAY_YES;
            pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

            ad->popup = wfd_draw_pop_type_c(ad->win, pop);
        }
        break;

    case WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ:
        {
            char *pin = (char *) userdata;
            snprintf(pop->text, sizeof(pop->text), _("IDS_WFD_POP_CONNECT_Q"),
                     ad->peer_name, 120);
            snprintf(pop->label1, sizeof(pop->label1), "%s",
                     dgettext("sys_string", "IDS_COM_SK_YES"));
            snprintf(pop->label2, sizeof(pop->label2), "%s",
                     dgettext("sys_string", "IDS_COM_SK_NO"));
            pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES;
            pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

            ad->popup = wfd_draw_pop_type_keypad(ad->win, pop);
        }
        break;

    case WFD_POP_PROG_CONNECT:
        {
            snprintf(pop->text, sizeof(pop->text), "%s",
                     _("IDS_WFD_POP_CONNECTING"));
            snprintf(pop->label1, sizeof(pop->label1), "%s",
                     dgettext("sys_string", "IDS_COM_POP_CANCEL"));
            pop->timeout = WFD_POP_TIMER_120;
            pop->resp_data1 = WFD_POP_RESP_PROG_CONNECT_CANCEL;

            ad->popup = wfd_draw_pop_type_b(ad->win, pop);
        }
        break;

    case WFD_POP_PROG_CONNECT_WITH_KEYPAD:
        {
            snprintf(pop->text, sizeof(pop->text), "%s",
                     _("IDS_WFD_POP_ENTER_PIN"));
            snprintf(pop->label1, sizeof(pop->label1), "%s",
                     dgettext("sys_string", "IDS_COM_SK_OK"));
            snprintf(pop->label2, sizeof(pop->label2), "%s",
                     dgettext("sys_string", "IDS_COM_POP_CANCEL"));
            pop->timeout = WFD_POP_TIMER_120;
            pop->resp_data1 = WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK;
            pop->resp_data2 = WFD_POP_RESP_PROG_CONNECT_CANCEL;

            ad->popup = wfd_draw_pop_type_keypad(ad->win, pop);
        }
        break;

    case WFD_POP_PROG_CONNECT_WITH_PIN:
        snprintf(pop->text, sizeof(pop->text), "%s %s",
                 _("IDS_WFD_POP_CONNECTING_WITH_PIN"), ad->pin_number);
        snprintf(pop->label1, sizeof(pop->label1), "%s",
                 dgettext("sys_string", "IDS_COM_POP_CANCEL"));
        pop->timeout = WFD_POP_TIMER_120;
        pop->resp_data1 = WFD_POP_RESP_PROG_CONNECT_CANCEL;

        ad->popup = wfd_draw_pop_type_c(ad->win, pop);
        break;

    case WFD_POP_PROG_CONNECT_CANCEL:
        {
            snprintf(pop->text, sizeof(pop->text), "%s",
                     dgettext("sys_string", "IDS_COM_POP_CANCEL"));
            pop->timeout = WFD_POP_TIMER_120;
            ad->popup = wfd_draw_pop_type_a(ad->win, pop);
        }
        break;

    case WFD_POP_INCORRECT_PIN:
        snprintf(pop->text, sizeof(pop->text), "%s",
                 _("IDS_WFD_POP_PIN_INVALID"));
        snprintf(pop->label1, sizeof(pop->label1), "%s",
                 dgettext("sys_string", "IDS_COM_SK_OK"));
        pop->timeout = WFD_POP_TIMER_3;
        pop->resp_data1 = WFD_POP_RESP_OK;
        break;

    case WFD_POP_NOTI_CONNECTED:
        snprintf(pop->text, sizeof(pop->text), "%s",
                 _("IDS_WFD_POP_CONNECTED"));
        pop->timeout = WFD_POP_TIMER_3;

        ad->popup = wfd_draw_pop_type_e(ad->win, pop);
        break;

    case WFD_POP_FAIL_CONNECT:
        snprintf(pop->text, sizeof(pop->text), "%s",
                 _("IDS_WFD_POP_ERROR_OCCURRED"));
        pop->timeout = WFD_POP_TIMER_3;

        ad->popup = wfd_draw_pop_type_e(ad->win, pop);
        break;

    default:
        break;
    }

    __WFD_APP_FUNC_EXIT__;
    return;
}
