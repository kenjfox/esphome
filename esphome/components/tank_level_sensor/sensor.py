import esphome.codegen as cg
import esphome.components.adc.sensor as adc
import esphome.config_validation as cv
from esphome.components import sensor
from esphome import pins
from esphome.const import (
    CONF_ID,
    ICON_WATER_PERCENT,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
)


CONF_SENSOR_PIN = "sensor_pin"
CONF_OUTPUT_PIN = "output_pin"
CONF_TANK_CAPACITY = "tank_capacity"
CONF_MARGIN = "margin_percent"
CONF_LEVELS = "tank_levels"
CONF_VOLTAGE = "voltage"
CONF_TANK_PERCENT_FULL = "percent_full"
CONF_TANK_UNITS_LEFT = "amount_left"
CONF_ADC_SENSOR = "adc_sensor"
ICON_WAVES_ARROW_UP = "mdi:waves-arrow-up"
UNIT_GALLON = "gal"
UNIT_LITRE = "L"

tank_level_sensor_ns = cg.esphome_ns.namespace("tank_level_sensor")

TankLevelSensor = tank_level_sensor_ns.class_(
    "TankLevelSensor", sensor.Sensor, cg.PollingComponent
)

tank_level_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_PERCENT,
    icon=ICON_WATER_PERCENT,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
)
tank_capacity_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_GALLON,
    icon=ICON_WAVES_ARROW_UP,
    accuracy_decimals=1,
    state_class=STATE_CLASS_MEASUREMENT,
)


entry = {
    cv.Required(CONF_VOLTAGE): cv.float_,
    cv.Required(CONF_TANK_PERCENT_FULL): cv.float_,
}


CONFIG_SCHEMA = (
    sensor.sensor_schema(
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(TankLevelSensor),
            cv.Required(CONF_ADC_SENSOR): cv.use_id(adc.ADCSensor),
            cv.Required(CONF_OUTPUT_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_MARGIN): cv.float_,
            cv.Required(CONF_LEVELS): cv.All(cv.ensure_list(entry), cv.Length(min=2)),
            cv.Optional(CONF_TANK_CAPACITY): cv.float_,
            cv.Optional(CONF_TANK_PERCENT_FULL): tank_level_schema,
            cv.Optional(CONF_TANK_UNITS_LEFT): tank_capacity_schema,
        }
    )
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    if CONF_ADC_SENSOR in config:
        conf = config[CONF_ADC_SENSOR]
        input_var = await cg.get_variable(conf)
        cg.add(var.set_adc(input_var))

    conf = config[CONF_TANK_PERCENT_FULL]
    sens = await sensor.new_sensor(conf)
    cg.add(var.set_level_sensor(sens))

    if CONF_TANK_UNITS_LEFT in config:
        conf = config[CONF_TANK_UNITS_LEFT]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_capacity_sensor(sens))

    if CONF_TANK_CAPACITY in config:
        conf = config[CONF_TANK_CAPACITY]
        cg.add(var.set_tank_capacity(conf))

    if CONF_MARGIN in config:
        conf = config[CONF_MARGIN]
        cg.add(var.set_margin_percent(conf))

    for lvl in config[CONF_LEVELS]:
        cg.add(var.add_level(lvl[CONF_VOLTAGE], lvl[CONF_TANK_PERCENT_FULL]))

    out_pin = await cg.gpio_pin_expression(config[CONF_OUTPUT_PIN])
    cg.add(var.set_output_pin(out_pin))
