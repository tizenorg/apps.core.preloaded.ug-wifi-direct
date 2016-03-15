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
 * This file implements wifi direct system popup view  functions.
 *
 * @file    wfd-app-popup-view.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <libintl.h>
#include <glib.h>

#include <Elementary.h>
#include <vconf.h>
#include <notification.h>
#include <feedback.h>
#include <efl_extension.h>

#include "wfd-app.h"
#include "wfd-app-strings.h"
#include "wfd-app-util.h"
#include "wfd-app-popup-view.h"

extern wfd_appdata_t *g_wfd_ad;
extern wfd_popup_t *g_wfd_pop;
extern unsigned char g_wfd_peer_mac[6];
extern unsigned char g_wfd_peer_name[32];
static Ecore_Timer *pb_timer = NULL;
static Ecore_Timer *keypad_popup_timer = NULL;
static int keypad_popup_timeout = 0;

void _replace_1PS_2PD(char *buf, int buf_len, char *format_str, char* SD_1, int PD_2);

static void mouseup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	if (ev->button == 3) {
		evas_object_del(obj);
	}
}

/*static void keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Key_Down *ev = event_info;
	if (!strcmp(ev->keyname, KEY_BACK)) {
		evas_object_del(obj);
	}
}*/

/**
 *	This function let the ug make a callback for click the button in popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void __popup_resp_cb(void *data, Evas_Object * obj, void *event_info)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();
	wfd_connection_info_s *connection = ad->connection;
	int result = -1;
	int resp = (int) data;
	char msg[WFD_POP_STR_MAX_LEN] = {0};

	WFD_APP_LOG(WFD_APP_LOG_HIGH, "popup resp : %d\n", resp);

	switch (resp) {
	case /* MT */ WFD_POP_RESP_APRV_CONNECT_PBC_YES:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_APRV_CONNECT_PBC_YES\n");
		wfd_destroy_popup();

		result = wifi_direct_accept_connection(connection->peer_addr);
		if (result != WIFI_DIRECT_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to accept connection(%d)", result);
			evas_object_hide(ad->win);

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WIFI_POP_FAILED_TO_CONNECT_TO_PS"),
							connection->peer_name);
			notification_status_message_post(msg);
		}
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Succeeded to accept connection");
	}
	break;

	case /* MT */ WFD_POP_RESP_APRV_CONNECT_DISPLAY_OK:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_APRV_CONNECT_DISPLAY_OK\n");
		wfd_destroy_popup();

		result = wifi_direct_accept_connection(connection->peer_addr);
		if (result == WIFI_DIRECT_ERROR_NONE) {
			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_PIN, NULL);
		} else {
			WFD_APP_LOG(WFD_APP_LOG_LOW,
				"wifi_direct_accept_connection() failed. result=[%d]\n", result);
			/* tickernoti popup */
			notification_status_message_post(_("IDS_WIFI_POP_FAILED_TO_CONNECT_TO_PS"));
		}
	}
	break;

	case /* MO */ WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK:
	{
		int result = 0;
		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK\n");

		wfd_destroy_popup();

		int len = strlen(connection->wps_pin);
		WFD_APP_LOGSECURE(WFD_APP_LOG_LOW, "PIN [%s]", connection->wps_pin);

		if (len < 8 || len > 64) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Error, Incorrect PIN!!\n");
			keypad_popup_timeout--;
			/* tickernoti popup */
			notification_status_message_post(_("IDS_COM_BODY_PINS_DO_NOT_MATCH"));

			/* redraw the popup */
			wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD, (void *) NULL);
			break;
		}
		keypad_popup_timeout = 0;

		result = wifi_direct_set_wps_pin(connection->wps_pin);
		if (result != WIFI_DIRECT_ERROR_NONE) {
			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WIFI_POP_FAILED_TO_CONNECT_TO_PS"),
							connection->peer_name);
			notification_status_message_post(msg);
			return;
		}

		result = wifi_direct_accept_connection(connection->peer_addr);
		if (result != WIFI_DIRECT_ERROR_NONE) {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to accept connection(%d)", result);
			evas_object_hide(ad->win);

			/* tickernoti popup */
			snprintf(msg, WFD_POP_STR_MAX_LEN, _("IDS_WIFI_POP_FAILED_TO_CONNECT_TO_PS"),
							connection->peer_name);
			notification_status_message_post(msg);
		}
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Succeeded to connect with [%s]", connection->peer_addr);
	}
	break;

	case /* MT */ WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES\n");
		wfd_destroy_popup();
		if (pb_timer) {
			ecore_timer_del(pb_timer);
			pb_timer = NULL;
		}

		wfd_prepare_popup(WFD_POP_PROG_CONNECT_WITH_KEYPAD, (void *) NULL);
	}
	break;

	case /* MT */ WFD_POP_RESP_APRV_CONNECT_NO:
	{
		WFD_APP_LOG(WFD_APP_LOG_HIGH,
				"WFD_POP_RESP_APRV_CONNECT_NO: destroy_popup...\n");

		if (connection->peer_addr[0] != '\0') {
			result = wifi_direct_reject_connection(connection->peer_addr);
			if (result != WIFI_DIRECT_ERROR_NONE)
				WFD_APP_LOG(WFD_APP_LOG_ERROR, "Failed to reject connection(%d)", result);
		} else {
			WFD_APP_LOG(WFD_APP_LOG_ERROR, "Peer's address is Zero MAC");
		}

		if (pb_timer) {
			ecore_timer_del(pb_timer);
			pb_timer = NULL;
		}

		wfd_destroy_popup();
		keypad_popup_timeout = 0;
	}
	break;

	default:
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Unknown respone\n");
		wfd_destroy_popup();
	}
	break;
	}

	__WFD_APP_FUNC_EXIT__;
}

