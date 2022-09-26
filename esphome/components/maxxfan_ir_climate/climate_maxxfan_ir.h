#pragma once
#include "esphome/components/climate_ir/climate_ir.h"
#include "esphome/components/number/number.h"
#include "esphome/components/remote_base/pronto_protocol.h"
#include "fanspeed.h"

namespace esphome {
namespace maxxfan_ir_climate {

using climate::ClimateCall;
using climate::ClimatePreset;
using climate::ClimateTraits;
using climate::ClimateMode;
using climate::ClimateSwingMode;
using climate::ClimateFanMode;
using climate::ClimateDeviceRestoreState;

using std::string;

// Temperature
const float TEMP_MIN = 15.5556f;  // 60F
const float TEMP_MAX = 29.4444f;  // 85F

const char *PRESET_VENT_ONLY = "VENT ONLY";
const char *PRESET_AIR_IN = "AIR IN";
const char *PRESET_AIR_OUT = "AIR OUT";

static map<const uint8_t, const char *> FAN_SPEEDS = {{10, " 10"}, {20, " 20"}, {30, " 30"}, {40, " 40"}, {50, " 50"},
                                                      {60, " 60"}, {70, " 70"}, {80, " 80"}, {90, " 90"}, {100, "100"}};

class MaxxFanIr : public climate_ir::ClimateIR {
 public:
  MaxxFanIr();
  void set_fahrenheit(bool set);
  void set_fanspeed_number(FanSpeed *num);
  void setup() override;
  void on_fanspeed_state(float speed);

 protected:
  void control(const climate::ClimateCall &call) override;
  ClimateTraits traits() override;
  void save_state_();
  void set_fanspeed(uint8_t);
  uint8_t get_fanspeed();

  /// Transmit via IR the state of this climate controller.

  void transmit_state() override;
  /// Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;

  ClimateMode mode_before_{climate::CLIMATE_MODE_OFF};
  string custom_preset_before_{"AIR IN"};
  remote_base::ProntoProtocol *pronto_;
  bool fahrenheit_ = true;
  string get_command_key();
  FanSpeed *fanspeed_component_;
  uint8_t fanspeed_{100};
  // void send_ir(MaxxFanMode mode, fan::FanDirection direction, uint8_t speed, uint8_t temp);
  void send_ir(const char *command_key);
};

}  // namespace maxxfan_ir_climate
}  // namespace esphome
