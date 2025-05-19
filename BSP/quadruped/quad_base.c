#include "quadruped.h"

#define TAG "QuadBase"

void quad_init(void)
{
  //CCMRAM Init
#if CONFIG_STM32_CCMRAM_ENBALE
  memset(_kine, 0, sizeof(_kine));
  memset(_acb_fcb, 0, sizeof(_acb_fcb));
  memset(_ccb_fcb, 0, sizeof(_ccb_fcb));
  memset(_acb, 0, sizeof(_acb));
  memset(_ccb, 0, sizeof(_ccb));
  memset(&_sync_acb_fcb, 0, sizeof(_sync_acb_fcb));
  memset(&_sync_ccb_fcb, 0, sizeof(_sync_ccb_fcb));
  memset(&_sync_acb, 0, sizeof(_sync_acb));
  memset(&_sync_ccb, 0, sizeof(_sync_ccb));
#endif
  //初始化舵机
  servo_set_freq(CONFIG_SERVO_PWM_FREQ);
  //初始化 ACB
  leg_sync_acb_init(CONFIG_ACB_DEFAULT_EASING_CALC);
  //初始化 CCB
  leg_sync_ccb_init(CONFIG_CCB_DEFAULT_EASING_CALC_X, CONFIG_CCB_DEFAULT_EASING_CALC_Z);
  //初始化舵机位置
  HAL_Delay(1000);
  // leg_sync_set_coord(cc_start);
  quad_coord cs[4] = {cc_stand0, cc_stand0, cc_stand0, cc_stand0};
  leg_sync_ccb_target2_blocking(cs, 500);

  elog_i(TAG, "Quad Init Success:");
  elog_i(TAG, "%-14s: %-07.3f", "Servo Freq" ,CONFIG_SERVO_PWM_FREQ);
  elog_i(TAG, "%-14s: RF[%-07.3f,%-07.3f] LF[%-07.3f,%-07.3f] RB[%-07.3f,%-07.3f] LB[%-07.3f,%-07.3f]", "Servo Offset",
    CONFIG_SERVO_OFFSET_RF_T, CONFIG_SERVO_OFFSET_RF_S,
    CONFIG_SERVO_OFFSET_LF_T, CONFIG_SERVO_OFFSET_LF_S,
    CONFIG_SERVO_OFFSET_RB_T, CONFIG_SERVO_OFFSET_RB_S,
    CONFIG_SERVO_OFFSET_LB_T, CONFIG_SERVO_OFFSET_LB_S);
  elog_i(TAG, "%-14s: RF[%-07.3f,%-07.3f] LF[%-07.3f,%-07.3f] RB[%-07.3f,%-07.3f] LB[%-07.3f,%-07.3f]", "Servo Limit",
    CONFIG_SERVO_LIMIT_MIN_T, CONFIG_SERVO_LIMIT_MAX_T,
    CONFIG_SERVO_LIMIT_MIN_S, CONFIG_SERVO_LIMIT_MAX_S,
    CONFIG_SERVO_LIMIT_MIN_T, CONFIG_SERVO_LIMIT_MAX_T,
    CONFIG_SERVO_LIMIT_MIN_S, CONFIG_SERVO_LIMIT_MAX_S);
  elog_i(TAG, "%-14s: RF[%-07.3f,%-07.3f] LF[%-07.3f,%-07.3f] RB[%-07.3f,%-07.3f] LB[%-07.3f,%-07.3f]", "Coord Offset",
    CONFIG_LEG_COORD_OFFSET_RF_X, CONFIG_LEG_COORD_OFFSET_RF_Z,
    CONFIG_LEG_COORD_OFFSET_LF_X, CONFIG_LEG_COORD_OFFSET_LF_Z,
    CONFIG_LEG_COORD_OFFSET_RB_X, CONFIG_LEG_COORD_OFFSET_RB_Z,
    CONFIG_LEG_COORD_OFFSET_LB_X, CONFIG_LEG_COORD_OFFSET_LB_Z);
}
