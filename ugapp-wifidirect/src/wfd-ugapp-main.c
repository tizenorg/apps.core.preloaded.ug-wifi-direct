/*
*  WiFi-Direct UG
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://floralicense.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

/**
 * This file implements wifi direct application main functions.
 *
 * @file    wfd-ug-app-main.c
 * @author  Dongwook Lee (dwmax.lee@samsung.com)
 * @version 0.1
 */

#include <ui-gadget-module.h>
#include <libintl.h>

#include "wfd-ugapp.h"
#include "wfd-ugapp-util.h"


wfd_appdata_t *g_wfd_ad = NULL;
static struct ug_cbs wifi_direct_cbs;

wfd_appdata_t *wfd_get_appdata()
{
	return g_wfd_ad;
}

void _ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	__WDUA_LOG_FUNC_ENTER__;

	Evas_Object *base = NULL;
	base = ug_get_layout(ug);

	if (!base) {
		WDUA_LOGE("ug_get_layout failed!");
		ug_destroy(ug);
		__WDUA_LOG_FUNC_EXIT__;
		return;
	}

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(ug_get_window(), base);
		evas_object_show(base);
		break;
	default:
		break;
	}

	__WDUA_LOG_FUNC_EXIT__;
}

void _ug_destroy_cb(ui_gadget_h ug, void *priv)
{
	__WDUA_LOG_FUNC_ENTER__;

	// TODO: free all memory allocation

	ug_destroy(ug);
	elm_exit();
}

void ug_result_cb(ui_gadget_h ug, service_h service, void *priv)
{
	__WDUA_LOG_FUNC_ENTER__;

	// TODO: free all memory allocation

	__WDUA_LOG_FUNC_EXIT__;
}

static int load_wifi_direct_ug(ui_gadget_h parent_ug, void *data)
{
	__WDUA_LOG_FUNC_ENTER__;
	wfd_appdata_t *ugd = (wfd_appdata_t *)data;
	service_h handle = NULL;

	UG_INIT_EFL(ugd->win, UG_OPT_INDICATOR_ENABLE);

	memset(&wifi_direct_cbs, 0, sizeof(struct ug_cbs));

	wifi_direct_cbs.layout_cb = _ug_layout_cb;
	wifi_direct_cbs.result_cb = ug_result_cb;
	wifi_direct_cbs.destroy_cb = _ug_destroy_cb;
	wifi_direct_cbs.priv = ugd;

	ugd->wifi_direct_ug = ug_create(parent_ug, "setting-wifidirect-efl", UG_MODE_FULLVIEW, handle, &wifi_direct_cbs);
	if (ugd->wifi_direct_ug) {
		__WDUA_LOG_FUNC_EXIT__;
		return TRUE;
	} else {
		__WDUA_LOG_FUNC_EXIT__;
		return FALSE;
	}

	__WDUA_LOG_FUNC_EXIT__;
}


static void _win_del(void *data, Evas_Object * obj, void *event)
{
	elm_exit();
}

static Evas_Object *_create_win(Evas_Object * parent, const char *name)
{
	Evas_Object *eo;
	int w, h;

	eo = elm_win_add(parent, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_alpha_set(eo, EINA_TRUE);
		elm_win_conformant_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", _win_del, NULL);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
		evas_object_resize(eo, w, h);
		evas_object_show(eo);
	}

	return eo;
}

static Evas_Object* _set_win_icon(wfd_appdata_t *ad)
{
	__WDUA_LOG_FUNC_ENTER__;

	Evas_Object *icon = evas_object_image_add(evas_object_evas_get(ad->win));
	evas_object_image_file_set(icon, DESKTOP_ICON, NULL);
	elm_win_icon_object_set(ad->win, icon);

	__WDUA_LOG_FUNC_EXIT__;
	return icon;
}

static Evas_Object *_create_bg(Evas_Object *parent)
{
	__WDUA_LOG_FUNC_ENTER__;

	if (NULL == parent) {
		WDUA_LOGE("Incorrect parameter\n");
		return NULL;
	}

	Evas_Object *bg = elm_bg_add(parent);
	if (NULL == bg) {
		WDUA_LOGE("Create background failed\n");
		return NULL;
	}

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);

	__WDUA_LOG_FUNC_EXIT__;
	return bg;
}

