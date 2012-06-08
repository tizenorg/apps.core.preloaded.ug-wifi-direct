/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This file implements Wi-Fi direct UI Gadget.
 *
 * @file    wfd_ug.c
 * @author  Gibyoung Kim (lastkgb.kim@samsung.com)
 * @version 0.1
 */


#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif


#include <sys/time.h>
#include <libintl.h>
#include <sys/utsname.h>

#include <Elementary.h>
#include <ui-gadget-module.h>
#include <wifi-direct.h>

#include "wfd_ug.h"
#include "wfd_ug_view.h"
#include "wfd_client.h"


static Evas_Object *_create_bg(Evas_Object *parent, char *style)
{
    __FUNC_ENTER__;
    Evas_Object *bg;

    bg = elm_bg_add(parent);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_style_set(bg, style);
    elm_win_resize_object_add(parent, bg);
    evas_object_show(bg);

    __FUNC_EXIT__;
    return bg;
}

static Evas_Object *_create_fullview(Evas_Object *parent, struct ug_data *ugd)
{
    __FUNC_ENTER__;
    Evas_Object *base;

    if (parent == NULL)
    {
        DBG(LOG_ERROR, "Incorrenct parameter");
        return NULL;
    }

    base = elm_layout_add(parent);
    if (!base)
    {
        DBG(LOG_ERROR, "Failed to add layout");
        return NULL;
    }

    elm_layout_theme_set(base, "layout", "application", "default");
    evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);

    __FUNC_EXIT__;
    return base;
}

static Evas_Object *_create_frameview(Evas_Object * parent, struct ug_data *ugd)
{
    __FUNC_ENTER__;
    Evas_Object *base;

    if (parent == NULL)
    {
        DBG(LOG_ERROR, "Incorrenct parameter");
        return NULL;
    }

    base = elm_layout_add(parent);
    if (!base)
    {
        DBG(LOG_ERROR, "Failed to add layout");
        return NULL;
    }

    elm_layout_theme_set(base, "layout", "application", "default");
    evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);

    __FUNC_EXIT__;
    return base;
}

static void *on_create(struct ui_gadget *ug, enum ug_mode mode, bundle * data,
                       void *priv)
{
    __FUNC_ENTER__;
    struct ug_data *ugd;

    if (!ug || !priv)
        return NULL;

    ugd = priv;
    ugd->ug = ug;

    bindtextdomain(PACKAGE, LOCALEDIR);

    ugd->win = ug_get_parent_layout(ug);
    if (!ugd->win)
        return NULL;

    if (mode == UG_MODE_FULLVIEW)
        ugd->base = _create_fullview(ugd->win, ugd);
    else
        ugd->base = _create_frameview(ugd->win, ugd);

    if (ugd->base)
    {
        ugd->bg = _create_bg(ugd->win, "group_list");
        elm_object_part_content_set(ugd->base, "elm.swallow.bg", ugd->bg);
    }
    else
    {
        DBG(LOG_ERROR, "Failed to create base layout\n");
        return NULL;
    }

    wfd_get_vconf_status(ugd);

    create_wfd_ug_view(ugd);
    evas_object_show(ugd->base);

    __FUNC_EXIT__;
    return ugd->base;
}

static void on_start(struct ui_gadget *ug, bundle * data, void *priv)
{
    __FUNC_ENTER__;
    struct ug_data *ugd;
    int res;    

    if (!ug || !priv)
        return;

    ugd = priv;

    struct utsname kernel_info;
    res = uname(&kernel_info);
    if(res != 0)
    {
        DBG(LOG_ERROR, "Failed to detect target type\n");
    }
    else
    {
        DBG(LOG_VERBOSE, "Node name of this device [%s]\n", kernel_info.nodename);
        DBG(LOG_VERBOSE, "HW ID of this device [%s]\n", kernel_info.machine);
        if(strcmp(kernel_info.nodename, "SLP_PQ") == 0 ||
            strncmp(kernel_info.machine, "arm", 3) != 0)
        {
            wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_NOT_SUPPORTED_DEVICE"), POPUP_TYPE_TERMINATE);
            return;
        }
    }

    res = init_wfd_client(ugd);
    if(res != 0)
    {
        DBG(LOG_ERROR, "Failed to initialize WFD client library\n");
        wfd_ug_warn_popup(ugd, _("IDS_WFD_POP_PROBLEM_WITH_WFD"), POPUP_TYPE_TERMINATE);
    }

    __FUNC_EXIT__;
}

