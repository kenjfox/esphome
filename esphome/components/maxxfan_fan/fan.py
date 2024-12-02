import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    output,
)
from esphome.components.hbridge import fan as hbridge_fan, hbridge_ns
from esphome.const import (
    CONF_ID,
    CONF_DECAY_MODE,
    CONF_SPEED_COUNT,
    CONF_PIN_A,
    CONF_PIN_B,
    CONF_ENABLE_PIN,
    CONF_PRESET_MODES,
)
from esphome.components.fan import validate_preset_modes

AUTO_LOAD = ["fan"]
CODEOWNERS = ["@kenjfox"]

CONF_PIN_COVER_A = "pin_cover_a"
CONF_PIN_COVER_B = "pin_cover_b"


DecayMode = hbridge_ns.enum("DecayMode")
DECAY_MODE_OPTIONS = {
    "SLOW": DecayMode.DECAY_MODE_SLOW,
    "FAST": DecayMode.DECAY_MODE_FAST,
}

maxxfan_ns = cg.esphome_ns.namespace("maxxfan_fan")
MaxxFan = maxxfan_ns.class_("MaxxFan", hbridge_fan.HBridgeFan, cg.Component)


CONFIG_SCHEMA = hbridge_fan.FAN_SCHEMA.extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(MaxxFan),
        cv.Required(CONF_PIN_A): cv.use_id(output.FloatOutput),
        cv.Required(CONF_PIN_B): cv.use_id(output.FloatOutput),
        cv.Required(CONF_PIN_COVER_A): cv.use_id(output.FloatOutput),
        cv.Required(CONF_PIN_COVER_B): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_DECAY_MODE, default="SLOW"): cv.enum(
            DECAY_MODE_OPTIONS, upper=True
        ),
        cv.Optional(CONF_SPEED_COUNT, default=100): cv.int_range(min=1),
        cv.Optional(CONF_ENABLE_PIN): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_PRESET_MODES): validate_preset_modes,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await hbridge_fan.register_hbridge_fan(var, config)
    pin_cover_a = await cg.get_variable(config[CONF_PIN_COVER_A])
    cg.add(var.set_pin_cover_a(pin_cover_a))
    pin_cover_b = await cg.get_variable(config[CONF_PIN_COVER_B])
    cg.add(var.set_pin_cover_b(pin_cover_b))
    if CONF_PRESET_MODES in config:
        cg.add(var.set_preset_modes(config[CONF_PRESET_MODES]))
