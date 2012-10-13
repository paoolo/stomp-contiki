ifndef CONTIKI
CONTIKI = ../..
endif

CONTIKI_PROJECT = stomp-client

# TARGET = avr-zigduino
# TARGET = native
TARGET = minimal-net
APPS = stomp 

# WITH_UIP6 = 1
# UIP_CONF_IPV6 = 1
AVRDUDE_PORT = /dev/ttyUSB0

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
