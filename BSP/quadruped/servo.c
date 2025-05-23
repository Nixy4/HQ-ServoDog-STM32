#include "quadruped.h"

#define TAG "Servo"

//! PCA9685
#define I2C_ALLCALL_ADDR      (0xE0 >> 1)

#define REG_MODE1             0x00
#define REG_MODE1_ALLCALL_BIT 0x01 //使能PCA的广播地址响应
#define REG_MODE1_SUB3_BIT    0x02 //使能PCA的子地址3响应
#define REG_MODE1_SUB2_BIT    0x04 //使能PCA的子地址2响应
#define REG_MODE1_SUB1_BIT    0x08 //使能PCA的子地址1响应
#define REG_MODE1_SLEEP_BIT   0x10 //睡眠模式
#define REG_MODE1_AUTOINC_BIT 0x20 //自动增加地址
#define REG_MODE1_EXTCLK_BIT  0x40 //外部时钟
#define REG_MODE1_RESTART_BIT 0x80 //重启功能

#define REG_MODE2             0x01
#define REG_MODE2_INVRT_BIT   0x10 //反转输出
#define REG_MODE2_OCH_BIT     0x08 //输出驱动
#define REG_MODE2_OUTDRV_BIT  0x04 //输出驱动
#define REG_MODE2_OUTNE1_BIT  0x02 //输出驱动
#define REG_MODE2_OUTNE0_BIT  0x01 //输出驱动

#define REG_LED_BASE          0x06                        // LED0~LED15 0x06~0x45
#define REG_LEDX_BASE(x)      ( REG_LED_BASE + 4U * x )
#define REG_LEDX_ON_L(x)      ( REG_LEDX_BASE(x) + 1U )
#define REG_LEDX_ON_H(x)      ( REG_LEDX_BASE(x) + 2U )
#define REG_LEDX_OFF_L(x)     ( REG_LEDX_BASE(x) + 3U )
#define REG_LEDX_OFF_H(x)     ( REG_LEDX_BASE(x) + 4U )

#define REG_PSC               0xFE

#define ONL(x)                ( ((uint16_t)(x) & 0xFFU) ) // 0x00FF
#define ONH(x)                ( ((uint16_t)(x) >> 8) )
#define OFFL(x)               ( ((uint16_t)(x) & 0xFFU) )
#define OFFH(x)               ( ((uint16_t)(x) >> 8) )

#define ICLK                  25000000U // PCA内部 25MHz
#define CNT_MAX               4096U

extern I2C_HandleTypeDef hi2c2;

static void pca9685_write_reg(uint8_t reg, uint8_t val)
{
  HAL_StatusTypeDef status;
  uint8_t buf[2] = {reg, val};
  status = HAL_I2C_Master_Transmit(&hi2c2, CONFIG_PCA9685_I2C_ADDR, buf, 2, CONFIG_PCA9685_I2C_TIMEOUT);
  if (status != HAL_OK) {
    elog_e(TAG, "PCA9685 write reg failed, reg: 0x%02X, val: 0x%02X", reg, val);
  }
  elog_v(TAG, "write reg [%02X] = %02X", reg, val);
}
 
static uint8_t pca9685_read_reg(uint8_t reg)
{
  HAL_StatusTypeDef status;
  uint8_t val = 0;
  status = HAL_I2C_Master_Transmit(&hi2c2, CONFIG_PCA9685_I2C_ADDR, &reg, 1, CONFIG_PCA9685_I2C_TIMEOUT);
  if (status != HAL_OK) {
    elog_e(TAG, "PCA9685 read reg failed, reg: 0x%02X", reg);
  }
  status = HAL_I2C_Master_Receive(&hi2c2, CONFIG_PCA9685_I2C_ADDR, &val, 1, CONFIG_PCA9685_I2C_TIMEOUT);
  if (status != HAL_OK) {
    elog_e(TAG, "PCA9685 read reg failed, reg: 0x%02X", reg);
  }
  elog_v(TAG, "read reg [%02X] = %02X", reg, val);
  return val;
}

