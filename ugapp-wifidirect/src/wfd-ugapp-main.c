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

void
_ug_layout_cb(struct ui_gadget *ug, enum ug_mode mode, void *priv)
{
	__WFD_APP_FUNC_ENTER__;

    Evas_Object *base = NULL;
    base = ug_get_layout(ug);

    if (!base)
    {
    	WFD_APP_LOG(WFD_APP_LOG_LOW,"ug_get_layout failed!");
        ug_destroy(ug);
        return;
    }

    switch (mode)
    {
        case UG_MODE_FULLVIEW:
            evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            elm_win_resize_object_add(ug_get_window(), base);
            evas_object_show(base);
            break;
        default:
          break;
    }
}

void
_ug_destroy_cb(struct ui_gadget *ug, void *priv)
{
	__WFD_APP_FUNC_ENTER__;

    // TODO: free all memory allocation

    ug_destroy(ug);
    elm_exit();
}

void ug_result_cb(struct ui_gadget *ug, bundle * result, void *priv)
{
	__WFD_APP_FUNC_ENTER__;

    // TODO: free all memory allocation

}

static int load_wifi_direct_ug(struct ui_gadget *parent_ug, void *data)
{
	__WFD_APP_FUNC_ENTER__;
    wfd_appdata_t *ugd = (wfd_appdata_t *)data;
    bundle *param = NULL;

    UG_INIT_EFL(ugd->win, UG_OPT_INDICATOR_ENABLE);

    memset(&wifi_direct_cbs, 0, sizeof(struct ug_cbs));

    wifi_direct_cbs.layout_cb = _ug_layout_cb;
    wifi_direct_cbs.result_cb = ug_result_cb;
    wifi_direct_cbs.destroy_cb = _ug_destroy_cb;
    wifi_direct_cbs.priv = ugd;

    ugd->wifi_direct_ug = ug_create(parent_ug, "setting-wifidirect-efl", UG_MODE_FULLVIEW, param, &wifi_direct_cbs);
    if (ugd->wifi_direct_ug)
        return TRUE;
    else
        return FALSE;
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
    if (eo)
    {
        elm_win_title_set(eo, name);
        elm_win_borderless_set(eo, EINA_TRUE);
        elm_win_alpha_set(eo, EINA_TRUE);
        evas_object_smart_callback_add(eo, "delete,request", _win_del, NULL);
        ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
        evas_object_resize(eo, w, h);
        evas_object_show(eo);
        //evas_object_raise(eo);
    }

    return eo;
}


static int _app_create(void *data)
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = wfd_get_appdata();

    if (data == NULL)
    {
        WFD_APP_LOG(WFD_APP_LOG_LOW, "Incorrect parameter\n");
        return -1;
    }

    bindtextdomain(PACKAGE, LOCALEDIR);

    ad->win = _create_win(NULL, PACKAGE);
    elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);

    int r;

    if (!ecore_x_display_get())
        return -1;

    r = appcore_set_i18n(PACKAGE, NULL);
    if (r != 0)
    {
        WFD_APP_LOG(WFD_APP_LOG_LOW, "appcore_set_i18n error\n");
        return -1;
    }

    __WFD_APP_FUNC_EXIT__;

    return 0;
}

static int _app_terminate(void *data)
{
	__WFD_APP_FUNC_ENTER__;

	if (data == NULL)
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "Incorrect parameter\n");
		return -1;
	}

	wfd_appdata_t *ad = (wfd_appdata_t *) data;

	if (ad->win)
	{
		evas_object_del(ad->win);
		ad->win = NULL;
	}

	__WFD_APP_FUNC_EXIT__;

	return 0;
}

static int _app_pause(void *data)
{
    __WFD_APP_FUNC_ENTER__;
    __WFD_APP_FUNC_EXIT__;
    return 0;
}

static int _app_resume(void *data)
{
    __WFD_APP_FUNC_ENTER__;
    __WFD_APP_FUNC_EXIT__;
    return 0;
}

static int _app_reset(bundle * b, void *data)
{
    __WFD_APP_FUNC_ENTER__;

    wfd_appdata_t *ad = wfd_get_appdata();

    load_wifi_direct_ug(NULL, ad);

    __WFD_APP_FUNC_EXIT__;
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
