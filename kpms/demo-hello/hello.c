/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <common.h>
#include <kputils.h>
#include <linux/string.h>

KPM_NAME("vtouch-demo");
KPM_VERSION("1.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("me");
KPM_DESCRIPTION("Virtual Touch Test Module");

static long vtouch_init(const char *args, const char *event, void *__user reserved)
{
    pr_info("vtouch-demo: module loaded!\n");
    return 0;
}

static long vtouch_exit(void *__user reserved)
{
    pr_info("vtouch-demo: module unloaded!\n");
    return 0;
}

KPM_INIT(vtouch_init);
KPM_EXIT(vtouch_exit);

