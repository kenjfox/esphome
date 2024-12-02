#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/adc/adc_sensor.h"
#include "esphome/core/hal.h"
#include "esphome/core/preferences.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace tank_level_sensor {

struct TankLevel {
  float voltage;
  float percent_full;
};

/// Struct used to save the min/max values in restore memory.
/// Make sure to update RESTORE_STATE_VERSION when changing the struct entries.
struct TankLevelSensorRestoreState {
  bool has_data = false;
  float voltage_high;
  float voltage_low;
};

/// This class provides an easy way to map fixed voltages to tank levels
class TankLevelSensor : public sensor::Sensor, public PollingComponent {
 public:
  // void set_margin_percent(float v);
  void set_tank_capacity(float v);
  void set_limits(float v);

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;
  void set_level_sensor(sensor::Sensor *level_sensor) { this->level_sensor_ = level_sensor; }
  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { this->voltage_sensor_ = voltage_sensor; }
  void set_capacity_sensor(sensor::Sensor *capacity_sensor) { this->capacity_sensor_ = capacity_sensor; }
  void set_adc(adc::ADCSensor *adc_sensor) { this->adc_sensor_ = adc_sensor; }
  // void add_level(float voltage, float percent_full);
  void save_state_();

  void restore_state_();
  void set_voltage_low(float v);
  void set_voltage_high(float v);
  void set_auto_range(bool setauto);
  void set_invert(bool invert);

 protected:
  // float max_level;
  // float min_level;

  float voltage_low_;
  float voltage_high_;
  bool auto_range_;
  bool invert_;

  ESPPreferenceObject rtc_;

  void read_voltage();
  float get_tank_level(float v);
  // std::vector<TankLevel> levels_{};
  sensor::Sensor *level_sensor_;
  sensor::Sensor *capacity_sensor_;
  sensor::Sensor *voltage_sensor_;
  adc::ADCSensor *adc_sensor_;
  // float margin_percent_;
  float tank_capacity_;
  float last_level_;
  int sample_count_ = 0;
  float adc_reading_ = 0;
  int pause_count_ = 0;
};

}  // namespace tank_level_sensor
}  // namespace esphome
