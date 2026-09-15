#include <kpm.h>
#include <linux/input.h>

static struct input_dev *vt_dev = NULL;

KPM_NAME("vt_test");
KPM_VERSION("1.0");
KPM_AUTHOR("test");

KPM_INIT()
{
    int ret;
    pr_info("virtual touch kpm init start\n");

    // 防止重复加载
    if (vt_dev)
    {
        pr_err("vt_dev already exists!\n");
        return -EEXIST;
    }

    // 堆分配 input_dev（禁止栈分配！）
    vt_dev = input_allocate_device();
    if (!vt_dev)
    {
        pr_err("input_allocate_device failed\n");
        return -ENOMEM;
    }

    vt_dev->name = "kpm_virtual_touch";
    vt_dev->phys = "kpm_vt/input0";

    // 开启事件类型
    __set_bit(EV_ABS, vt_dev->evbit);
    __set_bit(EV_KEY, vt_dev->evbit);
    __set_bit(BTN_TOUCH, vt_dev->keybit);

    // 多点触控ABS范围，按你屏幕分辨率修改
    input_set_abs_params(vt_dev, ABS_MT_POSITION_X, 0, 1080, 0, 0);
    input_set_abs_params(vt_dev, ABS_MT_POSITION_Y, 0, 2400, 0, 0);
    input_set_abs_params(vt_dev, ABS_MT_SLOT, 0, 9, 0, 0);
    input_set_abs_params(vt_dev, ABS_MT_TRACKING_ID, 0, 0xffff, 0, 0);

    // 注册设备
    ret = input_register_device(vt_dev);
    if (ret != 0)
    {
        pr_err("input_register_device fail, ret=%d\n", ret);
        input_free_device(vt_dev);
        vt_dev = NULL;
        return ret;
    }
    pr_info("virtual touch device registered ok\n");

    // ==========这里不写任何 input_event / input_mt_sync 上报代码！==========

    return 0;
}

KPM_EXIT()
{
    pr_info("virtual touch kpm exit\n");
    if (vt_dev)
    {
        // 卸载顺序绝对不能颠倒：先unregister，再free
        input_unregister_device(vt_dev);
        input_free_device(vt_dev);
        vt_dev = NULL;
    }
    return 0;
}
