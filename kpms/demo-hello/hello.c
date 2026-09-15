#include <kpmodule.h>

// ========= 手动定义 input 宏 =========
#define EV_KEY 0x01
#define EV_ABS 0x03
#define BTN_TOUCH 0x14a
#define ABS_MT_POSITION_X 0x35
#define ABS_MT_POSITION_Y 0x36
#define ABS_MT_TRACKING_ID 0x39
#define ABS_MT_TOUCH_MAJOR 0x3a
#define MT_TOOL_FINGER 1
#define BUS_VIRTUAL 0x6

struct input_dev;

// 函数指针，运行时动态查找内核符号
static struct input_dev* (*input_allocate_device)(void) = NULL;
static void (*input_free_device)(struct input_dev*) = NULL;
static int (*input_register_device)(struct input_dev*) = NULL;
static void (*input_unregister_device)(struct input_dev*) = NULL;
static void (*input_set_abs_params)(struct input_dev*, int, int, int, int, int) = NULL;
static void (*input_mt_slot)(struct input_dev*, int) = NULL;
static void (*input_mt_report_slot_state)(struct input_dev*, int, int) = NULL;
static void (*input_report_abs)(struct input_dev*, int, int) = NULL;
static void (*input_mt_report_pointer_emulation)(struct input_dev*, int) = NULL;
static void (*input_sync)(struct input_dev*) = NULL;
static void (*msleep)(unsigned int) = NULL;

static struct input_dev *vtouch_dev = NULL;

KPM_NAME("virtual_touch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");

static int kpm_init(void)
{
    printk("Virtual touch KPM start load\n");

    // 查找内核函数符号
    input_allocate_device = (void*)kallsyms_lookup_name("input_allocate_device");
    input_free_device = (void*)kallsyms_lookup_name("input_free_device");
    input_register_device = (void*)kallsyms_lookup_name("input_register_device");
    input_unregister_device = (void*)kallsyms_lookup_name("input_unregister_device");
    input_set_abs_params = (void*)kallsyms_lookup_name("input_set_abs_params");
    input_mt_slot = (void*)kallsyms_lookup_name("input_mt_slot");
    input_mt_report_slot_state = (void*)kallsyms_lookup_name("input_mt_report_slot_state");
    input_report_abs = (void*)kallsyms_lookup_name("input_report_abs");
    input_mt_report_pointer_emulation = (void*)kallsyms_lookup_name("input_mt_report_pointer_emulation");
    input_sync = (void*)kallsyms_lookup_name("input_sync");
    msleep = (void*)kallsyms_lookup_name("msleep");

    if(!input_allocate_device || !input_register_device || !msleep)
    {
        printk("Error: cannot find input functions\n");
        return -1;
    }

    vtouch_dev = input_allocate_device();
    if(!vtouch_dev)
    {
        printk("allocate input dev failed\n");
        return -1;
    }

    // 设置屏幕分辨率，改成你手机分辨率
    input_set_abs_params(vtouch_dev, ABS_MT_POSITION_X, 0, 1080, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_POSITION_Y, 0, 2400, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);

    int ret = input_register_device(vtouch_dev);
    if(ret != 0)
    {
        printk("register input dev failed, ret=%d\n", ret);
        input_free_device(vtouch_dev);
        vtouch_dev = NULL;
        return ret;
    }
    printk("✅ Virtual touch device registered\n");

    // 模拟点击 (500,1000)
    input_mt_slot(vtouch_dev,0);
    input_mt_report_slot_state(vtouch_dev, MT_TOOL_FINGER, 1);
    input_report_abs(vtouch_dev, ABS_MT_POSITION_X,500);
    input_report_abs(vtouch_dev, ABS_MT_POSITION_Y,1000);
    input_mt_report_pointer_emulation(vtouch_dev,1);
    input_sync(vtouch_dev);

    msleep(200);

    // 抬起手指
    input_mt_slot(vtouch_dev,0);
    input_mt_report_slot_state(vtouch_dev, MT_TOOL_FINGER, 0);
    input_mt_report_pointer_emulation(vtouch_dev,1);
    input_sync(vtouch_dev);

    return 0;
}

static void kpm_exit(void)
{
    if(vtouch_dev)
    {
        input_unregister_device(vtouch_dev);
        input_free_device(vtouch_dev);
        vtouch_dev = NULL;
        printk("Virtual touch unloaded\n");
    }
}

KPM_INIT(kpm_init);
KPM_EXIT(kpm_exit);
