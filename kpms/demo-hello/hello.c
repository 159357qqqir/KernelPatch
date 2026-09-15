#include <kpm.h>

static int hello_init(void)
{
    kpm_info("✅ Simple KPM Load Success!");
    return 0;
}

static void hello_exit(void)
{
    kpm_info("❌ Simple KPM Unload");
}

KPM_MODULE("simple_test", "simple test module", "1.0", hello_init, hello_exit);
