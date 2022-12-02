CONTIKI_PROJECT = hash
all: $(CONTIKI_PROJECT)

# Do not try to build on Sky because of code size limitation
PLATFORMS_EXCLUDE = sky z1

CONTIKI=../../../..

# Include libjss
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_LIB_DIR)/libjss

include $(CONTIKI)/Makefile.include
