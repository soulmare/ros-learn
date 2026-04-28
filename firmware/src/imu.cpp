#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "imu.h"
#include "config/params.h"

static Adafruit_MPU6050 s_mpu;
static float    s_bias_z   = 0.0f;   // gyro Z bias measured during calibration (rad/s)
static float    s_heading  = 0.0f;   // integrated yaw angle (degrees)
static uint32_t s_last_update_ms  = 0;

void imu_init() {
    Wire.begin();
    if (!s_mpu.begin()) { Serial.println(F("ERR INIT MPU")); return; }
    s_mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    // Calibration
    uint32_t start = millis();
    float sum = 0;
    uint32_t count = 0;
    sensors_event_t accel, gyro, temp;                                                                                             
    while (millis() - start < IMU_CALIBRATION_MS) {
        s_mpu.getEvent(&accel, &gyro, &temp);
        sum += gyro.gyro.z;
        count++;
        delay(IMU_SAMPLE_INTERVAL_MS);
    }
    s_bias_z = sum / count;

    s_last_update_ms = millis();
    Serial.print(F("OK INIT IMU bias="));
    Serial.println(s_bias_z, 6);
}

void imu_update() {
    if (millis() - s_last_update_ms < IMU_SAMPLE_INTERVAL_MS) { return; }

    sensors_event_t accel, gyro, temp;                                                                                             
    s_mpu.getEvent(&accel, &gyro, &temp);

    float yaw_rate_dps = (gyro.gyro.z - s_bias_z) * 57.2958f;
    float dt_s = (millis() - s_last_update_ms) / 1000.0f;
    s_heading += yaw_rate_dps * dt_s;

    s_last_update_ms = millis();
}

float imu_get_heading() {
    return s_heading;
}

void imu_reset_heading() {
    s_heading = 0.0f;
}