/**
 *	This function let the app destroy the popup
 *	@return   void
 *	@param[in] null
 */
void wfd_destroy_popup()
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();

	if (ad == NULL) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "ad is NULL\n");
		return;
	}

	if (keypad_popup_timer) {
		ecore_timer_del(keypad_popup_timer);
		keypad_popup_timer = NULL;
	}

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	if (ad->popup_timeout_handle > 0) {
		g_source_remove(ad->popup_timeout_handle);
		ad->popup_timeout_handle = 0;
	}

	if (ad->win) {
		evas_object_hide(ad->win);
	}

	if (pb_timer) {
		ecore_timer_del(pb_timer);
		pb_timer = NULL;
	}
	__WFD_APP_FUNC_EXIT__;
	return;
}

/**
 *	This function let the app create a popup which includes no button
 *	@return   popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
static Evas_Object *wfd_draw_pop_type_a(Evas_Object * win, wfd_popup_t * pop)
{
	__WFD_APP_FUNC_ENTER__;

	wfd_appdata_t *ad = wfd_get_appdata();
	Evas_Object *popup;

	popup = elm_popup_add(win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_resp_cb, NULL);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, mouseup_cb, ad);
//	evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, keydown_cb, ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_text_set(popup, PACKAGE, pop->text);
	elm_popup_timeout_set(popup, pop->timeout);

	evas_object_show(popup);
	evas_object_show(win);

	__WFD_APP_FUNC_EXIT__;
	return popup;
}

/**
 *	This function let the app create a popup which includes one button
 *	@return   popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
static Evas_Object *wfd_draw_pop_type_b(Evas_Object * win, wfd_popup_t * pop)
{
	__WFD_APP_FUNC_ENTER__;
	Evas_Object *popup = NULL;
	Evas_Object *btn = NULL;
	wfd_appdata_t *ad = wfd_get_appdata();

	popup = elm_popup_add(win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_resp_cb, (void *) pop->resp_data1);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, mouseup_cb, ad);
//	evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, keydown_cb, ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_text_set(popup, PACKAGE, pop->text);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_domain_translatable_text_set(btn, PACKAGE, pop->label1);
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", __popup_resp_cb, (void *) pop->resp_data1);

	evas_object_show(popup);
	evas_object_show(win);

	__WFD_APP_FUNC_EXIT__;
	return popup;
}

/**
 *	This function let the app create a popup which includes two buttons
 *	@return   popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
static Evas_Object *wfd_draw_pop_type_c(Evas_Object * win, wfd_popup_t * pop)
{
	__WFD_APP_FUNC_ENTER__;
	Evas_Object *popup = NULL;
	Evas_Object *btn1 = NULL, *btn2 = NULL;
	wfd_appdata_t *ad = wfd_get_appdata();

	popup = elm_popup_add(win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_resp_cb, (void *) pop->resp_data2);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, mouseup_cb, ad);
//	evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, keydown_cb, ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_part_text_set(popup, "title,text",
			PACKAGE, _("IDS_WIFI_HEADER_WI_FI_DIRECT_CONNECTION_ABB"));
	elm_object_domain_translatable_text_set(popup, PACKAGE, pop->text);

	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_domain_translatable_text_set(btn1, PACKAGE, pop->label2);
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
		(void *) pop->resp_data2);

	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_domain_translatable_text_set(btn2, PACKAGE, pop->label1);
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb,
		(void *) pop->resp_data1);

	evas_object_show(popup);
	evas_object_show(win);

	__WFD_APP_FUNC_EXIT__;
	return popup;
}

static void _wfd_ug_automatic_turn_off_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	if (ad->win) {
		evas_object_hide(ad->win);
	}

	__WFD_APP_FUNC_EXIT__;
}

Evas_Object *wfd_draw_pop_type_auto_deactivation(Evas_Object *win,  void *userdata)
{
	__WFD_APP_FUNC_ENTER__;

	Evas_Object *popup = NULL;
	Evas_Object *btn = NULL;
	char popup_text[MAX_POPUP_TEXT_SIZE] = {0};
	wfd_appdata_t *ad = wfd_get_appdata();

	popup = elm_popup_add(win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _wfd_ug_automatic_turn_off_popup_cb, userdata);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, mouseup_cb, ad);
//	evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, keydown_cb, ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_part_text_set(popup, "title,text",
			PACKAGE, "IDS_WIFI_BODY_WI_FI_DIRECT_ABB");
	snprintf(popup_text, MAX_POPUP_TEXT_SIZE,
		"IDS_WIFI_POP_THERE_HAS_BEEN_NO_ACTIVITY_FOR_PD_MINUTES_SINCE_WI_FI_DIRECT_WAS_ENABLED_MSG", 5);
	elm_object_domain_translatable_text_set(popup, PACKAGE, popup_text);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_domain_translatable_text_set(btn, PACKAGE, "IDS_BR_SK_OK");
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", _wfd_ug_automatic_turn_off_popup_cb, userdata);

	evas_object_show(popup);
	evas_object_show(win);

	__WFD_APP_FUNC_EXIT__;
	return popup;
}

/**
 *	This function let the ug make a change callback for password input
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
static void _smart_ime_cb(void *data, Evas_Object * obj, void *event_info)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();
	wfd_connection_info_s *connection = ad->connection;

	Ecore_IMF_Context *imf_context = NULL;
	imf_context = (Ecore_IMF_Context *) ad->pin_entry;

	if (NULL == imf_context) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Error!!! Ecore_IMF_Context is NULL!!");
		return;
	}

	char *txt = elm_entry_markup_to_utf8(elm_entry_entry_get((const Evas_Object *) imf_context));
	if (NULL != txt) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "* text [%s], len=[%d]", txt, strlen(txt));
		strncpy(connection->wps_pin, txt, sizeof(connection->wps_pin) - 1);
		WFD_IF_FREE_MEM(txt);
	} else {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Err!");
	}

	__WFD_APP_FUNC_EXIT__;
}

/**
 *	This function let the app make a callback for progressbar timer
 *	@return   if stop the timer, return ECORE_CALLBACK_CANCEL, else return ECORE_CALLBACK_RENEW
 *	@param[in] data the pointer to the wps structure
 */
