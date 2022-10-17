all: 02-udp-local-multicast

# Linker optimizations
SMALL = 1

# Includes the project-conf configuration file
CFLAGS += -DPROJECT_CONF_H=\"./project-conf.h\"

CONTIKI = ../../../../..

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_LIB_DIR)/json

include $(CONTIKI)/Makefile.include

include ../../Makefile.include
