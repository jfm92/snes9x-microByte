#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := snes9x-esp32

EXTRA_COMPONENT_DIRS := components/drivers/system_configuration \
						components/drivers/display/backlight_ctrl \
						components/drivers/display/ST7789 \
						components/drivers/display/display_HAL \
						components/drivers/user_input/TCA9555 \
						components/drivers/user_input/user_input_HAL \
						components/drivers/sd_storage \
						components/drivers/battery \
						components/drivers/sound \

include $(IDF_PATH)/make/project.mk


