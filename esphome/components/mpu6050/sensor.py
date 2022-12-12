import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_TEMPERATURE,
    ICON_BRIEFCASE_DOWNLOAD,
    STATE_CLASS_MEASUREMENT,
    UNIT_METER_PER_SECOND_SQUARED,
    ICON_SCREEN_ROTATION,
    UNIT_DEGREE_PER_SECOND,
    UNIT_CELSIUS,
    UNIT_DEGREES,
)

DEPENDENCIES = ["i2c"]

CONF_ACCEL_X = "accel_x"
CONF_ACCEL_Y = "accel_y"
CONF_ACCEL_Z = "accel_z"
CONF_GYRO_X = "gyro_x"
CONF_GYRO_Y = "gyro_y"
CONF_GYRO_Z = "gyro_z"
CONF_YAW = "yaw"
CONF_PITCH = "pitch"
CONF_ROLL = "roll"

mpu6050_ns = cg.esphome_ns.namespace("mpu6050")
MPU6050Component = mpu6050_ns.class_(
    "MPU6050Component", cg.PollingComponent, i2c.I2CDevice
)

accel_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
    icon=ICON_BRIEFCASE_DOWNLOAD,
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
)
gyro_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_DEGREE_PER_SECOND,
    icon=ICON_SCREEN_ROTATION,
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
)
temperature_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)
tilt_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_DEGREES,
    icon=ICON_SCREEN_ROTATION,
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MPU6050Component),
            cv.Optional(CONF_ACCEL_X): accel_schema,
            cv.Optional(CONF_ACCEL_Y): accel_schema,
            cv.Optional(CONF_ACCEL_Z): accel_schema,
            cv.Optional(CONF_GYRO_X): gyro_schema,
            cv.Optional(CONF_GYRO_Y): gyro_schema,
            cv.Optional(CONF_GYRO_Z): gyro_schema,
            cv.Optional(CONF_TEMPERATURE): temperature_schema,
            cv.Optional(CONF_YAW): tilt_schema,
            cv.Optional(CONF_PITCH): tilt_schema,
            cv.Optional(CONF_ROLL): tilt_schema,
        }
    )
    .extend(cv.polling_component_schema("1000ms"))
    .extend(i2c.i2c_device_schema(0x68))
)


async def to_code(config):

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    await i2c.register_i2c_device(var, config)

    for d in ["x", "y", "z"]:
        accel_key = f"accel_{d}"
        if accel_key in config:
            sens = await sensor.new_sensor(config[accel_key])
            cg.add(getattr(var, f"set_accel_{d}_sensor")(sens))
        accel_key = f"gyro_{d}"
        if accel_key in config:
            sens = await sensor.new_sensor(config[accel_key])
            cg.add(getattr(var, f"set_gyro_{d}_sensor")(sens))

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))
    if CONF_YAW in config:
        sens = await sensor.new_sensor(config[CONF_YAW])
        cg.add(var.set_yaw_sensor(sens))
    if CONF_PITCH in config:
        sens = await sensor.new_sensor(config[CONF_PITCH])
        cg.add(var.set_pitch_sensor(sens))
    if CONF_ROLL in config:
        sens = await sensor.new_sensor(config[CONF_ROLL])
        cg.add(var.set_roll_sensor(sens))
