#ifndef SPARTOR_SJCONSOLE_H_
#define SPARTOR_SJCONSOLE_H_

#include "SDL.h"
#include "font.h"

#define SJC_BUFLEN 200

typedef struct
{
  char   *buf[SJC_BUFLEN];
  size_t  size[SJC_BUFLEN];
  char   *rememory[SJC_BUFLEN];
  int     rememend;
  int     remempos;
} SJC_t;

extern SJC_t SJC;

void SJC_Init();
void SJC_Put(char c);
void SJC_Replace(const char *c);
void SJC_Write(const char *s,...);
void SJC_Rub();
void SJC_Clear();
void SJC_Up();
void SJC_Down();
int SJC_Submit();

#endif
