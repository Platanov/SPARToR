/**
 **  Dead Kings' Quest
 **  A special game for the SPARToR Network Game Engine
 **  Copyright (c) 2010-2011  Jer Wilson
 **
 **  See COPYING for details.
 **
 **  http://www.superjer.com/
 **  http://www.spartor.com/
 **  http://github.com/superjer/SPARToR
 **/

#include "obj_.h"

void obj_person_draw( int objid, Uint32 vidfr, OBJ_t *o, CONTEXT_t *co )
{
  PERSON_t *pe = o->data;
  int c = POINT2NATIVE_X(pe->pos)-10;
  int d = POINT2NATIVE_Y(pe->pos)+17;
  SJGL_SetTex( sys_tex[TEX_PERSON].num );
  SJGL_Blit( &(SDL_Rect){20,0  ,20,34}, c, d-34, d   );

  // draw shadow
  SJGL_Blit( &(SDL_Rect){0 ,246,20,10}, c, d-5 , d-1 );
}

void obj_person_adv( int objid, Uint32 a, Uint32 b, OBJ_t *oa, OBJ_t *ob )
{
  int i;
  PERSON_t  *pe = ob->data;
  CONTEXT_t *co = fr[b].objs[ob->context].data;

  pe->vel.y += 0.6f;      //gravity
}

