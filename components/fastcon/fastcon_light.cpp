
// Native BRMesh effect support (effect branch, re-send logic), based on
// scross01/esphome-fastcon@824e9777f3a8bd9abc2b1d9e2c65a4d9c0dc7ff8 (MIT License,
// see LICENSE and THIRD_PARTY_NOTICES.md).
#include "esphome/core/log.h"
#include "esphome/components/light/light_state.h"
#include "fastcon_controller.h"
#include "fastcon_light.h"
#include "version.h"

#ifndef FASTCON_VERSION
#define FASTCON_VERSION "0.3.3-dev"
#endif

namespace esphome {
namespace fastcon {

static const char *const TAG = "fastcon.light";

namespace {

// Type flag carried in the first color byte of an effect, see docs/PROTOCOL.md.
static constexpr uint8_t FASTCON_FLASH_FLAG = 0xC0;

}  // namespace

light::LightTraits FastconLight::get_traits() {
  light::LightTraits t;
  if (this->color_interlock_) {
    if (this->supports_cwww_) {
      t.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::COLD_WARM_WHITE});
    } else {
      t.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::WHITE});
    }
  } else {
    if (this->supports_cwww_) {
      t.set_supported_color_modes({light::ColorMode::RGB_COLD_WARM_WHITE});
    } else {
      t.set_supported_color_modes({light::ColorMode::RGB_WHITE});
    }
  }

  if (this->supports_cwww_) {
    t.set_min_mireds(153.0f);
    t.set_max_mireds(500.0f);
  }
  
  return t;
}

void FastconLight::setup_state(light::LightState *state) {
  this->state_ = state;
  state->add_target_state_reached_listener(this);
}

// Re-send of an unchanged effect, needed since ESPHome 2026.10.0
// (esphome/esphome#19916, light_call.cpp: validate_() and perform()): when a
// light that is on receives the effect that is already running (or a plain
// turn_on), perform() keeps the effect and calls neither start_effect_() nor
// set_immediately_(). write_state() is then not called and no BLE frame would
// be sent - re-applying an effect, e.g. to resync lamps or to apply a new
// effect speed, would have no effect.
//
// perform() calls on_light_target_state_reached() at the end of every call
// without a transition (LightState::loop() does so at the end of a
// transition), including this case. Whether a write_state() follows is not
// visible from here (LightState::next_write_ is private). So: remember it,
// wait for three own loop() calls and only send if no write_state() arrived
// in the meantime. Three calls guarantee at least one full Application::loop()
// pass in between, in which LightState::loop() would have run a scheduled
// write_state() (perform() enables that loop via schedule_write_()). In all
// other cases (effect change, off, brightness/color, turning on from off)
// write_state() follows and sets write_seen_, so no second frame is sent.
// Everything runs in the main loop.
void FastconLight::on_light_target_state_reached() {
  if (this->state_ == nullptr || this->state_->get_current_effect_index() == 0)
    return;
  this->write_seen_ = false;
  this->resend_check_loops_ = 3;
  this->enable_loop();
}

void FastconLight::loop() {
  if (this->resend_check_loops_ == 0) {
    this->disable_loop();
    return;
  }
  if (--this->resend_check_loops_ != 0)
    return;
  if (this->write_seen_ || this->controller_ == nullptr || !this->state_->current_values.is_on())
    return;
  const uint32_t effect_index = this->state_->get_current_effect_index();
  if (effect_index == 0)
    return;
  ESP_LOGI(TAG, "Re-sending running effect for light %u (no write_state, ESPHome #19916)",
           (unsigned)this->light_id_);
  this->send_effect_(this->state_, effect_index);
}

void FastconLight::write_state(light::LightState *state) {
  this->write_seen_ = true;
  if (this->controller_ == nullptr) {
    ESP_LOGW(TAG, "No controller bound; dropping command");
    return;
  }

  // Effect branch first, before the color/white handling: an active effect
  // sends an effect frame (frame type 8) instead of color/white bytes.
  // get_current_effect_index() is 0 when no effect is running - this also
  // covers the name "None", which get_effect_name() returns instead of ""
  // since ESPHome 2026.8.1. "None" therefore falls through to the unchanged
  // path below, whose static color/white command stops a running effect on
  // the lamp - the protocol has no separate "stop effect" command.
  //
  // Type and color list come from the active FastconColorEffect, created by
  // light.py from the "color_list" entries. static_cast instead of
  // dynamic_cast because ESPHome builds with -fno-rtti; the cast is safe
  // because the "color_list" effect registered in light.py is the only effect
  // that creates FastconColorEffect objects.
  const uint32_t effect_index = state->get_current_effect_index();
  if (effect_index != 0) {
    this->send_effect_(state, effect_index);
    return;
  }

  std::vector<uint8_t> light_bytes;
  auto &values = state->current_values;

  // Determine if it's a "white-only" command
  bool is_white_only = values.get_color_mode() == light::ColorMode::WHITE;

  if (is_white_only) {
    ESP_LOGD(TAG, "Sending white-only command for light %u", (unsigned)this->light_id_);
    light_bytes = this->controller_->get_white_light_data(state);
  } else {
    ESP_LOGD(TAG, "Sending RGB/color command for light %u", (unsigned)this->light_id_);
    light_bytes = this->controller_->get_light_data(state);
  }

  // Wrap into inner payload for this light_id
  std::vector<uint8_t> payload = this->controller_->single_control(this->light_id_, light_bytes);

  // Queue it for advertisement
  this->controller_->queueCommand(this->light_id_, payload);

  ESP_LOGD(TAG, "Queued state v%s: light_id=%u, payload_len=%d",
           FASTCON_VERSION, (unsigned)this->light_id_, (int)payload.size());
}

// Sends the effect frame for the given effect. Shared by write_state() and the
// re-send in loop() (ESPHome #19916). Callers ensure controller_ != nullptr
// and effect_index != 0.
void FastconLight::send_effect_(light::LightState *state, uint32_t effect_index) {
  auto *effect = static_cast<FastconColorEffect *>(state->get_effect_by_index(effect_index));
  const auto &colors = effect->get_colors();
  const bool is_flash = effect->get_type_flag() == FASTCON_FLASH_FLAG;
  const uint8_t effect_id = effect->get_type_flag() | colors[0];

  // The first color goes into the effect_id byte, the rest are parameters.
  // Single-color lists still need an explicit 00 as second color slot (as in
  // all captured single-color app payloads); without it the length nibble
  // would differ from what the app sends.
  std::vector<uint8_t> params(colors.begin() + 1, colors.end());
  if (params.empty()) {
    params.push_back(0x00);
  }

  // Speed encoding: direct for fade, inverse for flash (see docs/PROTOCOL.md).
  const uint8_t ui_speed = this->controller_->effect_speed_for(this->default_effect_speed_);
  const uint8_t speed = is_flash ? static_cast<uint8_t>(202 - 2 * ui_speed) : ui_speed;

  ESP_LOGD(TAG, "Starting native effect '%s' for light %u (speed=%u)", state->get_effect_name().c_str(),
           (unsigned)this->light_id_, (unsigned)speed);
  std::vector<uint8_t> payload = this->controller_->effect_control(this->light_id_, effect_id, speed, params);
  this->controller_->queueCommand(this->light_id_, payload);
}

}  // namespace fastcon
}  // namespace esphome
