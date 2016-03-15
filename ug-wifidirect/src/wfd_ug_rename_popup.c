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

#include <Elementary.h>
#include <vconf.h>
#include <wifi-direct.h>
#include <efl_extension.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"

#define WSC_SPEC_DEVICE_NAME_MAX_LEN 32
static char special_char[30] = "'!@$%^&*()-_=+[];:,<.>/?";

static bool is_space_str(const char *str)
{
	while (str) {
		if (*str != '\0' && *str != ' ') {
			return FALSE;
		} else if (*str == '\0') {
			return TRUE;
		}
		str++;
	}
	return TRUE;
}

/**
 *	This function check the max length of the device name
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 */
static void _rename_popop_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	WFD_RET_IF(ugd == NULL || obj == NULL, "Incorrect parameter(NULL)\n");
	WFD_RET_IF(ugd->rename_button == NULL, "ugd->rename_button(NULL)\n");

	const char *entry_text 	= NULL;
	char *input_str 		= NULL;
	bool is_space_string = FALSE;

	entry_text = elm_entry_entry_get(obj);
	input_str = elm_entry_markup_to_utf8(entry_text);

	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (elm_object_focus_get(obj)) {
			if (elm_entry_is_empty(obj))
				elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
			else
				elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		}
	}

	if (input_str == NULL || (strlen(input_str) == 0)) {
		elm_object_disabled_set(ugd->rename_button, TRUE);
		elm_entry_input_panel_return_key_disabled_set(ugd->rename_entry, TRUE);
		WFD_IF_FREE_MEM(input_str);
		return;
	}

	is_space_string = is_space_str(input_str);
	if (is_space_string) {
		elm_object_disabled_set(ugd->rename_button, TRUE);
		elm_entry_input_panel_return_key_disabled_set(ugd->rename_entry, TRUE);
		WFD_IF_FREE_MEM(input_str);
		return;
	}

	if (strlen(input_str) > 0) {
		elm_object_disabled_set(ugd->rename_button, FALSE);
		elm_entry_input_panel_return_key_disabled_set(ugd->rename_entry, FALSE);
	}

	WFD_IF_FREE_MEM(input_str);
	__FUNC_EXIT__;
}

static void _rename_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	WFD_IF_DEL_OBJ(ugd->rename_popup);
	WFD_IF_DEL_OBJ(ugd->rename_entry);

	__FUNC_EXIT__;
}

int _check_device_name(const char *device_name)
{
	__FUNC_ENTER__;

	int i = 0;
	int j = 0;
	bool is_unavailable_char_found = true;

	if ((device_name == NULL) || (strlen(device_name) == 0)) {
		return -1;
	}

	for (i = 0; i < strlen(device_name); i++) {
		if ((device_name[i] >= '0' && device_name[i] <= '9') ||
			(device_name[i] >= 'a' && device_name[i] <= 'z') ||
			(device_name[i] >= 'A' && device_name[i] <= 'Z') ||
			device_name[i] == ' ') {
			continue;
		}
		is_unavailable_char_found = true;
		for (j = 0; j < strlen(special_char); j++) {
			if (special_char[j] == device_name[i]) {
				is_unavailable_char_found = false;
				break;
			}
		}

		if (is_unavailable_char_found) {
			DBG(LOG_ERROR, "Unavailable char:[%c]\n", device_name[i]);
			return -1;
		}
	}

	__FUNC_EXIT__;
	return 0;
}

static void _rename_popup_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	const char *entry_str = elm_entry_entry_get(ugd->rename_entry);
	char *device_name_str = NULL;
	device_name_str = elm_entry_markup_to_utf8(entry_str);

	DBG(LOG_INFO, "New device name:[%s]\n", device_name_str);
	_check_device_name(device_name_str);

	if (0 != vconf_set_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR, device_name_str)) {
		DBG(LOG_ERROR, "Set vconf[%s] failed\n",VCONFKEY_SETAPPL_DEVICE_NAME_STR);
	}
#ifdef WFD_ON_OFF_GENLIST
	wfd_ug_view_refresh_glitem(ugd->item_wifi_onoff);
#endif
	wfd_ug_view_refresh_glitem(ugd->device_name_item);
	WFD_IF_DEL_OBJ(ugd->rename_popup);
	WFD_IF_DEL_OBJ(ugd->rename_entry);
	WFD_IF_FREE_MEM(device_name_str);
	__FUNC_EXIT__;
}

