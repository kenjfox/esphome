import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    fan,
    remote_transmitter,
)
from esphome.components.remote_base import CONF_TRANSMITTER_ID
from esphome.const import (
    CONF_ID,
    CONF_TARGET_TEMPERATURE,
)

AUTO_LOAD = ["fan", "remote_base"]
CODEOWNERS = ["@kenjfox"]

maxxfan_ns = cg.esphome_ns.namespace("maxxfan")
MaxxFan = maxxfan_ns.class_("MaxxFan", fan.Fan, cg.Component)
MaxxFanMode = maxxfan_ns.enum("Mode")

VentlidState = maxxfan_ns.enum("VentlidState")

VENTLID_OPEN = maxxfan_ns.VENTLID_OPEN
VENTLID_CLOSED = maxxfan_ns.VENTLID_CLOSED

VENTLID_STATES = {
    "OPEN": VentlidState.OPEN,
    "CLOSED": VentlidState.CLOSED,
}
validate_ventlid_state = cv.enum(VENTLID_STATES, upper=True)

CONF_MODES = {"AUTO": MaxxFanMode.AUTO, "MANUAL": MaxxFanMode.MANUAL}
validate_mode = cv.enum(CONF_MODES, upper=True)


CONF_VENTLID_STATE = "ventlid_state"
CONF_MODE = "mode"

CONFIG_SCHEMA = fan.FAN_SCHEMA.extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(MaxxFan),
        cv.GenerateID(CONF_TRANSMITTER_ID): cv.use_id(
            remote_transmitter.RemoteTransmitterComponent
        ),
        cv.Optional(CONF_MODE, default="MANUAL"): validate_mode,
        cv.Optional(CONF_TARGET_TEMPERATURE, default=75): cv.temperature,
        cv.Optional(CONF_VENTLID_STATE, default="OPEN"): validate_ventlid_state,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await fan.register_fan(var, config)

    transmitter = await cg.get_variable(config[CONF_TRANSMITTER_ID])
    cg.add(var.set_transmitter(transmitter))
    cg.add(var.set_mode(config[CONF_MODE]))
    cg.add(var.set_target_temperature(config[CONF_TARGET_TEMPERATURE]))
    cg.add(var.set_ventlid_state(config[CONF_VENTLID_STATE]))
