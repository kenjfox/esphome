#include "freertos/FreeRTOS.h"
#include "tank_level_sensor.h"
#include "esphome/core/log.h"
#include <stdio.h>
#include "freertos/task.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

namespace esphome {
namespace tank_level_sensor {

static const char *const TAG = "tank_level.sensor";
static const int NO_OF_SAMPLES = 32;
static const int PAUSE_INTERVALS = 1000;
static const uint32_t RESTORE_STATE_VERSION = 0x848EF6ADUL;

void TankLevelSensor::restore_state_() {
  this->rtc_ = global_preferences->make_preference<TankLevelSensorRestoreState>(this->get_object_id_hash() ^
                                                                                RESTORE_STATE_VERSION);
  TankLevelSensorRestoreState recovered{};
  if (!this->rtc_.load(&recovered)) {
    recovered.has_data = false;
  } else {
    recovered.has_data = true;
    this->voltage_high_ = recovered.voltage_high;
    this->voltage_low_ = recovered.voltage_low;
  }
  // return recovered;
}

void TankLevelSensor::save_state_() {
  TankLevelSensorRestoreState state{};
  state.voltage_high = this->voltage_high_;
  state.voltage_low = this->voltage_low_;
  this->rtc_.save(&state);
}

// get tank level:  find difference between measured voltage and empty voltage
//                   and difference between empty voltage and full voltage
// returns percent of the measure voltage compared to the full voltage

float TankLevelSensor::get_tank_level(float v) {
  // adjust full/empty voltages if auto-ranging
  float range = 0;
  float relative_v = 0;
  if (this->auto_range_)
    this->set_limits(v);

  range = this->voltage_high_ - this->voltage_low_;
  ESP_LOGD(TAG, " %s range=%.2f", this->get_name().c_str(), range);

  relative_v = v - this->voltage_low_;
  if (this->invert_)
    return (1.0f - (relative_v / range)) * 100.0f;
  else
    return (relative_v / range) * 100.0f;
}

void TankLevelSensor::set_limits(float v) {
  if (v > this->voltage_high_)
    this->voltage_high_ = v;
  else if (v < this->voltage_low_)
    this->voltage_low_ = v;
  else
    return;

  ESP_LOGD(TAG, " %s limits updated.  Low=%.2f  High=%.2f", this->get_name().c_str(), voltage_low_, voltage_high_);
  this->save_state_();
}

void TankLevelSensor::set_tank_capacity(float v) { this->tank_capacity_ = v; }
void TankLevelSensor::set_voltage_high(float v) { this->voltage_high_ = v; }
void TankLevelSensor::set_voltage_low(float v) { this->voltage_low_ = v; }
void TankLevelSensor::set_auto_range(bool setauto) { this->auto_range_ = setauto; }
void TankLevelSensor::set_invert(bool invert) { this->invert_ = invert; }
void TankLevelSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Tank Level Sensor: %s", this->get_name().c_str());
  this->sample_count_ = 0;
  this->adc_reading_ = 0;
  this->pause_count_ = 0;

  this->adc_sensor_->set_update_interval(2147483646);
  this->adc_sensor_->set_internal(true);

  // reading average values is slow, so do this outside of the update method
  this->set_interval("level_check", 5, [this]() { this->read_voltage(); });
}
void TankLevelSensor::read_voltage() {
  if (this->pause_count_ > 0) {
    this->pause_count_--;
    return;
  }

  this->adc_reading_ += this->adc_sensor_->sample();
  this->sample_count_++;

  if (this->sample_count_ >= NO_OF_SAMPLES) {
    this->adc_reading_ /= this->sample_count_;
    this->last_level_ = this->get_tank_level(this->adc_reading_);
    this->sample_count_ = 0;
    this->pause_count_ = PAUSE_INTERVALS;  // pause from sampling until PAUSE_INTERVALS of the loop
    ESP_LOGD(TAG, "'%s': Got voltage=%.4fV\tlevel=%.1f", this->get_name().c_str(), this->adc_reading_,
             this->last_level_);
  }
}

void TankLevelSensor::dump_config() { LOG_SENSOR("", "TankLevel Sensor", this); }

void TankLevelSensor::update() {
  // Enable sensor
  float level = this->last_level_;
  float amt_left = level * this->tank_capacity_ / 100.0f;
  ESP_LOGD(TAG, "%s Tank Level:%0f\tAmount Left: %.1f", this->get_name().c_str(), level, amt_left);
  this->capacity_sensor_->publish_state(amt_left);
  this->level_sensor_->publish_state(level);
}

float TankLevelSensor::get_setup_priority() const { return setup_priority::AFTER_WIFI; }

}  // namespace tank_level_sensor
}  // namespace esphome
