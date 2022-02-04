#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/preferences.h"
#include "esphome/core/helpers.h"
#include "mpu6050_driver.h"


namespace esphome {
namespace mpu6050 {

/// Struct used to save the calibration values in restore memory.
/// Make sure to update RESTORE_STATE_VERSION when changing the struct entries.
struct MPU6050RestoreState {
  bool has_data = false;
  int16_t accel_x_offset;
  int16_t accel_y_offset;
  int16_t accel_z_offset;

  int16_t gyro_x_offset;
  int16_t gyro_y_offset;
  int16_t gyro_z_offset;

  };

class MPU6050Component : public PollingComponent, public i2c::I2CDevice, public EntityBase {
 public:
  void setup() override;
  void dump_config() override;

  void update() override;

  float get_setup_priority() const override;

  /// Restore the state of the climate device, call this from your setup() method.
  MPU6050RestoreState restore_state_();
  
  /** Internal method to save the state of the climate device to recover memory. 
   */
  void save_state_( MPU6050RestoreState * state);

  void calibrate(bool do_accel, bool do_gyro);
  void set_accel_x_sensor(sensor::Sensor *accel_x_sensor) { accel_x_sensor_ = accel_x_sensor; }
  void set_accel_y_sensor(sensor::Sensor *accel_y_sensor) { accel_y_sensor_ = accel_y_sensor; }
  void set_accel_z_sensor(sensor::Sensor *accel_z_sensor) { accel_z_sensor_ = accel_z_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void set_gyro_x_sensor(sensor::Sensor *gyro_x_sensor) { gyro_x_sensor_ = gyro_x_sensor; }
  void set_gyro_y_sensor(sensor::Sensor *gyro_y_sensor) { gyro_y_sensor_ = gyro_y_sensor; }
  void set_gyro_z_sensor(sensor::Sensor *gyro_z_sensor) { gyro_z_sensor_ = gyro_z_sensor; }
  void set_yaw_sensor(sensor::Sensor *yaw_sensor) { yaw_sensor_ = yaw_sensor; }
  void set_roll_sensor(sensor::Sensor *roll_sensor) { roll_sensor_ = roll_sensor; }
  void set_pitch_sensor(sensor::Sensor *pitch_sensor) { pitch_sensor_ = pitch_sensor; }
  uint32_t hash_base() override;
 protected:  
  mpu6050_driver mpu;
  float accel_x;
  float accel_y;
  float accel_z;
  float gyro_x;
  float gyro_y;
  float gyro_z;
  float temperature;
  float yaw;
  float pitch;
  float roll;
  int16_t accel_x_offset;
  int16_t accel_y_offset;
  int16_t accel_z_offset;

  int16_t gyro_x_offset;
  int16_t gyro_y_offset;
  int16_t gyro_z_offset;

  ESPPreferenceObject rtc_;
  
  void getReading();
  
  
  sensor::Sensor *accel_x_sensor_{nullptr};
  sensor::Sensor *accel_y_sensor_{nullptr};
  sensor::Sensor *accel_z_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *gyro_x_sensor_{nullptr};
  sensor::Sensor *gyro_y_sensor_{nullptr};
  sensor::Sensor *gyro_z_sensor_{nullptr};
  
  sensor::Sensor *yaw_sensor_{nullptr};
  sensor::Sensor *roll_sensor_{nullptr};
  sensor::Sensor *pitch_sensor_{nullptr};
};
;

}  // namespace mpu6050
}  // namespace esphome
