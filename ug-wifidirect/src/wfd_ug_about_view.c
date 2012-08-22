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

static char *_wfd_gl_label_help_dialogue_get(void *data, Evas_Object *obj, const char *part)
{
	DBG(LOG_VERBOSE, "Adding text");

	if (!strcmp(part, "elm.text.2")) {
		return strdup(IDS_WFD_BODY_ABOUT_WIFI);
	}
	return NULL;
}

static Evas_Object *_wfd_gl_help_icon_get(void *data, Evas_Object * obj, const char *part)
{
    __FUNC_ENTER__;

    DBG(LOG_VERBOSE, "Current part: %s\n", part);
    Evas_Object *label = NULL;
    char content[1024] = {0};

    label = elm_label_add(obj);
    snprintf(content, 1024, "<color=#7C7C7CFF><font_size=32>%s</font_size></color>", IDS_WFD_BODY_ABOUT_WIFI);
    elm_label_line_wrap_set(label, ELM_WRAP_WORD);
    elm_object_text_set(label, content);
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, 0.0);
    evas_object_show(label);

    __FUNC_EXIT__;
    return label;
}

void _about_view_back_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
    __FUNC_ENTER__;
    struct ug_data *ugd = (struct ug_data*) data;

    if(!ugd)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

   elm_naviframe_item_pop(ugd->naviframe);

    __FUNC_EXIT__;
    return;
}

void _wifid_create_about_view(struct ug_data *ugd)
{

    Evas_Object *back_btn = NULL;
    Elm_Object_Item *navi_item = NULL;
    Evas_Object *control_bar = NULL;
    Elm_Object_Item *item = NULL;
    Evas_Object *genlist = NULL;
    if(ugd == NULL)
    {
        DBG(LOG_ERROR, "Incorrect parameter(NULL)");
        return;
    }

    genlist = elm_genlist_add(ugd->naviframe);
    elm_object_style_set(genlist, "dialogue");

    DBG(LOG_VERBOSE, "creating about view");
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
#if 0
    itc.item_style = "multiline/1text";
    itc.func.text_get = _wfd_gl_label_help_dialogue_get;
    itc.func.content_get = NULL;
#else
    itc.item_style = "1icon";
    itc.func.text_get = NULL;
    itc.func.content_get = _wfd_gl_help_icon_get;
#endif
    itc.func.state_get = NULL;
    itc.func.del = NULL;
    back_btn = elm_button_add(ugd->naviframe);
    elm_object_style_set(back_btn, "naviframe/back_btn/default");
    evas_object_smart_callback_add(back_btn, "clicked", _about_view_back_btn_cb, (void*) ugd);
    elm_object_focus_allow_set(back_btn, EINA_FALSE);

    item = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
    elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
    evas_object_show(genlist);
    navi_item = elm_naviframe_item_push(ugd->naviframe, IDS_WFD_TITLE_ABOUT_WIFI, back_btn, NULL, genlist, NULL);

    control_bar = elm_toolbar_add(ugd->naviframe);
    elm_toolbar_shrink_mode_set(control_bar, ELM_TOOLBAR_SHRINK_EXPAND);
    evas_object_show(control_bar);
    elm_object_item_part_content_set(navi_item, "controlbar", control_bar);

}
