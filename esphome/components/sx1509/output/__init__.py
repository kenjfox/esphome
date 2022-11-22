import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_PIN, CONF_ID, CONF_FREQUENCY
from .. import SX1509Component, sx1509_ns, CONF_SX1509_ID

DEPENDENCIES = ["sx1509"]
CONF_LOG = "logarithmic"

SX1509FloatOutputChannel = sx1509_ns.class_(
    "SX1509FloatOutputChannel", output.FloatOutput, cg.Component
)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(SX1509FloatOutputChannel),
        cv.GenerateID(CONF_SX1509_ID): cv.use_id(SX1509Component),
        cv.Required(CONF_PIN): cv.int_range(min=0, max=15),
        cv.Optional(CONF_FREQUENCY, default=1): cv.int_range(min=1, max=8),
        cv.Optional(CONF_LOG, default=False): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_SX1509_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)
    cg.add(var.set_pin(config[CONF_PIN]))
    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    cg.add(var.set_logarithmic(config[CONF_LOG]))
    cg.add(var.set_parent(parent))
