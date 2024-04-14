#pragma once

#include <utility>
#include "esphome/core/component.h"
#include "esphome/components/hbridge/fan/hbridge_fan.h"
#include "esphome/core/macros.h"

namespace esphome {
namespace maxxfan_fan {

/*
    A controller and state manager for Maxxfan series fansenum MaxxFan : unint

*/

enum CoverState { CLOSED = 0, OPEN = 1 };


struct MaxxFanRestoreState : fan::FanRestoreState {
 
  CoverState cover_state;
};

class MaxxFan : public hbridge::HBridgeFan, public Component {
 public:
  MaxxFan(int speed_count, hbridge::DecayMode decay_mode) : HBridgeFan(speed_count,decay_mode) {}

  void setup() override;
  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::DATA; }

    
  void set_cover_state(CoverState state);
  fan::FanTraits get_traits() override;
  
  CoverState get_cover_state() const { return this->cover_state_; }
  
  const char *PRESET_VENT_ONLY = "VENT ONLY";
  const char *PRESET_AIR_IN = "AIR IN";
  const char *PRESET_AIR_OUT = "AIR OUT";

 protected:
  std::string custom_preset_before_{"AIR IN"};
  CoverState cover_state_;
  output::FloatOutput *pin_lid_a;
  output::FloatOutput *pin_lid_b;
  uint8_t speed_count_{100};
  void write_state_();
  void control(const fan::FanCall &call) override;
  fan::FanCall set_cover_state(bool open);
};

}  // namespace maxxfan_ir_fan
}  // namespace esphome