"""Light platform for Fastcon BLE lights."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.components.light.effects import register_monochromatic_effect
from esphome.components.light.types import LightEffect
from esphome.const import (
    CONF_COLOR_INTERLOCK,
    CONF_COLORS,
    CONF_LIGHT_ID,
    CONF_NAME,
    CONF_OUTPUT_ID,
    CONF_TYPE,
)
from esphome.core import HexInt
from .fastcon_controller import FastconController

# New config key to toggle RGBCW capability per-entity
CONF_SUPPORTS_CWWW = "supports_cwww"

DEPENDENCIES = ["esp32_ble"]
AUTO_LOAD = ["light"]

CONF_CONTROLLER_ID = "controller_id"

# Optional fixed effect speed (1-100) for this light. Used until the shared
# effect speed number is set for the first time, see docs/PROTOCOL.md.
CONF_DEFAULT_EFFECT_SPEED = "default_effect_speed"

fastcon_ns = cg.esphome_ns.namespace("fastcon")
FastconLight = fastcon_ns.class_("FastconLight", light.LightOutput, cg.Component)

# A native BRMesh effect is fully defined by its type (fade/flash) and a list
# of color codes, see docs/PROTOCOL.md. Each "color_list" entry under
# effects: creates one FastconColorEffect; new effects need no C++ changes.
FastconColorEffect = fastcon_ns.class_("FastconColorEffect", LightEffect)

# Type flags, OR-ed into the first color code (see docs/PROTOCOL.md).
FASTCON_EFFECT_TYPES = {
    "fade": HexInt(0x40),
    "flash": HexInt(0xC0),
}


@register_monochromatic_effect(
    "color_list",
    FastconColorEffect,
    "Color List",
    {
        cv.Required(CONF_TYPE): cv.enum(FASTCON_EFFECT_TYPES, lower=True),
        # Color codes 1-15 (see docs/PROTOCOL.md). Up to 9 colors fit into the
        # 12-byte frame; lists of up to 7 colors are confirmed on real lights.
        cv.Required(CONF_COLORS): cv.All(
            cv.ensure_list(cv.int_range(min=1, max=15)), cv.Length(min=1, max=9)
        ),
    },
)
async def fastcon_color_list_effect_to_code(config, effect_id):
    return cg.new_Pvariable(
        effect_id, config[CONF_NAME], config[CONF_TYPE], config[CONF_COLORS]
    )

CONFIG_SCHEMA = cv.All(
    light.BRIGHTNESS_ONLY_LIGHT_SCHEMA
    .extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(FastconLight),
            cv.Required(CONF_LIGHT_ID): cv.int_range(min=1, max=255),
            cv.Optional(CONF_CONTROLLER_ID, default="fastcon_controller"): cv.use_id(FastconController),
            cv.Optional(CONF_SUPPORTS_CWWW, default=False): cv.boolean,
            cv.Optional(CONF_COLOR_INTERLOCK, default=False): cv.boolean,
            cv.Optional(CONF_DEFAULT_EFFECT_SPEED, default=1): cv.int_range(min=1, max=100),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID], config[CONF_LIGHT_ID])

    await cg.register_component(var, config)
    await light.register_light(var, config)

    if config.get(CONF_COLOR_INTERLOCK):
        cg.add(var.set_color_interlock(True))

    controller = await cg.get_variable(config[CONF_CONTROLLER_ID])
    cg.add(var.set_controller(controller))

    if config.get(CONF_SUPPORTS_CWWW):
        cg.add(var.set_supports_cwww(True))

    cg.add(var.set_default_effect_speed(config[CONF_DEFAULT_EFFECT_SPEED]))
