CONTIKI = ../..
CONTIKI_PROJECT = hello-world

WITH_UIP6 = 1
UIP_CONF_IPV6 = 1
UIP_CONF_RPL = 1
UIP_CONF_IPV6_RPL = 1
TARGET = avr-zigduino
# TARGET = minimal-net
AVRDUDE_PORT = /dev/ttyUSB0

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
