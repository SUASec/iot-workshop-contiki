/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
 * Copyright (c) 2022, Schmalkalden University of Applied Sciences
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"

/* The following libraries add IP/IPv6 support */
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"

/* This is quite handy, allows to print IPv6 related stuff in a readable way */
#include "net/ipv6/uip-debug.h"

/* The simple UDP library API */
#include "simple-udp.h"

/* Library used to read the metadata in the packets */
#include "net/packetbuf.h"

/* And we are including the conf.h with the example configuration */
#include "conf.h"

/* Plus sensors to send data */
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"

/* JSON library for decoding sensor data */
#include "jsonparse.h"

/* And this you should be familiar with from the basic lessons... */
#include "sys/etimer.h"
#include "dev/leds.h"

#include <stdio.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define SEND_INTERVAL	(15 * CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
/* The structure used in the Simple UDP library to create an UDP connection */
static struct simple_udp_connection mcast_connection;
/*---------------------------------------------------------------------------*/
/* Create buffer for JSON */
char json_buffer[JSON_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/* Keeps account of the number of messages sent */
static uint16_t counter = 0;
/*---------------------------------------------------------------------------*/
PROCESS(mcast_example_process, "UDP multicast example process");
AUTOSTART_PROCESSES(&mcast_example_process);
/*---------------------------------------------------------------------------*/
/* This is the receiver callback, we tell the Simple UDP library to call this
 * function each time we receive a packet
 */
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  /* Variable used to store the retrieved radio parameters */
  radio_value_t aux;

  leds_toggle(LEDS_GREEN);
  printf("\n***\nMessage from: ");

  /* Converts to readable IPv6 address */
  uip_debug_ipaddr_print(sender_addr);

  printf("\nData received on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);

  /* The following functions are the functions provided by the RF library to
   * retrieve information about the channel number we are on, the RSSI (received
   * strenght signal indication), and LQI (link quality information)
   */

  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &aux);
  printf("CH: %u ", (unsigned int) aux);

  aux = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  printf("RSSI: %ddBm ", (int8_t)aux);

  aux = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
  printf("LQI: %u\n", aux);

  /* Print the received payload */
  const char *received_payload = (const char*) data;
  printf("Received payload: %s\n", received_payload);
  
  /* Setup JSON parser */
  struct jsonparse_state json_state;
  jsonparse_setup(&json_state, received_payload, strlen(received_payload));
  
  /* Store type of next JSON element */
  int type;
  
  /* Variables used for storing message id and temperature */
  int message_id = -1;
  int temperature = -1;
  
  /* Parse JSON
   * Type 0 means there are no more tokens to parse or an error occurred.
   */
  while ((type = jsonparse_next(&json_state)) != 0) {
    /* Check if a pair consisting of a key and a value is present */
    if (type == JSON_TYPE_PAIR_NAME) {
      /* Check if a message_id was transmitted */
      if (jsonparse_strcmp_value(&json_state, "message_id") == 0) {
        /* If there is a field called message_id, the next field will be the value */
        jsonparse_next(&json_state);
        message_id = jsonparse_get_value_as_int(&json_state);
      /* else check if a temperature was transmitted */
      } else if (jsonparse_strcmp_value(&json_state, "temperature") == 0) {
        /* If there is a field called temperature, the next field will be the value */
        jsonparse_next(&json_state);
        temperature = jsonparse_get_value_as_int(&json_state);
      }
    }
  }
  
  printf("Transmitted values:\n");
  printf("\tMessage ID: %d\n", message_id);
  printf("\tTemperature: %d\n", temperature);
}
/*---------------------------------------------------------------------------*/

static void
read_temperature(void)
{
  /* Increment message counter */
  counter++;
  
  /* Read sensor values and store as JSON
   * Contiki's JSON library only supports double quotes (") as of now.
   */
  int result;
  result = snprintf(json_buffer, JSON_BUFFER_SIZE - 1,
                    "{\"message_id\": %d, \"temperature\": %d}",
                    counter, cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED));

  /* Check if JSON was stored */
  if (result < 0) {
    printf("An error occurred while generating the temperature message.\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
print_radio_values(void)
{
  radio_value_t aux;

  printf("\n* Radio parameters:\n");

  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &aux);
  printf("\tChannel %u", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &aux);
  printf(" (Min: %u, ", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &aux);
  printf("Max: %u)\n", aux);

  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &aux);
  printf("\tTx Power %3d dBm", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_TXPOWER_MIN, &aux);
  printf(" (Min: %3d dBm, ", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_TXPOWER_MAX, &aux);
  printf("Max: %3d dBm)\n", aux);

  /* This value is set in contiki-conf.h and can be changed */
  printf("\tPAN ID: 0x%02X\n", IEEE802154_CONF_PANID);
}
/*---------------------------------------------------------------------------*/
static void
set_radio_default_parameters(void)
{
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, EXAMPLE_TX_POWER);
  // NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, EXAMPLE_PANID);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, EXAMPLE_CHANNEL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mcast_example_process, ev, data)
{
  static struct etimer periodic_timer;

  /* Data container used to store the IPv6 addresses */
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  /* Alternatively if you want to change the channel or transmission power, these
   * are the functions to use.  You can also change these values in runtime.
   * To check the regular platform values, comment out the function
   * below, so the print_radio_values() function shows the default ones.
   */
  set_radio_default_parameters();

  /* This block prints out the radio constants (minimum and maximum channel,
   * transmission power and current PAN ID (more or less like a subnet).
   */
  print_radio_values();

  /* Create the UDP connection. This function registers a UDP connection and
   * attaches a callback function to it. The callback function will be
   * called for incoming packets. The local UDP port can be set to 0 to indicate
   * that an ephemeral UDP port should be allocated. The remote IP address can
   * be NULL, to indicate that packets from any IP address should be accepted.
   */
  simple_udp_register(&mcast_connection, UDP_CLIENT_PORT, NULL,
                      UDP_CLIENT_PORT, receiver);

  /* Activate the sensors */
  adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);

  etimer_set(&periodic_timer, SEND_INTERVAL);

  while(1) {
    /* Wait for a fixed time */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    printf("\n***\nSending packet to multicast adddress ");

    /* Create a link-local multicast address to all nodes */
    uip_create_linklocal_allnodes_mcast(&addr);

    uip_debug_ipaddr_print(&addr);
    printf("\n");

    /* Read temperature and store as JSON */
    read_temperature();

    /* Send the multicast packet to all devices */
    simple_udp_sendto(&mcast_connection, json_buffer, JSON_BUFFER_SIZE, &addr);
    printf("Sent payload: %s\n", json_buffer);

    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