static inline void pca9685_set_psc(uint8_t psc)
{
  uint8_t old, new;

  //使能PCA的广播地址响应, 用于同时控制多个PCA9685
  new = 0x00;
  SET_BIT(new, REG_MODE1_ALLCALL_BIT);
  pca9685_write_reg(REG_MODE1, new);

  //读取旧的模式值
  old = pca9685_read_reg(REG_MODE1); 
  new = old; 

  //设置新的模式值: 1.禁用重启  2.启用睡眠模式
  CLEAR_BIT(new, REG_MODE1_RESTART_BIT); //禁用重启功能
  SET_BIT(new, REG_MODE1_SLEEP_BIT);     //启用睡眠模式
  pca9685_write_reg(REG_MODE1, new);

  //设置预分频寄存器
  pca9685_write_reg(REG_PSC, psc);//设置预分频寄存器
  //恢复旧的模式值
  pca9685_write_reg(REG_MODE1, old);
  HAL_Delay(5);

  //设置新的模式值: 1.重启  2.启用自动增加地址  3.启用广播地址响应
  SET_BIT(old, REG_MODE1_RESTART_BIT|REG_MODE1_AUTOINC_BIT|REG_MODE1_ALLCALL_BIT);
  pca9685_write_reg(REG_MODE1, old);
}

// void pca9685_set_pwm(uint8_t ledx, uint16_t on, uint16_t off)
// {
//   uint8_t buf[5] = {REG_LEDX_BASE(ledx),ONL(on),ONH(on),OFFL(off),OFFH(off)};
//   HAL_I2C_Master_Transmit(&hi2c2, CONFIG_PCA9685_I2C_ADDR, buf, 5, CONFIG_PCA9685_I2C_TIMEOUT);
// }

//! Servo

inline quad_fp servo_angle_mirror(quad_fp angle)
{
  return 180.f - angle;
}

inline quad_fp servo_angle_offset(quad_fp angle, quad_fp offset)
{
  return angle + offset;
}

inline quad_fp servo_angle_limit_thigh(quad_fp angle)
{
  if(angle > CONFIG_SERVO_LIMIT_MAX_T) {
    return CONFIG_SERVO_LIMIT_MAX_T;
  } else if(angle < CONFIG_SERVO_LIMIT_MIN_T) {
    return CONFIG_SERVO_LIMIT_MIN_T;
  }
  return angle;
}

inline quad_fp servo_angle_limit_shank(quad_fp angle)
{
  if(angle > CONFIG_SERVO_LIMIT_MAX_S) {
    return CONFIG_SERVO_LIMIT_MAX_S;
  } else if(angle < CONFIG_SERVO_LIMIT_MIN_S) {
    return CONFIG_SERVO_LIMIT_MIN_S;
  }
  return angle;
}

void servo_set_freq(quad_fp freq)
{
  freq *= 0.98f;
  uint8_t psc = ( (uint8_t)( (quad_fp)(ICLK) / ( (quad_fp)CNT_MAX * freq ) ) ) - 1 ;
  pca9685_set_psc(psc);
}

void servo_set_angle(servo_index index, quad_fp angle)
{
  if (index > SERVO_END) {
    elog_e(TAG,"index %1u:%10.f°", index, angle);
    return;
  }else{
    elog_v(TAG,"index %1u:%10.f°", index, angle);
  }

  uint16_t off = (uint16_t)(angle * 2.276f + 0.5f) + 102;
  uint8_t buf[5] = {REG_LEDX_BASE((uint8_t)index), ONL(0), ONH(0), OFFL(off), OFFH(off)};
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
    &hi2c2, CONFIG_PCA9685_I2C_ADDR, buf, sizeof(buf), CONFIG_PCA9685_I2C_TIMEOUT);

  if (status != HAL_OK) {
    elog_e(TAG, "hal i2c transmit error");
  }
}

void servo_set_angle_sync(quad_fp angles[SERVO_COUNT])
{
  uint16_t off[8] = {0};
  uint8_t buf[8*4+1] = {REG_LED_BASE};

  for (uint32_t i = 0; i < 8; i++) {
    off[i] = (uint32_t)(angles[i] * 2.276f + 0.5f) + 102;
  }
  
  for (uint32_t i = 0; i < SERVO_COUNT; i++) {
    buf[i*4+1] = ONL(0);
    buf[i*4+2] = ONH(0);
    buf[i*4+3] = OFFL(off[i]);
    buf[i*4+4] = OFFH(off[i]);
  }

  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
    &hi2c2, CONFIG_PCA9685_I2C_ADDR, buf, sizeof(buf), CONFIG_PCA9685_I2C_TIMEOUT);
  if (status != HAL_OK) {
    elog_e(TAG, "hal i2c transmit error");
  }
}


