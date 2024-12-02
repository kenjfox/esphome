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

class MaxxFan : public hbridge::HBridgeFan{
 public:
  MaxxFan(int speed_count, hbridge::DecayMode decay_mode) : HBridgeFan(speed_count,decay_mode) {}


  void set_pin_cover_a(output::FloatOutput *pin_a) { pin_cover_a_ = pin_a; }
  void set_pin_cover_b(output::FloatOutput *pin_b) { pin_cover_b_ = pin_b; }

  void setup() override;
  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::DATA; }

    
  
  
  fan::FanTraits get_traits() override;
  
  CoverState get_cover_state() const { return this->cover_state_; }
  
  const std::string PRESET_OFF_OPEN = "OFF_OPEN";
  const std::string PRESET_OPEN = "OPEN";
  const std::string PRESET_CLOSED = "CLOSED";
  const  std::string PRESET_OFF = "OFF";

 protected:
  void set_cover_state();
  std::string custom_preset_before_{"OPEN"};
  CoverState cover_state_;
  output::FloatOutput *pin_cover_a_;
  output::FloatOutput *pin_cover_b_;

  uint8_t speed_count_{100};
  void write_state_();
  void control(const fan::FanCall &call) override;
  
  void set_cover_output(bool open);
};

}  // namespace maxxfan_ir_fan
}  // namespace esphome