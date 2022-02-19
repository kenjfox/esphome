#include "tank_level_sensor.h"
#include "esphome/core/log.h"
#include <stdio.h>

#include "esp_adc_cal.h"
#include "esp_log.h"

namespace esphome {
namespace tank_level_sensor {

static const char *const TAG = "tank_level.sensor";




void TankLevelSensor::set_margin_percent(float v){
  this->margin_percent_=v;
}
void TankLevelSensor::set_tank_capacity(float v){
  this->tank_capacity_=v;
}

void TankLevelSensor::add_level(float voltage, float percent_full){
  TankLevel level{
      .voltage = voltage,
      .percent_full = percent_full,
  };
  this->levels_.push_back(level);

}

void TankLevelSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Tank Level Sensor...");
  if (this->output_pin_ != nullptr) {
      this->output_pin_->pin_mode(gpio::FLAG_OUTPUT);
      this->output_pin_->setup();
      this->output_pin_->digital_write(false);
    } 
 
}

void TankLevelSensor::dump_config() {
  LOG_SENSOR("", "TankLevel Sensor", this);

}



void TankLevelSensor::update() {
  // Enable sensor
  ESP_LOGV(TAG, "Sending update...");
  
  float value_v = this->adc_sensor_->sample();
  ESP_LOGV(TAG, "'%s': Got voltage=%.4fV", this->get_name().c_str(), value_v);
  this->publish_state(value_v);
}


float TankLevelSensor::get_setup_priority() const { return setup_priority::DATA; }


}  // namespace tank_level_sensor
}  // namespace esphome
