#include "test_common.h"
TEST_START

extern PikaEventListener* g_pika_device_event_listener;
#if PIKA_EVENT_ENABLE
TEST(event, gpio) {
    /* init */
    PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
    /* run */
    pikaVM_runSingleFile(pikaMain, "test/python/TemplateDevice/gpio_cb.py");

#define EVENT_SIGAL_IO_RISING_EDGE 0x01
#define EVENT_SIGAL_IO_FALLING_EDGE 0x02
#define GPIO_PA8_EVENT_ID 0x08

    /* simulate run in the call back */
    pks_eventListener_sendSignal(g_pika_device_event_listener,
                                 GPIO_PA8_EVENT_ID, EVENT_SIGAL_IO_RISING_EDGE);
    pks_eventListener_sendSignal(g_pika_device_event_listener,
                                 GPIO_PA8_EVENT_ID,
                                 EVENT_SIGAL_IO_FALLING_EDGE);
    /* assert */

    obj_run(pikaMain, "io1.eventTest()");

    EXPECT_STREQ(log_buff[3], "get rising edge!\r\n");
    EXPECT_STREQ(log_buff[2], "get falling edge!\r\n");
    EXPECT_STREQ(log_buff[1], "get rising edge!\r\n");
    EXPECT_STREQ(log_buff[0], "get falling edge!\r\n");

    for (int i = 0; i < 255; i++) {
        obj_run(pikaMain, "io1.eventTest()");
    }

    /* simulate run in the call back */
    Arg* res_123 = pks_eventListener_sendSignalAwaitResult(
        g_pika_device_event_listener, GPIO_PA8_EVENT_ID, 123);
    Arg* res_456 = pks_eventListener_sendSignalAwaitResult(
        g_pika_device_event_listener, GPIO_PA8_EVENT_ID, 456);

    EXPECT_EQ(arg_getInt(res_123), 123);
    EXPECT_EQ(arg_getInt(res_456), 456);

    /* simulate task queue overflow */
    for (int i = 0; i < PIKA_EVENT_LIST_SIZE * 2; i++) {
        _do_pks_eventListener_send(
            g_pika_device_event_listener, GPIO_PA8_EVENT_ID,
            arg_newInt(EVENT_SIGAL_IO_FALLING_EDGE), PIKA_FALSE);
    }

    obj_run(pikaMain, "io1.close()");

    /* deinit */
    obj_deinit(pikaMain);

    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(event, remove_regist) {
    /* init */
    PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
    /* run */
    pks_eventListener_init(&g_pika_device_event_listener);
    PikaObj* testobj = newNormalObj(New_TinyObj);
    pks_eventListener_registEvent(g_pika_device_event_listener, 0, testobj);
    EXPECT_EQ(testobj->refcnt, 2);
    pks_eventListener_removeEvent(g_pika_device_event_listener, 0);
    EXPECT_EQ(testobj->refcnt, 1);
    /* deinit */
    obj_deinit(pikaMain);
    obj_deinit(testobj);
    pks_eventListener_deinit(&g_pika_device_event_listener);
    EXPECT_EQ(pikaMemNow(), 0);
}

#endif

TEST_END