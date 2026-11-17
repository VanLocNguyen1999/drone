/*
 * mpu_6050.c
 *
 *  Created on: Nov 10, 2026
 *      Author: vanlo
 */
#include "mpu_6050.h"

#define MPU_6050_ID          0x68<<1
#define DLPF_CONFIG          0x1A
#define GYRO_CONFIG          0x1B
#define ACCEL_CONFIG         0x1C
#define WHO_AM_I             0x75
#define GET_MPU_DATA         0x3B
#define MPU_DATA_SIZE        14
#define POWER_CTRL           0x6B

#define _impl(x)             ((mpu_6050_impl_t*)(x))

typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t temp;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} mpu6050_raw_data_t;

typedef struct {
    float roll;     // nghieng trai, phai
    float pitch;    // nghieng truoc, sau
    float yaw;      // xoay
} mpu6050_angle_data_t;

typedef struct mpu_6050_impl {
    sm_hal_i2c_t* m_i2c;

    mpu6050_raw_data_t m_raw_data;
    mpu6050_data_t m_data;
    mpu6050_angle_data_t m_angle;

    float m_accel_sensitivity;
    float m_gyro_sensitivity;

    mpu6050_data_t m_offset;    // bo sung offset, don vi g va do/s
} mpu_6050_impl_t;

static mpu_6050_impl_t g_mpu_6050;

mpu_6050_t* mpu_6050_init(sm_hal_i2c_t* i2c) {
    if (!i2c) return NULL;

    g_mpu_6050.m_i2c = i2c;
    g_mpu_6050.m_offset = (mpu6050_data_t){0};
    g_mpu_6050.m_angle = (mpu6050_angle_data_t){0};

    sm_hal_i2c_set_addr(i2c, MPU_6050_ID);

    return &g_mpu_6050;
}
int32_t mpu_6050_set_power_ctrl(mpu_6050_t *_this, uint8_t enable) {

	if (!_this)return -1;

	uint8_t value = 0;
	if (enable) {
		value &= (uint8_t) ~0x40U;
	} else {
		value |= 0x40U;
	}
	if (sm_hal_i2c_write(_impl(_this)->m_i2c, POWER_CTRL, I2C_MEMADD_SIZE_8BIT,
			&value, 1, 100) < 0)
		return -1;
	return 0;
}
int32_t mpu_6050_set_gyro_config(mpu_6050_t* _this, GYRO_CONFIG_DATA gyro_config) {
    if (!_this) return -1;

    uint8_t value = (uint8_t)gyro_config;

    if (value > 0x18 || (value & 0x07)) return -1;

    if (sm_hal_i2c_write(_impl(_this)->m_i2c, GYRO_CONFIG, I2C_MEMADD_SIZE_8BIT,
            &value, 1, 100) < 0) return -1;

    static const float sensitivity[] = { 131.0f, 65.5f, 32.8f, 16.4f };

    _impl(_this)->m_gyro_sensitivity = 1.0f / sensitivity[value >> 3];

    return 0;
}

int32_t mpu_6050_set_accel_config(mpu_6050_t* _this, ACCEL_CONFIG_DATA accel_config) {
    if (!_this) return -1;

    uint8_t value = (uint8_t)accel_config;

    if (value > 0x18 || (value & 0x07)) return -1;

    if (sm_hal_i2c_write(_impl(_this)->m_i2c, ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT,
            &value, 1, 100) < 0) return -1;

    _impl(_this)->m_accel_sensitivity = (float)((1U << (value >> 3)) / 16384.0f);

    return 0;
}

static int32_t mpu_6050_get_data_raw(mpu_6050_impl_t* _this) {
    if (!_this) return -1;

    mpu6050_raw_data_t raw_data = {0};

    int32_t ret = sm_hal_i2c_read(_this->m_i2c, GET_MPU_DATA, I2C_MEMADD_SIZE_8BIT,
    		(uint8_t*)&raw_data, MPU_DATA_SIZE, 100);

    if (ret < 0) return -1;

    uint8_t* p = (uint8_t*)&raw_data;

    for (uint32_t i = 0; i < MPU_DATA_SIZE; i += 2) {
        uint8_t temp = p[i];
        p[i] = p[i + 1];
        p[i + 1] = temp;
    }

    _this->m_raw_data = raw_data;

    _this->m_data.accel_x = raw_data.ax * _this->m_accel_sensitivity - _this->m_offset.accel_x;
    _this->m_data.accel_y = raw_data.ay * _this->m_accel_sensitivity - _this->m_offset.accel_y;
    _this->m_data.accel_z = raw_data.az * _this->m_accel_sensitivity - _this->m_offset.accel_z;

    _this->m_data.gyro_x = raw_data.gx * _this->m_gyro_sensitivity - _this->m_offset.gyro_x;
    _this->m_data.gyro_y = raw_data.gy * _this->m_gyro_sensitivity - _this->m_offset.gyro_y;
    _this->m_data.gyro_z = raw_data.gz * _this->m_gyro_sensitivity - _this->m_offset.gyro_z;

    return 0;
}

