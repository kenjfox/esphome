#pragma once

#include "esphome/components/sx1509/sx1509.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace sx1509 {

class SX1509Component;

class SX1509FloatOutputChannel : public output::FloatOutput, public Component {
 public:
  void set_parent(SX1509Component *parent) { this->parent_ = parent; }
  void set_pin(uint8_t pin) { pin_ = pin; }
  void set_frequency(uint8_t f) { frequency_ = f; }
  void set_logarithmic(bool l) { logarithmic_ = l; }
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  void write_state(float state) override;

  SX1509Component *parent_;
  uint8_t pin_;
  uint8_t frequency_;
  bool logarithmic_;
};

}  // namespace sx1509
}  // namespace esphome
