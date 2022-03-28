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
static const int NO_OF_SAMPLES = 64;


float TankLevelSensor::get_tank_level(float v) {
    float high, low;
    
    for(TankLevel tl : levels_)
    {
        
        high= tl.voltage * (1.0f + margin_percent_);
        low= tl.voltage * (1.0f - margin_percent_);
        //printf("voltage:%.1f\tchecking between %.1f and %.1f\n", v, low, high);
        if (v >= low && v <= high)
        {
            return tl.percent_full;
        }
    }
    return -100; // should not happen
}

void TankLevelSensor::set_output_pin(GPIOPin *pin)
{ this->output_pin_ = pin; }

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

// Driver function to sort the vector elements
// by voltage value
static bool sort_by_v(const TankLevel &a,
              const TankLevel &b)
{
    return (a.voltage < b.voltage);
}

void TankLevelSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Tank Level Sensor...");
  if (this->output_pin_ != nullptr) {
      this->output_pin_->pin_mode(gpio::FLAG_OUTPUT);
      this->output_pin_->setup();
      this->output_pin_->digital_write(false);
    } 
  this->adc_sensor_->set_update_interval(2147483646);
  this->adc_sensor_->set_internal(true);
  std::sort(this->levels_.begin(), this->levels_.end(), sort_by_v);

}

void TankLevelSensor::dump_config() {
  LOG_SENSOR("", "TankLevel Sensor", this);

}


void TankLevelSensor::update() {
  // Enable sensor
  ESP_LOGD(TAG, "Sending update...");
   float adc_reading = 0;
  this->output_pin_->digital_write(true);
  vTaskDelay(pdMS_TO_TICKS(20));
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
     adc_reading += this->adc_sensor_->sample();
     vTaskDelay(pdMS_TO_TICKS(10));
  }
  adc_reading /= NO_OF_SAMPLES;
  
  this->output_pin_->digital_write(false);

  float level = this->get_tank_level(adc_reading);
  float amt_left = level * this->tank_capacity_ / 100.0f;

  ESP_LOGD(TAG, "'%s': Got voltage=%.4fV\tlevel=%.1f", this->get_name().c_str(), adc_reading, level);

  this->capacity_sensor_->publish_state(amt_left);
  this->level_sensor_->publish_state(level);
}


float TankLevelSensor::get_setup_priority() const { return setup_priority::AFTER_WIFI; }


}  // namespace tank_level_sensor
}  // namespace esphome