// Dat MPU6050 nam ngang, dung yen, truc Z huong len khi calib.
int32_t mpu_6050_calibrate(mpu_6050_t* _this, mpu6050_data_t* _offset_out) {
    if (!_this) return -1;

    if (_impl(_this)->m_accel_sensitivity < 0.0f ||
        _impl(_this)->m_gyro_sensitivity < 0.0f) return -1;

    mpu6050_data_t offset = {0};
    const uint32_t sample_count = 2000;

    for (uint32_t i = 0; i < sample_count; i++) {
        if (mpu_6050_get_data_raw(_impl(_this)) < 0) return -1;

        offset.accel_x += _impl(_this)->m_raw_data.ax * _impl(_this)->m_accel_sensitivity;
        offset.accel_y += _impl(_this)->m_raw_data.ay * _impl(_this)->m_accel_sensitivity;
        offset.accel_z += _impl(_this)->m_raw_data.az * _impl(_this)->m_accel_sensitivity;

        offset.gyro_x += _impl(_this)->m_raw_data.gx * _impl(_this)->m_gyro_sensitivity;
        offset.gyro_y += _impl(_this)->m_raw_data.gy * _impl(_this)->m_gyro_sensitivity;
        offset.gyro_z += _impl(_this)->m_raw_data.gz * _impl(_this)->m_gyro_sensitivity;

        HAL_Delay(2);
    }

    offset.accel_x /= sample_count;
    offset.accel_y /= sample_count;
    offset.accel_z = offset.accel_z / sample_count - 1.0f;

    offset.gyro_x /= sample_count;
    offset.gyro_y /= sample_count;
    offset.gyro_z /= sample_count;

    _impl(_this)->m_offset = offset;
    memcpy(_offset_out,&_impl(_this)->m_offset,sizeof(mpu6050_data_t));
    _impl(_this)->m_angle.roll = 0.0f;
    _impl(_this)->m_angle.pitch = 0.0f;
    _impl(_this)->m_angle.yaw = 0.0f;

    return 0;
}

int32_t mpu_6050_set_offset(mpu_6050_t* _this, mpu6050_data_t _offset){
	if (!_this) return -1;
	memcpy(&_impl(_this)->m_offset,&_offset,sizeof(mpu6050_data_t));
	return 0;
}

int32_t mpu_6050_read_data(mpu_6050_t *_this, mpu6050_data_t *data) {
	if (!_this || !data)
		return -1;
	if (mpu_6050_get_data_raw(_impl(_this)) < 0)
		return -1;
	*data = _impl(_this)->m_data;

	return 0;
}
void mpu_6050_process(mpu_6050_t* _this, uint32_t time_diff) {
    if (!_this) return;

    if (mpu_6050_get_data_raw(_impl(_this)) < 0) return;

    float roll_acc = atan2f(_impl(_this)->m_data.accel_y,
                            _impl(_this)->m_data.accel_z) * 57.29578f;

    float pitch_acc = atan2f(-_impl(_this)->m_data.accel_x,
                            sqrtf(_impl(_this)->m_data.accel_y * _impl(_this)->m_data.accel_y +
                                  _impl(_this)->m_data.accel_z * _impl(_this)->m_data.accel_z)) * 57.29578f;

    float alpha = 0.995f;

    // time_diff tinh bang ms, can doi sang giay khi tich phan gyro.
    _impl(_this)->m_angle.roll = alpha * (_impl(_this)->m_angle.roll +
            _impl(_this)->m_data.gyro_x * time_diff * 0.001f)
            + (1.0f - alpha) * roll_acc;

    _impl(_this)->m_angle.pitch = alpha * (_impl(_this)->m_angle.pitch +
            _impl(_this)->m_data.gyro_y * time_diff * 0.001f)
            + (1.0f - alpha) * pitch_acc;

    _impl(_this)->m_angle.yaw += _impl(_this)->m_data.gyro_z * time_diff * 0.001f;
}
