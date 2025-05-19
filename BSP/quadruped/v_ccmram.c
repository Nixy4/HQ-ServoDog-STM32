#include "quadruped.h"

QUAD_FAST_MEM_DATA quad_kine _kine[LEG_COUNT];

QUAD_FAST_MEM_DATA quad_fcb _acb_fcb[SERVO_COUNT];
QUAD_FAST_MEM_DATA quad_fcb _ccb_fcb[LEG_COUNT];
QUAD_FAST_MEM_DATA quad_acb _acb[SERVO_COUNT];
QUAD_FAST_MEM_DATA quad_ccb _ccb[LEG_COUNT];

QUAD_FAST_MEM_DATA quad_fcb _sync_acb_fcb;
QUAD_FAST_MEM_DATA quad_fcb _sync_ccb_fcb;
QUAD_FAST_MEM_DATA quad_acb _sync_acb;
QUAD_FAST_MEM_DATA quad_ccb _sync_ccb; 