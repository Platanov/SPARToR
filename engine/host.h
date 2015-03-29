
#ifndef SPARTOR_HOST_H_
#define SPARTOR_HOST_H_

#include "SDL.h"
#include "SDL_net.h"

typedef struct{
  IPaddress addr;
  int connected;
  int lastconga;
  size_t buflen;
  size_t maxlen;
  Uint8 *buf;
} CLIENT_t;

extern UDPsocket  hostsock;

void host_start(int port);
void host_stop();
void host();
void host_write_packets();
void host_read_packet(int clientid);
void host_welcome();

// TODO Move this stuff to a different file:

void host_send_state(int clientid);
void host_outbox_write();
void host_inbox_read(int clientid);

#endif