static void on_pause(struct ui_gadget *ug, bundle * data, void *priv)
{
    __FUNC_ENTER__;
    __FUNC_EXIT__;
}

static void on_resume(struct ui_gadget *ug, bundle * data, void *priv)
{
    __FUNC_ENTER__;
    __FUNC_EXIT__;
}

static void on_destroy(struct ui_gadget *ug, bundle * data, void *priv)
{
    __FUNC_ENTER__;

    if (!ug || !priv)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

    struct ug_data *ugd = priv;
    if (ugd == NULL || ugd->base == NULL)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

    deinit_wfd_client(ugd);
    DBG(LOG_VERBOSE, "WFD client deregistered");

    destroy_wfd_ug_view(ugd);

    if (ugd->bg)
    {
        evas_object_del(ugd->bg);
        ugd->bg = NULL;
    }

    if (ugd->base)
    {
        evas_object_del(ugd->base);
        ugd->base = NULL;
    }

    __FUNC_EXIT__;
    return;
}

static void on_message(struct ui_gadget *ug, bundle * msg, bundle * data,
                       void *priv)
{
    __FUNC_ENTER__;
    __FUNC_EXIT__;
}

static void on_event(struct ui_gadget *ug, enum ug_event event, bundle * data,
                     void *priv)
{
    __FUNC_ENTER__;

    if (!ug || !priv)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

    switch (event)
    {
    case UG_EVENT_LOW_MEMORY:
        DBG(LOG_VERBOSE, "UG_EVENT_LOW_MEMORY\n");
        break;
    case UG_EVENT_LOW_BATTERY:
        DBG(LOG_VERBOSE, "UG_EVENT_LOW_BATTERY\n");
        break;
    case UG_EVENT_LANG_CHANGE:
        DBG(LOG_VERBOSE, "UG_EVENT_LANG_CHANGE\n");
        break;
    case UG_EVENT_ROTATE_PORTRAIT:
        DBG(LOG_VERBOSE, "UG_EVENT_ROTATE_PORTRAIT\n");
        break;
    case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
        DBG(LOG_VERBOSE, "UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN\n");
        break;
    case UG_EVENT_ROTATE_LANDSCAPE:
        DBG(LOG_VERBOSE, "UG_EVENT_ROTATE_LANDSCAPE\n");
        break;
    case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
        DBG(LOG_VERBOSE, "UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN\n");
        break;
    default:
        DBG(LOG_VERBOSE, "default\n");
        break;
    }

    __FUNC_EXIT__;
}

static void on_key_event(struct ui_gadget *ug, enum ug_key_event event,
                         bundle * data, void *priv)
{
    __FUNC_ENTER__;

    if (!ug || !priv)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

    switch (event)
    {
    case UG_KEY_EVENT_END:
        DBG(LOG_VERBOSE, "UG_KEY_EVENT_END\n");
        break;
    default:
        break;
    }

    __FUNC_EXIT__;
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
    __FUNC_ENTER__;
    struct ug_data *ugd;

    if (!ops)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return -1;
    }

    ugd = calloc(1, sizeof(struct ug_data));
    if (ugd == NULL)
    {
        DBG(LOG_ERROR, "Failed to allocate memory for UG data\n");
        return -1;
    }

    ops->create = on_create;
    ops->start = on_start;
    ops->pause = on_pause;
    ops->resume = on_resume;
    ops->destroy = on_destroy;
    ops->message = on_message;
    ops->event = on_event;
    ops->key_event = on_key_event;
    ops->priv = ugd;
    ops->opt = UG_OPT_INDICATOR_ENABLE;

    __FUNC_EXIT__;
    return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
    __FUNC_ENTER__;

    struct ug_data *ugd;

    if (!ops)
    {
        DBG(LOG_ERROR, "The param is NULL\n");
        return;
    }

    ugd = ops->priv;

    if (ugd)
        free(ugd);

    __FUNC_EXIT__;
}
