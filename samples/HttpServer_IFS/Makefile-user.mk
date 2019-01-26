## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

## Add your source directories here separated by space
MODULES = app app/ESP8266LLMNR
EXTRA_INCDIR = include

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
# ESP_HOME = /opt/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

## MacOS / Linux
# SMING_HOME = /opt/sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM3

## MacOS / Linux:
# COM_PORT = /dev/tty.usbserial

## Com port speed
# COM_SPEED	= 115200

## Configure flash parameters (for ESP12-E and other new boards):
# SPI_MODE = dio

## SPIFFS options
DISABLE_SPIFFS = 1

## Firmware filesystem additional build steps

FWFILES=out\fwfiles.bin

all: fsbuild 

clean: fsclean

fsbuild: out $(FWFILES)

out:
	$(Q) mkdir out

fsclean:
	$(Q) rm -rf $(FWFILES)

$(FWFILES):
	$(Q) python "$(SMING_HOME)/third-party/IFS/fsbuild/fsbuild.py"
	touch app/application.cpp