static Eina_Bool _fn_pb_timer(void *data)
{
	int step = 0;
	double value = 0.0;
	char time_label[32] = {0};
	wfd_wps_display_popup_t *wps_display_popup = (wfd_wps_display_popup_t *) data;

	if (NULL == wps_display_popup) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Param is NULL.\n");
		return ECORE_CALLBACK_CANCEL;
	}

	Evas_Object *progressbar = NULL;
	Evas_Object *time = NULL;

	progressbar = wps_display_popup->progressbar;
	time = wps_display_popup->time;
	value = elm_progressbar_value_get(progressbar);

	wps_display_popup->step++;
	step = wps_display_popup->step;
	value = ((double)step) / WFD_POP_TIMER_120;

	if (value >= 1.0) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Progress end.\n");
		if (pb_timer) {
			ecore_timer_del(pb_timer);
			pb_timer = NULL;
		}
		wfd_destroy_popup();
		return ECORE_CALLBACK_CANCEL;
	}

	elm_progressbar_value_set(progressbar, value);
	WFD_APP_LOG(WFD_APP_LOG_LOW, "step: %d, value: %f\n", wps_display_popup->step, value);

	/* show the time label */
	if (step < 60) {
		if (step < 10) {
			snprintf(time_label, 32, "00:0%d", step);
		} else {
			snprintf(time_label, 32, "00:%d", step);
		}
	} else {
		if (step%60 < 10) {
			snprintf(time_label, 32, "0%d:0%d", step/60, step%60);
		} else {
			snprintf(time_label, 32, "0%d:%d", step/60, step%60);
		}
	}

	char *remaining_time_str = g_strdup_printf(
			"<font_size=40><align=center>%s</align></font_size>", time_label);
	elm_object_text_set(time, remaining_time_str);
	g_free(remaining_time_str);

	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _keypad_popup_timer_cb(void *data)
{
	__WFD_APP_FUNC_ENTER__;

	char msg1[WFD_POP_STR_MAX_LEN] = {0};
	char msg2[WFD_POP_STR_MAX_LEN] = {0};
	char label_str[WFD_POP_STR_MAX_LEN] = {0, };

	Evas_Object *label = (Evas_Object*) data;
	wfd_appdata_t *ad = wfd_get_appdata();
	wfd_connection_info_s *connection = ad->connection;
	WFD_RETV_IF(NULL == ad, FALSE, "NULL parameters(ad)\n");
	WFD_RETV_IF(NULL == label, FALSE, "NULL parameters(label)\n");
	if (NULL == ad->popup) {
		keypad_popup_timeout = 0;
		return ECORE_CALLBACK_CANCEL;
	}

	keypad_popup_timeout --;

	if (keypad_popup_timeout > 0) {
		_replace_1PS_2PD((char *)msg1, sizeof(msg1),
				_("IDS_ST_BODY_CONNECT_WITH_PS_IN_PD_SECS_ABB"),
				connection->peer_name, keypad_popup_timeout);

		snprintf(msg2, sizeof(msg2), _("IDS_WIFI_POP_ENTER_PIN_TO_CONNECT_TO_PS"),
						connection->peer_name);
		snprintf(label_str, sizeof(label_str), "%s %s", msg1, msg2);
		elm_object_domain_translatable_text_set(label, PACKAGE, label_str);

	}

	if (keypad_popup_timeout <= 0) {
		wfd_destroy_popup();
		keypad_popup_timeout = 0;
		__WFD_APP_FUNC_EXIT__;
		return ECORE_CALLBACK_CANCEL;
	}

	__WFD_APP_FUNC_EXIT__;
	return ECORE_CALLBACK_RENEW;
}

/* This callback is for showing(hiding) X marked button. */
/*static void _changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_focus_get(obj)) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(data, "elm,state,eraser,hide", "");
		else
			elm_object_signal_emit(data, "elm,state,eraser,show", "");
	}
}*/

