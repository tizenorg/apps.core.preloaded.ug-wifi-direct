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
 * This file implements wifi direct application main functions.
 *
 * @file    wfd-app-main.c
 * @author  Sungsik Jang (sungsik.jang@samsung.com)
 * @version 0.1
 */

#include <libintl.h>

#include "wfd-app.h"
#include "wfd-app-util.h"

wfd_appdata_t *g_wfd_ad;


wfd_appdata_t *wfd_get_appdata()
{
    return g_wfd_ad;
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
        evas_object_raise(eo);
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

    ad->popup_data = (wfd_popup_t *) malloc(sizeof(wfd_popup_t));
    if (!ad->popup_data)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "malloc failed\n");
        return -1;
    }
    memset(ad->popup_data, 0x0, sizeof(wfd_popup_t));

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

    if (init_wfd_popup_client(ad) == FALSE)
    {
        WFD_APP_LOG(WFD_APP_LOG_ERROR, "init_wfd_popup_client error\n");
        wfd_prepare_popup(WFD_POP_FAIL_INIT, NULL);
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

	if (deinit_wfd_popup_client() == FALSE)
	{
		WFD_APP_LOG(WFD_APP_LOG_ERROR, "deinit_wfd_popup_client error\n");
	}
	else
	{
		if (ad->popup)
		{
			evas_object_del(ad->popup);
			ad->popup = NULL;
		}
		if (ad->win)
		{
			evas_object_del(ad->win);
			ad->win = NULL;
		}
		if (ad->discovered_peers)
		{
			free(ad->discovered_peers);
			ad->discovered_peers = NULL;
		}
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
