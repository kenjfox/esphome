#define _GLIBCXX_USE_C99
#include "climate_maxxfan_ir.h"
#include "esphome/core/log.h"
#include "maxxfan_ir_commands.h"
using std::stoi;
namespace esphome {
namespace maxxfan_ir_climate {

static const char *const TAG = "climate.climate_ir_maxxfan";

string ltrim(string &str) {
  size_t first = str.find_first_not_of(' ');

  return str.substr(first);
}

uint8_t get_speed_from_fanmode(string s) { return (uint8_t) stoi(s); }

const string get_fanmode_from_speed(uint8_t speed) {
  if (speed >= 100)
    return to_string(speed);
  else
    return " " + to_string(speed);
}

MaxxFanIr::MaxxFanIr() : climate_ir::ClimateIR(TEMP_MIN, TEMP_MAX, 0.555555f, false, false, {}, {}) {
  this->pronto_ = new remote_base::ProntoProtocol();
  this->ClimateIR::mode = ClimateMode::CLIMATE_MODE_COOL;
  this->ClimateIR::set_custom_fan_mode_(FAN_SPEEDS.at(100));
  this->ClimateIR::set_custom_preset_(PRESET_AIR_IN);

  // this->set_traits(this->mode);
}
// optional<ClimateDeviceRestoreState> MaxxFanIr::restore_state_() { return ClimateIR::restore_state_(); }
void MaxxFanIr::setup() {
  ESP_LOGD(TAG, "setup called...");
  ClimateIR::setup();
  if (this->custom_fan_mode.has_value()) {
    auto speed = get_speed_from_fanmode(this->custom_fan_mode.value());
    ESP_LOGD(TAG, "setup... speed=: %d", speed);
    this->set_fanspeed(speed);  // update number component
  } else {
    ESP_LOGD(TAG, "setup: NO CUSTOM FANMODE YET");
  }
  this->custom_preset_before_ =
      this->custom_preset.value() == PRESET_VENT_ONLY ? PRESET_AIR_OUT : this->custom_preset.value_or(PRESET_AIR_OUT);
}

ClimateTraits MaxxFanIr::traits() {
  ESP_LOGD(TAG, "traits()");
  auto traits = ClimateTraits();
  traits.set_supported_modes(
      {ClimateMode::CLIMATE_MODE_OFF, ClimateMode::CLIMATE_MODE_COOL, ClimateMode::CLIMATE_MODE_FAN_ONLY});
  traits.set_visual_min_temperature(TEMP_MIN);
  traits.set_visual_max_temperature(TEMP_MAX);
  traits.set_visual_temperature_step(this->fahrenheit_ ? 1.0f : 0.5f);
  traits.set_supported_custom_presets({PRESET_VENT_ONLY, PRESET_AIR_IN, PRESET_AIR_OUT});
  traits.set_supported_custom_fan_modes({" 10", " 20", " 30", " 40", " 50", " 60", " 70", " 80", " 90", "100"});

  return traits;
}

void MaxxFanIr::on_fanspeed_state(float speed) {
  // fanspeed number component updated directly
  ESP_LOGD(TAG, "on_fanspeed_state: %2.0f", speed);
  if (speed != this->fanspeed_) {
    if (fmod(speed, 10) != 0) {
      speed = std::round(speed / 10) * 10;
      if (speed > 100)
        speed = 100;
      else if (speed < 10)
        speed = 10;
    }
    ESP_LOGD(TAG, "on_fanspeed_state: speed changed from %d to %2.0f ", this->fanspeed_, speed);
    // set custom fan mode to match speed from fanspeed component
    auto call = this->make_call();
    this->fanspeed_ = speed;
    call.set_fan_mode(get_fanmode_from_speed(speed));
    call.perform();
  }
}

void MaxxFanIr::set_fanspeed_component(FanSpeed *num) {
  this->fanspeed_component_ = num;
  num->add_on_state_callback([this](float state) { this->on_fanspeed_state(state); });
}

void MaxxFanIr::set_fanspeed(uint8_t speed) {
  ESP_LOGD(TAG, "set_fanspeed: %d", speed);
  auto call = this->fanspeed_component_->make_call();
  call.set_value(speed);
  call.perform();
}

uint8_t MaxxFanIr::get_fanspeed() { return (uint8_t) this->fanspeed_component_->state; }

void MaxxFanIr::set_direction(bool fwd) {
  auto call = this->make_call();
  call.set_preset(fwd ? PRESET_AIR_OUT : PRESET_AIR_IN);
  call.perform();
}
bool MaxxFanIr::get_direction() { return (this->custom_preset == PRESET_AIR_OUT); }

void MaxxFanIr::MaxxFanIr::control(const ClimateCall &call) {
  ESP_LOGD(TAG, "control...");
  // conditionally set available traits based on the mode...
  if (call.get_mode().has_value()) {
    this->mode = call.get_mode().value();

    if (mode != this->mode_before_) {
      ESP_LOGD(TAG, "get mode changed");
      if (this->mode != ClimateMode::CLIMATE_MODE_OFF)
        if (this->custom_preset == PRESET_VENT_ONLY)
          this->custom_preset = this->custom_preset_before_;  // handle VENT ONLY case
      this->mode_before_ = mode;
    }
  }
  if (call.get_target_temperature().has_value()) {
    this->target_temperature = clamp<float>(call.get_target_temperature().value(), TEMP_MIN, TEMP_MAX);
  }

  if (call.get_preset().has_value()) {
    this->preset = call.get_preset().value();
  } else if (call.get_custom_preset().has_value()) {
    this->custom_preset = call.get_custom_preset().value();
    if (this->custom_preset_before_ != this->custom_preset) {  // custom preset changed
      if (this->custom_preset == PRESET_VENT_ONLY)
        this->mode = ClimateMode::CLIMATE_MODE_OFF;
      else
        this->custom_preset_before_ = this->custom_preset.value();  // don't save vent-only as custom preset
    }
  }
  if (call.get_fan_mode().has_value()) {
    this->fan_mode = call.get_fan_mode().value();
  } else if (call.get_custom_fan_mode().has_value()) {
    ESP_LOGD(TAG, "Custom fan mode has value");
    if (this->custom_fan_mode.value() != call.get_custom_fan_mode().value()) {
      ESP_LOGD(TAG, "Custom fan mode changed to %s", call.get_custom_fan_mode().value().c_str());
      this->custom_fan_mode = call.get_custom_fan_mode().value();
      auto speed = get_speed_from_fanmode(this->custom_fan_mode.value());
      if (this->fanspeed_ != speed)  // if triggered from thermostat, then update number component
      {
        this->set_fanspeed(speed);  // update number component
      }
    }
  }

  climate_ir::ClimateIR::control(call);  // this calls transmit_state
}

void MaxxFanIr::transmit_state() {
  ESP_LOGD(TAG, "transmit_state");
  string key;
  auto target_temp_F = (uint8_t) roundf((clamp<float>(this->target_temperature, TEMP_MIN, TEMP_MAX) * 9 / 5) + 32);

  key += (this->mode == ClimateMode::CLIMATE_MODE_COOL) ? "AUTO_" : "MAN_";
  key += (this->custom_preset == PRESET_AIR_IN) ? "IN_" : "OUT_";

  switch (this->mode) {
    case ClimateMode::CLIMATE_MODE_OFF:
      send_ir("OFF");
      if (this->custom_preset.value() == PRESET_VENT_ONLY)
        send_ir("OFF_OPEN");
      break;
    case ClimateMode::CLIMATE_MODE_COOL:
      key += to_string(target_temp_F);
      send_ir(key.c_str());
      break;
    case ClimateMode::CLIMATE_MODE_FAN_ONLY:
      key += ltrim(this->custom_fan_mode.value());
      send_ir(key.c_str());
      break;
    default:
      break;
  }
}
void MaxxFanIr::set_fahrenheit(bool set) { this->fahrenheit_ = set; }
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

bool MaxxFanIr::on_receive(remote_base::RemoteReceiveData data) {
  uint8_t nbits = 0;
  uint32_t remote_state = 0;
  return true;
}

}  // namespace maxxfan_ir_climate
}  // namespace esphome
