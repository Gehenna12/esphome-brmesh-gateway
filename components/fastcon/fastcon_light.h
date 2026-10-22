
#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_effect.h"
#include "esphome/components/light/light_state.h"

namespace esphome {
namespace fastcon {

class FastconController; // fwd decl

// A native BRMesh effect is fully defined by its type (fade/flash) and a list
// of color codes, see docs/PROTOCOL.md. Each "effects: - color_list:" entry
// (light.py) creates one instance; new effects need no C++ changes.
//
// apply() is intentionally empty: LightState::loop() calls it on EVERY loop
// tick while an effect is active. The effect runs on the lamp itself, so the
// frame is sent from FastconLight::write_state(), which runs once per effect
// change or light.turn_on call.
class FastconColorEffect : public light::LightEffect {
 public:
  FastconColorEffect(const char *name, uint8_t type_flag, std::vector<uint8_t> colors)
      : light::LightEffect(name), type_flag_(type_flag), colors_(std::move(colors)) {}

  void apply() override {}

  uint8_t get_type_flag() const { return type_flag_; }
  const std::vector<uint8_t> &get_colors() const { return colors_; }

 protected:
  uint8_t type_flag_;
  std::vector<uint8_t> colors_;
};

class FastconLight : public Component,
                     public light::LightOutput,
                     public light::LightTargetStateReachedListener {
 public:
  FastconLight() = default;
  explicit FastconLight(int light_id) { this->light_id_ = static_cast<uint8_t>(light_id); }

  void set_controller(FastconController *c) { controller_ = c; }
  void set_light_id(uint8_t id) { light_id_ = id; }
  void set_supports_cwww(bool v) { supports_cwww_ = v; }
  void set_color_interlock(bool v) { color_interlock_ = v; }
  // Optional fixed effect speed (1-100) for this light. Used until the shared
  // effect speed (FastconController::set_effect_speed) is set for the first time.
  void set_default_effect_speed(uint8_t v) { default_effect_speed_ = v; }

  // LightOutput interface
  light::LightTraits get_traits() override;
  void setup_state(light::LightState *state) override;
  void write_state(light::LightState *state) override;

  // Since ESPHome 2026.10.0 (esphome/esphome#19916) LightCall::perform() no
  // longer calls write_state() when a light that is on receives the effect
  // that is already running (or a plain turn_on). The listener sees every
  // perform() without a transition; loop() re-sends the effect frame if no
  // write_state() followed. Details in fastcon_light.cpp.
  void on_light_target_state_reached() override;
  void loop() override;

 protected:
  void send_effect_(light::LightState *state, uint32_t effect_index);

  FastconController *controller_{nullptr};
  light::LightState *state_{nullptr};
  // Own loop() calls left until the re-send check; 0 = nothing pending.
  uint8_t resend_check_loops_{0};
  bool write_seen_{false};
  uint8_t light_id_{0};
  bool supports_cwww_{false};
  bool color_interlock_{false};
  uint8_t default_effect_speed_{1};
};

}  // namespace fastcon
}  // namespace esphome
