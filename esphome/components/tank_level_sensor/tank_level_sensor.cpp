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

float TankLevelSensor::get_tank_level(float v) {
  float high, low;
  float vdiff, vdiff_last;
  TankLevel lastLevel = levels_[0];
  vdiff_last = 100;

  for (TankLevel tl : levels_) {
    // for given levels find the level closest to the measured value

    // check difference between voltage and level voltage

    // high= tl.voltage * (1.0f + (margin_percent_/100.0f));
    // low= tl.voltage * (1.0f -( margin_percent_/100.0f));
    vdiff = v - tl.voltage;
    ESP_LOGD(TAG, "level: %0f\tvoltage:%.2f\t  levelVoltage: %.2f \t difference: %.2f \t lastDifference: %.2f",
             tl.percent_full, v, tl.voltage, vdiff, vdiff_last);
    if (abs(vdiff) > abs(vdiff_last)) {
      return lastLevel.percent_full;
    } else {
      lastLevel = tl;
      vdiff_last = vdiff;
    }
    }
  return lastLevel.percent_full;
}

void TankLevelSensor::set_output_pin(GPIOPin *pin) { this->output_pin_ = pin; }

void TankLevelSensor::set_margin_percent(float v) { this->margin_percent_ = v; }
void TankLevelSensor::set_tank_capacity(float v) { this->tank_capacity_ = v; }

void TankLevelSensor::add_level(float voltage, float percent_full) {
  TankLevel level{
      .voltage = voltage,
      .percent_full = percent_full,
  };
  this->levels_.push_back(level);
}

// Driver function to sort the vector elements
// by voltage value
static bool sort_by_v(const TankLevel &a, const TankLevel &b) { return (a.voltage < b.voltage); }

void TankLevelSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Tank Level Sensor...");
  this->sample_count_ = 0;
  this->adc_reading_ = 0;
  this->pause_count_ = 0;

  if (this->output_pin_ != nullptr) {
    this->output_pin_->pin_mode(gpio::FLAG_OUTPUT);
    this->output_pin_->setup();
    this->output_pin_->digital_write(false);
  }
  this->adc_sensor_->set_update_interval(2147483646);
  this->adc_sensor_->set_internal(true);
  std::sort(this->levels_.begin(), this->levels_.end(), sort_by_v);

  // reading average values is slow, so do this outside of the update method
  this->set_interval("level_check", 5, [this]() { this->read_voltage(); });
}
void TankLevelSensor::read_voltage() {
  // static int sample_count = 0;
  // static float adc_reading = 0;
  // static int pause_count = 0;
  if (this->pause_count_ > 0) {
    this->pause_count_--;
    return;
  }
  // if start of loop, turn on output pin and wait for next loop
  if (this->sample_count_ == -1) {
    this->output_pin_->digital_write(true);
    this->adc_reading_ = 0;
    this->sample_count_++;
    return;
  }

  this->adc_reading_ += this->adc_sensor_->sample();
  this->sample_count_++;

  if (this->sample_count_ >= NO_OF_SAMPLES) {
    this->adc_reading_ /= this->sample_count_;
    this->output_pin_->digital_write(false);
    this->last_level_ = this->get_tank_level(this->adc_reading_);
    this->sample_count_ = -1;
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
  ESP_LOGD(TAG, "Tank Level:%0f\tAmount Left: %.1f", level, amt_left);
  this->capacity_sensor_->publish_state(amt_left);
  this->level_sensor_->publish_state(level);
}

float TankLevelSensor::get_setup_priority() const { return setup_priority::AFTER_WIFI; }

}  // namespace tank_level_sensor
}  // namespace esphome
