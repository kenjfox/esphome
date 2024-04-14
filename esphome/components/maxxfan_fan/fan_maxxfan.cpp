#include "fan_maxxfan.h"
#include "esphome/core/log.h"

namespace esphome {
namespace maxxfan_fan {
static const char *const TAG = "maxxfan";

MaxxFan::MaxxFan() {
  // this->transmitter_ = new remote_transmitter::RemoteTransmitterComponent(pin);
  // this->transmitter_->set_carrier_duty_percent(carrier_duty_percent);
}

void MaxxFan::setup() {
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


void MaxxFan::set_cover_state(CoverState state) {
  this->cover_state_ = state;
  this->write_state_();
}

void MaxxFan::dump_config() {
  ESP_LOGD(TAG, "dump_config...");
  LOG_FAN("", "MaxxFan", this);
  // ESP_LOGCONFIG(TAG, "  Min. Temperature: %.1f°C", this->minimum_temperature_);
}

fan::FanTraits MaxxFan::get_traits() {
  ESP_LOGD(TAG, "get traits");
  auto traits = fan::FanTraits(false, true, true, this->speed_count_);
  traits.set_supported_custom_presets({PRESET_VENT_ONLY, PRESET_AIR_IN, PRESET_AIR_OUT});
 HBridgeFan::Component::
 
}

void MaxxFan::control(const fan::FanCall &call) {
  ESP_LOGD(TAG, "control...");
  if (call.get_state().has_value())
    this->state = *call.get_state();
  if (call.get_speed().has_value())
    this->speed = *call.get_speed();

  if (call.get_direction().has_value())
    this->direction = *call.get_direction();
  if(call.)
  this->write_state_();
}





void MaxxFan::write_state_() {
  const char *MAXXFAN_OFF = "OFF";
  
  this->publish_state();
}
}  // namespace maxxfan_fan
}  // namespace esphome