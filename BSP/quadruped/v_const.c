#include "quadruped.h"

#if CONFIG_CONST_TYPE == 0

const quad_fp PI  = CONFIG_PI;
const quad_fp HPI = CONFIG_HPI;
const quad_fp DPI = CONFIG_DPI;

//!大腿组
const quad_fp L1  = CONFIG_L1;
const quad_fp L2  = CONFIG_L2;
const quad_fp L3  = CONFIG_L3;
const quad_fp L5  = CONFIG_L5;
const quad_fp R15 = CONFIG_R15;

//!小腿组
const quad_fp L8  = CONFIG_L8;
const quad_fp L9  = CONFIG_L9;

//!常用坐标
const quad_coord cc_stand0    = {0.f,115.0f};
const quad_coord cc_stand1    = {0.f,110.0f};
const quad_coord cc_start     = {21.330410f,36.030797f};
const quad_coord cc_end       = {21.330410f,36.030797f};

#endif
