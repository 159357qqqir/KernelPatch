#include <kpmodule.h>
#include <linux/input.h>
#include <linux/input/mt.h>

struct input_dev *vtouch_dev = NULL;

KPM_NAME("vtouch");
KPM_VERSION("1.0");
KPM_LICENSE("GPL");

static int kpm_init(void)
{
    int ret;
    kp_info("Virtual Touch KPM loading...\n");

    vtouch_dev = input_allocate_device();
    if (!vtouch_dev) {
        kp_info("vtouch allocate device failed\n");
        return -1;
    }

    vtouch_dev->name = "APatch-VirtualTouch";
    vtouch_dev->id.bustype = BUS_VIRTUAL;

    __set_bit(EV_ABS, vtouch_dev->evbit);
    __set_bit(EV_KEY, vtouch_dev->keybit);
    __set_bit(BTN_TOUCH, vtouch_dev->keybit);

    // ⚠️ 修改成你手机屏幕分辨率！示例 1080 × 2400
    input_set_abs_params(vtouch_dev, ABS_MT_POSITION_X, 0, 1080, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_POSITION_Y, 0, 2400, 0, 0);

    input_set_abs_params(vtouch_dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);
    input_set_abs_params(vtouch_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);

    ret = input_register_device(vtouch_dev);
    if(ret) {
        kp_info("register vtouch failed %d\n", ret);
        input_free_device(vtouch_dev);
        vtouch_dev = NULL;
        return ret;
    }
    kp_info("✅ Virtual Touch loaded successfully!\n");

    // 测试：自动点击坐标(500,1000)
    input_mt_slot(vtouch_dev, 0);
    input_mt_report_slot_state(vtouch_dev, MT_TOOL_FINGER, true);
    input_report_abs(vtouch_dev, ABS_MT_POSITION_X, 500);
    input_report_abs(vtouch_dev, ABS_MT_POSITION_Y, 1000);
    input_mt_report_pointer_emulation(vtouch_dev, true);
    input_sync(vtouch_dev);

    msleep(200);

    input_mt_slot(vtouch_dev,0);
    input_mt_report_slot_state(vtouch_dev, MT_TOOL_FINGER, false);
    input_mt_report_pointer_emulation(vtouch_dev, true);
    input_sync(vtouch_dev);

    return 0;
}

static void kpm_exit(void)
{
    if(vtouch_dev) {
        input_unregister_device(vtouch_dev);
        input_free_device(vtouch_dev);
        vtouch_dev = NULL;
        kp_info("✅ Virtual Touch unloaded\n");
    }
}

// 这两行宏必须保留！！负责告诉APatch入口在哪里
KPM_INIT(kpm_init);
KPM_EXIT(kpm_exit);
