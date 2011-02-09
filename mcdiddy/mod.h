
#ifndef SPARTOR_MOD_H_
#define SPARTOR_MOD_H_

#include "SDL.h"
#include "SDL_net.h"
#include "main.h"


#define NATIVEW 384
#define NATIVEH 240


typedef struct {
  char name[16];
  int  presscmd;
  int  releasecmd;
} INPUTNAME_t;


extern int numinputnames;
extern INPUTNAME_t inputnames[];


void mod_setup(Uint32 setupfr);
void mod_setvideo(int w,int h);
void mod_quit();
void mod_keybind(int device,int sym,int press,char cmd);
char mod_key2cmd(int device,int sym,int press);
int mod_command(char *q);
void mod_loadsurfs(int quit);
void mod_predraw(SDL_Surface *screen,Uint32 vidfr);
void mod_draw(SDL_Surface *screen,int objid,OBJ_t *o);
void mod_adv(Uint32 objid,Uint32 a,Uint32 b,OBJ_t *oa,OBJ_t *ob);


#endif
