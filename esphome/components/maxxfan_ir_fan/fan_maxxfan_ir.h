#pragma once

#include <utility>

#include "esphome/components/fan/fan.h"
#include "esphome/components/remote_transmitter/remote_transmitter.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/remote_base/pronto_protocol.h"
#include "maxxfan_ir_commands.h"
#include "esphome/core/macros.h"
namespace esphome {
namespace maxxfan_ir_fan {

/*
    A controller and state manager for Maxxfan series fansenum MaxxFan : unint

*/

enum VentlidState { CLOSED = 0, OPEN = 1 };

enum MaxxFanMode { AUTO = 0, MANUAL = 1 };

struct MaxxFanRestoreState : fan::FanRestoreState {
  bool auto_mode = false;
  VentlidState ventlid_state;
};

class MaxxFanIr : public fan::Fan, public Component {
 public:
  MaxxFanIr();

  void setup() override;
  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_carrier_duty_percent(uint8_t carrier_duty_percent) {}

  void set_transmitter(remote_transmitter::RemoteTransmitterComponent *transmitter);
  void set_target_temperature(float target_temperature);
  void set_mode(MaxxFanMode mode);
  void set_ventlid_state(VentlidState state);
  fan::FanTraits get_traits() override;

 protected:
  MaxxFanMode mode_ = MaxxFanMode::MANUAL;
  float target_temperature_ = 75.0f;

  VentlidState ventlid_state_;

  uint8_t speed_count_{10};
  void write_state_();
  void control(const fan::FanCall &call) override;

  remote_transmitter::RemoteTransmitterComponent *transmitter_;
  remote_base::ProntoProtocol *pronto_;

  void send_ir(MaxxFanMode mode, fan::FanDirection direction, uint8_t speed, uint8_t temp);
  void send_ir(const char *command_key);

  fan::FanCall set_open();
};

}  // namespace maxxfan_ir_fan
}  // namespace esphome