/* Focused callback will show X marked button and hide guidetext. */
/*static void _focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(data, "elm,state,eraser,show", "");
	elm_object_signal_emit(data, "elm,state,rename,hide", "");
}*/

/*  Unfocused callback will show guidetext and hide X marked button. */
/*static void _unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_signal_emit(data, "elm,state,eraser,hide", "");
	elm_object_signal_emit(data, "elm,state,rename,show", "");
}*/

/* When X marked button clicked, make string as empty. */
/*static void _eraser_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_focus_set(data, EINA_TRUE);
	elm_entry_entry_set(data, "");
}*/

/**
 *	This function let the app create a edit filed
 *	@return   edit filed
 *	@param[in] parent the parent object
 *	@param[in] title the pointer to the title of edit field
 *	@param[in] guide the pointer to the text of guide
 *	@param[in] single_line whether it can support single line
 *	@param[in] is_editable whether it is avaliable to edit
 */
/*static Evas_Object *_add_edit_field(Evas_Object *parent, const char *title, const char *guide, Eina_Bool single_line, Eina_Bool is_editable)
{
	assertm_if(NULL == parent, "parent is NULL!!");

	Evas_Object *layout = elm_layout_add(parent);
	assertm_if(NULL == layout, "layout is NULL!!");

	if (title && title[0] != '\0') {
		elm_layout_theme_set(layout, "layout", "dialogue/editfield/title", "default");
		elm_object_part_text_set(layout, "elm.text", title);
	} else {
		elm_layout_theme_set(layout, "layout", "dialogue/editfield", "default");
	}

	Evas_Object *entry = elm_entry_add(layout);
	assertm_if(NULL == entry, "entry is NULL!!");

	if (guide && guide[0] != '\0') {
		elm_object_part_text_set(layout, "elm.guidetext", guide);
	}

	evas_object_smart_callback_add(entry, "changed", _changed_cb, layout);
	evas_object_smart_callback_add(entry, "focused", _focused_cb, layout);
	evas_object_smart_callback_add(entry, "unfocused", _unfocused_cb, layout);

	elm_object_part_content_set(layout, "elm.icon.entry", entry);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_entry_single_line_set(entry, single_line);
	elm_entry_scrollable_set(entry, single_line);

	Evas_Object *button = elm_button_add(parent);
	elm_object_style_set(button, "editfield_clear");
	elm_object_part_content_set(layout, "elm.icon.eraser", button);
	evas_object_smart_callback_add(button, "clicked", _eraser_btn_clicked_cb, entry);
	return layout;
}*/

/**
 *	This function let the app create a display popup
 *	@return   display popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
Evas_Object *wfd_draw_pop_type_display(Evas_Object * win, wfd_popup_t * pop)
{
	__WFD_APP_FUNC_ENTER__;
	WFD_APP_LOG(WFD_APP_LOG_LOW, "wfd_draw_pop_type_display\n");

	Evas_Object *popup = NULL;
	Evas_Object *progressbar = NULL;
	Evas_Object *time = NULL;
	Evas_Object *layout;
	static wfd_wps_display_popup_t wps_display_popup;
	wfd_appdata_t *ad = wfd_get_appdata();

	popup = elm_popup_add(win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, mouseup_cb, ad);
//	evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, keydown_cb, ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, 0.0);
	elm_object_domain_translatable_part_text_set(popup, "title,text",
			 PACKAGE, _("IDS_WIFI_HEADER_WI_FI_DIRECT_CONNECTION_ABB"));

	layout = elm_layout_add(popup);
	if (layout == NULL) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Layout failed so returning !!");
		return NULL;
	}

	elm_layout_file_set(layout, WFD_EDJ_POPUP_PATH, "popup_wps_pin_layout");
	elm_object_domain_translatable_part_text_set(layout, "elm.text.description",
			 PACKAGE, pop->text);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* add progressbar */
	progressbar = elm_progressbar_add(layout);
	elm_object_style_set(progressbar, "list_progress");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(progressbar, 0.0);
	evas_object_show(progressbar);

	/* add time */
	time = elm_label_add(layout);
	elm_label_line_wrap_set(time, ELM_WRAP_MIXED);
	elm_object_text_set(time, _("<font_size=40><align=center>00:00</align></font_size>"));
	evas_object_size_hint_weight_set(time, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(time, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(time);

	elm_object_part_content_set(layout, "slider", progressbar);
	elm_object_part_content_set(layout, "timer_label", time);


	/* start progressbar timer */
	wps_display_popup.step = 0;
	wps_display_popup.progressbar = progressbar;
	wps_display_popup.time = time;
	pb_timer = ecore_timer_add(1, _fn_pb_timer, &wps_display_popup);

	/* add buttons */
	if (pop->resp_data2 == WFD_POP_RESP_APRV_CONNECT_NO) {
		Evas_Object *btn2 = NULL;
		btn2 = elm_button_add(popup);
		elm_object_style_set(btn2, "popup");
		elm_object_domain_translatable_text_set(btn2, PACKAGE, pop->label2);
		elm_object_part_content_set(popup, "button1", btn2);
		evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb,
			(void *) pop->resp_data2);
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_resp_cb, (void *) pop->resp_data2);
	}

	if (pop->resp_data1 == WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES || pop->resp_data1 == WFD_POP_RESP_APRV_CONNECT_PBC_YES ) {
		Evas_Object *btn1 = NULL;
		btn1 = elm_button_add(popup);
		elm_object_style_set(btn1, "popup");
		elm_object_domain_translatable_text_set(btn1, PACKAGE, pop->label1);
		elm_object_part_content_set(popup, "button2", btn1);
		evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
			(void *) pop->resp_data1);
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_resp_cb, (void *) pop->resp_data1);
	}

	elm_object_content_set(popup, layout);
	evas_object_show(popup);
	evas_object_show(win);

	__WFD_APP_FUNC_EXIT__;
	return popup;
}

