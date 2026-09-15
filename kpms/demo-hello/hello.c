#include <kpmodule.h>

KPM_NAME("minimal_test");
KPM_VERSION("1.0");

static int kpm_init(void)
{
    kp_info("✅ Minimal KPM loaded!\n");
    return 0;
}

static void kpm_exit(void)
{
    kp_info("❌ Minimal KPM unloaded!\n");
}

KPM_INIT(kpm_init);
KPM_EXIT(kpm_exit);
