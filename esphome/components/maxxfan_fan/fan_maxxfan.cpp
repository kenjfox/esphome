#include "fan_maxxfan.h"
#include "esphome/core/log.h"

namespace esphome {
namespace maxxfan_fan {
static const char *const TAG = "maxxfan";



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



void MaxxFan::set_cover_output(bool open) {
  if (open) {
    this->pin_cover_a_->set_level(1.0f);
    this->pin_cover_b_->set_level(0.0f);
  } else {
    this->pin_cover_a_->set_level(0.0f);
    this->pin_cover_b_->set_level(1.0f);  
  }
}

void MaxxFan::set_cover_state() {
  auto mode=this->preset_mode;
  if (mode == PRESET_OPEN || mode == PRESET_OFF_OPEN) {
      this->set_cover_output(true);
      this->cover_state_ = OPEN;

  }
  else if (mode == PRESET_CLOSED || mode == PRESET_OFF) {
      this->set_cover_output(false);
      this->cover_state_ = CLOSED;
  }
     
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
  traits.set_supported_preset_modes({PRESET_OFF, PRESET_OFF_OPEN, PRESET_OPEN, PRESET_CLOSED});
 
}

void MaxxFan::control(const fan::FanCall &call) {
  ESP_LOGD(TAG, "control...");
  if (call.get_state().has_value())
    this->state = *call.get_state();
  if (call.get_speed().has_value())
    this->speed = *call.get_speed();

  if (call.get_direction().has_value())
    this->direction = *call.get_direction();
  
  this->preset_mode = call.get_preset_mode();
  
  this->write_state_();
}





void MaxxFan::write_state_() {
  
  if (speed == 0.0f) 
  {
    if(this->preset_mode != PRESET_OFF_OPEN)
    {
      this->preset_mode = PRESET_OFF;
    }
  }
   this->set_cover_state();
  this->publish_state();
}
}  // namespace maxxfan_fan
}  // namespace esphome