CONTIKI = ../..
CONTIKI_PROJECT = stomp-client
APPS = stomp-simple stomp-common
# APPS = stomp stomp-common

STOMP_PROFILE = 0
STOMP_DEBUG = 1
WITH_UIP6 = 1
UIP_CONF_IPV6 = 1
WITH_UDP = 1
TARGET = avr-zigduino
# TARGET = minimal-net
AVRDUDE_PORT = /dev/ttyUSB0

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
