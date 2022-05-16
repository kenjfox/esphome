#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/adc/adc_sensor.h"
#include "esphome/core/hal.h"

namespace esphome {
  namespace tank_level_sensor {

    struct TankLevel {
      float voltage;
      float percent_full;
    };

    /// This class provides an easy way to map fixed voltages to tank levels
    class TankLevelSensor :  public sensor::Sensor, public PollingComponent {
    public:
      void set_output_pin (GPIOPin *pin) ;
      void set_margin_percent(float v);
      void set_tank_capacity(float v);
      void setup() override;
      void dump_config() override;
      float get_setup_priority() const override;
      void update() override;
      void set_level_sensor(sensor::Sensor *level_sensor) { this->level_sensor_ = level_sensor; }
      void set_capacity_sensor(sensor::Sensor *capacity_sensor) { this->capacity_sensor_ = capacity_sensor; }
      void set_adc(adc::ADCSensor *adc_sensor) { this->adc_sensor_ = adc_sensor;}
      void add_level(float voltage, float percent_full);
    protected:
      void read_voltage();
      float get_tank_level(float v);
      std::vector<TankLevel> levels_{};
      GPIOPin *output_pin_ {nullptr};
      //GPIOPin *sense_pin_ {nullptr};
      sensor::Sensor *level_sensor_;
      sensor::Sensor *capacity_sensor_;
      adc::ADCSensor *adc_sensor_;
      float margin_percent_;
      float tank_capacity_;
      float last_level_;
      int sample_count_ = 0;
      float adc_reading_ = 0;
      int pause_count_ = 0;
    };

  }  // namespace tank_level_sensor 
}  // namespace esphome
