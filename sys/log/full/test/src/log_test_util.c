#include "log_test.h"

static struct flash_area fcb_areas[] = {
    [0] = {
        .fa_off = 0x00000000,
        .fa_size = 16 * 1024
    },
    [1] = {
        .fa_off = 0x00004000,
        .fa_size = 16 * 1024
    }
};
struct fcb_log ltu_fcb_log;
struct log my_log;

void
ltu_setup_fcb(void)
{
    int rc;
    int i;

    ltu_fcb_log = (struct fcb_log) { 0 };

    ltu_fcb_log.fl_fcb.f_sectors = fcb_areas;
    ltu_fcb_log.fl_fcb.f_sector_cnt = sizeof(fcb_areas) / sizeof(fcb_areas[0]);
    ltu_fcb_log.fl_fcb.f_magic = 0x7EADBADF;
    ltu_fcb_log.fl_fcb.f_version = 0;

    for (i = 0; i < ltu_fcb_log.fl_fcb.f_sector_cnt; i++) {
        rc = flash_area_erase(&fcb_areas[i], 0, fcb_areas[i].fa_size);
        TEST_ASSERT(rc == 0);
    }
    rc = fcb_init(&ltu_fcb_log.fl_fcb);
    TEST_ASSERT(rc == 0);

    log_register("log", &my_log, &log_fcb_handler, &ltu_fcb_log, LOG_SYSLEVEL);
}