static void __popup_eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WFD_APP_FUNC_ENTER__;
	elm_entry_entry_set(data, "");
	__WFD_APP_FUNC_EXIT__;
}

static void _entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WFD_APP_FUNC_ENTER__;
	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (elm_object_focus_get(obj)) {
			if (elm_entry_is_empty(obj))
				elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
			else
				elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		}
	}
	__WFD_APP_FUNC_EXIT__;
}

static void _entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	__WFD_APP_FUNC_ENTER__;

	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (!elm_entry_is_empty(obj))
			elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		else
			elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	}
	elm_object_signal_emit(obj, "elm,state,focus,on", "");
	__WFD_APP_FUNC_EXIT__;
}

static void _entry_keydown_cb(void *data, Evas *e, Evas_Object *obj,
		void *event_info)
{
	__WFD_APP_FUNC_ENTER__;

	Evas_Event_Key_Down *ev;
	Evas_Object *entry = obj;

	WFD_RET_IF(data == NULL, "Incorrect parameter data(NULL)\n");
	WFD_RET_IF(event_info == NULL, "Incorrect parameter event_info(NULL)\n");
	WFD_RET_IF(entry == NULL, "Incorrect parameter entry(NULL)\n");

	ev = (Evas_Event_Key_Down *)event_info;

	if (g_strcmp0(ev->key, "KP_Enter") == 0 || g_strcmp0(ev->key, "Return") == 0) {
		Ecore_IMF_Context *imf_context;

		imf_context = (Ecore_IMF_Context*)elm_entry_imf_context_get(entry);
		if (imf_context) {
			ecore_imf_context_input_panel_hide(imf_context);
		}

		elm_object_focus_set(entry, EINA_FALSE);
	}
}

static char *__wfd_main_desc_label_get(void *data, Evas_Object *obj,
		const char *part)
{
	__WFD_APP_FUNC_ENTER__;
	if (obj == NULL || part == NULL) {
		return NULL;
	}
	WFD_APP_LOG(WFD_APP_LOG_LOW, "wfd_rename desc\n");
	char buf[WFD_POP_STR_MAX_LEN] = {0, };
	char msg1[WFD_POP_STR_MAX_LEN] = {0, };
	char msg2[WFD_POP_STR_MAX_LEN] = {0, };
	wfd_appdata_t *ad = wfd_get_appdata();
	WFD_RETV_IF(ad == NULL, NULL, "Incorrect parameter(NULL)\n");
	wfd_connection_info_s *connection = ad->connection;
	WFD_RETV_IF(connection == NULL, NULL, "Incorrect parameter(NULL)\n");

	if (!strcmp("elm.text.multiline", part)) {
		if (keypad_popup_timeout > 0) {
				ad->timeout = keypad_popup_timeout;
		}
		_replace_1PS_2PD((char *)msg1, WFD_POP_STR_MAX_LEN,
				_("IDS_ST_BODY_CONNECT_WITH_PS_IN_PD_SECS_ABB"),
				connection->peer_name, ad->timeout);

		snprintf(msg2, WFD_POP_STR_MAX_LEN,
				_("IDS_WIFI_POP_ENTER_PIN_TO_CONNECT_TO_PS"),
				connection->peer_name);

		WFD_APP_LOG(WFD_APP_LOG_LOW, "string %s %s", msg1, msg2);
		snprintf(buf, WFD_POP_STR_MAX_LEN,
				"<font_size=30>%s %s</font_size>",
				msg1, msg2);
		__WFD_APP_FUNC_EXIT__;
		return g_strdup(buf);
	}
	__WFD_APP_FUNC_EXIT__;
	return NULL;
}

