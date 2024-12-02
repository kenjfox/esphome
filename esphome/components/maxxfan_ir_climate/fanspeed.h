#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"

namespace esphome {
namespace maxxfan_ir_climate {

class FanSpeed : public number::Number, public Component {
 public:
  void setup() override{

  };

 protected:
  void control(float value) override { this->publish_state(value); }
};

}  // namespace maxxfan_ir_climate
}  // namespace esphome
