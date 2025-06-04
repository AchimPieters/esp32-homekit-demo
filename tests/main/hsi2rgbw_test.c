#include <unity.h>
#include "hsi2rgbw.h"

static void test_primary_colors(void)
{
    int rgbw[4];

    hsi2rgbw(0, 1, 1, rgbw);
    TEST_ASSERT_EQUAL_INT(255, rgbw[0]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[1]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[2]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[3]);

    hsi2rgbw(120, 1, 1, rgbw);
    TEST_ASSERT_EQUAL_INT(0, rgbw[0]);
    TEST_ASSERT_EQUAL_INT(255, rgbw[1]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[2]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[3]);

    hsi2rgbw(240, 1, 1, rgbw);
    TEST_ASSERT_EQUAL_INT(0, rgbw[0]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[1]);
    TEST_ASSERT_EQUAL_INT(255, rgbw[2]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[3]);
}

static void test_white_and_dimming(void)
{
    int rgbw[4];

    hsi2rgbw(0, 0, 1, rgbw);
    TEST_ASSERT_EQUAL_INT(0, rgbw[0]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[1]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[2]);
    TEST_ASSERT_EQUAL_INT(255, rgbw[3]);

    hsi2rgbw(0, 1, 0.5, rgbw);
    TEST_ASSERT_EQUAL_INT(127, rgbw[0]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[1]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[2]);
    TEST_ASSERT_EQUAL_INT(0, rgbw[3]);
}

void app_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_primary_colors);
    RUN_TEST(test_white_and_dimming);
    UNITY_END();
}
