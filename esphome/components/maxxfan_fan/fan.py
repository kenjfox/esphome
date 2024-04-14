import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    fan,
    
)

from esphome.const import (
    CONF_ID,
    
)

AUTO_LOAD = ["fan", ]
CODEOWNERS = ["@kenjfox"]

maxxfan_ns = cg.esphome_ns.namespace("maxxfan")
MaxxFan = maxxfan_ns.class_("MaxxFan", fan.Fan, cg.Component)

VentlidState = maxxfan_ns.enum("VentlidState")

VENTLID_OPEN = maxxfan_ns.VENTLID_OPEN
VENTLID_CLOSED = maxxfan_ns.VENTLID_CLOSED

VENTLID_STATES = {
    "OPEN": VentlidState.OPEN,
    "CLOSED": VentlidState.CLOSED,
}
validate_ventlid_state = cv.enum(VENTLID_STATES, upper=True)


CONF_VENTLID_STATE = "ventlid_state"
CONF_MODE = "mode"

CONFIG_SCHEMA = fan.FAN_SCHEMA.extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(MaxxFan),
        cv.Optional(CONF_VENTLID_STATE, default="OPEN"): validate_ventlid_state,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await fan.register_fan(var, config)
    cg.add(var.set_ventlid_state(config[CONF_VENTLID_STATE]))