static Evas_Object *__wfd_pin_entry_icon_get(void *data, Evas_Object *obj,
		const char *part)
{
	__WFD_APP_FUNC_ENTER__;
	if (obj == NULL || part == NULL) {
		return NULL;
	}
	wfd_appdata_t *ad = wfd_get_appdata();
	WFD_RETV_IF(ad == NULL, NULL, "Incorrect parameter(NULL)\n");
	Evas_Object *entry = NULL;
	Evas_Object *button = NULL;
	Ecore_IMF_Context *imf_context;
	if (g_strcmp0(part, "elm.icon.entry")) {
		__WFD_APP_FUNC_EXIT__;
		return NULL;
	}
	static Elm_Entry_Filter_Accept_Set accept_set = {"0123456789", NULL};
	entry = elm_entry_add(obj);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_style_set(entry, "editfield");
	elm_entry_scrollable_set(entry, EINA_TRUE);

	eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_password_set(entry, EINA_TRUE);
	elm_entry_prediction_allow_set(entry, EINA_FALSE);

	elm_entry_markup_filter_append(entry,
			elm_entry_filter_accept_set,&accept_set);
	elm_entry_input_panel_layout_set(entry,
			ELM_INPUT_PANEL_LAYOUT_DATETIME);
	elm_object_signal_emit(entry, "elm,action,hide,search_icon", "");
	elm_object_domain_translatable_part_text_set(entry, "elm.guide",
			 PACKAGE, _("IDS_WIFI_POP_PIN"));
	elm_entry_input_panel_return_key_autoenabled_set(entry,EINA_TRUE);
	elm_entry_input_panel_return_key_type_set(entry,
			ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	ad->pin_entry = entry;
	imf_context = (Ecore_IMF_Context*)elm_entry_imf_context_get(entry);
	if (imf_context) {
		ecore_imf_context_input_panel_return_key_type_set(imf_context,
		ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	}

	button = elm_button_add(obj);
	elm_object_style_set(button, "search_clear");
	elm_object_focus_allow_set(button, EINA_FALSE);
	elm_object_part_content_set(entry, "elm.swallow.clear", button);
	evas_object_smart_callback_add(button, "clicked",
			__popup_eraser_clicked_cb, entry);

	elm_object_signal_emit (entry, "elm,action,hide,search_icon", "");
	evas_object_smart_callback_add(entry, "changed",
			_entry_changed_cb, NULL);
	evas_object_smart_callback_add(entry, "preedit,changed",
			_entry_changed_cb, NULL);
	evas_object_smart_callback_add(entry, "focused",
			_entry_focused_cb, NULL);
	evas_object_event_callback_add(entry, EVAS_CALLBACK_KEY_DOWN,
			_entry_keydown_cb, ad);

	elm_object_content_set(obj,entry);
	evas_object_show(entry);
	evas_object_smart_callback_add(entry, "changed", _smart_ime_cb, NULL);
	elm_object_focus_set(entry, EINA_TRUE);
	__WFD_APP_FUNC_EXIT__;
	return entry;
}

static void _chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	__WFD_APP_FUNC_ENTER__;
	if (obj == NULL || data == NULL) {
		return;
	}
	Eina_Bool state = elm_check_state_get(obj);
	if (state) {
		elm_entry_password_set((Evas_Object *)data, EINA_FALSE);
	} else {
		elm_entry_password_set((Evas_Object *)data, EINA_TRUE);
	}
	__WFD_APP_FUNC_EXIT__;
}

static void _gl_pswd_check_box_sel(void *data, Evas_Object *obj, void *ei)
{
	__WFD_APP_FUNC_ENTER__;
	Elm_Object_Item *item = NULL;
	item = (Elm_Object_Item *)ei;
	if (item == NULL) {
		return;
	}
	wfd_appdata_t *ad = wfd_get_appdata();
	if (ad == NULL) {
		WFD_APP_LOG(WFD_APP_LOG_LOW, "Incorrect parameter(NULL)\n");
		return;
	}
	Evas_Object *ck = elm_object_item_part_content_get(ei, "elm.icon.left");
	elm_genlist_item_selected_set(item, EINA_FALSE);
	Eina_Bool state = elm_check_state_get(ck);
	elm_check_state_set(ck, !state);
	if (ad) {
		_chk_changed_cb(ad->pin_entry, ck, NULL);
	}
	__WFD_APP_FUNC_EXIT__;
}

static char *__wfd_password_label(void *data, Evas_Object *obj, const char *part)
{
	__WFD_APP_FUNC_ENTER__;

	if (data == NULL || part == NULL) {
		return NULL;
	}
	WFD_APP_LOG(WFD_APP_LOG_LOW, "Part %s", part);

	if (!strcmp("elm.text", part)) {
		__WFD_APP_FUNC_EXIT__;
		return g_strdup(" Show password");
	}
	__WFD_APP_FUNC_EXIT__;
	return NULL;
}

static Evas_Object *__wfd_password_check(void *data, Evas_Object *obj,
		const char *part)
{
	__WFD_APP_FUNC_ENTER__;
	if (obj == NULL || part == NULL) {
		return NULL;
	}
	wfd_appdata_t *ad = wfd_get_appdata();
	WFD_RETV_IF(ad == NULL, NULL, "Incorrect parameter(NULL)\n");
	Evas_Object *check = NULL;

	WFD_APP_LOG(WFD_APP_LOG_LOW, "Part %s", part);

	if (!strcmp("elm.swallow.icon", part)) {
		check = elm_check_add(obj);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(check,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_smart_callback_add(check, "changed",
				_chk_changed_cb, ad->pin_entry);
		elm_object_focus_allow_set(check, EINA_FALSE);
		__WFD_APP_FUNC_EXIT__;
		return check;
	}
	__WFD_APP_FUNC_EXIT__;
	return check;
}


/**
 *	This function let the app create a keypad popup
 *	@return   keypad popup
 *	@param[in] win the window object
 *	@param[in] pop the pointer to the prepared popup
 */
Evas_Object *wfd_draw_pop_type_keypad(Evas_Object * win, wfd_popup_t * pop)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();
	WFD_RETV_IF(ad == NULL, NULL, "Incorrect parameter(NULL)\n");
	wfd_connection_info_s *connection = ad->connection;
	WFD_RETV_IF(connection == NULL, NULL, "Incorrect parameter(NULL)\n");

	Evas_Object *pinpopup = NULL;
	Evas_Object *btn1 = NULL, *btn2 = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *git = NULL;

	ad->timeout = pop->timeout;

	pinpopup = elm_popup_add(ad->layout);
	elm_popup_align_set(pinpopup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(pinpopup, EEXT_CALLBACK_BACK, eext_popup_back_cb,
			NULL);
	evas_object_size_hint_weight_set(pinpopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_part_text_set(pinpopup, "title,text",
			 PACKAGE, _("IDS_WIFI_HEADER_WI_FI_DIRECT_CONNECTION_ABB"));

	genlist = elm_genlist_add(pinpopup);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	evas_object_size_hint_weight_set(genlist,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_scroller_content_min_limit(genlist, EINA_FALSE, EINA_TRUE);

	/* Entry genlist item */
	ad->pin_desc_itc = elm_genlist_item_class_new();
	if(ad->pin_desc_itc != NULL) {
		ad->pin_desc_itc->item_style = WFD_GENLIST_MULTILINE_TEXT_STYLE;
		ad->pin_desc_itc->func.text_get = __wfd_main_desc_label_get;
		ad->pin_desc_itc->func.content_get = NULL;
		ad->pin_desc_itc->func.state_get = NULL;
		ad->pin_desc_itc->func.del = NULL;

		git = elm_genlist_item_append(genlist, ad->pin_desc_itc, ad, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		if(git != NULL)
			elm_genlist_item_select_mode_set(git,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		keypad_popup_timeout = pop->timeout;
		keypad_popup_timer = ecore_timer_add(1, _keypad_popup_timer_cb,
				ad->pin_desc_itc);
	}

	ad->pin_entry_itc = elm_genlist_item_class_new();
	if(ad->pin_entry_itc != NULL) {
		ad->pin_entry_itc->item_style = "entry";
		ad->pin_entry_itc->func.text_get = NULL;
		ad->pin_entry_itc->func.content_get = __wfd_pin_entry_icon_get;
		ad->pin_entry_itc->func.state_get = NULL;
		ad->pin_entry_itc->func.del = NULL;

		elm_genlist_item_append(genlist, ad->pin_entry_itc, pinpopup,
				NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}

	ad->paswd_itc = elm_genlist_item_class_new();
	if(ad->paswd_itc != NULL) {
		ad->paswd_itc->item_style = WFD_GENLIST_1LINE_TEXT_ICON_STYLE;
		ad->paswd_itc->func.text_get = __wfd_password_label;
		ad->paswd_itc->func.content_get = __wfd_password_check;
		ad->paswd_itc->func.state_get = NULL;
		ad->paswd_itc->func.del = NULL;

		elm_genlist_item_append(genlist, ad->paswd_itc, pinpopup,
				NULL, ELM_GENLIST_ITEM_NONE, _gl_pswd_check_box_sel, (void *)ad );
	}

	btn1 = elm_button_add(pinpopup);
	elm_object_style_set(btn1, "popup");
	elm_object_domain_translatable_text_set(btn1, PACKAGE, pop->label2);
	elm_object_part_content_set(pinpopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", __popup_resp_cb,
			(void *)pop->resp_data2);

	btn2 = elm_button_add(pinpopup);
	elm_object_style_set(btn2, "popup");
	elm_object_domain_translatable_text_set(btn2, PACKAGE, pop->label1);
	elm_object_part_content_set(pinpopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", __popup_resp_cb,
			(void *)pop->resp_data1);
#if defined(GENLIST_REALIZATION_MOTE_SET)
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
#endif
	evas_object_show(genlist);
	elm_object_content_set(pinpopup, genlist);

	evas_object_show(pinpopup);
	evas_object_show(win);
	elm_object_focus_set(ad->pin_entry, EINA_TRUE);

	__WFD_APP_FUNC_EXIT__;
	return pinpopup;
}


void _replace_int(char *haystack, int size, char *niddle, int value)
{
	__WFD_APP_FUNC_ENTER__;
	char*buf = NULL;
	char *p = NULL;
	char *q = NULL;

	if (haystack == NULL || niddle == NULL || size <= 1)
		return;

	buf = g_strdup(haystack);
	p = strstr(buf, niddle);
	if (p==NULL) {
		free(buf);
		return;
	}
	q = p + strlen(niddle);
	*p = '\0';

	snprintf(haystack, size-1, "%s%d%s", buf, value, q);
	free(buf);
	__WFD_APP_FUNC_EXIT__;
}

void _replace_1PS_2PD(char *buf, int buf_len, char *format_str, char* SD_1, int PD_2)
{
	__WFD_APP_FUNC_ENTER__;
	char text[WFD_POP_STR_MAX_LEN] = {0, };

	strncpy(text, format_str, WFD_POP_STR_MAX_LEN-1);
	_replace_int(text, WFD_POP_STR_MAX_LEN, "%d", PD_2);
	_replace_int(text, WFD_POP_STR_MAX_LEN, "%2$d", PD_2);
	snprintf(buf, buf_len-1, text, SD_1);
	__WFD_APP_FUNC_EXIT__;
}


/**
 *	This function let the app create a popup
 *	@return   void
 *	@param[in] type the type of popup
 *	@param[in] userdata the pointer to the data which will be used
 */
void wfd_prepare_popup(int type, void *user_data)
{
	__WFD_APP_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();
	wfd_popup_t *pop = ad->popup_data;
	wfd_connection_info_s *connection = ad->connection;
	char text[WFD_POP_STR_MAX_LEN+1] = {0, };
	char text1[WFD_POP_STR_MAX_LEN+1] = {0, };
	wfd_destroy_popup();
	char *peer_name;
	peer_name = elm_entry_utf8_to_markup(connection->peer_name);

	memset(pop, 0, sizeof(wfd_popup_t));

	pop->type = type;

	switch (pop->type) {
	case /* MT */ WFD_POP_APRV_CONNECTION_WPS_PUSHBUTTON_REQ:

		_replace_1PS_2PD((char *)pop->text, sizeof(pop->text),
				_("IDS_WIFI_POP_CONNECT_TO_PS_IN_PD_SECONDS"),
				peer_name, WFD_POP_TIMER_120);

		snprintf(pop->label1, sizeof(pop->label1), "%s", _("IDS_WIFI_SK2_OK"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", _("IDS_WIFI_SK_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_PBC_YES;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_display(ad->win, pop);
 		break;

	case /* MT */ WFD_POP_APRV_CONNECTION_WPS_DISPLAY_REQ:
		snprintf(pop->text, sizeof(pop->text),
				_("IDS_WIFI_BODY_PS_IS_REQUESTING_A_WI_FI_DIRECT_CONNECTION_ALLOW_Q"),
				peer_name);
		snprintf(pop->label1, sizeof(pop->label1), "%s", _("IDS_WIFI_BUTTON_ALLOW"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", _("IDS_BR_SK_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_DISPLAY_OK;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_c(ad->win, pop);
 		break;

	case /* MT */ WFD_POP_APRV_CONNECTION_WPS_KEYPAD_REQ:
		_replace_1PS_2PD((char *)pop->text, sizeof(pop->text),
					_("IDS_WIFI_POP_CONNECT_TO_PS_IN_PD_SECONDS"),
					peer_name, WFD_POP_TIMER_120);
		snprintf(pop->label1, sizeof(pop->label1), "%s", _("IDS_BR_SK_OK"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", _("IDS_BR_SK_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_KEYPAD_YES;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_display(ad->win, pop);
 		break;

	case /* MT */ WFD_POP_PROG_CONNECT:
		snprintf(pop->text, sizeof(pop->text), "%s", _("IDS_WIFI_BODY_CONNECTING_ING"));
		snprintf(pop->label1, sizeof(pop->label1), "%s", _("IDS_BR_SK_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_b(ad->win, pop);
 		break;

	case /* MO */ WFD_POP_PROG_CONNECT_WITH_KEYPAD:
		_replace_1PS_2PD((char *)text, sizeof(text),
						_("IDS_WIFI_POP_CONNECT_TO_PS_IN_PD_SECONDS"),
						peer_name, WFD_POP_TIMER_120);

		snprintf(text1, WFD_POP_STR_MAX_LEN, "%s %s",
				text, _("IDS_WIFI_POP_ENTER_PIN_TO_CONNECT_TO_PS"));

		snprintf(pop->text, sizeof(pop->text), text1, connection->peer_name);

		snprintf(pop->label1, sizeof(pop->label1), "%s", _("IDS_WIFI_SK_CONNECT"));
		snprintf(pop->label2, sizeof(pop->label2), "%s", _("IDS_BR_SK_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data1 = WFD_POP_RESP_PROG_CONNECT_KEYPAD_OK;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_keypad(ad->win, pop);
 		break;

	case /* MO/MT */ WFD_POP_PROG_CONNECT_WITH_PIN:
		_replace_1PS_2PD((char *)text, sizeof(text),
				_("IDS_WIFI_POP_CONNECT_TO_PS_IN_PD_SECONDS"),
				peer_name, WFD_POP_TIMER_120);

		snprintf(text1, WFD_POP_STR_MAX_LEN, "%s %s %s",
				text,
				"<br>",
				_("IDS_WIFI_POP_PIN_CODE_PS"));
		snprintf(pop->text, sizeof(pop->text), text1, connection->wps_pin);

		snprintf(pop->label2, sizeof(pop->label2), "%s", _("IDS_BR_SK_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		pop->resp_data2 = WFD_POP_RESP_APRV_CONNECT_NO;

		ad->popup = wfd_draw_pop_type_display(ad->win, pop);
 		break;

	case WFD_POP_PROG_CONNECT_CANCEL:
		snprintf(pop->text, sizeof(pop->text), "%s", _("IDS_BR_SK_CANCEL"));
		pop->timeout = WFD_POP_TIMER_120;
		ad->popup = wfd_draw_pop_type_a(ad->win, pop);
 		break;

	default:
		break;
	}

	/* feedback play */
	int ret = -1;
	ret = feedback_play(FEEDBACK_PATTERN_GENERAL);
	if (ret != FEEDBACK_ERROR_NONE) {
		WFD_APP_LOG(WFD_APP_LOG_ERROR,"feedback_play error : %d\n", ret);
	}
	WFD_IF_FREE_MEM(peer_name);
	__WFD_APP_FUNC_EXIT__;
	return;
}
