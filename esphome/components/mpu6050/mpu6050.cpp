
#include "mpu6050.h"
#include "esphome/core/log.h"
#include "mpu6050_driver.h"
//#include "MPU6050_6Axis_MotionApps20.h"

namespace esphome {
namespace mpu6050 {

constexpr int8_t MPU6050_CALIBRATION_LOOPS = 7;
static const char *const TAG = "mpu6050";
#define mpu6050_driver_GYRO_GAIN 16.4
#define mpu6050_driver_ACCEL_GAIN 16384.0

static const uint32_t RESTORE_STATE_VERSION = 0x848EA6ADUL;
uint32_t MPU6050Component::hash_base() { return 3114124497UL; }

MPU6050RestoreState MPU6050Component::restore_state_() {
  this->rtc_ =
      global_preferences->make_preference<MPU6050RestoreState>(this->get_object_id_hash() ^ RESTORE_STATE_VERSION);
  MPU6050RestoreState recovered{};
  if (!this->rtc_.load(&recovered)) {
    recovered.has_data = false;
  } else {
    recovered.has_data = true;
  }
  return recovered;
}

void MPU6050Component::reset() {
  mpu.reset();
  delay_microseconds_safe(50000);
  // clear offsets
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
  MPU6050RestoreState state{};
  state.accel_x_offset = 0;
  state.accel_y_offset = 0;
  state.accel_z_offset = 0;
  state.gyro_x_offset = 0;
  state.gyro_y_offset = 0;
  state.gyro_z_offset = 0;
  this->rtc_.save(&state);

  ESP_LOGW(TAG, "MPU6050 RESET.");
}
void MPU6050Component::calibrate(bool doAccel, bool doGyro) {
  ESP_LOGD(TAG, "beginning calibration...");
  MPU6050RestoreState state{};
  if (doAccel) {
    ESP_LOGD(TAG, "   Accel");

    mpu.CalibrateAccel(MPU6050_CALIBRATION_LOOPS);
    state.accel_x_offset = mpu.getXAccelOffset();
    state.accel_y_offset = mpu.getYAccelOffset();
    state.accel_z_offset = mpu.getZAccelOffset();
  }
  if (doGyro) {
    ESP_LOGD(TAG, "   Gyro");

    mpu.CalibrateGyro(MPU6050_CALIBRATION_LOOPS);
    state.gyro_x_offset = mpu.getXGyroOffset();
    state.gyro_y_offset = mpu.getYGyroOffset();
    state.gyro_z_offset = mpu.getZGyroOffset();
  }
  ESP_LOGD(TAG, "   Saving to rtc.");
  this->rtc_.save(&state);
  ESP_LOGD(TAG, "Calibration done.");
}

void MPU6050Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MPU6050Component...");
  mpu = mpu6050_driver();
  uint8_t devStatus;  // return status after each device operation (0 = success, !0 = error)
  mpu.initialize();

  devStatus = mpu.dmpInitialize();
  if (devStatus != 0) {
    ESP_LOGE(TAG, "mpu6050_driver DMP initialize failed!(return code %i)", devStatus);
  }
  ESP_LOGD(TAG, "Restoring state...");
  MPU6050RestoreState state;
  state = this->restore_state_();
  if (state.has_data) {
    ESP_LOGD(TAG, "   State data found...");
    mpu.setXGyroOffset(state.gyro_x_offset);
    mpu.setYGyroOffset(state.gyro_y_offset);
    mpu.setZGyroOffset(state.gyro_z_offset);
    mpu.setXAccelOffset(state.accel_x_offset);
    mpu.setYAccelOffset(state.accel_y_offset);
    mpu.setZAccelOffset(state.accel_z_offset);
  } else {
    ESP_LOGD(TAG, "   No State data found.  Calibrating.");
    this->calibrate(true, true);
  }

