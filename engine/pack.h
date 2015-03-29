
#ifndef SPARTOR_PACK_H_
#define SPARTOR_PACK_H_


#define PROTONAME "SPARToR Protocol"
#define HOSTPORT 31103
#define CLIENTPORT 31109
#define PACKET_SIZE 80000
#define MAX_PORTWALK 4
#define OUTBOX_LEN 80000

extern Uint8 outbox[OUTBOX_LEN];
extern size_t outbox_len;

Uint8 *packframe(      Uint32 packfr,size_t *n);
Uint8 *packframecmds(  Uint32 packfr,size_t *n);
int    unpackframe(    Uint32 packfr,Uint8  *data, size_t  len);
int    unpackframecmds(Uint32 packfr,Uint8  *data, size_t  len);
void   packbytes(      Uint8 *data,  Uint64  value,size_t *offset,int width);
Uint64 unpackbytes(    Uint8 *data,  size_t  len,  size_t *offset,int width);
void   inspectbytes(   Uint8 *data,  int     n);

#endif