static Evas_Object *_create_layout_main(Evas_Object *parent)
{
	__WDUA_LOG_FUNC_ENTER__;

	if (NULL == parent) {
		WDUA_LOGE("Incorrect parameter\n");
		return NULL;
	}

	Evas_Object *layout = elm_layout_add(parent);
	if (NULL == layout) {
		WDUA_LOGE("Create layout failed\n");
		return NULL;
	}

	const char *profile = elm_config_profile_get();
	if (!strcmp(profile, "mobile")) {
		elm_layout_theme_set(layout, "layout", "application", "default");
	} else if (!strcmp(profile, "desktop")) {
		elm_layout_theme_set(layout, "layout", "application", "noindicator");
	}

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(parent, layout);
	evas_object_show(layout);

	__WDUA_LOG_FUNC_EXIT__;
	return layout;
}

static void _win_profile_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	__WDUA_LOG_FUNC_ENTER__;

	if (data == NULL) {
		WDUA_LOGE("Incorrect parameter\n");
		return -1;
	}

	wfd_appdata_t *ad = (wfd_appdata_t *)data;
	const char *profile = elm_config_profile_get();

	if (!strcmp(profile, "desktop")) {  /* desktop mode */
		/* hide layout's indicator area */
		elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_HIDE);

		/* set window icon */
		if (!ad->icon)	{
			ad->icon = _set_win_icon(ad);
		}
	}
	else {	/* mobile mode */
		/* show layout's indicator area */
		elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	}

	__WDUA_LOG_FUNC_EXIT__;
}

static int _app_create(void *data)
{
	__WDUA_LOG_FUNC_ENTER__;
	wfd_appdata_t *ad = wfd_get_appdata();

	if (data == NULL) {
		WDUA_LOGE("Incorrect parameter\n");
		return -1;
	}

	bindtextdomain(PACKAGE, LOCALEDIR);

	ad->win = _create_win(NULL, PACKAGE);
	evas_object_smart_callback_add(ad->win, "profile,changed", _win_profile_changed_cb, ad);

	/*Add conformat for indicator */
	ad->bg = _create_bg(ad->win);
	if (ad->bg == NULL) {
		WDUA_LOGE("Failed to create background");
		return -1;
	}

	ad->conform = elm_conformant_add(ad->win);
	if (ad->conform == NULL) {
		WDUA_LOGE("Failed to create elm conformant");
		return -1;
	}

	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->conform);

	ad->top_layout = _create_layout_main(ad->conform);
	if (ad->top_layout == NULL) {
		WDUA_LOGE("Failed to create top layout");
		return -1;
	}

	elm_object_content_set(ad->conform, ad->top_layout);

	int r;

	if (!ecore_x_display_get()) {
		return -1;
	}

	r = appcore_set_i18n(PACKAGE, NULL);
	if (r != 0) {
		WDUA_LOGE("appcore_set_i18n error\n");
		return -1;
	}

	__WDUA_LOG_FUNC_EXIT__;
	return 0;
}

static int _app_terminate(void *data)
{
	__WDUA_LOG_FUNC_ENTER__;

	if (data == NULL) {
		WDUA_LOGE("Incorrect parameter\n");
		return -1;
	}

	wfd_appdata_t *ad = (wfd_appdata_t *) data;

	if (ad->win) {
		evas_object_del(ad->win);
		ad->win = NULL;
	}

	if (ad->bg) {
		evas_object_del(ad->bg);
		ad->bg = NULL;
	}

	if (ad->conform) {
		evas_object_del(ad->conform);
		ad->conform = NULL;
	}

	if (ad->top_layout) {
		evas_object_del(ad->top_layout);
		ad->top_layout = NULL;
	}

	if (ad->icon) {
		evas_object_del(ad->icon);
		ad->icon = NULL;
	}

	__WDUA_LOG_FUNC_EXIT__;
	return 0;
}

static int _app_pause(void *data)
{
	__WDUA_LOG_FUNC_ENTER__;
	__WDUA_LOG_FUNC_EXIT__;
	return 0;
}

static int _app_resume(void *data)
{
	__WDUA_LOG_FUNC_ENTER__;
	__WDUA_LOG_FUNC_EXIT__;
	return 0;
}

static int _app_reset(bundle *b, void *data)
{
	__WDUA_LOG_FUNC_ENTER__;

	wfd_appdata_t *ad = wfd_get_appdata();
	load_wifi_direct_ug(NULL, ad);

	__WDUA_LOG_FUNC_EXIT__;
	return 0;
}

int main(int argc, char *argv[])
{
	wfd_appdata_t ad;
	struct appcore_ops ops = {
		.create = _app_create,
		.terminate = _app_terminate,
		.pause = _app_pause,
		.resume = _app_resume,
		.reset = _app_reset,
	};

	memset(&ad, 0x0, sizeof(wfd_appdata_t));
	ops.data = &ad;
	g_wfd_ad = &ad;

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
