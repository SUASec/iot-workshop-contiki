all: tcp-server

# Linker optimizations
SMALL = 1

# Use the examples project configuration
CFLAGS += -DPROJECT_CONF_H=\"../project-conf.h\"

# Enable TCP stack
CFLAGS += -DUIP_CONF_TCP=1
MAKE_NET = MAKE_NET_IPV6

CONTIKI = ../../../../..

include $(CONTIKI)/Makefile.include

include ../../Makefile.include
