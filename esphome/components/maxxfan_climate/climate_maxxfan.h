#pragma once
#include "esphome/components/climate/climate.h"
#include "esphome/components/number/number.h"
#include "esphome/components/homeassistant/sensor/homeassistant_sensor.h"
#include "fanspeed.h"
#include <map>
#include <string>

namespace esphome {
namespace maxxfan_climate {

using climate::ClimateCall;
using climate::ClimatePreset;
using climate::ClimateTraits;
using climate::ClimateMode;
using climate::ClimateSwingMode;
using climate::ClimateFanMode;
using climate::ClimateDeviceRestoreState;

using esphome::to_string;
using std::string;
using std::map;

const map<const uint8_t, const char *> FAN_SPEEDS = {{10, " 10"}, {20, " 20"}, {30, " 30"}, {40, " 40"}, {50, " 50"},
                                                     {60, " 60"}, {70, " 70"}, {80, " 80"}, {90, " 90"}, {100, "100"}};

class MaxxFan: public climate::Climate {
 public:
  // Temperature
  const float TEMP_MIN = 15.5556f;  // 60F
  const float TEMP_MAX = 29.4444f;  // 85F

  const char *PRESET_VENT_ONLY = "VENT ONLY";
  const char *PRESET_AIR_IN = "AIR IN";
  const char *PRESET_AIR_OUT = "AIR OUT";

  MaxxFan();
  void setup() ;
  void set_fahrenheit(bool set);
  void set_fanspeed_component(FanSpeed *num);
  void set_homeassistant_sensor(homeassistant::HomeassistantSensor *sensor);
  
  void on_fanspeed_state(float speed);

 protected:
  void control(const climate::ClimateCall &call) override;
  ClimateTraits traits() override;
  void transmit_state();
  void save_state_();
  void set_fanspeed(uint8_t);
  uint8_t get_fanspeed();
  bool get_direction();
  void set_direction(bool fwd);
  /// Transmit via IR the state of this climate controller.

  ClimateMode mode_before_{climate::CLIMATE_MODE_OFF};
  std::string custom_preset_before_{"AIR IN"};

  bool fahrenheit_ = true;
 
  FanSpeed *fanspeed_component_;

  uint8_t fanspeed_{100};
  
};

}  // namespace maxxfan_ir_climate
}  // namespace esphome
