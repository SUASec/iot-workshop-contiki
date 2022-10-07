CONTIKI_PROJECT=border-router
all: $(CONTIKI_PROJECT) 

CONTIKI = ../../../../..

SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"../project-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c

CFLAGS += -DUIP_FALLBACK_INTERFACE=rpl_interface

WITH_WEBSERVER=1
CFLAGS += -DUIP_CONF_TCP=1
CFLAGS += -DWEBSERVER=1
PROJECT_SOURCEFILES += httpd-simple.c

ifeq ($(PREFIX),)
 PREFIX = fd00::1/64
endif

CONTIKI_WITH_IPV6 = 1
MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
CFLAGS += -DSLIP_BRIDGE_CONF_NO_PUTCHAR=1

include $(CONTIKI)/Makefile.include

include ../../Makefile.include

$(CONTIKI)/tools/serial-io/tunslip6:	$(CONTIKI)/tools/serial-io/tunslip6.c
	(cd $(CONTIKI)/tools/serial-io && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/serial-io/tunslip6
	sudo $(CONTIKI)/tools/serial-io/tunslip6 $(PREFIX)

connect-router-cooja:	$(CONTIKI)/tools/serial-io/tunslip6
	sudo $(CONTIKI)/tools/serial-io/tunslip6 -a 127.0.0.1 $(PREFIX)
