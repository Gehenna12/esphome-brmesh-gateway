#pragma once

// Native BRMesh effect support, based on
// scross01/esphome-fastcon@824e9777f3a8bd9abc2b1d9e2c65a4d9c0dc7ff8 (MIT License,
// Copyright (c) 2025 Dennis George, see LICENSE and THIRD_PARTY_NOTICES.md).
// The effect frame layout was confirmed independently from app captures; it had
// previously been described in zentik-fr/esphome-brmesh (MIT). No code from that
// project is used here. See docs/PROTOCOL.md.

#include <queue>
#include <mutex>
#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/esp32_ble_server/ble_server.h"

namespace esphome
{
    namespace fastcon
    {

        class FastconController : public Component
        {
        public:
            FastconController() = default;

            void setup() override;
            void loop() override;

            std::vector<uint8_t> get_light_data(light::LightState *state);
            std::vector<uint8_t> get_white_light_data(light::LightState *state);
            std::vector<uint8_t> single_control(uint32_t addr, const std::vector<uint8_t> &light_data);

            // Builds a native BRMesh effect frame (frame type 8, see
            // docs/PROTOCOL.md) and passes it straight to generate_command().
            // Deliberately does NOT reuse single_control(): its length formula
            // is tied to frame type 2 and does not fit effect frames.
            std::vector<uint8_t> effect_control(uint32_t light_id_, uint8_t effect_id, uint8_t speed,
                                                 const std::vector<uint8_t> &params);

            // Effect speed (1-100), shared by all lights of this controller.
            // Meant to be called from the set_action of a template number (see
            // the example config). TemplateNumber only runs set_action on a real
            // number.set call, never when restoring the value at boot, so until
            // the number is used for the first time effect_speed_for() returns
            // the light's own default_effect_speed.
            void set_effect_speed(uint8_t speed)
            {
                effect_speed_ = speed;
                effect_speed_used_ = true;
            }
            uint8_t effect_speed_for(uint8_t fallback_default) const
            {
                return effect_speed_used_ ? effect_speed_ : fallback_default;
            }

            void queueCommand(uint32_t light_id_, const std::vector<uint8_t> &data);

            void clear_queue();
            bool is_queue_empty() const
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                return queue_.empty();
            }
            size_t get_queue_size() const
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                return queue_.size();
            }
            void set_max_queue_size(size_t size) { max_queue_size_ = size; }

            void set_mesh_key(std::array<uint8_t, 4> key) { mesh_key_ = key; }
            void set_adv_interval_min(uint16_t val) { adv_interval_min_ = val; }
            void set_adv_interval_max(uint16_t val)
            {
                adv_interval_max_ = val;
                if (adv_interval_max_ < adv_interval_min_)
                {
                    adv_interval_max_ = adv_interval_min_;
                }
            }
            void set_adv_duration(uint16_t val) { adv_duration_ = val; }
            void set_adv_gap(uint16_t val) { adv_gap_ = val; }

        protected:
            struct Command
            {
                std::vector<uint8_t> data;
                uint32_t timestamp;
                uint8_t retries{0};
                static constexpr uint8_t MAX_RETRIES = 3;
            };

            std::queue<Command> queue_;
            mutable std::mutex queue_mutex_;
            size_t max_queue_size_{100};

            uint8_t effect_speed_{1};
            bool effect_speed_used_{false};

            enum class AdvertiseState
            {
                IDLE,
                ADVERTISING,
                GAP
            };

            AdvertiseState adv_state_{AdvertiseState::IDLE};
            uint32_t state_start_time_{0};

            // Protocol implementation
            std::vector<uint8_t> generate_command(uint8_t n, uint32_t light_id_, const std::vector<uint8_t> &data, bool forward = true);

            std::array<uint8_t, 4> mesh_key_{};

            uint16_t adv_interval_min_{0x20};
            uint16_t adv_interval_max_{0x40};
            uint16_t adv_duration_{50};
            uint16_t adv_gap_{10};

            static const uint16_t MANUFACTURER_DATA_ID = 0xfff0;
        };

    } // namespace fastcon
} // namespace esphome