  ESP_LOGD(TAG, "Enabling DMP");
  mpu.setDMPEnabled(true);
  ESP_LOGD(TAG, "Setup() done.");
}

void MPU6050Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MPU6050Component:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with MPU6050Component failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Acceleration X", this->accel_x_sensor_);
  LOG_SENSOR("  ", "Acceleration Y", this->accel_y_sensor_);
  LOG_SENSOR("  ", "Acceleration Z", this->accel_z_sensor_);
  LOG_SENSOR("  ", "Gyro X", this->gyro_x_sensor_);
  LOG_SENSOR("  ", "Gyro Y", this->gyro_y_sensor_);
  LOG_SENSOR("  ", "Gyro Z", this->gyro_z_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Yaw", this->yaw_sensor_);
  LOG_SENSOR("  ", "Pitch", this->pitch_sensor_);
  LOG_SENSOR("  ", "Roll", this->roll_sensor_);
}

void MPU6050Component::getReading() {
  uint16_t packetSize = 42;  // expected DMP packet size (default is 42 bytes)
  uint16_t fifoCount;        // count of all bytes currently in FIFO
  uint8_t fifoBuffer[64];    // FIFO storage buffer
  uint8_t mpuIntStatus;      // holds actual interrupt status byte from MPU

  // orientation/motion vars
  Quaternion q;         // [w, x, y, z]         quaternion container
  VectorInt16 aa;       // [x, y, z]            accel sensor measurements
  VectorInt16 gy;       // [x, y, z]            gyro sensor measurements
  VectorInt16 aaReal;   // [x, y, z]            gravity-free accel sensor measurements
  VectorInt16 aaWorld;  // [x, y, z]            world-frame accel sensor measurements
  VectorFloat gravity;  // [x, y, z]            gravity vector
  float euler[3];       // [psi, theta, phi]    Euler angle container
  float ypr[3];         // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

  mpuIntStatus = mpu.getIntStatus();
  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    ESP_LOGD(TAG, "mpuStatus is 0x10: %02X \t fifoCount: %d", mpuIntStatus, fifoCount);
    mpu.resetFIFO();

    // otherwise, check for DMP data ready interrupt frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize)
      fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    mpu.dmpGetQuaternion(&q, fifoBuffer);

    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    mpu.dmpGetGyro(&gy, fifoBuffer);

    this->accel_x = aaReal.x / mpu6050_driver_ACCEL_GAIN;  //* GRAVITY_EARTH
    this->accel_y = aaReal.y / mpu6050_driver_ACCEL_GAIN;  //* GRAVITY_EARTH
    this->accel_z = aaReal.z / mpu6050_driver_ACCEL_GAIN;  //* GRAVITY_EARTH;
    this->temperature = mpu.getTemperature() / 340.0f + 36.53f;

    this->gyro_x = gy.x / mpu6050_driver_GYRO_GAIN;
    this->gyro_y = gy.y / mpu6050_driver_GYRO_GAIN;
    this->gyro_z = gy.z / mpu6050_driver_GYRO_GAIN;

    this->yaw = ypr[0] * 180 / M_PI;
    this->pitch = ypr[1] * 180 / M_PI;
    this->roll = ypr[2] * 180 / M_PI;
  } else {
    ESP_LOGD(TAG, "mpuStatus is something else: %u \t fifoCount: %d", mpuIntStatus, fifoCount);
  }
  u_int8_t p;
  mpu.ReadRegister(mpu6050_driver_RA_PWR_MGMT_1, &p, sizeof(p));

  ESP_LOGD(TAG, "Power Management...%02X", p);
  return;
}

void MPU6050Component::update() {
  ESP_LOGD(TAG, "    Updating MPU6050Component...");

  getReading();

  ESP_LOGD(TAG,
           "Got accel={x=%.3f m/s², y=%.3f m/s², z=%.3f m/s²}, "
           "gyro={x=%.3f °/s, y=%.3f °/s, z=%.3f °/s}, temp=%.3f°C",
           this->accel_x, this->accel_y, this->accel_z, this->gyro_x, this->gyro_y, this->gyro_z, this->temperature);

  if (this->accel_x_sensor_ != nullptr)
    this->accel_x_sensor_->publish_state(this->accel_x);
  if (this->accel_y_sensor_ != nullptr)
    this->accel_y_sensor_->publish_state(this->accel_y);
  if (this->accel_z_sensor_ != nullptr)
    this->accel_z_sensor_->publish_state(this->accel_z);

  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(this->temperature);

  if (this->gyro_x_sensor_ != nullptr)
    this->gyro_x_sensor_->publish_state(this->gyro_x);
  if (this->gyro_y_sensor_ != nullptr)
    this->gyro_y_sensor_->publish_state(this->gyro_y);
  if (this->gyro_z_sensor_ != nullptr)
    this->gyro_z_sensor_->publish_state(this->gyro_z);
  if (this->yaw_sensor_ != nullptr)
    this->yaw_sensor_->publish_state(this->yaw);
  if (this->pitch_sensor_ != nullptr)
    this->pitch_sensor_->publish_state(this->pitch);
  if (this->roll_sensor_ != nullptr)
    this->roll_sensor_->publish_state(this->roll);

  this->status_clear_warning();
}
float MPU6050Component::get_setup_priority() const { return setup_priority::AFTER_WIFI; }

}  // namespace mpu6050
}  // namespace esphome
