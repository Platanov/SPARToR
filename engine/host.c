/**
 **  SPARToR
 **  Network Game Engine
 **  Copyright (C) 2010-2012  Jer Wilson
 **
 **  See COPYING for details.
 **
 **  http://www.superjer.com/
 **  http://www.spartor.com/
 **  http://github.com/superjer/SPARToR
 **/

#include "SDL.h"
#include "SDL_net.h"
#include "mod.h"
#include "main.h"
#include "console.h"
#include "command.h"
#include "host.h"
#include "client.h"
#include "pack.h"

UDPsocket hostsock = NULL;

static CLIENT_t *clients;
static UDPpacket *pkt;

void host_start(int port)
{
  if( !port )
    port = HOSTPORT;

  if( hostsock )
  {
    SJC_Write("Already running as a host. Type disconnect to stop.");
    return;
  }

  if( clientsock )
  {
    SJC_Write("Already connected to a host. Type disconnect if that ain't cool.");
    return;
  }

  if( !(hostsock = SDLNet_UDP_Open(port)) )
  {
    SJC_Write("Error: Could not open host socket!");
    SJC_Write(SDL_GetError());
    return;
  }

  clients = calloc(maxclients, sizeof(CLIENT_t));
  me = 0;
  clients[me].connected = 1;
  clients[me].addr = (IPaddress){0, 0};
  clients[me].lastconga = 0;

  SJC_Write("Host started on port %u.", port);
  pkt = SDLNet_AllocPacket(PACKET_SIZE);
  outbox_len = 0;
}

void host_stop()
{
  free(clients);
  clients = NULL;
  SDLNet_FreePacket(pkt);
  SDLNet_UDP_Close(hostsock);
  hostsock = NULL;
}

void host()
{
  int     status;
  int     i;

  //recv from clients
  for(;;)
  {
    status = SDLNet_UDP_Recv(hostsock, pkt);

    if( status==-1 )
    {
      SJC_Write("Network Error: Recv failed!");
      SJC_Write(SDL_GetError());
    }

    if( status!=1 )
      break;

    for( i=0; i<maxclients; i++ )
      if( clients[i].connected &&
          clients[i].addr.host==pkt->address.host &&
          clients[i].addr.port==pkt->address.port )
        break;

    if( i == maxclients ) //a new client is connecting?
      host_welcome();
    else
      host_read_packet(i);
  }

  host_write_packets();
}

//TODO send everything in the outbox to all the clients
void host_write_packets()
{
  int i;
 
  if( !outbox_len )
    return;

  //TODO break up into smaller packets as needed
  memcpy(pkt->data, outbox, outbox_len);
  outbox_len = 0;

  for( i=0; i<maxclients; i++ )
  {
    pkt->address = clients[i].addr;

    if( !clients[i].connected || !pkt->address.host )
      continue;

    if( !SDLNet_UDP_Send(hostsock, -1, pkt) )
    {
      SJC_Write("Error: Could not send cmds packet!");
      SJC_Write(SDL_GetError());
    }
  }
}

// Write stuff into the outbox so that the network layer can send it to all the clients
void host_read_packet(int clientid)
{
  size_t n = 0;
  int conga = unpackbytes(pkt->data, pkt->len, &n, 4);

  if( conga != clients[clientid].lastconga + 1 )
    return; // silently drop packet on the floor

  size_t payloadlen = unpackbytes(pkt->data, pkt->len, &n, 4);

  if( payloadlen != pkt->len - n )
  {
    SJC_Write("Packet length & payloadlen do not match");
    return;
  }

  if( payloadlen > clients[clientid].maxlen - clients[clientid].buflen )
  {
    SJC_Write("Not enough room to store payload");
    return;
  }

  memcpy(
    clients[clientid].buf + clients[clientid].buflen,
    pkt->data,
    payloadlen
  );

  clients[clientid].buflen += payloadlen;
}

//accept a new client and store the "connection"
void host_welcome()
{
  int     i;
  char   *p = (char *)pkt->data;

  if( strncmp(p, PROTONAME, strlen(PROTONAME)) )
  {
    SJC_Write("Junk packet from unknown client at %u:%u.", pkt->address.host, pkt->address.port);
    return;
  }

  p += strlen(PROTONAME);

  if( *p!='/' )
  {
    SJC_Write("Malformed packet from unknown client.");
    return;
  }

  p++;
  int versiondiff = strncmp(p, VERSION, strlen(VERSION));
  int lenmismatch = (pkt->len != strlen(PROTONAME) + 1 + strlen(VERSION));

  if( lenmismatch || versiondiff )
  {
    SJC_Write("Wrong protocol version from unknown client.");
    //TODO: inform client of the problem
    return;
  }

  for( i=0; i<maxclients; i++ )
    if( !clients[i].connected ) break;

  if( i==maxclients )
  {
    SJC_Write("New client at %u:%u not accepted b/c server is full.", pkt->address.host, pkt->address.port);
    //TODO: inform client
    sprintf((char *)pkt->data, "MFULL: Server is full!");
    pkt->len = strlen((char *)pkt->data)+1;
    SDLNet_UDP_Send(hostsock, -1, pkt);
    return;
  }

  SJC_Write("New client at %u:%u accepted.", pkt->address.host, pkt->address.port);

  clients[i].connected = 1;
  clients[i].addr = pkt->address;
  clients[i].lastconga = 0;
  clients[i].buf = malloc(80000);
  clients[i].maxlen = 80000;
  clients[i].buflen = 0;
}