static void _eraser_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	WFD_RET_IF(ugd == NULL, "Incorrect parameter(NULL)\n");
	WFD_RET_IF(ugd->rename_entry == NULL, "ugd->rename_entry(NULL)\n");
	elm_entry_entry_set(ugd->rename_entry, "");
	elm_object_domain_translatable_part_text_set(ugd->rename_entry, "elm.guide",
			 PACKAGE, "IDS_STU_HEADER_ENTER_DEVICE_NAME");

	WFD_RET_IF(ugd->rename_button == NULL, "ugd->rename_button(NULL)\n");
	elm_object_disabled_set(ugd->rename_button, TRUE);
	elm_entry_input_panel_return_key_disabled_set(ugd->rename_entry, TRUE);
	__FUNC_EXIT__;
}

static void __device_name_maxlength_reached_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	int ret = notification_status_message_post(
			D_("IDS_COM_POP_MAXIMUM_NUMBER_OF_CHARACTERS_REACHED"));
	if (ret != NOTIFICATION_ERROR_NONE) {
		DBG(LOG_ERROR, "notification_status_message_post() ERROR [%d]", ret);
	}

	__FUNC_EXIT__;
}

static void _entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (!elm_entry_is_empty(obj))
			elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		else
			elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	}
	elm_object_signal_emit(obj, "elm,state,focus,on", "");
}

static void _rename_entry_keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;

	Evas_Event_Key_Down *ev;
	Evas_Object *entry = obj;

	WFD_RET_IF(data == NULL, "Incorrect parameter data(NULL)\n");
	WFD_RET_IF(event_info == NULL, "Incorrect parameter event_info(NULL)\n");
	WFD_RET_IF(entry == NULL, "Incorrect parameter entry(NULL)\n");

	ev = (Evas_Event_Key_Down *)event_info;
	DBG(LOG_INFO, "ENTER ev->key:%s", ev->key);

	if (g_strcmp0(ev->key, "KP_Enter") == 0 || g_strcmp0(ev->key, "Return") == 0) {
		Ecore_IMF_Context *imf_context;

		imf_context = (Ecore_IMF_Context*)elm_entry_imf_context_get(entry);
		if (imf_context) {
			ecore_imf_context_input_panel_hide(imf_context);
		}

		elm_object_focus_set(entry, EINA_FALSE);
	}
}

static char *__wfd_main_rename_desc_label_get(void *data, Evas_Object *obj,
					      const char *part)
{
	__FUNC_ENTER__;

	DBG(LOG_INFO,"Part = %s", part);
	if (obj == NULL || part == NULL) {
		return NULL;
	}

	if (!strcmp("elm.text.multiline", part)) {
		char buf[WFD_GLOBALIZATION_STR_LENGTH] = {0, };
		snprintf(buf, WFD_GLOBALIZATION_STR_LENGTH, "<font_size=30>%s</font_size>",
			D_("IDS_ST_POP_DEVICE_NAMES_ARE_DISPLAYED_TO_DISTINGUISH_EACH_OF_THE_DEVICES_AVAILABLE_MSG"));
		return g_strdup(buf);
	}
	__FUNC_EXIT__;
	return NULL;
}

static Evas_Object *__wfd_main_rename_entry_icon_get(
				void *data, Evas_Object *obj, const char *part)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *)data;
	WFD_RETV_IF(ugd == NULL, NULL, "Incorrect parameter(NULL)\n");
	Evas_Object *entry = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *button = NULL;
	Ecore_IMF_Context *imf_context;
	char *name_value = NULL;

	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	if (!strcmp(part, "elm.icon.entry")) {

		name_value = elm_entry_utf8_to_markup(ugd->dev_name);
		entry = elm_entry_add(obj);
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_object_style_set(entry, "editfield");
		elm_entry_scrollable_set(entry, EINA_TRUE);
		limit_filter_data.max_char_count = WSC_SPEC_DEVICE_NAME_MAX_LEN;
		elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

		evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
		eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
		elm_object_signal_emit(entry, "elm,action,hide,search_icon", "");
		elm_object_domain_translatable_part_text_set(entry, "elm.guide",
				 PACKAGE, "IDS_STU_HEADER_ENTER_DEVICE_NAME");

		imf_context = (Ecore_IMF_Context*)elm_entry_imf_context_get(entry);
		if (imf_context) {
			ecore_imf_context_input_panel_return_key_type_set(imf_context,
			ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
		}

		ugd->rename_entry = entry;
		elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
		elm_entry_entry_set(entry, name_value);
		WFD_IF_FREE_MEM(name_value);
		elm_entry_cursor_end_set(entry);

		evas_object_smart_callback_add(entry, "changed,user", _rename_popop_entry_changed_cb, ugd);
		evas_object_smart_callback_add(entry, "changed", _rename_popop_entry_changed_cb, ugd);
		evas_object_smart_callback_add(entry, "preedit,changed", _rename_popop_entry_changed_cb, ugd);
		evas_object_smart_callback_add(entry, "maxlength,reached",
				__device_name_maxlength_reached_cb, ugd);

		button = elm_button_add(obj);
		elm_object_style_set(button, "search_clear");
		elm_object_focus_allow_set(button, EINA_FALSE);
		elm_object_part_content_set(entry, "elm.swallow.clear", button);
		evas_object_smart_callback_add(button, "clicked", _eraser_btn_clicked_cb, ugd);

		evas_object_smart_callback_add(entry, "focused", _entry_focused_cb, NULL);
		evas_object_event_callback_add(entry, EVAS_CALLBACK_KEY_DOWN, _rename_entry_keydown_cb, ugd);
		elm_object_part_content_set(layout, "entry_part", entry);
		evas_object_show(entry);
		elm_object_focus_set(entry, EINA_TRUE);

		__FUNC_EXIT__;
		return entry;
	}
	__FUNC_EXIT__;
	return NULL;
}


