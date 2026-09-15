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
struct work_struct;

// 函数指针，运行时动态查找内核符号
static void (*__set_bit)(unsigned long nr, volatile unsigned long *addr) = NULL;
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
static void (*INIT_WORK)(struct work_struct *, void (*)(struct work_struct *)) = NULL;
static bool (*schedule_work)(struct work_struct *) = NULL;
static void (*flush_work)(struct work_struct *) = NULL;

static struct input_dev *vtouch_dev = NULL;
static struct work_struct vt_work;

// 工作队列函数：这里上报触摸事件
static void vt_work_handler(struct work_struct *work)
{
    if (!vtouch_dev) return;

    // 示例：点击屏幕中间，触摸坐标 X=2048 Y=2048（范围0~4095）
    input_mt_slot(vtouch_dev,0);
    input_mt_report_slot_state(vtouch_dev, MT_TOOL_FINGER, 1);
    input_report_abs(vtouch_dev, ABS_MT_POSITION_X,2048);
    input_report_abs(vtouch_dev, ABS_MT_POSITION_Y,2048);
    input_mt_report_pointer_emulation(vtouch_dev,1);
    input_sync(vtouch_dev);

    // 抬起手指
    input_mt_slot(vtouch_dev,0);
    input_mt_report_slot_state(vtouch_dev, MT_TOOL_FINGER, 0);
    input_mt_report_pointer_emulation(vtouch_dev,1);
    input_sync(vtouch_dev);
}

KPM_NAME("virtual_touch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");

static int kpm_init(void)
{
    printk("Virtual touch KPM start load\n");

    // 查找内核函数符号
    __set_bit = (void*)kallsyms_lookup_name("__set_bit");
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
    INIT_WORK = (void*)kallsyms_lookup_name("INIT_WORK");
    schedule_work = (void*)kallsyms_lookup_name("schedule_work");
    flush_work = (void*)kallsyms_lookup_name("flush_work");

    // 校验所有必须的符号
    if(!__set_bit || !input_allocate_device || !input_register_device || !INIT_WORK || !schedule_work)
    {
        printk("Error: cannot find required input/workqueue functions\n");
        return -1;
    }

    vtouch_dev = input_allocate_device();
    if(!vtouch_dev)
    {
        printk("allocate input dev failed\n");
        return -1;
    }

    // 设备基础信息
    vtouch_dev->name = "kpm_virtual_touch";
    vtouch_dev->phys = "vtouch/input0";
    vtouch_dev->id.bustype = BUS_VIRTUAL;

    // 标记设备支持的事件类型
    __set_bit(EV_ABS, vtouch_dev->evbit);
    __set_bit(EV_KEY, vtouch_dev->evbit);
    __set_bit(BTN_TOUCH, vtouch_dev->keybit);

    // ========== 重点！和你手机触摸IC匹配：0~4095 ==========
    input_set_abs_params(vtouch_dev, ABS_MT_POSITION_X, 0, 4095, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_POSITION_Y, 0, 4095, 0, 0);
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

    // 初始化工作队列，延后执行触摸上报
    INIT_WORK(&vt_work, vt_work_handler);
    schedule_work(&vt_work);

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

