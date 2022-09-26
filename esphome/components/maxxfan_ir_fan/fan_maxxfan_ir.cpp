#include "fan_maxxfan_ir.h"
#include "esphome/core/log.h"
#include "maxxfan_ir_commands.h"

namespace esphome {
namespace maxxfan_ir_fan {
static const char *const TAG = "maxxfan";

MaxxFanIr::MaxxFanIr() {
  // this->transmitter_ = new remote_transmitter::RemoteTransmitterComponent(pin);
  // this->transmitter_->set_carrier_duty_percent(carrier_duty_percent);
}

void MaxxFanIr::setup() {
  ESP_LOGD(TAG, "setup...");
  auto restore = this->restore_state_();
  ESP_LOGD(TAG, "got restore_state()");
  if (restore.has_value()) {
    ESP_LOGD(TAG, "restore does have value");
    restore->apply(*this);
    ESP_LOGD(TAG, "writing state...");
    this->write_state_();
  }
}
void MaxxFanIr::set_transmitter(remote_transmitter::RemoteTransmitterComponent *transmitter) {
  ESP_LOGD(TAG, "set_transmitter");
  this->transmitter_ = transmitter;
  this->pronto_ = new remote_base::ProntoProtocol();
}
void MaxxFanIr::set_target_temperature(float target_temperature) {
  this->target_temperature_ = target_temperature;
  this->write_state_();
}
void MaxxFanIr::set_mode(MaxxFanMode mode) {
  this->mode_ = mode;
  this->write_state_();
}

void MaxxFanIr::set_ventlid_state(VentlidState state) {
  this->ventlid_state_ = state;
  this->write_state_();
}

void MaxxFanIr::dump_config() {
  ESP_LOGD(TAG, "dump_config...");
  LOG_FAN("", "MaxxFan", this);
  // ESP_LOGCONFIG(TAG, "  Min. Temperature: %.1f°C", this->minimum_temperature_);
}

fan::FanTraits MaxxFanIr::get_traits() {
  ESP_LOGD(TAG, "get traits");
  return fan::FanTraits(false, true, true, this->speed_count_);
}

void MaxxFanIr::control(const fan::FanCall &call) {
  ESP_LOGD(TAG, "control...");
  if (call.get_state().has_value())
    this->state = *call.get_state();
  if (call.get_speed().has_value())
    this->speed = *call.get_speed();

  if (call.get_direction().has_value())
    this->direction = *call.get_direction();

  this->write_state_();
}

void MaxxFanIr::send_ir(MaxxFanMode mode, fan::FanDirection direction, uint8_t speed, uint8_t temp) {
  const char *fmt = "%s_%s_%d";
  char key[25] = "";
  bool autoOn = mode == MaxxFanMode::AUTO;

  snprintf(key, 25, fmt, (autoOn ? "AUTO" : "MAN"), (direction == fan::FanDirection::REVERSE ? "OUT" : "IN"),
           (autoOn ? temp : speed * 10.0));
  ESP_LOGD(TAG, "send_ir key built: %s", key);
  send_ir(key);
}

void MaxxFanIr::send_ir(const char *command_key) {
  // const uint32_t now = millis();
  ESP_LOGD(TAG, "send_ir Key=%s", command_key);
  if (COMMANDS.find(command_key) == COMMANDS.end()) {
    ESP_LOGE(TAG, "Command with key=%s not found.", command_key);
    return;
  }
  ESP_LOGD(TAG, "send_ir ready to transmit");

  auto transmit = this->transmitter_->transmit();
  auto data = transmit.get_data();
  remote_base::ProntoData prontodata{};
  prontodata.data = COMMANDS[command_key];
  this->pronto_->encode(data, prontodata);
  transmit.perform();
  // ESP_LOGD(TAG, "sending took %u ms", millis() - now);
}

void MaxxFanIr::write_state_() {
  const char *MAXXFAN_OFF = "OFF";
  float speed = this->state ? static_cast<float>(this->speed) / static_cast<float>(this->speed_count_) : 0.0f;
  if (speed == 0.0f) {  // off means idle
    send_ir("OFF");

    if (this->ventlid_state_ == VentlidState::OPEN)
      send_ir("OFF_OPEN");
  } else {
    send_ir(this->mode_, this->direction, this->speed, (uint8_t) this->target_temperature_);
  }
  this->publish_state();
}
}  // namespace maxxfan_ir_fan
}  // namespace esphome