/**
 *	This function let the ug create rename popup
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _gl_rename_device_sel(void *data, Evas_Object *obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;
	WFD_RET_IF(ugd == NULL, "The param is NULL\n");
	WFD_IF_DEL_OBJ(ugd->ctxpopup);
	WFD_IF_DEL_OBJ(ugd->rename_popup);
	Evas_Object *popup;
	Evas_Object *button;
	Evas_Object *genlist = NULL;
	Evas_Object *layout = NULL;
	Elm_Object_Item *git = NULL;

	popup = elm_popup_add(ugd->base);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _rename_popup_cancel_cb, ugd);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_part_text_set(popup, "title,text",
			 PACKAGE, "IDS_ST_HEADER_RENAME_DEVICE");

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, WFD_UG_EDJ_PATH, "main_rename_device_ly");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	evas_object_size_hint_weight_set(genlist,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_scroller_content_min_limit(genlist, EINA_FALSE, EINA_TRUE);

	/* Entry genlist item */
	ugd->rename_entry_itc = elm_genlist_item_class_new();
	if(ugd->rename_entry_itc != NULL) {
		ugd->rename_entry_itc->item_style = "entry";
		ugd->rename_entry_itc->func.text_get = NULL;
		ugd->rename_entry_itc->func.content_get = __wfd_main_rename_entry_icon_get;
		ugd->rename_entry_itc->func.state_get = NULL;
		ugd->rename_entry_itc->func.del = NULL;

		elm_genlist_item_append(genlist, ugd->rename_entry_itc, ugd,
				NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}

	ugd->rename_desc_itc = elm_genlist_item_class_new();
	if(ugd->rename_desc_itc != NULL) {
		ugd->rename_desc_itc->item_style = WFD_GENLIST_MULTILINE_TEXT_STYLE;
		ugd->rename_desc_itc->func.text_get = __wfd_main_rename_desc_label_get;
		ugd->rename_desc_itc->func.content_get = NULL;
		ugd->rename_desc_itc->func.state_get = NULL;
		ugd->rename_desc_itc->func.del = NULL;

		git = elm_genlist_item_append(genlist, ugd->rename_desc_itc, NULL, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		if(git != NULL)
			elm_genlist_item_select_mode_set(git,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	button = elm_button_add(popup);
	elm_object_style_set(button, "popup");
	elm_object_domain_translatable_text_set(button, PACKAGE,
			"IDS_BR_SK_CANCEL");
	elm_object_part_content_set(popup, "button1", button);
	evas_object_smart_callback_add(button, "clicked", _rename_popup_cancel_cb, ugd);

	button = elm_button_add(popup);
	elm_object_style_set(button, "popup");
	elm_object_domain_translatable_text_set(button, PACKAGE,
			"IDS_ST_SK_RENAME");
	elm_object_part_content_set(popup, "button2", button);
	evas_object_smart_callback_add(button, "clicked", _rename_popup_ok_cb, ugd);
	ugd->rename_button = button;

#if defined(GENLIST_REALIZATION_MOTE_SET)
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
#endif
	evas_object_show(genlist);
	elm_object_part_content_set(layout, "elm.swallow.layout", genlist);
	evas_object_show(layout);
	elm_object_content_set(popup, layout);
	evas_object_show(popup);
	elm_object_focus_set(ugd->rename_entry, EINA_TRUE);
	ugd->rename_popup = popup;

	__FUNC_EXIT__;
}

