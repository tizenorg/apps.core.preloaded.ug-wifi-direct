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

static Elm_Genlist_Item_Class itc;

/**
 *	This function let the ug get the label of about
 *	@return   the label of about
 *	@param[in] parent the pointer to the label's parent
  *	@param[in] obj the pointer to the evas object
 *	@param[in] part the pointer to the part of item
 */
static char *_wfd_gl_label_help_dialogue_get(void *data, Evas_Object *obj, const char *part)
{
	return strdup(_("IDS_WFD_BODY_ABOUT_WIFI"));
}

/**
 *	This function let the ug call it when click 'back' button
 *	@return   void
 *	@param[in] data the pointer to the main data structure
 *	@param[in] obj the pointer to the evas object
 *	@param[in] event_info the pointer to the event information
 */
void _about_view_back_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
	__FUNC_ENTER__;
	struct ug_data *ugd = (struct ug_data *) data;

	if (!ugd) {
		DBG(LOG_ERROR, "The param is NULL\n");
		return;
	}

	elm_naviframe_item_pop(ugd->naviframe);

	__FUNC_EXIT__;
	return;
}

/**
 *	This function let the ug create about view
 *	@return   void
 *	@param[in] ugd the pointer to the main data structure
 */
void _wifid_create_about_view(struct ug_data *ugd)
{
	__FUNC_ENTER__;

	Evas_Object *back_btn = NULL;
	Elm_Object_Item *navi_item = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *item = NULL;

	if (ugd == NULL) {
		DBG(LOG_ERROR, "Incorrect parameter(NULL)");
		return;
	}

	genlist = elm_genlist_add(ugd->naviframe);
	if (NULL == genlist) {
		DBG(LOG_ERROR, "Create genlist failed\n");
		return;
	}

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	/* Set multiline item class */
	itc.item_style = "multiline/1text";
	itc.func.text_get = _wfd_gl_label_help_dialogue_get;
	itc.func.content_get = NULL;
	itc.func.state_get = NULL;
	itc.func.del = NULL;

	item = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	back_btn = elm_button_add(ugd->naviframe);
	elm_object_style_set(back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(back_btn, "clicked", _about_view_back_btn_cb, (void *)ugd);
	elm_object_focus_allow_set(back_btn, EINA_FALSE);

	navi_item = elm_naviframe_item_push(ugd->naviframe, IDS_WFD_TITLE_ABOUT_WIFI_DIRECT, back_btn, NULL, genlist, NULL);

	__FUNC_EXIT__;
}
