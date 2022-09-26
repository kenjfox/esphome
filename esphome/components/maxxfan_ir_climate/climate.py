import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir, number
from esphome.const import (
    CONF_ID,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_STEP,
    CONF_NAME,
)


AUTO_LOAD = ["climate_ir", "number"]

maxxfan_ir_ns = cg.esphome_ns.namespace("maxxfan_ir_climate")
FanSpeed = maxxfan_ir_ns.class_("FanSpeed", number.Number, cg.Component)

MaxxFanIrClimate = maxxfan_ir_ns.class_(
    "MaxxFanIr",
    climate_ir.ClimateIR,
    number.Number,
)
CONF_USE_FAHRENHEIT = "use_fahrenheit"
CONF_FAN_SPEED = "fan_speed"

# CUSTOM_FAN_MODES = {"IN": maxxfan_ir_ns.FANMODE_IN, "OUT": maxxfan_ir_ns.FANMODE_OUT}
# CUSTOM_CLIMATE_MODES = {"OFF_LID_UP": maxxfan_ir_ns.CLIMATE_MODE_OFF_LID_UP}

CONFIG_SCHEMA = climate_ir.CLIMATE_IR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MaxxFanIrClimate),
        cv.Optional(CONF_USE_FAHRENHEIT, default=False): cv.boolean,
        cv.Optional(
            CONF_FAN_SPEED,
            default={
                CONF_NAME: "Fan Speed",
                CONF_MIN_VALUE: 0,
                CONF_MAX_VALUE: 100,
                CONF_STEP: 10,
            },
        ): number.NUMBER_SCHEMA.extend(cv.COMPONENT_SCHEMA).extend(
            {
                cv.GenerateID(): cv.declare_id(FanSpeed),
                cv.Required(CONF_MIN_VALUE): cv.float_,
                cv.Required(CONF_MAX_VALUE): cv.float_,
                cv.Required(CONF_STEP): cv.float_,
            }
        ),
    }
)


async def to_code(config):
    maxxfan = cg.new_Pvariable(config[CONF_ID])
    await climate_ir.register_climate_ir(maxxfan, config)
    cg.add(maxxfan.set_fahrenheit(config[CONF_USE_FAHRENHEIT]))

    conf = config[CONF_FAN_SPEED]
    var = cg.new_Pvariable(conf[CONF_ID])
    await cg.register_component(var, conf)
    await number.register_number(
        var,
        conf,
        min_value=conf[CONF_MIN_VALUE],
        max_value=conf[CONF_MAX_VALUE],
        step=conf[CONF_STEP],
    )
    cg.add(maxxfan.set_fanspeed_number(var))
