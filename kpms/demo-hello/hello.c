#include <kpmodule.h>

#define EV_KEY 0x01
#define EV_ABS 0x03
#define BTN_TOUCH 0x14a
#define ABS_MT_POSITION_X 0x35
#define ABS_MT_POSITION_Y 0x36
#define ABS_MT_TRACKING_ID 0x39
#define ABS_MT_TOUCH_MAJOR 0x3a
#define BUS_VIRTUAL 0x6

struct input_dev;

// 符号指针声明
static struct input_dev* (*input_allocate_device)(void) = NULL;
static void (*input_free_device)(struct input_dev *) = NULL;
static int (*input_register_device)(struct input_dev *) = NULL;
static void (*input_unregister_device)(struct input_dev *) = NULL;
static void (*input_set_abs_params)(struct input_dev *, int, int, int, int, int) = NULL;
static void (*input_set_capability)(struct input_dev *, unsigned int, unsigned int) = NULL;
static void (*input_set_devname)(struct input_dev *, const char *) = NULL;
static void (*input_set_phys)(struct input_dev *, const char *) = NULL;

static struct input_dev *vtouch_dev = NULL;

static void kpm_init(void)
{
    printk("Virtual touch module load\n");

    // 查找内核符号
    input_allocate_device = (void *)kallsyms_lookup_name("input_allocate_device");
    input_free_device = (void *)kallsyms_lookup_name("input_free_device");
    input_register_device = (void *)kallsyms_lookup_name("input_register_device");
    input_unregister_device = (void *)kallsyms_lookup_name("input_unregister_device");
    input_set_abs_params = (void *)kallsyms_lookup_name("input_set_abs_params");
    input_set_capability = (void *)kallsyms_lookup_name("input_set_capability");
    input_set_devname = (void *)kallsyms_lookup_name("input_set_devname");
    input_set_phys = (void *)kallsyms_lookup_name("input_set_phys");

    if(!input_allocate_device || !input_register_device || !input_set_capability)
    {
        printk("Missing required symbols\n");
        return;
    }

    vtouch_dev = input_allocate_device();
    if(!vtouch_dev)
    {
        printk("Allocate input device failed\n");
        return;
    }

    // 设置设备名称、phys（使用内核API，不再直接访问结构体成员）
    input_set_devname(vtouch_dev, "kpm_virtual_touch");
    input_set_phys(vtouch_dev, "vtouch/input0");

    // 设置事件能力，替代手动 __set_bit
    input_set_capability(vtouch_dev, EV_KEY, BTN_TOUCH);
    input_set_capability(vtouch_dev, EV_ABS, ABS_MT_POSITION_X);
    input_set_capability(vtouch_dev, EV_ABS, ABS_MT_POSITION_Y);
    input_set_capability(vtouch_dev, EV_ABS, ABS_MT_TRACKING_ID);
    input_set_capability(vtouch_dev, EV_ABS, ABS_MT_TOUCH_MAJOR);

    // ABS范围 0~4095，匹配你的触摸IC
    input_set_abs_params(vtouch_dev, ABS_MT_POSITION_X, 0, 4095, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_POSITION_Y, 0, 4095, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);

    int ret = input_register_device(vtouch_dev);
    if(ret != 0)
    {
        printk("Register device failed, ret=%d\n", ret);
        input_free_device(vtouch_dev);
        vtouch_dev = NULL;
        return;
    }
    printk("✅ Virtual touch device registered\n");
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

    return 0;
}

static void kpm_exit(void)
{
    if(vtouch_dev)
    {
        flush_work(&vt_work); // 等待工作队列执行完毕，防止竞态
        input_unregister_device(vtouch_dev);
        input_free_device(vtouch_dev);
        vtouch_dev = NULL;
        printk("Virtual touch unloaded\n");
    }
}

KPM_INIT(kpm_init);
KPM_EXIT(kpm_exit);