// TODO Move everything after to a different file:
void host_send_state(int clientid)
{
  // TODO: write to client outbox rather than sending packet directly
  size_t  n;
  Uint32  u;
  Uint8  *data;

  setcmdfr(metafr);
  sethotfr(metafr-1);
  fr[metafr%maxframes].dirty = 1;
  fr[metafr%maxframes].cmds[clientid].flags |= CMDF_NEW;
  // send state!
  data = packframe(surefr, &n);
  SJC_Write("Frame %u packed into %d bytes, ready to send state.", surefr, n);

  if( n+10 > (size_t)pkt->maxlen ) //TODO is this just PACKET_SIZE?
  {
    SJC_Write("Error: Packed frame is too big to send!");
    free(data);
    return;
  }

  pkt->len = n+10;
  packbytes(pkt->data+0,      'S', NULL, 1);
  packbytes(pkt->data+1, clientid, NULL, 1);
  packbytes(pkt->data+2,   metafr, NULL, 4);
  packbytes(pkt->data+6,   surefr, NULL, 4);
  memcpy(pkt->data+10, data, n);

  if( !SDLNet_UDP_Send(hostsock, -1, pkt) )
  {
    SJC_Write("Error: Could not send state packet!");
    SJC_Write(SDL_GetError());
    free(data);
    return;
  }

  //dirty all unsure frames
  SJC_Write("%u: Dirtying all frames from %u to %u", hotfr, surefr, cmdfr);

  for(u=surefr+1;u<cmdfr;u++)
    fr[u%maxframes].dirty = 1;

  free(data);
}

void host_outbox_write()
{
  Uint32  u;
  Uint8  *data;
  size_t  n;
  int     framecount = 0;

  //send to clients
  outbox[0] = 'C';
  outbox[1] = 0;
  outbox[2] = 0;
  outbox_len = 3;

  for( u=surefr+1; u<=cmdfr; u++ ) //scan for dirty frames to send
  {
    if( fr[u%maxframes].dirty )
    {
      data = packframecmds(u, &n);

      if( outbox_len + 4 + n >= OUTBOX_LEN )
      {
        SJC_Write("%u: Packed too many cmds! Will get the rest next frame...", hotfr);
        free(data);
        break;
      }

      packbytes(outbox, u, &outbox_len, 4);
      memcpy(outbox + outbox_len, data, n);
      outbox_len += n;

      free(data);
      framecount++;
      fr[u%maxframes].dirty = 0;
    }
  }

  // store number of frames written back at the top of the buffer
  packbytes(outbox + 1, framecount, NULL, 2);

  if( !framecount )
    outbox_len = 0;
}

void host_inbox_read(int clientid)
{
  FCMD_t *pcmd;
  size_t n = 0;
  CLIENT_t *cl = clients + clientid;

  Uint32 packfr = unpackbytes(cl->buf, cl->buflen, &n, 4);

  if( packfr<metafr-30 )
  {
    SJC_Write("Ignoring too old cmd from client %d", clientid);
    return;
  }

  if( packfr>metafr+10 ) // check for ring buffer safety!
  {
    SJC_Write("Ignoring too new cmd from client %d", clientid);
    return;
  }

  setcmdfr(packfr); // this can NOT be right!!!!!

  if( hotfr>packfr-1 ) // this either!!!
    sethotfr(packfr-1);

  fr[packfr%maxframes].dirty = 1;
  pcmd = fr[packfr%maxframes].cmds+clientid;
  pcmd->cmd     = unpackbytes(cl->buf, cl->buflen, &n, 1);
  pcmd->mousehi = unpackbytes(cl->buf, cl->buflen, &n, 1);
  pcmd->mousex  = unpackbytes(cl->buf, cl->buflen, &n, 1);
  pcmd->mousey  = unpackbytes(cl->buf, cl->buflen, &n, 1);
  pcmd->flags   = unpackbytes(cl->buf, cl->buflen, &n, 2);

  if( pcmd->flags & CMDF_DATA ) //check for variable data
  {
    pcmd->datasz = unpackbytes(cl->buf, cl->buflen, &n, 2);

    if( pcmd->datasz > sizeof pcmd->data )
    {
      SJC_Write("Treachery: datasz too large (%d) from client %d", pcmd->datasz, clientid);
      return;
    }

    memcpy(pcmd->data, cl->buf+n, pcmd->datasz);
    n += pcmd->datasz;
  }
}
