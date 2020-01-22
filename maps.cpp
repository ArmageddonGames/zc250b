//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  maps.cc
//
//  Map and screen scrolling stuff for zelda.cc
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include <string.h>
#include <assert.h>
#include <vector>
#include <deque>
#include <string>

#include "zc_math.h"
#include "maps.h"
#include "zelda.h"
#include "tiles.h"
#include "sprite.h"
#include "jwin.h"
#include "zsys.h"
#include "subscr.h"
#include "zc_subscr.h"
#include "link.h"
#include "guys.h"
#include "ffscript.h"
#include "particles.h"
#include "trapper_keeper.h"

#define EPSILON 0.01 // Define your own tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#define DegtoFix(d)		((d)*0.71111111)

FONT *get_zc_font(int index);

extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations, particles;
extern movingblock mblock2;                                 //mblock[4]?
extern zinitdata zinit;
extern LinkClass Link;
int current_ffcombo=-1;

short ffposx[32]={-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                  -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
short ffposy[32]={-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                  -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
long ffprvx[32]={-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000};
long ffprvy[32]={-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000};

int draw_screen_clip_rect_x1=0;
int draw_screen_clip_rect_x2=255;
int draw_screen_clip_rect_y1=0;
int draw_screen_clip_rect_y2=223;

extern class DrawingContainer draw_container;

class TileHelper
{
public:

	static void OldPutTile( BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip )
	{
		switch (flip)
		{
			case 1:
				for(int j=0;j<h;j++)
					for(int k=w-1;k>=0;k--)
						oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip);
				break;
			case 2:
				for(int j=h-1;j>=0;j--)
					for(int k=0;k<w;k++)
						oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip);
				break;
			case 3:
				for(int j=h-1;j>=0;j--)
					for(int k=w-1;k>=0;k--)
						oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip);
				break;
			case 0:
			default:
				for(int j=0;j<h;j++)
					for(int k=0;k<w;k++)
						oldputtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip);
				break;
		}
	}

	static void OverTile( BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip )
	{
		switch (flip)
		{
			case 1:
				for(int j=0;j<h;j++)
					for(int k=w-1;k>=0;k--)
						overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip);
				break;
			case 2:
				for(int j=h-1;j>=0;j--)
					for(int k=0;k<w;k++)
						overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip);
				break;
			case 3:
				for(int j=h-1;j>=0;j--)
					for(int k=w-1;k>=0;k--)
						overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip);
				break;
			default:
				for(int j=0;j<h;j++)
					for(int k=0;k<w;k++)
						overtile16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip);
				break;
		}
	}

	static void OverTileTranslucent( BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip, int opacity )
	{
		switch (flip)
		{
			case 1:
				for(int j=0;j<h;j++)
					for(int k=w-1;k>=0;k--)
						overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip, opacity);
				break;
			case 2:
				for(int j=h-1;j>=0;j--)
					for(int k=0;k<w;k++)
						overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip, opacity);
				break;
			case 3:
				for(int j=h-1;j>=0;j--)
					for(int k=w-1;k>=0;k--)
						overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip, opacity);
				break;
			default:
				for(int j=0;j<h;j++)
					for(int k=0;k<w;k++)
						overtiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip, opacity);
				break;
		}
	}

	static void PutTileTranslucent( BITMAP* _Dest, int tile, int x, int y, int w, int h, int color, int flip, int opacity )
	{
		switch (flip)
		{
			case 1:
				for(int j=0;j<h;j++)
					for(int k=w-1;k>=0;k--)
						puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+j*16, color, flip, opacity);
				break;
			case 2:
				for(int j=h-1;j>=0;j--)
					for(int k=0;k<w;k++)
						puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+((h-1)-j)*16, color, flip, opacity);
				break;
			case 3:
				for(int j=h-1;j>=0;j--)
					for(int k=w-1;k>=0;k--)
						puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+((w-1)-k)*16, y+((h-1)-j)*16, color, flip, opacity);
				break;
			default:
				for(int j=0;j<h;j++)
					for(int k=0;k<w;k++)
						puttiletranslucent16(_Dest, tile+(j*TILES_PER_ROW)+k, x+k*16, y+j*16, color, flip, opacity);
				break;
		}
	}


};




//bool draw_screen_clip_rect_show_link=true;
//bool draw_screen_clip_rect_show_guys=false;
bool checktrigger=false;

void debugging_box(int x1, int y1, int x2, int y2)
{
  int *sdci=NULL;
  int i=0;
  for(; i<1000; i++)
  {
    sdci=script_drawing_commands[i];
    if(sdci[0] == 0)
      break;
  }
  if(i >= 1000)
    return;
  sdci[0] = RECTR;
  sdci[1] = 30000;
  sdci[2] = x1*10000;
  sdci[3] = y1*10000;
  sdci[4] = x2*10000;
  sdci[5] = y2*10000;
  sdci[6] = 10000;
  sdci[7] = 10000;
  sdci[8] = 0;
  sdci[9] = 0;
  sdci[10] = 0;
  sdci[11] = 10000;
  sdci[12] = 1280000;
}

void clear_dmap(word i)
{
  memset(&DMaps[i],0,sizeof(dmap));
}

void clear_dmaps()
{
  for (int i=0; i<MAXDMAPS; i++)
  {
    clear_dmap(i);
  }
}

int isdungeon()
{
  // dungeons can have any dlevel above 0
  if((DMaps[currdmap].type&dmfTYPE) == dmDNGN)
  {
    if(tmpscr->flags6&fCAVEROOM)
      return 0;
    return 1;
  }

  // dlevels that aren't dungeons are caves
  if(tmpscr->flags6&fDUNGEONROOM)
    return 1;
  return 0;
}

int MAPCOMBO(int x,int y)
{
	//extend combos outwards if out of bounds -DD
	x = vbound(x, 0, 16*16);
	y = vbound(y, 0, 11*16);
  int combo = (y&0xF0)+(x>>4);
  if(combo>175 || combo < 0)
    return 0;
  return tmpscr->data[combo];                               // entire combo code
}

int MAPFFCOMBO(int x,int y)
{
  for(int i=0;i<32;i++)
  {
    if(x>=(tmpscr->ffx[i]/10000)&&x<(tmpscr->ffx[i]/10000)+(tmpscr->ffwidth[i]&63)+1)
    if(y>=(tmpscr->ffy[i]/10000)&&y<(tmpscr->ffy[i]/10000)+(tmpscr->ffheight[i]&63)+1)
      if(!(tmpscr->ffflags[i]&ffCHANGER) && !(tmpscr->ffflags[i]&ffETHEREAL))
		return tmpscr->ffdata[i];
  }
  return 0;
}

int MAPCSET(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175 || combo < 0)
    return 0;
  return tmpscr->cset[combo];                               // entire combo code
}

int MAPFLAG(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175 || combo < 0)
    return 0;
  return tmpscr->sflag[combo];                              // flag
}

int COMBOTYPE(int x,int y)
{
  return combobuf[MAPCOMBO(x,y)].type;
}

int FFCOMBOTYPE(int x,int y)
{
if (combobuf[MAPFFCOMBO(x,y)].type != 0) al_trace("%d\n",combobuf[MAPFFCOMBO(x,y)].type);
  return combobuf[MAPFFCOMBO(x,y)].type;
}

int MAPCOMBOFLAG(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  return combobuf[tmpscr->data[combo]].flag;                               // entire combo code
}

int MAPFFCOMBOFLAG(int x,int y)
{
  for(int i=0;i<32;i++)
  {
    if(x>=(tmpscr->ffx[i]/10000)&&x<((tmpscr->ffx[i]/10000)+(tmpscr->ffwidth[i]&63)+1))
    if(y>=(tmpscr->ffy[i]/10000)&&y<((tmpscr->ffy[i]/10000)+(tmpscr->ffheight[i]&63)+1))
    {
		if(!(tmpscr->ffflags[i]&ffCHANGER) && !(tmpscr->ffflags[i]&ffETHEREAL))
		{
			current_ffcombo = i;
			return combobuf[tmpscr->ffdata[i]].flag;
		}
    }
  }
  current_ffcombo=-1;
  return 0;
}

int getFFCAt(int x, int y)
{
  for(int i=0;i<32;i++)
  {
    if(x>=(tmpscr->ffx[i]/10000)&&x<((tmpscr->ffx[i]/10000)+(tmpscr->ffwidth[i]&63)+1))
    if(y>=(tmpscr->ffy[i]/10000)&&y<((tmpscr->ffy[i]/10000)+(tmpscr->ffheight[i]&63)+1))
    {
		if(!(tmpscr->ffflags[i]&ffCHANGER) && !(tmpscr->ffflags[i]&ffETHEREAL))
		{
			return i;
		}
    }
  }
  return -1;
}

int MAPCOMBO2(int layer,int x,int y)
{
  if(layer==-1) return MAPCOMBO(x,y);
  if(tmpscr2[layer].data.empty()) return 0;
  if (tmpscr2[layer].valid==0) return 0;
  int combo = (y&0xF0)+(x>>4);
  if(combo>175 || combo < 0)
    return 0;
  return tmpscr2[layer].data[combo];                        // entire combo code
}

int MAPCSET2(int layer,int x,int y)
{
  if(layer==-1) return MAPCSET(x,y);

  if(tmpscr2[layer].cset.empty()) return 0;
  if (tmpscr2[layer].valid==0) return 0;
  int combo = (y&0xF0)+(x>>4);
  if(combo>175 || combo < 0)
    return 0;
  return tmpscr2[layer].cset[combo];                        // entire combo code
}

int MAPFLAG2(int layer,int x,int y)
{
  if(layer==-1) return MAPFLAG(x,y);

  if(tmpscr2[layer].sflag.empty()) return 0;
  if (tmpscr2[layer].valid==0) return 0;
  int combo = (y&0xF0)+(x>>4);
  if(combo>175 || combo < 0)
    return 0;
  return tmpscr2[layer].sflag[combo];                       // flag
}

int COMBOTYPE2(int layer,int x,int y)
{
  if(layer==-1) return COMBOTYPE(x,y);

  if (tmpscr2[layer].valid==0)
  {
    return 0;
  }
  return combobuf[MAPCOMBO2(layer,x,y)].type;
}

int MAPCOMBOFLAG2(int layer,int x,int y)
{
  if(layer==-1) return MAPCOMBOFLAG(x,y);

  if(tmpscr2[layer].data.empty()) return 0;
  if (tmpscr2[layer].valid==0) return 0;
  int combo = (y&0xF0)+(x>>4);
  if(combo>175 || combo < 0)
    return 0;
  return combobuf[tmpscr2[layer].data[combo]].flag;                        // entire combo code
}

void setmapflag(int flag)
{
  setmapflag((currmap<<7)+homescr,flag);
}

// set specific flag
void setmapflag(int mi2, int flag)
{
  byte cscr = mi2&((1<<7)-1);
  byte cmap = (mi2>>7);
  char buf[20];
  sprintf(buf,"Screen (%d, %02X)",cmap,cscr);

  game->maps[mi2] |= flag;
  Z_eventlog("%s's State was set: %s\n",
    mi2 != (currmap<<7)+homescr ? buf : "Current Screen",
    flag==mSECRET ? "Secrets" : flag==mITEM ? "Item" :
    flag==mBELOW ? "Special Item" : flag==mLOCKBLOCK ? "Lock Blocks" :
    flag==mBOSSLOCKBLOCK ? "Boss Lock Blocks" : flag==mCHEST ? "Treasure Chests" :
    flag==mLOCKEDCHEST ? "Locked Chests" : flag==mBOSSCHEST ? "Boss Locked Chests" :
    flag==mTMPNORET ? "Temporary No Return" : flag==mNEVERRET ? "No Return" : flag==mVISITED ? "Visited" :
	flag==mDOOR_UP ? "Door Up" : flag==mDOOR_DOWN ? "Door Down" :
	flag==mDOOR_LEFT ? "Door Left" : flag==mDOOR_RIGHT ? "Door Right" :
    "<Unknown>");
  if(flag==mSECRET||flag==mITEM||flag==mBELOW||flag==mLOCKBLOCK||
     flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
  {
    byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
    byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;

    std::vector<int> done;
    bool looped = (nmap==cmap && nscr==cscr);
    while((nmap!=0) && !looped && !(nscr>=128))
    {
      if((tmpscr->nocarry&flag)!=flag)
      {
        Z_eventlog("State change carried over to (%d, %02X)\n",nmap,nscr);
        game->maps[((nmap-1)<<7)+nscr] |= flag;
      }
      cmap=nmap;
      cscr=nscr;
      nmap=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextmap;
      nscr=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextscr;

      for(std::vector<int>::iterator it = done.begin(); it != done.end(); it++)
      {
        if (*it == ((nmap-1)<<7)+nscr)
          looped = true;
      }
      done.push_back(((nmap-1)<<7)+nscr);
    }
  }
}

void unsetmapflag(int flag)
{
  unsetmapflag((currmap<<7)+homescr,flag);
}

void unsetmapflag(int mi2, int flag)
{
  if(flag==mITEM || flag==mBELOW) {
    if(!(tmpscr->flags4&fNOITEMRESET))
      game->maps[mi2] &= ~flag;
  }
  else game->maps[(currmap<<7)+homescr] &= ~flag;
  Z_eventlog("Screen State was unset: %s\n",
    flag==mSECRET ? "Secrets" : flag==mITEM ? "Item" :
    flag==mBELOW ? "Special Item" : flag==mLOCKBLOCK ? "Lock Blocks" :
    flag==mBOSSLOCKBLOCK ? "Boss Lock Blocks" : flag==mCHEST ? "Treasure Chests" :
    flag==mLOCKEDCHEST ? "Locked Chests" : flag==mBOSSCHEST ? "Boss Locked Chests" :
    flag==mTMPNORET ? "Temporary No Return" : flag==mNEVERRET ? "No Return" : flag==mVISITED ? "Visited" :
    "<Unknown>");

  if(flag==mSECRET||flag==mITEM||flag==mBELOW||flag==mLOCKBLOCK||
     flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
  {
    byte cscr = mi2&((1<<7)-1);
    byte cmap = (mi2>>7);
    byte nmap=TheMaps[((cmap)*MAPSCRS)+cscr].nextmap;
    byte nscr=TheMaps[((cmap)*MAPSCRS)+cscr].nextscr;

    std::vector<int> done;
    bool looped = (nmap==cmap && nscr==cscr);
    while((nmap!=0) && !looped && !(nscr>=128))
    {
      if((tmpscr->nocarry&flag)!=flag)
      {
        Z_eventlog("State change carried over to (%d, %02X)\n",nmap,nscr);
        game->maps[((nmap-1)<<7)+nscr] &= ~flag;
      }
      cmap=nmap;
      cscr=nscr;
      nmap=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextmap;
      nscr=TheMaps[((cmap-1)*MAPSCRS)+cscr].nextscr;

      for(std::vector<int>::iterator it = done.begin(); it != done.end(); it++)
      {
        if (*it == ((nmap-1)<<7)+nscr)
          looped = true;
      }
      done.push_back(((nmap-1)<<7)+nscr);
    }
  }
}

bool getmapflag(int flag)
{
  return (game->maps[(currmap<<7)+homescr] & flag) != 0;
}

int WARPCODE(int dmap,int scr,int dw)
  // returns: -1 = not a warp screen
  //          0+ = warp screen code ( high byte=dmap, low byte=scr )
{
  mapscr *s = &TheMaps[DMaps[dmap].map*MAPSCRS+scr];
  if(s->room!=rWARP)
    return -1;

  int ring=s->catchall;
  int size=QMisc.warp[ring].size;
  if(size==0)
    return -2;

  int index=-1;
  for(int i=0; i<size; i++)
    if(dmap==QMisc.warp[ring].dmap[i] && scr==
	(QMisc.warp[ring].scr[i] + DMaps[dmap].xoff))
      index=i;

    if(index==-1)
    return -3;

  index = (index+dw)%size;
  return (QMisc.warp[ring].dmap[index] << 8) + QMisc.warp[ring].scr[index];
}

void update_combo_cycling()
{
  int x,y;
  for (int i=0; i<176; i++)
  {
    x=tmpscr->data[i];
    y=animated_combo_table[x][0];
    if(combobuf[x].animflags & AF_FRESH) continue;
    //time to restart
    if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
    {
	  screen_combo_modify_preroutine(tmpscr,i);
      tmpscr->data[i]=combobuf[x].nextcombo;
      tmpscr->cset[i]=combobuf[x].nextcset;
      int c=tmpscr->data[i];
      if(combobuf[c].animflags & AF_CYCLE)
      {
        combobuf[c].tile = animated_combo_table[c][1];
        animated_combo_table4[animated_combo_table[c][0]][1]=0;
      }
	  screen_combo_modify_postroutine(tmpscr,i);
    }
  }

  for (int i=0; i<176; i++)
  {
    x=tmpscr->data[i];
    y=animated_combo_table2[x][0];
    if(!(combobuf[x].animflags & AF_FRESH)) continue;
    //time to restart
    if ((animated_combo_table24[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table2[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
    {
	  screen_combo_modify_preroutine(tmpscr,i);
      tmpscr->data[i]=combobuf[x].nextcombo;
      tmpscr->cset[i]=combobuf[x].nextcset;
      int c=tmpscr->data[i];
      if(combobuf[c].animflags & AF_CYCLE)
      {
        combobuf[c].tile = animated_combo_table2[c][1];
        animated_combo_table4[animated_combo_table[c][0]][1]=0;
      }
      screen_combo_modify_postroutine(tmpscr,i);
    }
  }

  for(int i=0;i<32;i++)
  {
    x=tmpscr->ffdata[i];
    y=animated_combo_table[x][0];
    if(combobuf[x].animflags & AF_FRESH) continue;
    //time to restart
    if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
    {
      tmpscr->ffdata[i]=combobuf[x].nextcombo;
      tmpscr->ffcset[i]=combobuf[x].nextcset;
      int c=tmpscr->ffdata[i];
      if(combobuf[c].animflags & AF_CYCLE)
      {
        combobuf[c].tile = animated_combo_table[c][1];
        animated_combo_table4[animated_combo_table[c][0]][1]=0;
      }
    }
  }

  if(get_bit(quest_rules,qr_CMBCYCLELAYERS))
  {
    for (int j=0; j<6; j++)
    {
      if(tmpscr2[j].data.empty()) continue;
      for (int i=0; i<176; i++)
      {
        x=(tmpscr2+j)->data[i];
        y=animated_combo_table[x][0];
        if(combobuf[x].animflags & AF_FRESH) continue;
        //time to restart
        if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
            (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
            (combobuf[x].nextcombo!=0))
        {
		  screen_combo_modify_preroutine(tmpscr2+j,i);
          (tmpscr2+j)->data[i]=combobuf[x].nextcombo;
          (tmpscr2+j)->cset[i]=combobuf[x].nextcset;
          int c=(tmpscr2+j)->data[i];
          if(combobuf[c].animflags & AF_CYCLE)
	      {
            combobuf[c].tile = animated_combo_table[c][1];
	          animated_combo_table4[animated_combo_table[c][0]][1]=0;
          }
		  screen_combo_modify_postroutine(tmpscr2+j,i);
		}
      }
      for (int i=0; i<176; i++)
      {
        x=(tmpscr2+j)->data[i];
        y=animated_combo_table2[x][0];
        if(!(combobuf[x].animflags & AF_FRESH)) continue;
        //time to restart
        if ((animated_combo_table24[y][1]>=combobuf[x].speed) &&
            (combobuf[x].tile-combobuf[x].frames>=animated_combo_table2[x][1]-1) &&
            (combobuf[x].nextcombo!=0))
        {
          (tmpscr2+j)->data[i]=combobuf[x].nextcombo;
          (tmpscr2+j)->cset[i]=combobuf[x].nextcset;
          int c=(tmpscr2+j)->data[i];
          int cs=(tmpscr2+j)->cset[i];
          if(combobuf[c].animflags & AF_CYCLE)
          {
            combobuf[c].tile = animated_combo_table2[c][1];
            animated_combo_table4[animated_combo_table[c][0]][1]=0;
          }
          if (combobuf[c].type==cSPINTILE1)
          {
            // Uses animated_combo_table2
            addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table2[c][1]+zc_max(1,combobuf[c].frames));
          }
        }
      }
    }
  }
}

bool iswater_type(int type)
{
//  return type==cOLD_WATER || type==cSWIMWARP || type==cDIVEWARP || type==cDIVEWARPB || type==cDIVEWARPC || type==cDIVEWARPD || type==cSWIMWARPB || type==cSWIMWARPC || type==cSWIMWARPD;
  return (combo_class_buf[type].water!=0);
}

bool iswater(int combo)
{
  return iswater_type(combobuf[combo].type) && !((tmpscr->flags7 & fWHISTLEWATER) && (whistleclk>=88));
}

bool isstepable(int combo)                                  //can use ladder on it
{
  return (combo_class_buf[combobuf[combo].type].ladder_pass!=0);
}

bool ishookshottable(int bx, int by)
{
  if (!_walkflag(bx,by,1))
    return true;

  bool ret = true;
  for (int i=2; i>=0; i--)
  {
    int c = MAPCOMBO2(i-1,bx,by);
    int t = combobuf[c].type;
    if(i == 0 && t == cHOOKSHOTONLY) return true;
    bool dried = (iswater_type(t) && (tmpscr->flags7 & fWHISTLEWATER) && (whistleclk>=88));

    int b=1;
    if(bx&8) b<<=2;
    if(by&8) b<<=1;

    if(combobuf[c].walk&b && !dried && !(combo_class_buf[t].ladder_pass) && t!=cHOOKSHOTONLY)
      ret = false;
  }
  return ret;
}

bool hiddenstair(int tmp,bool redraw)                       // tmp = index of tmpscr[]
{
  mapscr *s = tmpscr + tmp;

  if(s->stairx || s->stairy)
  {
    int di = (s->stairy&0xF0)+(s->stairx>>4);
    s->data[di] = s->secretcombo[sSTAIRS];
    s->cset[di] = s->secretcset[sSTAIRS];
    s->sflag[di] = s->secretflag[sSTAIRS];
    if(redraw)
      putcombo(scrollbuf,s->stairx,s->stairy,s->data[di],s->cset[di]);
    return true;
  }
  return false;
}

bool remove_screenstatecombos(int tmp, int what1, int what2)
{
  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  bool didit=false;

  for (int i=0; i<176; i++)
  {
    if ((combobuf[s->data[i]].type== what1) ||
        (combobuf[s->data[i]].type== what2) )
    {
      s->data[i]++;
      didit=true;
    }
  }
  for (int j=0; j<6; j++)
  {
    if(t[j].data.empty()) continue;
    for (int i=0; i<176; i++)
    {
      if ((combobuf[t[j].data[i]].type== what1) ||
          (combobuf[t[j].data[i]].type== what2) )
      {
        t[j].data[i]++;
        didit=true;
      }
    }
  }
  return didit;
}

bool remove_lockblocks(int tmp)
{
  return remove_screenstatecombos(tmp, cLOCKBLOCK, cLOCKBLOCK2);
}

bool remove_bosslockblocks(int tmp)
{
  return remove_screenstatecombos(tmp, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2);
}

bool remove_chests(int tmp)                 // tmp = index of tmpscr[]
{
  return remove_screenstatecombos(tmp, cCHEST, cCHEST2);
}

bool remove_lockedchests(int tmp)                 // tmp = index of tmpscr[]
{
  return remove_screenstatecombos(tmp, cLOCKEDCHEST, cLOCKEDCHEST2);
}

bool remove_bosschests(int tmp)                 // tmp = index of tmpscr[]
{
  return remove_screenstatecombos(tmp, cBOSSCHEST, cBOSSCHEST2);
}


bool overheadcombos(mapscr *s)
{
  for (int i=0; i<176; i++)
  {
//    if (combobuf[s->data[i]].type==cOLD_OVERHEAD)
    if (combo_class_buf[combobuf[s->data[i]].type].overhead)
    {
      return true;
    }
  }
  return false;
}

void delete_fireball_shooter(mapscr *s, int i)
{
  int cx=0, cy=0;
  int ct=combobuf[s->data[i]].type;
  if (ct!=cL_STATUE && ct!=cR_STATUE && ct!=cC_STATUE)
    return;

  switch (ct)
  {
    case cL_STATUE:
    cx=((i&15)<<4)+4;
    cy=(i&0xF0)+7;
    break;
    case cR_STATUE:
    cx=((i&15)<<4)-8;
    cy=(i&0xF0)-1;
    break;
    case cC_STATUE:
    cx=((i&15)<<4);
    cy=(i&0xF0);
    break;
  }
  for (int j=0; j<guys.Count(); j++)
  {
    if ((int(guys.spr(j)->x)==cx)&&(int(guys.spr(j)->y)==cy)&&(guysbuf[(guys.spr(j)->id)&0xFFF].flags2 & eneflag_fire))
    {
      guys.del(j);
    }
  }
}

bool findtrigger(int scombo, bool ff)
{
  int checkflag=0;
  int iter;
  for(int j=0;j<(ff?32:176);j++)
  {
    if(ff)
    {
      checkflag=combobuf[tmpscr->ffdata[j]].flag;
      iter=1;
    }
    else iter=2;

    for(int layer=-1; !ff && layer<6; layer++)
    {
      if (layer>-1 && tmpscr2[layer].valid==0) continue;
      for(int i=0; i<iter;i++)
      {
        if(i==0&&!ff)
          checkflag = (layer>-1 ? combobuf[tmpscr2[layer].data[j]].flag : combobuf[tmpscr->data[j]].flag);
        else if(i==1&&!ff)
          checkflag = (layer>-1 ? tmpscr2[layer].sflag[j] : tmpscr->sflag[j]);
        switch(checkflag)
        {
          case mfBCANDLE:
          case mfRCANDLE:
          case mfWANDFIRE:
          case mfDINSFIRE:
          case mfARROW:
          case mfSARROW:
          case mfGARROW:
          case mfSBOMB:
          case mfBOMB:
          case mfBRANG:
          case mfMBRANG:
          case mfFBRANG:
          case mfWANDMAGIC:
          case mfREFMAGIC:
          case mfREFFIREBALL:
          case mfSWORD:
          case mfWSWORD:
          case mfMSWORD:
          case mfXSWORD:
          case mfSWORDBEAM:
          case mfWSWORDBEAM:
          case mfMSWORDBEAM:
          case mfXSWORDBEAM:
          case mfHOOKSHOT:
          case mfWAND:
          case mfHAMMER:
          case mfSTRIKE:
            if(scombo!=j)
              return true;
          default:
          break;
        }
      }
    }
  }
  return false;
}

// single:
// >-1 : the singular triggering combo
// -1: triggered by some other cause
// -2: triggered by Enemies->Secret
// -3: triggered by Secrets screen state
// -4: Screen->TriggerSecrets()
void hidden_entrance(int tmp,bool , bool high16only,int single) //Perhaps better known as 'Trigger Secrets'
{ //There are no calls to 'hidden_entrance' in the code where tmp != 0
  Z_eventlog("%sScreen Secrets triggered%s.\n",
    single>-1? "Restricted ":"",
	  single==-2? " by the 'Enemies->Secret' screen flag":
	  single==-3? " by the 'Secrets' screen state" :
	  single==-4? " by a script":"");
  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  int ft=0; //Flag trigger?

  for(int i=0; i<176; i++) //Do the 'trigger flags' (non 16-31)
  {
    if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
    bool putit;

    if(!high16only || single>=0)
    {
      int newflag = -1;
      for (int iter=0; iter<2; ++iter)
      {
        putit=true;
        int checkflag=combobuf[s->data[i]].flag; //Inherent
        if(iter==1) checkflag=s->sflag[i]; //Placed

        switch(checkflag)
	   {
          case mfBCANDLE: ft=sBCANDLE; break;
          case mfRCANDLE: ft=sRCANDLE; break;
          case mfWANDFIRE: ft=sWANDFIRE; break;
          case mfDINSFIRE: ft=sDINSFIRE; break;
          case mfARROW: ft=sARROW; break;
          case mfSARROW: ft=sSARROW; break;
          case mfGARROW: ft=sGARROW; break;
          case mfSBOMB: ft=sSBOMB; break;
          case mfBOMB: ft=sBOMB; break;
          case mfBRANG: ft=sBRANG; break;
          case mfMBRANG: ft=sMBRANG; break;
          case mfFBRANG: ft=sFBRANG; break;
          case mfWANDMAGIC: ft=sWANDMAGIC; break;
          case mfREFMAGIC: ft=sREFMAGIC; break;
          case mfREFFIREBALL: ft=sREFFIREBALL; break;
          case mfSWORD: ft=sSWORD; break;
          case mfWSWORD: ft=sWSWORD; break;
          case mfMSWORD: ft=sMSWORD; break;
          case mfXSWORD: ft=sXSWORD; break;
          case mfSWORDBEAM: ft=sSWORDBEAM; break;
          case mfWSWORDBEAM: ft=sWSWORDBEAM; break;
          case mfMSWORDBEAM: ft=sMSWORDBEAM; break;
          case mfXSWORDBEAM: ft=sXSWORDBEAM; break;
          case mfHOOKSHOT: ft=sHOOKSHOT; break;
          case mfWAND: ft=sWAND; break;
          case mfHAMMER: ft=sHAMMER; break;
          case mfSTRIKE: ft=sSTRIKE; break;
          default: putit = false; break;
        }
        if (putit) //Change the combos for the secret
        {
          screen_combo_modify_preroutine(s,i);
          s->data[i] = s->secretcombo[ft];
          s->cset[i] = s->secretcset[ft];
          newflag = s->secretflag[ft];
          screen_combo_modify_postroutine(s,i);
        }
      }
      if(newflag >-1) s->sflag[i] = newflag; //Tiered secret

      for (int j=0; j<6; j++) //Layers
      {
        if(t[j].data.empty()||t[j].cset.empty()) continue; //If layer isn't used
        if(single>=0 && i!=single) continue; //If it's got a singular flag and i isn't where the flag is
        int newflag2 = -1;
        for (int iter=0; iter<2; ++iter)
        {
          putit=true;
          int checkflag=combobuf[t[j].data[i]].flag; //Inherent
          if (iter==1) checkflag=t[j].sflag[i]; //Placed
          switch(checkflag)
	     {
		  case mfBCANDLE: ft=sBCANDLE; break;
            case mfRCANDLE: ft=sRCANDLE; break;
            case mfWANDFIRE: ft=sWANDFIRE; break;
            case mfDINSFIRE: ft=sDINSFIRE; break;
            case mfARROW: ft=sARROW; break;
            case mfSARROW: ft=sSARROW; break;
            case mfGARROW: ft=sGARROW; break;
            case mfSBOMB: ft=sSBOMB; break;
            case mfBOMB: ft=sBOMB; break;
            case mfBRANG: ft=sBRANG; break;
            case mfMBRANG: ft=sMBRANG; break;
            case mfFBRANG: ft=sFBRANG; break;
            case mfWANDMAGIC: ft=sWANDMAGIC; break;
            case mfREFMAGIC: ft=sREFMAGIC; break;
            case mfREFFIREBALL: ft=sREFFIREBALL; break;
            case mfSWORD: ft=sSWORD; break;
            case mfWSWORD: ft=sWSWORD; break;
            case mfMSWORD: ft=sMSWORD; break;
            case mfXSWORD: ft=sXSWORD; break;
            case mfSWORDBEAM: ft=sSWORDBEAM; break;
            case mfWSWORDBEAM: ft=sWSWORDBEAM; break;
            case mfMSWORDBEAM: ft=sMSWORDBEAM; break;
            case mfXSWORDBEAM: ft=sXSWORDBEAM; break;
            case mfHOOKSHOT: ft=sHOOKSHOT; break;
            case mfWAND: ft=sWAND; break;
            case mfHAMMER: ft=sHAMMER; break;
            case mfSTRIKE: ft=sSTRIKE; break;
            default: putit = false; break;
          }
          if (putit) //Change the combos for the secret
          {
            t[j].data[i] = t[j].secretcombo[ft];
            t[j].cset[i] = t[j].secretcset[ft];
            newflag2 = t[j].secretflag[ft];
            int c=t[j].data[i];
            int cs=t[j].cset[i];
            if (combobuf[c].type==cSPINTILE1) //Surely this means we can have spin tiles on layers 3+? Isn't that bad? ~Joe123
              addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table[c][1]+zc_max(1,combobuf[c].frames));
          }
        }
        if (newflag2 >-1) t[j].sflag[i] = newflag2; //Tiered secret
      }
    }
  }

  for(int i=0; i<32; i++) //FFC 'trigger flags'
  {
    if(single>=0) if(i+176!=single) continue;
    bool putit;

    if((!high16only)||(single>=0))
    {
      for (int iter=0; iter<1; ++iter) // Only one kind of FFC flag now.
      {
        putit=true;
        int checkflag=combobuf[s->ffdata[i]].flag; //Inherent
        //No placed flags yet
        switch(checkflag)
	   {
          case mfBCANDLE: ft=sBCANDLE; break;
          case mfRCANDLE: ft=sRCANDLE; break;
          case mfWANDFIRE: ft=sWANDFIRE; break;
          case mfDINSFIRE: ft=sDINSFIRE; break;
          case mfARROW: ft=sARROW; break;
          case mfSARROW: ft=sSARROW; break;
          case mfGARROW: ft=sGARROW; break;
          case mfSBOMB: ft=sSBOMB; break;
          case mfBOMB: ft=sBOMB; break;
          case mfBRANG: ft=sBRANG; break;
          case mfMBRANG: ft=sMBRANG; break;
          case mfFBRANG: ft=sFBRANG; break;
          case mfWANDMAGIC: ft=sWANDMAGIC; break;
          case mfREFMAGIC: ft=sREFMAGIC; break;
          case mfREFFIREBALL: ft=sREFFIREBALL; break;
          case mfSWORD: ft=sSWORD; break;
          case mfWSWORD: ft=sWSWORD; break;
          case mfMSWORD: ft=sMSWORD; break;
          case mfXSWORD: ft=sXSWORD; break;
          case mfSWORDBEAM: ft=sSWORDBEAM; break;
          case mfWSWORDBEAM: ft=sWSWORDBEAM; break;
          case mfMSWORDBEAM: ft=sMSWORDBEAM; break;
          case mfXSWORDBEAM: ft=sXSWORDBEAM; break;
          case mfHOOKSHOT: ft=sHOOKSHOT; break;
          case mfWAND: ft=sWAND; break;
          case mfHAMMER: ft=sHAMMER; break;
          case mfSTRIKE: ft=sSTRIKE; break;
          default: putit = false; break;
        }
	   if (putit) //Change the ffc's combo
	   {
		s->ffdata[i] = s->secretcombo[ft];
		s->ffcset[i] = s->secretcset[ft];
	   }
      }
    }
  }

  if(checktrigger) //Hit all triggers->16-31
  {
    checktrigger=false;
    if(tmpscr->flags6&fTRIGGERF1631)
    {
    	 //If we find a trigger, we haven't hit them all
	 if(findtrigger(-1,false)) goto endhe; //Normal flags
      if(findtrigger(-1,true)) goto endhe; //FFCs
    }
  }

  for(int i=0; i<176; i++) // Do the 16-31 secrets
  {
    //If it's an enemies->secret screen, only do the high 16 if told to
    //That way you can have secret and burn/bomb entrance separately
    if((!(s->flags2&fCLEARSECRET) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM)
    {
      int newflag = -1;
      for (int iter=0; iter<2; ++iter)
      {
        int checkflag=combobuf[s->data[i]].flag; //Inherent
        if (iter==1) checkflag=s->sflag[i]; //Placed
        if((checkflag > 15)&&(checkflag < 32)) //If we've got a 16->32 flag change the combo
        {
          screen_combo_modify_preroutine(s,i);
          s->data[i] = s->secretcombo[checkflag-16+4];
          s->cset[i] = s->secretcset[checkflag-16+4];
          newflag = s->secretflag[checkflag-16+4];
          screen_combo_modify_postroutine(s,i);
        }
      }
      if (newflag >-1) s->sflag[i] = newflag; //Tiered flag

      for (int j=0; j<6; j++) //Layers
      {
        if(t[j].data.empty()||t[j].cset.empty()) continue; //If layer is not valid (surely checking for 'valid' would be better?)
        int newflag2 = -1;
        for (int iter=0; iter<2; ++iter)
        {
          int checkflag=combobuf[t[j].data[i]].flag; //Inherent
          if (iter==1) checkflag=t[j].sflag[i]; //Placed
          if((checkflag > 15)&&(checkflag < 32)) //If we've got a 16->32 flag change the combo
          {
            t[j].data[i] = t[j].secretcombo[checkflag-16+4];
            t[j].cset[i] = t[j].secretcset[checkflag-16+4];
            newflag2 = t[j].secretflag[checkflag-16+4];
          }
        }
        if (newflag2 >-1) t[j].sflag[i] = newflag2; //Tiered flag
      }
    }

    /*
      if(putit && refresh)
      putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
      */
  }

  for(int i=0; i<32; i++) // FFCs
  {
    if((!(s->flags2&fCLEARSECRET) /*Enemies->Secret*/ && single < 0) || high16only || s->flags4&fENEMYSCRTPERM)
    {
	  for (int iter=0; iter<1; ++iter) // Only one kind of FFC flag now.
      {
        int checkflag=combobuf[s->ffdata[i]].flag; //Inherent
        //No placed flags yet
        if((checkflag > 15)&&(checkflag < 32)) //If we find a flag, change the combo
        {
          s->ffdata[i] = s->secretcombo[checkflag-16+4];
          s->ffcset[i] = s->secretcset[checkflag-16+4];
        }
      }
    }
  }

endhe:
  if(tmpscr->flags4&fDISABLETIME) //Finish timed warp if 'Secrets Disable Timed Warp'
  {
    activated_timed_warp=true;
    tmpscr->timedwarptics = 0;
  }
}


bool findentrance(int x, int y, int flag, bool setflag)
{
  bool foundflag=false;
  bool foundcflag=false;
  bool foundnflag=false;
  bool foundfflag=false;
  //bool ffcombosingle = false;
  int ffcombos[4] = {-1, -1, -1, -1};
  bool single16=false;
  int scombo=-1;

  for (int i=-1; i<6; i++) // Layers. -1 = screen.
  {
    if(MAPFLAG2(i,x,y)==flag || MAPFLAG2(i,x+15,y)==flag ||
       MAPFLAG2(i,x,y+15)==flag || MAPFLAG2(i,x+15,y+15)==flag)
    {
      foundflag=true;
      foundnflag=true;
    }
  }

  for (int i=-1; i<6; i++) // Layers. -1 = screen.
  {
    if(MAPCOMBOFLAG2(i,x,y)==flag || MAPCOMBOFLAG2(i,x+15,y)==flag ||
       MAPCOMBOFLAG2(i,x,y+15)==flag || MAPCOMBOFLAG2(i,x+15,y+15)==flag)
    {
      foundflag=true;
      foundcflag=true;
    }
  }

  if(MAPFFCOMBOFLAG(x,y)==flag)
  {
  foundflag=true;
    foundfflag=true;
  }
  ffcombos[0] = current_ffcombo;

  if(MAPFFCOMBOFLAG(x+15,y)==flag)
  {
  foundflag=true;
    foundfflag=true;
  }
  ffcombos[1] = current_ffcombo;

  if(MAPFFCOMBOFLAG(x,y+15)==flag)
  {
  foundflag=true;
    foundfflag=true;
  }
  ffcombos[2] = current_ffcombo;

  if(MAPFFCOMBOFLAG(x+15,y+15)==flag)
  {
  foundflag=true;
    foundfflag=true;
  }
  ffcombos[3] = current_ffcombo;

  if (!foundflag)
  {
    return false;
  }

  for (int i=-1; i<6; i++) // Look for Trigger->Self on all layers
  {
    if(foundnflag) // Trigger->Self (a.k.a Singular) is inherent
    {
      if((MAPCOMBOFLAG2(i,x,y)==mfSINGLE)&&(MAPFLAG2(i,x,y)==flag))
      {
        scombo=(y&0xF0)+(x>>4);
      }
      else if((MAPCOMBOFLAG2(i,x,y)==mfSINGLE16)&&(MAPFLAG2(i,x,y)==flag))
      {
        scombo=(y&0xF0)+(x>>4);
        single16=true;
      }
      else if((MAPCOMBOFLAG2(i,x+15,y)==mfSINGLE)&&(MAPFLAG2(i,x+15,y)==flag))
        {
          scombo=(y&0xF0)+((x+15)>>4);
        }
        else if((MAPCOMBOFLAG2(i,x+15,y)==mfSINGLE16)&&(MAPFLAG2(i,x+15,y)==flag))
          {
            scombo=(y&0xF0)+((x+15)>>4);
            single16=true;
          }
          else if((MAPCOMBOFLAG2(i,x,y+15)==mfSINGLE)&&(MAPFLAG2(i,x,y+15)==flag))
            {
              scombo=((y+15)&0xF0)+(x>>4);
            }
            else if((MAPCOMBOFLAG2(i,x,y+15)==mfSINGLE16)&&(MAPFLAG2(i,x,y+15)==flag))
              {
                scombo=((y+15)&0xF0)+(x>>4);
                single16=true;
              }
              else if((MAPCOMBOFLAG2(i,x+15,y+15)==mfSINGLE)&&(MAPFLAG2(i,x+15,y+15)==flag))
                {
                  scombo=((y+15)&0xF0)+((x+15)>>4);
                }
                else if((MAPCOMBOFLAG2(i,x+15,y+15)==mfSINGLE16)&&(MAPFLAG2(i,x+15,y+15)==flag))
                  {
                    scombo=((y+15)&0xF0)+((x+15)>>4);
                    single16=true;
                  }
    }
    if(foundcflag) // Trigger->Self (a.k.a Singular) is non-inherent
    {
      if((MAPFLAG2(i,x,y)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x,y)==flag))
      {
        scombo=(y&0xF0)+(x>>4);
      }
      else if((MAPFLAG2(i,x,y)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x,y)==flag))
      {
        scombo=(y&0xF0)+(x>>4);
        single16=true;
      }
      else if((MAPFLAG2(i,x+15,y)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x+15,y)==flag))
        {
          scombo=(y&0xF0)+((x+15)>>4);
        }
        else if((MAPFLAG2(i,x+15,y)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x+15,y)==flag))
          {
            scombo=(y&0xF0)+((x+15)>>4);
            single16=true;
          }
          else if((MAPFLAG2(i,x,y+15)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x,y+15)==flag))
            {
              scombo=((y+15)&0xF0)+(x>>4);
            }
            else if((MAPFLAG2(i,x,y+15)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x,y+15)==flag))
              {
                scombo=((y+15)&0xF0)+(x>>4);
                single16=true;
              }
              else if((MAPFLAG2(i,x+15,y+15)==mfSINGLE)&&(MAPCOMBOFLAG2(i,x+15,y+15)==flag))
                {
                  scombo=((y+15)&0xF0)+((x+15)>>4);
                }
                else if((MAPFLAG2(i,x+15,y+15)==mfSINGLE16)&&(MAPCOMBOFLAG2(i,x+15,y+15)==flag))
                  {
                    scombo=((y+15)&0xF0)+((x+15)>>4);
                    single16=true;
                  }
    }
  }
  if(scombo<0)
  {
    checktrigger=true;
    hidden_entrance(0,true);
  }
  else
  {
    checktrigger=true;
    hidden_entrance(0,true,single16,scombo);
  }
  sfx(tmpscr->secretsfx);
  if(tmpscr->flags6&fTRIGGERFPERM)
  {
    if(findtrigger(-1,false)) setflag=false;
    if(findtrigger(-1,true)) setflag=false;
  }
  if(setflag && !isdungeon())
    if(!(tmpscr->flags5&fTEMPSECRETS))
      setmapflag(mSECRET);
  return true;
}

void update_freeform_combos()
{
	ffscript_engine(false);
	for(int i=0;i<32;i++)
	{
		if(!(tmpscr->ffflags[i]&ffCHANGER) && tmpscr->ffdata[i]!=0 && !(tmpscr->ffflags[i]&ffSTATIONARY))
		{

			for(int j=0;j<32;j++)
			{
				if(i!=j)
				{
					if(tmpscr->ffflags[j]&ffCHANGER && tmpscr->ffdata[j] != 0)
					{
						if((((tmpscr->ffx[j]/10000)!=ffposx[i])||((tmpscr->ffy[j]/10000)!=ffposy[i]))&&(tmpscr->fflink[i]==0))
						{
							if((isonline(tmpscr->ffx[i],tmpscr->ffy[i],ffprvx[i],ffprvy[i],tmpscr->ffx[j],tmpscr->ffy[j])||
								((tmpscr->ffx[i]==tmpscr->ffx[j])&&(tmpscr->ffy[i]==tmpscr->ffy[j])))&&(ffprvx[i]>-10000000&&ffprvy[i]>-10000000))
							{
								//tmpscr->ffdata[i]=tmpscr->ffdata[j];
								//tmpscr->ffcset[i]=tmpscr->ffcset[j];
								if(tmpscr->ffflags[j]&ffCHANGETHIS)
								{
									tmpscr->ffdata[i] = tmpscr->ffdata[j];
									tmpscr->ffcset[i] = tmpscr->ffcset[j];
								}
								if(tmpscr->ffflags[j]&ffCHANGENEXT)
									tmpscr->ffdata[i]++;
								if(tmpscr->ffflags[j]&ffCHANGEPREV)
									tmpscr->ffdata[i]--;
								tmpscr->ffdelay[i]=tmpscr->ffdelay[j];
								tmpscr->ffx[i]=tmpscr->ffx[j];
								tmpscr->ffy[i]=tmpscr->ffy[j];
								tmpscr->ffxdelta[i]=tmpscr->ffxdelta[j];
								tmpscr->ffydelta[i]=tmpscr->ffydelta[j];
								tmpscr->ffxdelta2[i]=tmpscr->ffxdelta2[j];
								tmpscr->ffydelta2[i]=tmpscr->ffydelta2[j];
								tmpscr->fflink[i]=tmpscr->fflink[j];
								tmpscr->ffwidth[i]=tmpscr->ffwidth[j];
								tmpscr->ffheight[i]=tmpscr->ffheight[j];
								if(tmpscr->ffflags[i]&ffCARRYOVER)
									tmpscr->ffflags[i]=tmpscr->ffflags[j]|ffCARRYOVER;
								else
								{
									tmpscr->ffflags[i]=tmpscr->ffflags[j];
								}
								tmpscr->ffflags[i]&=~ffCHANGER;
								ffposx[i]=(short)(tmpscr->ffx[j]/10000);
								ffposy[i]=(short)(tmpscr->ffy[j]/10000);

								if(combobuf[tmpscr->ffdata[j]].flag>15 && combobuf[tmpscr->ffdata[j]].flag<32)
								{
									tmpscr->ffdata[j]=tmpscr->secretcombo[combobuf[tmpscr->ffdata[j]].flag-16+4];
								}
								if((tmpscr->ffflags[j]&ffSWAPNEXT)||(tmpscr->ffflags[j]&ffSWAPPREV))
								{
									int k=0;
									if(tmpscr->ffflags[j]&ffSWAPNEXT)
										k=j<31?j+1:0;
									if(tmpscr->ffflags[j]&ffSWAPPREV)
										k=j>0?j-1:31;
									zc_swap(tmpscr->ffdata[j],tmpscr->ffdata[k]);
									zc_swap(tmpscr->ffcset[j],tmpscr->ffcset[k]);
									zc_swap(tmpscr->ffdelay[j],tmpscr->ffdelay[k]);
									zc_swap(tmpscr->ffxdelta[j],tmpscr->ffxdelta[k]);
									zc_swap(tmpscr->ffydelta[j],tmpscr->ffydelta[k]);
									zc_swap(tmpscr->ffxdelta2[j],tmpscr->ffxdelta2[k]);
									zc_swap(tmpscr->ffydelta2[j],tmpscr->ffydelta2[k]);
									zc_swap(tmpscr->fflink[j],tmpscr->fflink[k]);
									zc_swap(tmpscr->ffwidth[j],tmpscr->ffwidth[k]);
									zc_swap(tmpscr->ffheight[j],tmpscr->ffheight[k]);
									zc_swap(tmpscr->ffflags[j],tmpscr->ffflags[k]);
								}
							}
						}
					}
				}
			}

			if(tmpscr->fflink[i] ? !tmpscr->ffdelay[tmpscr->fflink[i]] : !tmpscr->ffdelay[i])
			{
				if(tmpscr->fflink[i]&&(tmpscr->fflink[i]-1)!=i)
				{
					ffprvx[i] = tmpscr->ffx[i];
					ffprvy[i] = tmpscr->ffy[i];
					tmpscr->ffx[i]+=tmpscr->ffxdelta[tmpscr->fflink[i]-1];
					tmpscr->ffy[i]+=tmpscr->ffydelta[tmpscr->fflink[i]-1];
				}
				else
				{
					ffprvx[i] = tmpscr->ffx[i];
					ffprvy[i] = tmpscr->ffy[i];
					tmpscr->ffx[i]+=tmpscr->ffxdelta[i];
					tmpscr->ffy[i]+=tmpscr->ffydelta[i];
					tmpscr->ffxdelta[i]+=tmpscr->ffxdelta2[i];
					tmpscr->ffydelta[i]+=tmpscr->ffydelta2[i];
					if(tmpscr->ffxdelta[i]>1280000) tmpscr->ffxdelta[i]=1280000;
					if(tmpscr->ffxdelta[i]<-1280000) tmpscr->ffxdelta[i]=-1280000;
					if(tmpscr->ffydelta[i]>1280000) tmpscr->ffydelta[i]=1280000;
					if(tmpscr->ffydelta[i]<-1280000) tmpscr->ffydelta[i]=-1280000;
				}
			}
			else
			{
				if(!tmpscr->fflink[i] || (tmpscr->fflink[i]-1)==i)
					tmpscr->ffdelay[i]--;
			}
			if(tmpscr->ffx[i]<-320000)
			{
				if(tmpscr->flags6&fWRAPAROUNDFF)
				{
					tmpscr->ffx[i] = 2880000+(tmpscr->ffx[i]+320000);
					ffprvy[i] = tmpscr->ffy[i];
				}
				else
				{
					tmpscr->ffdata[i]=0;
					tmpscr->ffflags[i]&=~ffCARRYOVER;
				}
			}
			if(tmpscr->ffy[i]<-320000)
			{
				if(tmpscr->flags6&fWRAPAROUNDFF)
				{
					tmpscr->ffy[i] = 2080000+(tmpscr->ffy[i]+320000);
					ffprvx[i] = tmpscr->ffx[i];
				}
				else
				{
					tmpscr->ffdata[i]=0;
					tmpscr->ffflags[i]&=~ffCARRYOVER;
				}
			}
			if(tmpscr->ffx[i]>=2880000)
			{
				if(tmpscr->flags6&fWRAPAROUNDFF)
				{
					tmpscr->ffx[i] = tmpscr->ffx[i]-2880000-320000;
					ffprvy[i] = tmpscr->ffy[i];
				}
				else
				{
					tmpscr->ffdata[i]=0;
					tmpscr->ffflags[i]&=~ffCARRYOVER;
				}
			}
			if(tmpscr->ffy[i]>=2080000)
			{
				if(tmpscr->flags6&fWRAPAROUNDFF)
				{
					tmpscr->ffy[i] = tmpscr->ffy[i]-2080000-320000;
					ffprvy[i] = tmpscr->ffy[i];
				}
				else
				{
					tmpscr->ffdata[i]=0;
					tmpscr->ffflags[i]&=~ffCARRYOVER;
				}
			}
		}
	}
}

bool hitcombo(int x, int y, int combotype)
{
  return (COMBOTYPE(x,y)==combotype);
}

bool hitflag(int x, int y, int flagtype)
{
  return (MAPFLAG(x,y)==flagtype||MAPCOMBOFLAG(x,y)==flagtype);
}

int nextscr(int dir)
{
  int m = currmap;
  int s = currscr;

  switch(dir)
  {
    case up:    s-=16; break;
    case down:  s+=16; break;
    case left:  s-=1;  break;
    case right: s+=1;  break;
  }

  // need to check for screens on other maps, 's' not valid, etc.

  int index = (tmpscr->sidewarpindex >> (dir*2))&3;
  if(tmpscr->sidewarptype[index] == 3)                                // scrolling warp
  {
    switch(dir)
    {
      case up:    if(!(tmpscr->flags2&wfUP))    goto skip; break;
      case down:  if(!(tmpscr->flags2&wfDOWN))  goto skip; break;
      case left:  if(!(tmpscr->flags2&wfLEFT))  goto skip; break;
      case right: if(!(tmpscr->flags2&wfRIGHT)) goto skip; break;
    }
    m = DMaps[tmpscr->sidewarpdmap[index]].map;
    s = tmpscr->sidewarpscr[index] + DMaps[tmpscr->sidewarpdmap[index]].xoff;
  }

  if(s<0||s>=128)
    return 0xFFFF;

skip:

  return (m<<7) + s;
}

void bombdoor(int x,int y)
{
  if(tmpscr->door[0]==dBOMB && isinRect(x,y,100,0,139,48))
  {
    tmpscr->door[0]=dBOMBED;
    putdoor(scrollbuf,0,0,dBOMBED);
    setmapflag(mDOOR_UP);
    markBmap(-1);
    if(nextscr(up)!=0xFFFF)
    {
      setmapflag(nextscr(up), mDOOR_DOWN);
      markBmap(-1,nextscr(up)-(get_currdmap()<<7));
    }
  }
  if(tmpscr->door[1]==dBOMB && isinRect(x,y,100,112,139,176))
  {
    tmpscr->door[1]=dBOMBED;
    putdoor(scrollbuf,0,1,dBOMBED);
    setmapflag(mDOOR_DOWN);
    markBmap(-1);
    if(nextscr(down)!=0xFFFF)
    {
      setmapflag(nextscr(down), mDOOR_UP);
      markBmap(-1,nextscr(down)-(get_currdmap()<<7));
    }
  }
  if(tmpscr->door[2]==dBOMB && isinRect(x,y,0,60,48,98))
  {
    tmpscr->door[2]=dBOMBED;
    putdoor(scrollbuf,0,2,dBOMBED);
    setmapflag(mDOOR_LEFT);
    markBmap(-1);
    if(nextscr(left)!=0xFFFF)
    {
      setmapflag(nextscr(left), mDOOR_RIGHT);
      markBmap(-1,nextscr(left)-(get_currdmap()<<7));
    }
  }
  if(tmpscr->door[3]==dBOMB && isinRect(x,y,192,60,240,98))
  {
    tmpscr->door[3]=dBOMBED;
    putdoor(scrollbuf,0,3,dBOMBED);
    setmapflag(mDOOR_RIGHT);
    markBmap(-1);
    if(nextscr(right)!=0xFFFF)
    {
      setmapflag(nextscr(right), mDOOR_LEFT);
      markBmap(-1,nextscr(right)-(get_currdmap()<<7));
    }
  }
}

void do_scrolling_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, bool scrolling, int tempscreen)
{
  static int mf;
  switch (type)
  {
    case -4: //overhead FFCs
    case -3:                                                //freeform combos
      for(int i=31;i>=0;i--)
      {
        if(layer->ffdata[i])
        {
          if(!(layer->ffflags[i]&ffCHANGER) && (!(layer->ffflags[i]&ffLENSVIS) || lensclk))
          {
           if(!!(layer->ffflags[i]&ffOVERLAY) == (type==-4))
            {
              int tx=((layer->ffx[i]/10000));
              int ty=((layer->ffy[i]/10000))+playing_field_offset;
              if(layer->ffflags[i]&ffTRANS)
              {
                overcomboblocktranslucent(bmp, tx-x, ty-y, layer->ffdata[i], layer->ffcset[i], 1+(layer->ffwidth[i]>>6), 1+(layer->ffheight[i]>>6),128);
              }
              else
              {
                overcomboblock(bmp, tx-x, ty-y, layer->ffdata[i], layer->ffcset[i], 1+(layer->ffwidth[i]>>6), 1+(layer->ffheight[i]>>6));
              }
            }
          }
        }
      }
      break;
    case -2:                                                //push blocks
      for (int i=0; i<176; i++)
      {
        mf=layer->sflag[i];
        if (mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED || ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS)))
        {
          overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
        }
        else
        {
          mf=combobuf[layer->data[i]].flag;
          if (mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED || ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS)))
          {
            overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
          }
        }
      }
      break;
    case -1:                                                //over combo
      for (int i=0; i<176; i++)
      {
//        if (combobuf[layer->data[i]].type==cOLD_OVERHEAD)
        if (combo_class_buf[combobuf[layer->data[i]].type].overhead)
        {
          overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
        }
      }
      break;
    case 0:
    //case 1:
    //case 2:
    case 3:
    case 4:
    case 5:
      if (TransLayers || layer->layeropacity[type]==255)
      {
        if (layer->layermap[type]>0)
        {
          if (scrolling)
          {
            if (layer->layeropacity[type]==255)
            {
              if (tempscreen==2)
              {
                for (int i=0; i<176; i++)
                {
                  overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                }
              }
              else
              {
                for (int i=0; i<176; i++)
                {
                  overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                }
              }
            }
            else
            {
              if (tempscreen==2)
              {
                for (int i=0; i<176; i++)
                {
                  overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                }
              }
              else
              {
                for (int i=0; i<176; i++)
                {
                  overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                }
              }
            }
          }
          else
          {
            if (layer->layeropacity[type]==255)
            {
              if (tempscreen==2)
              {
                for (int i=0; i<176; i++)
                {
                  overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                }
              }
              else
              {
                for (int i=0; i<176; i++)
                {
                  overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                }
              }
            }
            else
            {
              if (tempscreen==2)
              {
                for (int i=0; i<176; i++)
                {
                  overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                }
              }
              else
              {
                for (int i=0; i<176; i++)
                {
                  overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                }
              }
            }
          }
        }
      }
      break;
    case 1:
      if (TransLayers || layer->layeropacity[type]==255)
      {
        if (layer->layermap[type]>0)
        {
          if (scrolling)
          {
            if (layer->layeropacity[type]==255)
            {
              if(layer->flags7&fLAYER2BG)
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
              else
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
            }
            else
            {
              if(layer->flags7&fLAYER2BG)
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
              else
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                  }
                }
              }
            }
          }
          else
          {
            if (layer->layeropacity[type]==255)
            {
              if(layer->flags7&fLAYER2BG)
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
              else
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
            }
            else
            {
              if(layer->flags7&fLAYER2BG)
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
              else
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                  }
                }
              }
            }
          }
        }
      }
      break;
    case 2:
      if (TransLayers || layer->layeropacity[type]==255)
      {
        if (layer->layermap[type]>0)
        {
          if (scrolling)
          {
            if (layer->layeropacity[type]==255)
            {
              if(layer->flags7&fLAYER3BG&&!(layer->flags7&fLAYER2BG))
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
              else
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
            }
            else
            {
              if(layer->flags7&fLAYER3BG&&!(layer->flags7&fLAYER2BG))
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
              else
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                  }
                }
              }
            }
          }
          else
          {
            if (layer->layeropacity[type]==255)
            {
              if(layer->flags7&fLAYER3BG&&!(layer->flags7&fLAYER2BG))
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
              else
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
            }
            else
            {
              if(layer->flags7&fLAYER3BG&&!(layer->flags7&fLAYER2BG))
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i]);
                  }
                }
              }
              else
              {
                if (tempscreen==2)
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr2[type].data[i],tmpscr2[type].cset[i],layer->layeropacity[type]);
                  }
                }
                else
                {
                  for (int i=0; i<176; i++)
                  {
                    overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tmpscr3[type].data[i],tmpscr3[type].cset[i],layer->layeropacity[type]);
                  }
                }
              }
            }
          }
        }
      }
      break;
  }
}

void do_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, int tempscreen)
{
  bool showlayer = true;
  switch(type)
  {
    case -4:
    case -3:
      if (!show_ffcs)
      {
        showlayer = false;
      }
      break;
    case -2:
      if (!show_layer_push)
      {
        showlayer = false;
      }
      break;
    case -1:
      if (!show_layer_over)
      {
        showlayer = false;
      }
      break;
    case 0:
      if (!show_layer_1)
      {
        showlayer = false;
      }
      break;
    case 1:
      if (!show_layer_2)
      {
        showlayer = false;
      }
      break;
    case 2:
      if (!show_layer_3)
      {
        showlayer = false;
      }
      break;
    case 3:
      if (!show_layer_4)
      {
        showlayer = false;
      }
      break;
    case 4:
      if (!show_layer_5)
      {
        showlayer = false;
      }
      break;
    case 5:
      if (!show_layer_6)
      {
        showlayer = false;
      }
      break;
  }

  if(type==(int)(layer->lens_layer&7) && ((layer->lens_layer&llLENSSHOWS && !lensclk) || (layer->lens_layer&llLENSHIDES && lensclk)))
  {
    showlayer = false;
  }

  if(showlayer)
  {
    do_scrolling_layer(bmp, type, layer, x, y, false, tempscreen);
    if(type>=0&&type<=5)
    {
      do_primitives(bmp, type+1, layer, x, y+playing_field_offset);
    }
  }
}

void do_primitives(BITMAP *bmp, int type, mapscr *, int xoffset, int yoffset)
{
  color_map=&trans_table2;
  //was this next variable ever used? -- DN
  //bool drawsubscr=false;
  /*if(prim_bmp == NULL || prim_bmp->w != bmp->w || prim_bmp->h != bmp->h)
  {
    if(prim_bmp != NULL)
    {
      destroy_bitmap(prim_bmp);
    }
    prim_bmp = create_bitmap_ex(8, bmp->w, bmp->h);
  }*/
  //clear_bitmap(prim_bmp);

  TileHelper Tile;
  //TileHelper *Tile = new TileHelper(); ..any lingering returns?

  for(int i=0; i<MAX_SCRIPT_DRAWING_COMMANDS; i++)
  {
    int *sdci=script_drawing_commands[i];
    if(sdci[0]==0)
    {
    //continuing here is too slow
    //there shouldn't be any discontinuities here anyway -DD
      break;
    }
    //clear_bitmap(prim_bmp);

    if(sdci[1]/10000 == type) //layer?
    {
      switch(sdci[0])
      {
        case RECTR:
          {
            //sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=x2
            //sdci[5]=y2
            //sdci[6]=color
            //sdci[7]=scale factor
            //sdci[8]=rotation anchor x
            //sdci[9]=rotation anchor y
            //sdci[10]=rotation angle
            //sdci[11]=fill
            //sdci[12]=opacity
            if (sdci[7]==0) //scale
            {
              break;
            }
            int x1=sdci[2]/10000;
            int y1=sdci[3]/10000;
            int x2=sdci[4]/10000;
            int y2=sdci[5]/10000;
            if (x1>x2)
            {
              zc_swap(x1,x2);
            }
            if (y1>y2)
            {
              zc_swap(y1,y2);
            }
            if (sdci[7] != 10000)
            {
              int w=x2-x1+1;
              int h=y2-y1+1;
              int w2=(w*sdci[7])/10000;
              int h2=(h*sdci[7])/10000;
              x1=x1-((w2-w)/2);
              x2=x2+((w2-w)/2);
              y1=y1-((h2-h)/2);
              y2=y2+((h2-h)/2);
            }
            int color=sdci[6]/10000;

            if(sdci[12]/10000<=127) //translucent
            {
              drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
            }
            if(sdci[10]==0) //no rotation
            {
              if(sdci[11]) //filled
              {
                rectfill(bmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
              }
              else //outline
              {
                rect(bmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
              }
            }
            else  //rotate
            {
              int xy[16];
              int rx=sdci[8]/10000;
              int ry=sdci[9]/10000;
              fixed ra1=itofix(sdci[10]%10000)/10000;
              fixed ra2=itofix(sdci[10]/10000);
              fixed ra=ra1+ra2;
              ra = (ra/360)*256;

              xy[ 0]=xoffset+rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
              xy[ 1]=yoffset+ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
              xy[ 2]=xoffset+rx + fixtoi( ( fixcos(ra) * (x2 - rx) - fixsin(ra) * (y1 - ry) ) ); //x2
              xy[ 3]=yoffset+ry + fixtoi( ( fixsin(ra) * (x2 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
              xy[ 4]=xoffset+rx + fixtoi( ( fixcos(ra) * (x2 - rx) - fixsin(ra) * (y2 - ry) ) ); //x2
              xy[ 5]=yoffset+ry + fixtoi( ( fixsin(ra) * (x2 - rx) + fixcos(ra) * (y2 - ry) ) ); //y2
              xy[ 6]=xoffset+rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y2 - ry) ) ); //x1
              xy[ 7]=yoffset+ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y2 - ry) ) ); //y2
              xy[ 8]=xoffset+rx + fixtoi( ( fixcos(ra) * (x1 - rx    ) - fixsin(ra) * (y1 - ry + 1) ) ); //x1
              xy[ 9]=yoffset+ry + fixtoi( ( fixsin(ra) * (x1 - rx    ) + fixcos(ra) * (y1 - ry + 1) ) ); //y1
              xy[10]=xoffset+rx + fixtoi( ( fixcos(ra) * (x2 - rx - 1) - fixsin(ra) * (y1 - ry    ) ) ); //x2
              xy[11]=yoffset+ry + fixtoi( ( fixsin(ra) * (x2 - rx - 1) + fixcos(ra) * (y1 - ry    ) ) ); //y1
              xy[12]=xoffset+rx + fixtoi( ( fixcos(ra) * (x2 - rx    ) - fixsin(ra) * (y2 - ry - 1) ) ); //x2
              xy[13]=yoffset+ry + fixtoi( ( fixsin(ra) * (x2 - rx    ) + fixcos(ra) * (y2 - ry - 1) ) ); //y2
              xy[14]=xoffset+rx + fixtoi( ( fixcos(ra) * (x1 - rx + 1) - fixsin(ra) * (y2 - ry    ) ) ); //x1
              xy[15]=yoffset+ry + fixtoi( ( fixsin(ra) * (x1 - rx + 1) + fixcos(ra) * (y2 - ry    ) ) ); //y2
              if(sdci[11]) //filled
              {
                polygon(bmp, 4, xy, color);
              }
              else //outline
              {
                line(bmp, xy[0], xy[1], xy[10], xy[11], color);
                line(bmp, xy[2], xy[3], xy[12], xy[13], color);
                line(bmp, xy[4], xy[5], xy[14], xy[15], color);
                line(bmp, xy[6], xy[7], xy[ 8], xy[ 9], color);
              }
            }
            drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
          }
          break;
        case CIRCLER:
          {
            //sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=radius
            //sdci[5]=color
            //sdci[6]=scale factor
            //sdci[7]=rotation anchor x
            //sdci[8]=rotation anchor y
            //sdci[9]=rotation angle
            //sdci[10]=fill
            //sdci[11]=opacity
            if (sdci[6]==0) //scale
            {
              break;
            }
            int x1=sdci[2]/10000;
            int y1=sdci[3]/10000;
            qword r=sdci[4];
            if (sdci[6] != 10000)
            {
              r*=sdci[6];
              r/=10000;
            }
            r/=10000;
            int color=sdci[5]/10000;

            if(sdci[11]/10000<=127) //translucent
            {
              drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
            }
            if(sdci[9]!=0&&(sdci[2]!=sdci[7]||sdci[3]!=sdci[8])) //rotation
            {
              int xy[2];
              int rx=sdci[7]/10000;
              int ry=sdci[8]/10000;
              fixed ra1=itofix(sdci[9]%10000)/10000;
              fixed ra2=itofix(sdci[9]/10000);
              fixed ra=ra1+ra2;
              ra = (ra/360)*256;

              xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
              xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
              x1=xy[0];
              y1=xy[1];
            }
            if(sdci[10]) //filled
            {
              circlefill(bmp, x1+xoffset, y1+yoffset, r, color);
            }
            else //outline
            {
              circle(bmp, x1+xoffset, y1+yoffset, r, color);
            }
            drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
          }
          break;
        case ARCR:
          {
            //sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=radius
            //sdci[5]=start angle
            //sdci[6]=end angle
            //sdci[7]=color
            //sdci[8]=scale factor
            //sdci[9]=rotation anchor x
            //sdci[10]=rotation anchor y
            //sdci[11]=rotation angle
            //sdci[12]=closed
            //sdci[13]=fill
            //sdci[14]=opacity

            if (sdci[8]==0) //scale
            {
              break;
            }
            int cx=sdci[2]/10000;
            int cy=sdci[3]/10000;
            qword r=sdci[4];
            if (sdci[8] != 10000)
            {
              r*=sdci[8];
              r/=10000;
            }
            r/=10000;

            int color=sdci[7]/10000;

            fixed ra1=itofix(sdci[11]%10000)/10000;
            fixed ra2=itofix(sdci[11]/10000);
            fixed ra=ra1+ra2;
            ra = (ra/360)*256;


            fixed a1=itofix(sdci[5]%10000)/10000;
            fixed a2=itofix(sdci[5]/10000);
            fixed sa=a1+a2;
            sa = (sa/360)*256;

            a1=itofix(sdci[6]%10000)/10000;
            a2=itofix(sdci[6]/10000);
            fixed ea=a1+a2;
            ea = (ea/360)*256;
            if(sdci[11]!=0) //rotation
            {
              int rx=sdci[9]/10000;
              int ry=sdci[10]/10000;

              cx=rx + fixtoi( ( fixcos(ra) * (cx - rx) - fixsin(ra) * (cy - ry) ) ); //x1
              cy=ry + fixtoi( ( fixsin(ra) * (cx - rx) + fixcos(ra) * (cy - ry) ) ); //y1
              ea-=ra;
              sa-=ra;
            }

            int fx=cx+fixtoi(fixcos(-(ea+sa)/2)*r/2);
            int fy=cy+fixtoi(fixsin(-(ea+sa)/2)*r/2);

            if(sdci[12]) //closed
            {
              if(sdci[13]) //filled
              {
			    clear_bitmap(prim_bmp);
                arc(prim_bmp, cx+xoffset, cy+yoffset, sa, ea, int(r), color);
                line(prim_bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-sa)*r), cy+yoffset+fixtoi(fixsin(-sa)*r), color);
                line(prim_bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-ea)*r), cy+yoffset+fixtoi(fixsin(-ea)*r), color);
                floodfill(prim_bmp, zc_max(0,fx)+xoffset, zc_max(0,fy)+yoffset, color);
                if(sdci[14]/10000<=127) //translucent
                {
                  draw_trans_sprite(bmp, prim_bmp, 0,0);
                }
                else
                {
                  draw_sprite(bmp, prim_bmp, 0,0);
                }
              }
              else
              {
                arc(bmp, cx+xoffset, cy+yoffset, sa, ea, int(r), color);
                line(bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-sa)*r), cy+yoffset+fixtoi(fixsin(-sa)*r), color);
                line(bmp, cx+xoffset, cy+yoffset, cx+xoffset+fixtoi(fixcos(-ea)*r), cy+yoffset+fixtoi(fixsin(-ea)*r), color);
              }
            }
            else
            {
              if(sdci[14]/10000<=127) //translucent
              {
                drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
              }
              arc(bmp, cx+xoffset, cy+yoffset, sa, ea, int(r), color);
              drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
            }
          }
          break;
        case ELLIPSER:
          {
            //sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=radiusx
            //sdci[5]=radiusy
            //sdci[6]=color
            //sdci[7]=scale factor
            //sdci[8]=rotation anchor x
            //sdci[9]=rotation anchor y
            //sdci[10]=rotation angle
            //sdci[11]=fill
            //sdci[12]=opacity

            if (sdci[7]==0) //scale
            {
              break;
            }

            int x1=sdci[2]/10000;
            int y1=sdci[3]/10000;
            int radx=sdci[4]/10000;
                  radx*=sdci[7]/10000;
            int rady=sdci[5]/10000;
                  rady*=sdci[7]/10000;
            int color=sdci[6]/10000;
			float rotation = sdci[10]/10000;

            int rx=sdci[8]/10000;
            int ry=sdci[9]/10000;
            fixed ra1=itofix(sdci[10]%10000)/10000;
            fixed ra2=itofix(sdci[10]/10000);
            fixed ra=ra1+ra2;
            ra = (ra/360)*256;

			int xy[2];
            xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
            xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
            x1=xy[0];
            y1=xy[1];

			if(radx<1||rady<1||radx>255||rady>255) break; //why are we even trying to draw an ellipse then?

			BITMAP* bitty = create_bitmap_ex(8, radx*2+1, rady*2+1);
			clear_bitmap(bitty);

            if(sdci[11]) //filled
            {

              if(sdci[12]/10000<128) //translucent
              {
				  clear_bitmap(prim_bmp);
				  ellipsefill(bitty, radx, rady, radx, rady, color);
				  rotate_sprite(prim_bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, ftofix(DegtoFix(rotation)));
				  draw_trans_sprite(bmp, prim_bmp, 0, 0);
              }
              else // no opacity
              {
				  ellipsefill(bitty, radx, rady, radx, rady, color);
				  rotate_sprite(bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, ftofix(DegtoFix(rotation)));
              }
            }
            else //not filled
            {
              if(sdci[12]/10000<128) //translucent
              {
				  clear_bitmap(prim_bmp);
				  ellipse(bitty, radx, rady, radx, rady, color);
				  rotate_sprite(prim_bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, ftofix(DegtoFix(rotation)));
				  draw_trans_sprite(bmp, prim_bmp, 0, 0);
              }
              else // no opacity
              {
				  ellipse(bitty, radx, rady, radx, rady, color);
				  rotate_sprite(bmp, bitty, x1+xoffset-radx,y1+yoffset-rady, ftofix(DegtoFix(rotation)));
              }
            }

			destroy_bitmap(bitty);
          }
          break;
        case LINER:
          {
            //sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=x2
            //sdci[5]=y2
            //sdci[6]=color
            //sdci[7]=scale factor
            //sdci[8]=rotation anchor x
            //sdci[9]=rotation anchor y
            //sdci[10]=rotation angle
            //sdci[11]=opacity
            if (sdci[7]==0) //scale
            {
              break;
            }
            int x1=sdci[2]/10000;
            int y1=sdci[3]/10000;
            int x2=sdci[4]/10000;
            int y2=sdci[5]/10000;

            if (sdci[7] != 10000)
            {
              int w=x2-x1+1;
              int h=y2-y1+1;
              int w2=int(w*((double)sdci[7]/10000.0));
              int h2=int(h*((double)sdci[7]/10000.0));
              x1=x1-((w2-w)/2);
              x2=x2+((w2-w)/2);
              y1=y1-((h2-h)/2);
              y2=y2+((h2-h)/2);
            }
            int color=sdci[6]/10000;

            if(sdci[11]/10000<=127) //translucent
            {
              drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
            }
            if(sdci[10]!=0) //rotation
            {
              int xy[4];
              int rx=sdci[8]/10000;
              int ry=sdci[9]/10000;
              fixed ra1=itofix(sdci[10]%10000)/10000;
              fixed ra2=itofix(sdci[10]/10000);
              fixed ra=ra1+ra2;

              xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
              xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
              xy[ 2]=rx + fixtoi( ( fixcos(ra) * (x2 - rx) - fixsin(ra) * (y2 - ry) ) ); //x2
              xy[ 3]=ry + fixtoi( ( fixsin(ra) * (x2 - rx) + fixcos(ra) * (y2 - ry) ) ); //y2
              x1=xy[0];
              y1=xy[1];
              x2=xy[2];
              y2=xy[3];
            }
            line(bmp, x1+xoffset, y1+yoffset, x2+xoffset, y2+yoffset, color);
            drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
          }
          break;

	    case SPLINER:
          {
			/* layer, x1, y1, x2, y2, x3, y3, x4, y4, color, opacity */

			  int points[8] = {	  xoffset + (sdci[2]/10000), yoffset + (sdci[3]/10000),
								  xoffset + (sdci[4]/10000), yoffset + (sdci[5]/10000),
								  xoffset + (sdci[6]/10000), yoffset + (sdci[7]/10000),
								  xoffset + (sdci[8]/10000), yoffset + (sdci[9]/10000)
							  };
			  if( sdci[11]/10000 < 128 ) //translucent
			  {
				  drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
			  }
			  spline( bmp, points, sdci[10]/10000 );

			  drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
		  }
		  break;

        case PUTPIXELR:
          {
            //sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=color
            //sdci[5]=rotation anchor x
            //sdci[6]=rotation anchor y
            //sdci[7]=rotation angle
            //sdci[8]=opacity
            int x1=sdci[2]/10000;
            int y1=sdci[3]/10000;
            int color=sdci[4]/10000;

            if(sdci[8]/10000<=127) //translucent
            {
              drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
            }
            if(sdci[7]!=0) //rotation
            {
              int xy[2];
              int rx=sdci[5]/10000;
              int ry=sdci[6]/10000;
              fixed ra1=itofix(sdci[7]%10000)/10000;
              fixed ra2=itofix(sdci[7]/10000);
              fixed ra=ra1+ra2;

              xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
              xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
              x1=xy[0];
              y1=xy[1];
            }
            putpixel(bmp, x1+xoffset, y1+yoffset, color);
            drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
          }
          break;
        case DRAWTILER:
          {
            //sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=tile
            //sdci[5]=tile width
            //sdci[6]=tile height
            //sdci[7]=color (cset)
            //sdci[8]=scale x
		    //sdci[9]=scale y
            //sdci[10]=rotation anchor x
            //sdci[11]=rotation anchor y
            //sdci[12]=rotation angle
            //sdci[13]=flip
            //sdci[14]=transparency
            //sdci[15]=opacity

            int w = sdci[5]/10000;
            int h = sdci[6]/10000;
            if(w<1||h<1||h>20||w>20)
            {
              break;
            }

            int xscale=sdci[8]/10000;
			int yscale=sdci[9]/10000;
            int rx = sdci[10]/10000;
            int ry = sdci[11]/10000;
            float rotation=sdci[12]/10000;
			int flip=(sdci[13]/10000)&3;
            bool transparency=sdci[14]!=0;
            int opacity=sdci[15]/10000;
            int color=sdci[7]/10000;

            int x1=sdci[2]/10000;
            int y1=sdci[3]/10000;

			//don't scale if it's not safe to do so
			bool canscale = true;
			if (xscale==0||yscale==0)
            {
				break;
            }
			if (xscale<0||yscale<0)
				canscale = false; //default size

            if ( (xscale>0 && yscale>0) || rotation) //scaled or rotated
			{
			  BITMAP* pbitty = create_bitmap_ex(8, w*16, h*16);
			  clear_bitmap(pbitty);

              if(transparency) //transparency
			  {
				  Tile.OverTile( pbitty, (sdci[4]/10000), 0, 0, w, h, color, flip );
              }
              else //no transparency
              {
				  Tile.OldPutTile( pbitty, (sdci[4]/10000), 0, 0, w, h, color, flip );
			  }

			  if(rotation != 0)
			  {
				  //low negative values indicate no anchor-point rotation
				  if(rx>-777||ry>-777)
				  {
					  int xy[2];
					  fixed ra1=itofix(sdci[12]%10000)/10000;
					  fixed ra2=itofix(sdci[12]/10000);
					  fixed ra=ra1+ra2;
					  xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
					  xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
					  x1=xy[0];
					  y1=xy[1];
				  }

				  if(canscale) //scale first
				  {
					  BITMAP* tempbit = create_bitmap_ex(8, xscale>512?512:xscale, yscale>512?512:yscale);
					  clear_bitmap(tempbit);

					  stretch_sprite(tempbit, pbitty, 0, 0, xscale, yscale);

					  if(opacity < 128)
					  {
						  clear_bitmap(prim_bmp);
						  rotate_sprite(prim_bmp, tempbit, 0, 0, ftofix(DegtoFix(rotation)) );
						  draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
					  }
					  else
					  {
						  rotate_sprite(bmp, tempbit, x1+xoffset, y1+yoffset, ftofix(DegtoFix(rotation)) );
					  }

					  destroy_bitmap(tempbit);
				  }
				  else //no scale
				  {
					  if(opacity < 128)
					  {
						  clear_bitmap(prim_bmp);
						  rotate_sprite(prim_bmp, pbitty, 0, 0, ftofix(DegtoFix(rotation)) );
						  draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
					  }
					  else
					  {
						  rotate_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, ftofix(DegtoFix(rotation)) );
					  }
				  }
			  }
			  else //scale only
			  {
				  if(canscale)
				  {
					  if(opacity<128)
					  {
						  clear_bitmap(prim_bmp);
						  stretch_sprite(prim_bmp, pbitty, 0, 0, xscale, yscale);
						  draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
					  }
					  else
					  {
						  stretch_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, xscale, yscale);
					  }
				  }
				  else //error -do not scale
				  {
					  if(opacity<128)
					  {
						  draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
					  }
					  else
					  {
						  draw_sprite(bmp, pbitty, x1+xoffset, y1+yoffset);
					  }
				  }
			  }

			  destroy_bitmap(pbitty); //rap sucks

            }
            else // no scale or rotation
			{
				if(transparency)
				{
				if(opacity<=127)
					Tile.OverTileTranslucent( bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip, opacity );
				else
					Tile.OverTile( bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip );
				}
				else
				{
				if(opacity<=127)
					Tile.PutTileTranslucent( bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip, opacity );
				else
					Tile.OldPutTile( bmp, (sdci[4]/10000), xoffset+x1, yoffset+y1, w, h, color, flip );
				}
            }
		}
        break;

        case DRAWCOMBOR:
        {
			//break;
			//sdci[1]=layer
			//sdci[2]=x
			//sdci[3]=y
			//sdci[4]=combo
			//sdci[5]=tile width
			//sdci[6]=tile height
			//sdci[7]=color (cset)
			//sdci[8]=scale x
			//sdci[9]=scale y
			//sdci[10]=rotation anchor x
			//sdci[11]=rotation anchor y
			//sdci[12]=rotation angle
			//sdci[13]=frame
			//sdci[14]=flip
			//sdci[15]=transparency
			//sdci[16]=opacity

			int w = sdci[5]/10000;
			int h = sdci[6]/10000;
			if(w<1||h<1||h>20||w>20)
			{
			  break;
			}
			int flip=(sdci[14]/10000)&3;

			int xscale=sdci[8]/10000;
			int yscale=sdci[9]/10000;
			int rx = sdci[10]/10000; //these work now
			int ry = sdci[11]/10000; //these work now
			float rotation=sdci[12]/10000;

			bool transparency=sdci[15]!=0;
			int opacity=sdci[16]/10000;
			int color=sdci[7]/10000;
			int x1=sdci[2]/10000;
			int y1=sdci[3]/10000;
			int tiletodraw = combo_tile(sdci[4]/10000, x1, y1);

			//don't scale if it's not safe to do so
			bool canscale = true;
			if (xscale==0||yscale==0)
			{
				break;
			}
			if (xscale<0||yscale<0)
				canscale = false; //default size

			if ( (xscale>0 && yscale>0) || rotation) //scaled or rotated
			{
			  BITMAP* pbitty = create_bitmap_ex(8, w*16, h*16); //-pbitty in the hisouse. :D
			  clear_bitmap(pbitty);

			  if(transparency)
			  {
				  Tile.OverTile( pbitty, tiletodraw, 0, 0, w, h, color, flip );
			  }
			  else //no transparency
			  {
				  Tile.OldPutTile( pbitty, tiletodraw, 0, 0, w, h, color, flip );
			  }
			  if(rotation != 0) // rotate
			  {
				  //fixed point sucks ;0
				  if(rx>-777||ry>-777) //set the rotation anchor and rotate around that
				  {
					  int xy[2];
					  fixed ra1=itofix(sdci[12]%10000)/10000;
					  fixed ra2=itofix(sdci[12]/10000);
					  fixed ra=ra1+ra2;
					  xy[ 0]=rx + fixtoi( ( fixcos(ra) * (x1 - rx) - fixsin(ra) * (y1 - ry) ) ); //x1
					  xy[ 1]=ry + fixtoi( ( fixsin(ra) * (x1 - rx) + fixcos(ra) * (y1 - ry) ) ); //y1
					  x1=xy[0];
					  y1=xy[1];
				  }

				  if(canscale) //scale first
				  {
					  BITMAP* tempbit = create_bitmap_ex(8, xscale>512?512:xscale, yscale>512?512:yscale);
					  clear_bitmap(tempbit);

					  stretch_sprite(tempbit, pbitty, 0, 0, xscale, yscale);

					  if(opacity < 128)
					  {
						  clear_bitmap(prim_bmp);
						  rotate_sprite(prim_bmp, tempbit, 0, 0, ftofix(DegtoFix(rotation)) );
						  draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
					  }
					  else
					  {
						  rotate_sprite(bmp, tempbit, x1+xoffset, y1+yoffset, ftofix(DegtoFix(rotation)) );
					  }

					  destroy_bitmap(tempbit);
				  }
				  else //no scale
				  {
					  if(opacity < 128)
					  {
						  clear_bitmap(prim_bmp);
						  rotate_sprite(prim_bmp, pbitty, 0, 0, ftofix(DegtoFix(rotation)) );
						  draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
					  }
					  else
					  {
						  rotate_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, ftofix(DegtoFix(rotation)) );
					  }
				  }
			  }
			  else //scale only
			  {
				  if(canscale)
				  {
					  if(opacity<128)
					  {
						  clear_bitmap(prim_bmp);
						  stretch_sprite(prim_bmp, pbitty, 0, 0, xscale, yscale);
						  draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
					  }
					  else
					  {
						  stretch_sprite(bmp, pbitty, x1+xoffset, y1+yoffset, xscale, yscale);
					  }
				  }
				  else //error -do not scale
				  {
					  if(opacity<128)
					  {
						  draw_trans_sprite(bmp, prim_bmp, x1+xoffset, y1+yoffset);
					  }
					  else
					  {
						  draw_sprite(bmp, pbitty, x1+xoffset, y1+yoffset);
					  }
				  }
			  }

			  destroy_bitmap(pbitty); //rap sucks
			}
            else // no scale or rotation
			{
				if(transparency)
				{
					if(opacity<=127)
						Tile.OverTileTranslucent( bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, opacity );
					else
						Tile.OverTile( bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip );
				}
				else
				{
					if(opacity<=127)
						Tile.PutTileTranslucent( bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip, opacity );
					else
						Tile.OldPutTile( bmp, tiletodraw, xoffset+x1, yoffset+y1, w, h, color, flip);
				}
			}
		}
        break;

	  case FASTTILER:
		{
			/* layer, x, y, tile, color opacity */

			int opacity = sdci[6]/10000;

			if( opacity < 128 )
				overtiletranslucent16(bmp, sdci[4]/10000, xoffset+(sdci[2]/10000), yoffset+(sdci[3]/10000), sdci[5]/10000, 0, opacity);
			else
				overtile16(bmp, sdci[4]/10000, xoffset+(sdci[2]/10000), yoffset+(sdci[3]/10000), sdci[5]/10000, 0);
		}
		break;

	  case FASTCOMBOR:
		{
			/* layer, x, y, tile, color opacity */

			int opacity = sdci[6]/10000;
			int x1 = sdci[2]/10000;
			int y1 = sdci[3]/10000;

			if( opacity < 128 )
				overtiletranslucent16(bmp, combo_tile(sdci[4]/10000, x1, y1), xoffset+x1, yoffset+y1, sdci[5]/10000, 0, opacity);
			else
				overtile16(bmp, combo_tile(sdci[4]/10000, x1, y1), xoffset+x1, yoffset+y1, sdci[5]/10000, 0);
		}
		break;

        case DRAWCHARR:
		  {
			//sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=font
            //sdci[5]=color
            //sdci[6]=bg color
			//sdci[7]=strech x (width)
			//sdci[8]=stretch y (height)
            //sdci[9]=char
            //sdci[10]=opacity

			int x=sdci[2]/10000;
			int y=sdci[3]/10000;
			int font_index=sdci[4]/10000;
			int color=sdci[5]/10000;
			int bg_color=sdci[6]/10000; //-1 = transparent
			int w=sdci[7]/10000;
			int h=sdci[8]/10000;
			char glyph=char(sdci[9]/10000);
			int opacity=sdci[10]/10000;

			//safe check
			if(bg_color < -1) bg_color = -1;
			if(w>512) w=512; //w=vbound(w,0,512);
			if(h>512) h=512; //h=vbound(h,0,512);
			//undone
			if(w>0&&h>0)//stretch the character
			{
				BITMAP *pbmp = create_bitmap_ex(8,16,16);
				clear_bitmap(pbmp);

				if(opacity < 128)
				{
					if(w>128||h>128)
					{
						clear_bitmap(prim_bmp);

						textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
						stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
						draw_trans_sprite(bmp, prim_bmp, x+xoffset, y+yoffset);
					}
					else //this is faster
					{
						BITMAP *pbmp2 = create_bitmap_ex(8,w,h);
						clear_bitmap(pbmp2);

						textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
						stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
						draw_trans_sprite(bmp, pbmp2, x+xoffset, y+yoffset);

						destroy_bitmap(pbmp2);
					}
				}
				else // no opacity
				{
					textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
					stretch_sprite(bmp, pbmp, x+xoffset, y+yoffset, w, h);
				}

				destroy_bitmap(pbmp);
			}
			else //no stretch
			{
				if(opacity < 128)
				{
					BITMAP *pbmp = create_bitmap_ex(8,16,16);
					clear_bitmap(pbmp);

					textprintf_ex(pbmp, get_zc_font(font_index), 0, 0, color, bg_color, "%c", glyph);
					draw_trans_sprite(bmp, pbmp, x+xoffset, y+yoffset);

					destroy_bitmap(pbmp);
				}
				else // no opacity
				{
					textprintf_ex(bmp, get_zc_font(font_index), x+xoffset, y+yoffset, color, bg_color, "%c", glyph);
				}
			}
		}
        break;

        case DRAWINTR:
		  {
			//sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=font
            //sdci[5]=color
            //sdci[6]=bg color
			//sdci[7]=strech x (width)
			//sdci[8]=stretch y (height)
            //sdci[9]=integer
			//sdci[10]=num decimal places
            //sdci[11]=opacity

			int x=sdci[2]/10000;
			int y=sdci[3]/10000;
			int font_index=sdci[4]/10000;
			int color=sdci[5]/10000;
			int bg_color=sdci[6]/10000; //-1 = transparent
			int w=sdci[7]/10000;
			int h=sdci[8]/10000;
			float number =(sdci[9]/10000);
			int decplace = sdci[10]/10000;
			int opacity=sdci[11]/10000;

			//safe check
			if(bg_color < -1) bg_color = -1;
			if(w>512) w=512; //w=vbound(w,0,512);
			if(h>512) h=512; //h=vbound(h,0,512);

			char numbuf[15];
			switch(decplace)
			{
				default:
				case 0:	sprintf(numbuf,"%d",int(number)); break;
				case 1: sprintf(numbuf,"%.01f",number); break;
				case 2:	sprintf(numbuf,"%.02f",number); break;
				case 3: sprintf(numbuf,"%.03f",number); break;
				case 4: sprintf(numbuf,"%.04f",number); break;
			}

			if(w>0&&h>0)//stretch
			{
				BITMAP *pbmp = create_bitmap_ex(8,16,16);
				clear_bitmap(pbmp);

				if(opacity < 128)
				{
					if(w>128||h>128)
					{
						clear_bitmap(prim_bmp);

						textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
						stretch_sprite(prim_bmp, pbmp, 0, 0, w, h);
						draw_trans_sprite(bmp, prim_bmp, x+xoffset, y+yoffset);
					}
					else
					{
						BITMAP *pbmp2 = create_bitmap_ex(8,w,h);
						clear_bitmap(pbmp2);

						textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
						stretch_sprite(pbmp2, pbmp, 0, 0, w, h);
						draw_trans_sprite(bmp, pbmp2, x+xoffset, y+yoffset);

						destroy_bitmap(pbmp2);
					}
				}
				else // no opacity
				{
					textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
					stretch_sprite(bmp, pbmp, x+xoffset, y+yoffset, w, h);
				}

				destroy_bitmap(pbmp);
			}
			else //no stretch
			{
				if(opacity < 128)
				{
					BITMAP *pbmp = create_bitmap_ex(8,16,16);
					clear_bitmap(pbmp);

					textout_ex(pbmp, get_zc_font(font_index), numbuf, 0, 0, color, bg_color);
					draw_trans_sprite(bmp, pbmp, x+xoffset, y+yoffset);

					destroy_bitmap(pbmp);
				}
				else // no opacity
				{
					textout_ex(bmp, get_zc_font(font_index), numbuf, x+xoffset, y+yoffset, color, bg_color);
				}
			}
		}
        break;

        case DRAWSTRINGR:
		  {
			//sdci[1]=layer
            //sdci[2]=x
            //sdci[3]=y
            //sdci[4]=font
            //sdci[5]=color
            //sdci[6]=bg color
			//sdci[7]=format_option
			//sdci[8]=opacity
            //sdci[9]=char
			if( draw_container.drawstring.empty() )
			{
				al_trace( "String Container is empty! Could not complete DrawString. \n");
				break;
			}

			std::list< std::pair <int, std::string> > ::iterator ds_it
				= draw_container.getDrawStringIterator( sdci[19] );

			if( ds_it == draw_container.drawstring.end() )
				break; //I don't know why this would happen.....

			int x=sdci[2]/10000;
			int y=sdci[3]/10000;
			int font_index=sdci[4]/10000;
			int color=sdci[5]/10000;
			int bg_color=sdci[6]/10000; //-1 = transparent
			int format_type=sdci[7]/10000;
			int opacity=sdci[9]/10000;
			//sdci[8] not needed :)

			//safe check
			if(bg_color < -1) bg_color = -1;

			if(opacity < 128)
				drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);

			if( format_type == 2 ) // right-sided text
			{
				textout_right_ex( bmp, get_zc_font(font_index), (*ds_it).second.c_str(), x+xoffset, y+yoffset, color, bg_color );
			}
			else if( format_type == 1 ) // centered text
			{
				textout_centre_ex( bmp, get_zc_font(font_index), (*ds_it).second.c_str(), x+xoffset, y+yoffset, color, bg_color );
			}
			else // standard left-sided text
			{
				textout_ex( bmp, get_zc_font(font_index), (*ds_it).second.c_str(), x+xoffset, y+yoffset, color, bg_color );
			}

			if(opacity < 128)
				drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);

			ds_it = draw_container.drawstring.erase( ds_it );
		  }
          break;

		case QUADR:
		{
			//sdci[1]=layer
			//sdci[2]=x1
			//sdci[3]=y1
			//sdci[4]=x2
			//sdci[5]=y2
			//sdci[6]=x3
			//sdci[7]=y3
			//sdci[8]=x4
			//sdci[9]=y4
			//sdci[10]=width
			//sdci[11]=height
			//sdci[12]=cset
			//sdci[13]=flip
			//sdci[14]=tile/combo
			//sdci[15]=polytype

			int x1 = sdci[2]/10000;
			int y1 = sdci[3]/10000;
			int x2 = sdci[4]/10000;
			int y2 = sdci[5]/10000;
			int x3 = sdci[6]/10000;
			int y3 = sdci[7]/10000;
			int x4 = sdci[8]/10000;
			int y4 = sdci[9]/10000;
			int w = sdci[10]/10000;
			int h = sdci[11]/10000;
			int color = sdci[12]/10000;
			int flip=(sdci[13]/10000)&3;
			int tile = sdci[14]/10000;
			int polytype = sdci[15]/10000;

//todo: finish palette shading
/*
POLYTYPE_FLAT
POLYTYPE_GCOL
POLYTYPE_GRGB
POLYTYPE_ATEX
POLYTYPE_PTEX
POLYTYPE_ATEX_MASK
POLYTYPE_PTEX_MASK
POLYTYPE_ATEX_LIT
POLYTYPE_PTEX_LIT
POLYTYPE_ATEX_MASK_LIT
POLYTYPE_PTEX_MASK_LIT
POLYTYPE_ATEX_TRANS
POLYTYPE_PTEX_TRANS
POLYTYPE_ATEX_MASK_TRANS
POLYTYPE_PTEX_MASK_TRANS
*/
			polytype = vbound(polytype, 0, 14);
			if( ((w-1) & w) != 0 || ((h-1) & h) != 0 ) break; //non power of two error

			BITMAP *tex = create_bitmap_ex(8,16*w,16*h);
			clear_bitmap(tex);

			int col[4];
			if( color < 0 )
			{
				col[0]=draw_container.color_buffer[0];
				col[1]=draw_container.color_buffer[1];
				col[2]=draw_container.color_buffer[2];
				col[3]=draw_container.color_buffer[3];
			}
			else
			{
				col[0]=col[1]=col[2]=col[3]=color;
			}

			if(tile > 0)	// TILE
			{
				Tile.OverTile( tex, tile, 0, 0, w, h, color, flip );
			}
			else			// COMBO
			{
				const int tiletodraw = combo_tile( abs(tile), x1, y1 );
				Tile.OldPutTile( tex, tiletodraw, 0, 0, w, h, color, flip );
			}

			int tex_width = w*16;
			int tex_height = h*16;

			V3D_f V1 = { x1+xoffset, y1+yoffset, draw_container.depth_buffer[0], 		 0,			 0, col[0] };
			V3D_f V2 = { x2+xoffset, y2+yoffset, draw_container.depth_buffer[1], 		 0, tex_height, col[1] };
			V3D_f V3 = { x3+xoffset, y3+yoffset, draw_container.depth_buffer[2], tex_width, tex_height, col[2] };
			V3D_f V4 = { x4+xoffset, y4+yoffset, draw_container.depth_buffer[3], tex_width,			 0, col[3] };

			quad3d_f(bmp, polytype, tex, &V1, &V2, &V3, &V4);
			destroy_bitmap(tex);
		}
		break;

		case QUAD3DR:
		{
			//sdci[1]=layer
			//sdci[2]=pos[12]
			//sdci[3]=uv[8]
			//sdci[4]=color[4]
			//sdci[5]=size[2]
			//sdci[6]=tile/combo
			//sdci[7]=polytype

			if( draw_container.quad3D.empty() )
			{
				return;
			}

			std::deque< quad3Dstruct > ::iterator q3d_it
			= draw_container.getQuad3dIterator( sdci[19] );

			// dereferenced, for her pleasure. :p
			const quad3Dstruct *q = & (*q3d_it);

			int w = q->size[0];
			int h = q->size[1];
			int flip = (sdci[6]/10000)&3;
			int tile = sdci[7]/10000;
			int polytype = sdci[8]/10000;

			polytype = vbound(polytype, 0, 14);
			if( !(zc::math::IsPowerOfTwo(w) && zc::math::IsPowerOfTwo(h)) )
			{
				draw_container.quad3D.erase( q3d_it );
				break;
			}

			BITMAP *tex = create_bitmap_ex(8,16*w,16*h);
			clear_bitmap(tex);

			if(tile > 0)	// TILE
			{
				Tile.OverTile( tex, tile, 0, 0, w, h, q->color[0], flip );
			}
			else			// COMBO
			{
				const int tiletodraw = combo_tile( abs(tile), int(q->pos[0]), int(q->pos[1]) );
				Tile.OldPutTile( tex, tiletodraw, 0, 0, w, h, q->color[0], flip );
			}

			V3D_f V1 = { q->pos[0]+xoffset, q->pos[1] +yoffset, q->pos[2] ,	q->uv[0], q->uv[1], q->color[0] };
			V3D_f V2 = { q->pos[3]+xoffset, q->pos[4] +yoffset, q->pos[5] ,	q->uv[2], q->uv[3], q->color[1] };
			V3D_f V3 = { q->pos[6]+xoffset, q->pos[7] +yoffset, q->pos[8] , q->uv[4], q->uv[5], q->color[2] };
			V3D_f V4 = { q->pos[9]+xoffset, q->pos[10]+yoffset, q->pos[11], q->uv[6], q->uv[7], q->color[3] };

			quad3d_f(bmp, polytype, tex, &V1, &V2, &V3, &V4);
			destroy_bitmap(tex);

			draw_container.quad3D.erase( q3d_it );
		}
		break;

		case TRIANGLER:
		{
			//sdci[1]=layer
			//sdci[2]=x1
			//sdci[3]=y1
			//sdci[4]=x2
			//sdci[5]=y2
			//sdci[6]=x3
			//sdci[7]=y3
			//sdci[8]=width
			//sdci[9]=height
			//sdci[10]=cset
			//sdci[11]=flip
			//sdci[12]=tile/combo
			//sdci[13]=polytype

			int x1 = sdci[2]/10000;
			int y1 = sdci[3]/10000;
			int x2 = sdci[4]/10000;
			int y2 = sdci[5]/10000;
			int x3 = sdci[6]/10000;
			int y3 = sdci[7]/10000;
			int w = sdci[8]/10000;
			int h = sdci[9]/10000;
			int color = sdci[10]/10000;
			int flip=(sdci[11]/10000)&3;
			int tile = sdci[12]/10000;
			int polytype = sdci[13]/10000;

			polytype = vbound(polytype, 0, 14);
			if( ((w-1) & w) != 0 || ((h-1) & h) != 0 ) break; //non power of two error

			BITMAP *tex = create_bitmap_ex(8,16*w,16*h);
			clear_bitmap(tex);

			int col[3];
			if( color < 0 )
			{
				col[0]=draw_container.color_buffer[0];
				col[1]=draw_container.color_buffer[1];
				col[2]=draw_container.color_buffer[2];
			}
			else
			{
				col[0]=col[1]=col[2]=color;
			}

			if(tile > 0)	// TILE
			{
				Tile.OverTile( tex, tile, 0, 0, w, h, color, flip );
			}
			else			// COMBO
			{
				const int tiletodraw = combo_tile( abs(tile), x1, y1 );
				Tile.OldPutTile( tex, tiletodraw, 0, 0, w, h, color, flip );
			}

			int tex_width = w*16;
			int tex_height = h*16;

			V3D_f V1 = { x1+xoffset, y1+yoffset, draw_container.depth_buffer[0],		0 ,			 0 , col[0] };
			V3D_f V2 = { x2+xoffset, y2+yoffset, draw_container.depth_buffer[1],		0 , tex_height , col[1] };
			V3D_f V3 = { x3+xoffset, y3+yoffset, draw_container.depth_buffer[2], tex_width, tex_height , col[2] };


			triangle3d_f(bmp, polytype, tex, &V1, &V2, &V3);
			destroy_bitmap(tex);
		}
		break;

		case TRIANGLE3DR:
		{
			//sdci[1]=layer
			//sdci[6]=flip
			//sdci[7]=tile/combo
			//sdci[8]=polytype

			if( draw_container.triangle3D.empty() )
			{
				return;
			}
			std::deque< triangle3Dstruct > ::iterator t3d_it
				= draw_container.getTriangle3dIterator( sdci[19] );

			// dereferenced, for her pleasure. :p
			const triangle3Dstruct *q = & (*t3d_it);

			int w = q->size[0];
			int h = q->size[1];
			int flip = (sdci[6]/10000)&3;
			int tile = sdci[7]/10000;
			int polytype = sdci[8]/10000;

			polytype = vbound(polytype, 0, 14);
			if( !(zc::math::IsPowerOfTwo(w) && zc::math::IsPowerOfTwo(h)) )
				break;

			BITMAP *tex = create_bitmap_ex(8,16*w,16*h);
			clear_bitmap(tex);

			if(tile > 0)	// TILE
			{
				Tile.OverTile( tex, tile, 0, 0, w, h, q->color[0], flip );
			}
			else			// COMBO
			{
				const int tiletodraw = combo_tile( abs(tile), int(q->pos[0]), int(q->pos[1]) );
				Tile.OldPutTile( tex, tiletodraw, 0, 0, w, h, q->color[0], flip );
			}

			V3D_f V1 = { q->pos[0]+xoffset, q->pos[1] +yoffset, q->pos[2] ,	q->uv[0], q->uv[1], q->color[0] };
			V3D_f V2 = { q->pos[3]+xoffset, q->pos[4] +yoffset, q->pos[5] ,	q->uv[2], q->uv[3], q->color[1] };
			V3D_f V3 = { q->pos[6]+xoffset, q->pos[7] +yoffset, q->pos[8] , q->uv[4], q->uv[5], q->color[2] };

			triangle3d_f(bmp, polytype, tex, &V1, &V2, &V3);
			destroy_bitmap(tex);

			draw_container.triangle3D.erase(t3d_it);
		}
		break;
      }
    }
  }

  //delete Tile;

  color_map=&trans_table;
}


void put_walkflags(BITMAP *dest,int x,int y,word cmbdat,int layer)
{
  newcombo c = combobuf[cmbdat];

  for(int i=0; i<4; i++)
  {
    int tx=((i&2)<<2)+x;
    int ty=((i&1)<<3)+y;
    if(layer==0 && combo_class_buf[c.type].water!=0 && get_bit(quest_rules, qr_DROWN))
      rectfill(dest,tx,ty,tx+7,ty+7,vc(9));
    if(c.walk&(1<<i))
      rectfill(dest,tx,ty,tx+7,ty+7,vc(12));
  }
}

// Walkflags L4 cheat
void do_walkflags(BITMAP *dest,mapscr* layer,int x, int y)
{
  if (show_walkflags)
  {
    for(int i=0; i<176; i++)
    {
      put_walkflags(dest,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i], 0);
	  put_walkflags(dest,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i], 0);
    }
	int layermap, layerscreen;
    for (int k=0; k<2; k++)
    {
      layermap=layer->layermap[k%2]-1;
      if (layermap>-1)
      {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k%2];
        for (int i=0; i<176; i++)
        {
          put_walkflags(temp_buf,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,TheMaps[layerscreen].data[i], k%2+1);
		  put_walkflags(scrollbuf,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,TheMaps[layerscreen].data[i], k%2+1);
        }
      }
    }
  }
}

void draw_screen(mapscr* this_screen, mapscr* next_screen, int x1, int y1, int x2, int y2, bool showlink)
{
  //The Plan:
  //1. Draw some layers onto scrollbuf with clipping
  //2. Blit scrollbuf onto framebuf
  //3. Draw some sprites onto framebuf
  //4. Blit framebuf onto temp_buf
  //5. Draw some layers onto temp_buf and scrollbuf
  //6. Blit temp_buf onto framebuf with clipping
  //6b. Draw the subscreen onto temp_buf, without clipping
  //7. Draw some flying sprites onto framebuf
  //8. Blit frame_buf onto temp_buf
  //9. Draw some layers onto temp_buf
  //10. Blit temp_buf onto framebuf with clipping
  //11. Draw some text on framebuf and scrollbuf
  //12. Draw the subscreen onto framebuf, without clipping
  clear_bitmap(framebuf);
  set_clip_rect(framebuf,0,0,256,224);

  clear_bitmap(temp_buf);
  set_clip_state(temp_buf,1);
  set_clip_rect(temp_buf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);

  int cmby2=0;
  int pcounter;

  // For the scripted drawing functions.
  draw_container.SortContainers();

  //1. Draw some layers onto temp_buf
  clear_bitmap(scrollbuf);

  if (next_screen!=NULL)
  {
    if(next_screen->flags7&fLAYER2BG) do_layer(scrollbuf,1, next_screen, x2, y2+playing_field_offset, 3);
    if(next_screen->flags7&fLAYER3BG) do_layer(scrollbuf,2, next_screen, x2, y2+playing_field_offset, 3);
  }
  if(this_screen->flags7&fLAYER2BG)
  {
    do_layer(scrollbuf,1, this_screen, x1, y1, 2);
    for (pcounter=0;pcounter<particles.Count();pcounter++)
    {
      if (((particle*)particles.spr(pcounter))->layer==1)
      {
        particles.spr(pcounter)->draw(scrollbuf);
      }
    }
  }
  if(this_screen->flags7&fLAYER3BG)
  {
    do_layer(scrollbuf,2, this_screen, x1, y1, 2);
    for (pcounter=0;pcounter<particles.Count();pcounter++)
    {
      if (((particle*)particles.spr(pcounter))->layer==2)
      {
        particles.spr(pcounter)->draw(scrollbuf);
      }
    }
  }
  if (next_screen!=NULL)
  {
    putscr(scrollbuf,x2,y2,next_screen);
  }
  putscr(scrollbuf,x1,y1+playing_field_offset,this_screen);
  // Lens hints, then primitives, then particles.
  if((lensclk || (get_debug() && key[KEY_L])) && !get_bit(quest_rules, qr_OLDLENSORDER))
  {
    draw_lens_under(scrollbuf, false);
  }
  if (show_layer_0)
    do_primitives(scrollbuf, 0, this_screen, x1,y1+playing_field_offset);

  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==-3)
    {
      particles.spr(pcounter)->draw(scrollbuf);
    }
  }

  set_clip_rect(scrollbuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);

  if(!(get_bit(quest_rules,qr_LAYER12UNDERCAVE)))
  {
    if (showlink &&
    ((Link.getAction()==climbcovertop)||(Link.getAction()==climbcoverbottom)))
    {
      if (Link.getAction()==climbcovertop)
      {
        cmby2=16;
      }
      else if (Link.getAction()==climbcoverbottom)
      {
        cmby2=-16;
      }
      decorations.draw2(scrollbuf,true);
      Link.draw(scrollbuf);
      decorations.draw(scrollbuf,true);
      int ccx = (int)(Link.getClimbCoverX());
      int ccy = (int)(Link.getClimbCoverY());

      overcombo(scrollbuf,ccx,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
      putcombo (scrollbuf,ccx,ccy+playing_field_offset,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
      if(int(Link.getX())&15)
      {
        overcombo(scrollbuf,ccx+16,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
        putcombo (scrollbuf,ccx+16,ccy+playing_field_offset,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
      }
    }
  }
  if (next_screen!=NULL)
  {
    do_layer(scrollbuf,0, next_screen, x2, y2, 3);
    if(!(next_screen->flags7&fLAYER2BG))
    {
      do_layer(scrollbuf,1, next_screen, x2, y2, 3);
    }
    do_layer(scrollbuf,-2, next_screen, x2, y2, 3);
  }
  do_layer(scrollbuf,0, this_screen, x1, y1, 2); // LAYER 1
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==0)
    {
      particles.spr(pcounter)->draw(scrollbuf);
    }
  }
  if(!(this_screen->flags7&fLAYER2BG))
  {
    do_layer(scrollbuf,1, this_screen, x1, y1, 2); // LAYER 2
    for (pcounter=0;pcounter<particles.Count();pcounter++)
    {
      if (((particle*)particles.spr(pcounter))->layer==1)
      {
        particles.spr(pcounter)->draw(scrollbuf);
      }
    }
  }
  if(get_bit(quest_rules,qr_LAYER12UNDERCAVE))
  {
    if (showlink &&
    ((Link.getAction()==climbcovertop)||(Link.getAction()==climbcoverbottom)))
    {
      if (Link.getAction()==climbcovertop)
      {
        cmby2=16;
      }
      else if (Link.getAction()==climbcoverbottom)
      {
        cmby2=-16;
      }
      decorations.draw2(scrollbuf,true);
      Link.draw(scrollbuf);
      decorations.draw(scrollbuf,true);
      int ccx = (int)(Link.getClimbCoverX());
      int ccy = (int)(Link.getClimbCoverY());

      overcombo(scrollbuf,ccx,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx,ccy+cmby2),MAPCSET(ccx,ccy+cmby2));
      putcombo (scrollbuf,ccx,ccy+playing_field_offset,MAPCOMBO(ccx,ccy),MAPCSET(ccx,ccy));
      if(int(Link.getX())&15)
      {
        overcombo(scrollbuf,ccx+16,ccy+cmby2+playing_field_offset,MAPCOMBO(ccx+16,ccy+cmby2),MAPCSET(ccx+16,ccy+cmby2));
        putcombo (scrollbuf,ccx+16,ccy+playing_field_offset,MAPCOMBO(ccx+16,ccy),MAPCSET(ccx+16,ccy));
      }
    }
  }
  do_layer(scrollbuf,-2, this_screen, x1, y1, 2); // push blocks!
  do_layer(scrollbuf,-3, this_screen, x1, y1, 2); // freeform combos!
  if (next_screen!=NULL)
  {
    putscrdoors(scrollbuf,x2,y2,next_screen);
  }
  putscrdoors(scrollbuf,x1,y1+playing_field_offset,this_screen);
  // Lens hints, doors etc.
  if(lensclk || (get_debug() && key[KEY_L]))
  {
    if(get_bit(quest_rules, qr_OLDLENSORDER))
    {
      draw_lens_under(scrollbuf, false);
    }
    draw_lens_under(scrollbuf, true);
  }

  //2. Blit those layers onto framebuf


  set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
  masked_blit(scrollbuf, framebuf, 0, 0, 0, 0, 256, 224);


  //3. Draw some sprites onto framebuf
  set_clip_rect(framebuf,0,0,256,224);

  if(!(pricesdisplaybuf->clip))
  {
    masked_blit(pricesdisplaybuf,framebuf,0,0,0,playing_field_offset,256,168);
  }

  if (showlink && ((Link.getAction()!=climbcovertop)&&(Link.getAction()!=climbcoverbottom)))
  {
    Link.draw_under(framebuf);
    if (Link.isSwimming())
    {
      decorations.draw2(framebuf,true);
      Link.draw(framebuf);
      decorations.draw(framebuf,true);
    }
  }

  if(drawguys)
  {
    if(get_bit(quest_rules,qr_NOFLICKER) || (frame&1))
    {
      for(int i=0; i<Ewpns.Count(); i++)
      {
        if(((weapon *)Ewpns.spr(i))->behind)
        Ewpns.spr(i)->draw(framebuf);
      }
      if (get_bit(quest_rules,qr_SHADOWS)&&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
      {
        guys.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0,true);
      }
      guys.draw(framebuf,true);
      chainlinks.draw(framebuf,true);
      Lwpns.draw(framebuf,true);
      for(int i=0; i<Ewpns.Count(); i++)
      {
        if(!((weapon *)Ewpns.spr(i))->behind)
        Ewpns.spr(i)->draw(framebuf);
      }

      items.draw(framebuf,true);
    }
    else
    {
      for(int i=0; i<Ewpns.Count(); i++)
      {
        if(((weapon *)Ewpns.spr(i))->behind)
        Ewpns.spr(i)->draw(framebuf);
      }
      if (get_bit(quest_rules,qr_SHADOWS)&&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
      {
        guys.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0,true);
      }
      items.draw(framebuf,false);
      chainlinks.draw(framebuf,false);
      Lwpns.draw(framebuf,false);
      guys.draw(framebuf,false);
      for(int i=0; i<Ewpns.Count(); i++)
      {
        if(!((weapon *)Ewpns.spr(i))->behind)
        {
          Ewpns.spr(i)->draw(framebuf);
        }
      }
    }
    guys.draw2(framebuf,true);
  }
  if (showlink && ((Link.getAction()!=climbcovertop)&& (Link.getAction()!=climbcoverbottom)))
  {
    mblock2.draw(framebuf);
    if (!Link.isSwimming())
    {
      if (Link.getZ()>0 &&(!get_bit(quest_rules,qr_SHADOWSFLICKER)||frame&1))
      {
        Link.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS)!=0);
      }
      decorations.draw2(framebuf,true);
      Link.draw(framebuf);
      decorations.draw(framebuf,true);
    }
  }

  for(int i=0; i<guys.Count(); i++)
  {
    if(((enemy*)guys.spr(i))->family == eeWALK)
    {
      if(((eStalfos*)guys.spr(i))->haslink)
      {
        guys.spr(i)->draw(framebuf);
      }
    }
    if(((enemy*)guys.spr(i))->family == eeWALLM)
    {
      if(((eWallM*)guys.spr(i))->haslink)
      {
        guys.spr(i)->draw(framebuf);
      }
    }
    if (guys.spr(i)->z > Link.getZ())
    { //Jumping enemies in front of Link.
      guys.spr(i)->draw(framebuf);
    }
  }
  //4. Blit framebuf onto temp_buf

  //you have to do this, because do_layer calls overcombo, which doesn't respect the clipping rectangle, which messes up the triforce curtain. -DD
  blit(framebuf, temp_buf, 0, 0, 0, 0, 256, 224);

  //5. Draw some layers onto temp_buf and scrollbuf

  if (next_screen!=NULL)
  {
    if(!(next_screen->flags7&fLAYER3BG))
	{
		do_layer(temp_buf,2, next_screen, x2, y2, 3);
		do_layer(scrollbuf, 2, next_screen, x2, y2, 3);
	}
    do_layer(temp_buf,3, next_screen, x2, y2, 3);
	do_layer(scrollbuf, 3, next_screen, x2, y2, 3);
    do_layer(temp_buf,-1, next_screen, x2, y2, 3);
	do_layer(scrollbuf, 3, next_screen, x2, y2, 3);
  }


  if(!(this_screen->flags7&fLAYER3BG))
  {
    do_layer(temp_buf,2, this_screen, x1, y1, 2);
	do_layer(scrollbuf, 2, this_screen, x1, y1, 2);
    for (pcounter=0;pcounter<particles.Count();pcounter++)
    {
      if (((particle*)particles.spr(pcounter))->layer==2)
      {
        particles.spr(pcounter)->draw(temp_buf);
      }
    }
  }
  do_layer(temp_buf,3, this_screen, x1, y1, 2);
  do_layer(scrollbuf, 3, this_screen, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==3)
    {
      particles.spr(pcounter)->draw(temp_buf);
    }
  }
  do_layer(temp_buf,-1, this_screen, x1, y1, 2);
  do_layer(scrollbuf,-1, this_screen, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==-1)
    {
      particles.spr(pcounter)->draw(temp_buf);
    }
  }

  //6. Blit temp_buf onto framebuf with clipping

  set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
  blit(temp_buf, framebuf, 0, 0, 0, 0, 256, 224);

  //6b. Draw the subscreen, without clipping
  if(!get_bit(quest_rules,qr_SUBSCREENOVERSPRITES))
  {
    set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
    put_passive_subscr(framebuf, &QMisc, 0, passive_subscreen_offset, false, sspUP);
  }


  //7. Draw some flying sprites onto framebuf
  set_clip_rect(framebuf,0,0,256,224);

  //Jumping Link and jumping enemies are drawn on this layer.
  if (Link.getZ() > (fix)zinit.jump_link_layer_threshold) {
      decorations.draw2(framebuf,false);
      Link.draw(framebuf);
      chainlinks.draw(framebuf,true);
      for(int i=0; i<Lwpns.Count(); i++)
      {
  if (Lwpns.spr(i)->z > (fix)zinit.jump_link_layer_threshold)
        {
          Lwpns.spr(i)->draw(framebuf);
        }
      }
      decorations.draw(framebuf,false);
  }

  if (!get_bit(quest_rules,qr_ENEMIESZAXIS)) for(int i=0; i<guys.Count(); i++)
  {
    if((isflier(guys.spr(i)->id)) || guys.spr(i)->z > (fix)zinit.jump_link_layer_threshold)
    {
      guys.spr(i)->draw(framebuf);
    }/*
    if(guys.spr(i)->id == eCEILINGM)
    {
      if (((eCEILINGM*)guys.spr(i))->haslink)
      {
        guys.spr(i)->draw(temp_buf2);
      }
    }*/
  }
  else
  {
    for(int i=0; i<guys.Count(); i++)
    {
      if((isflier(guys.spr(i)->id)) || guys.spr(i)->z > 0)
        {
          guys.spr(i)->draw(framebuf);
        }
    }
  }
  // Draw the Moving Fairy above layer 3
  for(int i=0; i<items.Count(); i++)
    if (itemsbuf[items.spr(i)->id].family == itype_fairy && itemsbuf[items.spr(i)->id].misc3)
      items.spr(i)->draw(framebuf);

  //8. Blit framebuf onto temp_buf

  masked_blit(framebuf, temp_buf, 0, 0, 0, 0, 256, 224);

  //9. Draw some layers onto temp_buf and scrollbuf

  set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);

  if (next_screen!=NULL)
  {
    do_layer(temp_buf,4, next_screen, x2, y2, 3);
	do_layer(scrollbuf, 4, next_screen, x2, y2, 3);
    do_layer(temp_buf,-4, this_screen, x2, y2, 3); // overhead freeform combos!
	do_layer(scrollbuf, -4, this_screen, x2, y2, 3);
    do_layer(temp_buf,5, next_screen, x2, y2, 3);
	do_layer(scrollbuf, 5, next_screen, x2, y2, 3);
  }

  do_layer(temp_buf,4, this_screen, x1, y1, 2);
  do_layer(scrollbuf, 4, this_screen, x1, y1, 2);

  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==4)
    {
      particles.spr(pcounter)->draw(temp_buf);
    }
  }

  do_layer(temp_buf,-4, this_screen, x1, y1, 2); // overhead freeform combos!
  do_layer(scrollbuf, -4, this_screen, x1, y1, 2);

  do_layer(temp_buf,5, this_screen, x1, y1, 2);
  do_layer(scrollbuf, 5, this_screen, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==5)
    {
      particles.spr(pcounter)->draw(temp_buf);
    }
  }
  //10. Show walkflags cheat
  do_walkflags(temp_buf,this_screen,x1,y1);
  do_walkflags(scrollbuf,this_screen,x1,y1);
  if (next_screen!=NULL)
  {
    do_walkflags(temp_buf,next_screen,x2,y2);
    do_walkflags(scrollbuf,next_screen,x2,y2);
  }

  //10. Blit temp_buf onto framebuf with clipping

  set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2);
  blit(temp_buf, framebuf, 0, 0, 0, 0, 256, 224);


  //11. Draw some text on framebuf

  set_clip_rect(framebuf,0,0,256,224);
  set_clip_rect(scrollbuf,0,0,256,224);

  if(!(msgdisplaybuf->clip))
  {
    masked_blit(msgdisplaybuf,framebuf,0,0,0,playing_field_offset,256,168);
    masked_blit(msgdisplaybuf,scrollbuf,0,0,0,playing_field_offset,256,168);
  }

  //12. Draw the subscreen, without clipping

  if(get_bit(quest_rules,qr_SUBSCREENOVERSPRITES))
  {
    put_passive_subscr(framebuf, &QMisc, 0, passive_subscreen_offset, false, sspUP);
    do_primitives(framebuf, 7, this_screen, x1,y1+playing_field_offset); //Layer '7' appears above subscreen if quest rule is set
  }
  memset(script_drawing_commands, 0, MAX_SCRIPT_DRAWING_COMMANDS*SCRIPT_DRAWING_COMMAND_VARIABLES*sizeof(int));
  set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
}

void put_door(BITMAP *dest,int t,int pos,int side,int type,bool redraw,bool even_walls)
{
  int d=tmpscr[t].door_combo_set;
  switch (type)
  {
    case dt_wall:
    case dt_walk:
      if(!even_walls)
          break;
    case dt_pass:
      if(!get_bit(quest_rules, qr_REPLACEOPENDOORS) && !even_walls)
        break;
    case dt_lock:
    case dt_shut:
    case dt_boss:
    case dt_olck:
    case dt_osht:
    case dt_obos:
    case dt_bomb:
      switch (side)
      {
        case up:
          tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_u[type][0];
          tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_u[type][0];
          tmpscr[t].sflag[pos]  = 0;
          tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_u[type][1];
          tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_u[type][1];
          tmpscr[t].sflag[pos+1]  = 0;
          tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_u[type][2];
          tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_u[type][2];
          tmpscr[t].sflag[pos+16]  = 0;
          tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_u[type][3];
          tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_u[type][3];
          tmpscr[t].sflag[pos+16+1]  = 0;
          if(redraw)
          {
            putcombo(dest,(pos&15)<<4,pos&0xF0,
                     DoorComboSets[d].doorcombo_u[type][0],
                     DoorComboSets[d].doorcset_u[type][0]);
            putcombo(dest,((pos&15)<<4)+16,pos&0xF0,
                     DoorComboSets[d].doorcombo_u[type][1],
                     DoorComboSets[d].doorcset_u[type][1]);
          }
          break;
        case down:
          tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_d[type][0];
          tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_d[type][0];
          tmpscr[t].sflag[pos]  = 0;
          tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_d[type][1];
          tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_d[type][1];
          tmpscr[t].sflag[pos+1]  = 0;
          tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_d[type][2];
          tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_d[type][2];
          tmpscr[t].sflag[pos+16]  = 0;
          tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_d[type][3];
          tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_d[type][3];
          tmpscr[t].sflag[pos+16+1]  = 0;
          if(redraw)
          {
            putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
                     DoorComboSets[d].doorcombo_d[type][2],
                     DoorComboSets[d].doorcset_d[type][2]);
            putcombo(dest,((pos&15)<<4)+16,(pos&0xF0)+16,
                     DoorComboSets[d].doorcombo_d[type][3],
                     DoorComboSets[d].doorcset_d[type][3]);
          }
          break;
        case left:
          tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_l[type][0];
          tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_l[type][0];
          tmpscr[t].sflag[pos]  = 0;
          tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_l[type][1];
          tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_l[type][1];
          tmpscr[t].sflag[pos+1]  = 0;
          tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_l[type][2];
          tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_l[type][2];
          tmpscr[t].sflag[pos+16]  = 0;
          tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_l[type][3];
          tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_l[type][3];
          tmpscr[t].sflag[pos+16+1]  = 0;
          tmpscr[t].data[pos+32]   = DoorComboSets[d].doorcombo_l[type][4];
          tmpscr[t].cset[pos+32]   = DoorComboSets[d].doorcset_l[type][4];
          tmpscr[t].sflag[pos+32]  = 0;
          tmpscr[t].data[pos+32+1]   = DoorComboSets[d].doorcombo_l[type][5];
          tmpscr[t].cset[pos+32+1]   = DoorComboSets[d].doorcset_l[type][5];
          tmpscr[t].sflag[pos+32+1]  = 0;
          if(redraw)
          {
            putcombo(dest,(pos&15)<<4,pos&0xF0,
                     DoorComboSets[d].doorcombo_l[type][0],
                     DoorComboSets[d].doorcset_l[type][0]);
            putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
                     DoorComboSets[d].doorcombo_l[type][2],
                     DoorComboSets[d].doorcset_l[type][2]);
            putcombo(dest,(pos&15)<<4,(pos&0xF0)+32,
                     DoorComboSets[d].doorcombo_l[type][4],
                     DoorComboSets[d].doorcset_l[type][4]);
          }
          break;
        case right:
          tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_r[type][0];
          tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_r[type][0];
          tmpscr[t].sflag[pos]  = 0;
          tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_r[type][1];
          tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_r[type][1];
          tmpscr[t].sflag[pos+1]  = 0;
          tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_r[type][2];
          tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_r[type][2];
          tmpscr[t].sflag[pos+16]  = 0;
          tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_r[type][3];
          tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_r[type][3];
          tmpscr[t].sflag[pos+16+1]  = 0;
          tmpscr[t].data[pos+32]   = DoorComboSets[d].doorcombo_r[type][4];
          tmpscr[t].cset[pos+32]   = DoorComboSets[d].doorcset_r[type][4];
          tmpscr[t].sflag[pos+32]  = 0;
          tmpscr[t].data[pos+32+1]   = DoorComboSets[d].doorcombo_r[type][5];
          tmpscr[t].cset[pos+32+1]   = DoorComboSets[d].doorcset_r[type][5];
          tmpscr[t].sflag[pos+32+1]  = 0;
          if(redraw)
          {
            putcombo(dest,(pos&15)<<4,pos&0xF0,
                     DoorComboSets[d].doorcombo_r[type][0],
                     DoorComboSets[d].doorcset_r[type][0]);
            putcombo(dest,(pos&15)<<4,(pos&0xF0)+16,
                     DoorComboSets[d].doorcombo_r[type][2],
                     DoorComboSets[d].doorcset_r[type][2]);
            putcombo(dest,(pos&15)<<4,(pos&0xF0)+32,
                     DoorComboSets[d].doorcombo_r[type][4],
                     DoorComboSets[d].doorcset_r[type][4]);
          }
          break;
      }
      break;
    default:
      break;
  }
}

void over_door(BITMAP *dest,int t, int pos,int side, int xoff, int yoff)
{
  int d=tmpscr[t].door_combo_set;
  int x=(pos&15)<<4;
  int y=(pos&0xF0);
  switch (side)
  {
    case up:
    overcombo2(dest,x+xoff,y+yoff,
               DoorComboSets[d].bombdoorcombo_u[0],
               DoorComboSets[d].bombdoorcset_u[0]);
    overcombo2(dest,x+16+xoff,y+yoff,
               DoorComboSets[d].bombdoorcombo_u[1],
               DoorComboSets[d].bombdoorcset_u[1]);
    break;
    case down:
    overcombo2(dest,x+xoff,y+yoff,
               DoorComboSets[d].bombdoorcombo_d[0],
               DoorComboSets[d].bombdoorcset_d[0]);
    overcombo2(dest,x+16+xoff,y+yoff,
               DoorComboSets[d].bombdoorcombo_d[1],
               DoorComboSets[d].bombdoorcset_d[1]);
    break;
    case left:
    overcombo2(dest,x+xoff,y+yoff,
               DoorComboSets[d].bombdoorcombo_l[0],
               DoorComboSets[d].bombdoorcset_l[0]);
    overcombo2(dest,x+xoff,y+yoff+16,
               DoorComboSets[d].bombdoorcombo_l[1],
               DoorComboSets[d].bombdoorcset_l[1]);
    overcombo2(dest,x+xoff,y+yoff+16,
               DoorComboSets[d].bombdoorcombo_l[2],
               DoorComboSets[d].bombdoorcset_l[2]);
    break;
    case right:
    overcombo2(dest,x+xoff,y+yoff,
               DoorComboSets[d].bombdoorcombo_r[0],
               DoorComboSets[d].bombdoorcset_r[0]);
    overcombo2(dest,x+xoff,y+yoff+16,
               DoorComboSets[d].bombdoorcombo_r[1],
               DoorComboSets[d].bombdoorcset_r[1]);
    overcombo2(dest,x+xoff,y+yoff+16,
               DoorComboSets[d].bombdoorcombo_r[2],
               DoorComboSets[d].bombdoorcset_r[2]);
    break;
  }
}

void putdoor(BITMAP *dest,int t,int side,int door,bool redraw,bool even_walls)
{
  /*
    #define dWALL           0  //  000    0
    #define dBOMB           6  //  011    0
    #define              8  //  100    0
    enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
    */

  if(!even_walls&&(door==dWALL||door==dWALK))
  {
    return;
  }
  int doortype;
  switch (door)
  {
    case dWALL:
      doortype=dt_wall;
      break;
    case dWALK:
      doortype=dt_walk;
      break;
    case dOPEN:
      doortype=dt_pass;
      break;
    case dLOCKED:
      doortype=dt_lock;
      break;
    case dUNLOCKED:
      doortype=dt_olck;
      break;
    case dSHUTTER:
      if (screenscrolling && ((LinkDir()^1)==side))
      {
	doortype=dt_osht; opendoors=-4; break;
      }
      //fallthrough
    case d1WAYSHUTTER:
      doortype=dt_shut;
      break;
    case dOPENSHUTTER:
      doortype=dt_osht;
      break;
    case dBOSS:
      doortype=dt_boss;
      break;
    case dOPENBOSS:
      doortype=dt_obos;
      break;
    case dBOMBED:
      doortype=dt_bomb;
      break;
    default:
      return;
  }

  switch(side)
  {
    case up:
      switch(door)
      {
        case dBOMBED:
          if(redraw)
          {
            over_door(dest,t,39,side,0,0);
          }
        default:
          put_door(dest,t,7,side,doortype,redraw, even_walls);
          break;
      }
      break;
    case down:
      switch(door)
      {
        case dBOMBED:
          if(redraw)
          {
            over_door(dest,t,135,side,0,0);
          }
        default:
          put_door(dest,t,151,side,doortype,redraw, even_walls);
          break;
      }
      break;
    case left:
      switch(door)
      {
        case dBOMBED:
          if(redraw)
          {
            over_door(dest,t,66,side,0,0);
          }
        default:
          put_door(dest,t,64,side,doortype,redraw, even_walls);
          break;
      }
      break;
    case right:
      switch(door)
      {
        case dBOMBED:
          if(redraw)
          {
            over_door(dest,t,77,side,0,0);
          }
        default:
          put_door(dest,t,78,side,doortype,redraw, even_walls);
          break;
      }
      break;
  }
}

void putcombo_not_zero(BITMAP *dest, int x, int y, int combo, int cset)
{
  if (combo!=0)
  {
    putcombo(dest,x, y, combo, cset);
  }
}

void overcombo_not_zero(BITMAP *dest, int x, int y, int combo, int cset)
{
  if (combo!=0)
  {
    overcombo(dest,x, y, combo, cset);
  }
}

void showbombeddoor(BITMAP *dest, int side)
{
  int d=tmpscr->door_combo_set;
  switch(side)
  {
    case up:
      putcombo_not_zero(dest,(7&15)<<4,(7&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_u[dt_bomb][0],
               DoorComboSets[d].doorcset_u[dt_bomb][0]);
      putcombo_not_zero(dest,(8&15)<<4,(8&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_u[dt_bomb][1],
               DoorComboSets[d].doorcset_u[dt_bomb][1]);
      putcombo_not_zero(dest,(23&15)<<4,(23&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_u[dt_bomb][2],
               DoorComboSets[d].doorcset_u[dt_bomb][2]);
      putcombo_not_zero(dest,(24&15)<<4,(24&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_u[dt_bomb][3],
               DoorComboSets[d].doorcset_u[dt_bomb][3]);
      overcombo_not_zero(dest,(39&15)<<4,(39&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_u[0],
                DoorComboSets[d].bombdoorcset_u[0]);
      overcombo_not_zero(dest,(40&15)<<4,(40&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_u[1],
                DoorComboSets[d].bombdoorcset_u[1]);
      break;
    case down:
      putcombo_not_zero(dest,(151&15)<<4,(151&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_d[dt_bomb][0],
               DoorComboSets[d].doorcset_d[dt_bomb][0]);
      putcombo_not_zero(dest,(152&15)<<4,(152&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_d[dt_bomb][1],
               DoorComboSets[d].doorcset_d[dt_bomb][1]);
      putcombo_not_zero(dest,(167&15)<<4,(167&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_d[dt_bomb][2],
               DoorComboSets[d].doorcset_d[dt_bomb][2]);
      putcombo_not_zero(dest,(168&15)<<4,(168&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_d[dt_bomb][3],
               DoorComboSets[d].doorcset_d[dt_bomb][3]);
      overcombo_not_zero(dest,(135&15)<<4,(135&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_d[0],
                DoorComboSets[d].bombdoorcset_d[0]);
      overcombo_not_zero(dest,(136&15)<<4,(136&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_d[1],
                DoorComboSets[d].bombdoorcset_d[1]);
      break;
    case left:
      putcombo_not_zero(dest,(64&15)<<4,(64&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_l[dt_bomb][0],
               DoorComboSets[d].doorcset_l[dt_bomb][0]);
      putcombo_not_zero(dest,(65&15)<<4,(65&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_l[dt_bomb][1],
               DoorComboSets[d].doorcset_l[dt_bomb][1]);
      putcombo_not_zero(dest,(80&15)<<4,(80&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_l[dt_bomb][2],
               DoorComboSets[d].doorcset_l[dt_bomb][2]);
      putcombo_not_zero(dest,(81&15)<<4,(81&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_l[dt_bomb][3],
               DoorComboSets[d].doorcset_l[dt_bomb][3]);
      putcombo_not_zero(dest,(96&15)<<4,(96&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_l[dt_bomb][4],
               DoorComboSets[d].doorcset_l[dt_bomb][4]);
      putcombo_not_zero(dest,(97&15)<<4,(97&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_l[dt_bomb][5],
               DoorComboSets[d].doorcset_l[dt_bomb][5]);
      overcombo_not_zero(dest,(66&15)<<4,(66&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_l[0],
                DoorComboSets[d].bombdoorcset_l[0]);
      overcombo_not_zero(dest,(82&15)<<4,(82&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_l[1],
                DoorComboSets[d].bombdoorcset_l[1]);
      overcombo_not_zero(dest,(98&15)<<4,(98&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_l[2],
                DoorComboSets[d].bombdoorcset_l[2]);
      break;
    case right:
      putcombo_not_zero(dest,(78&15)<<4,(78&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_r[dt_bomb][0],
               DoorComboSets[d].doorcset_r[dt_bomb][0]);
      putcombo_not_zero(dest,(79&15)<<4,(79&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_r[dt_bomb][1],
               DoorComboSets[d].doorcset_r[dt_bomb][1]);
      putcombo_not_zero(dest,(94&15)<<4,(94&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_r[dt_bomb][2],
               DoorComboSets[d].doorcset_r[dt_bomb][2]);
      putcombo_not_zero(dest,(95&15)<<4,(95&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_r[dt_bomb][3],
               DoorComboSets[d].doorcset_r[dt_bomb][3]);
      putcombo_not_zero(dest,(110&15)<<4,(110&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_r[dt_bomb][4],
               DoorComboSets[d].doorcset_r[dt_bomb][4]);
      putcombo_not_zero(dest,(111&15)<<4,(111&0xF0)+playing_field_offset,
               DoorComboSets[d].doorcombo_r[dt_bomb][5],
               DoorComboSets[d].doorcset_r[dt_bomb][5]);
      overcombo_not_zero(dest,(77&15)<<4,(77&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_r[0],
                DoorComboSets[d].bombdoorcset_r[0]);
      overcombo_not_zero(dest,(93&15)<<4,(93&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_r[1],
                DoorComboSets[d].bombdoorcset_r[1]);
      overcombo_not_zero(dest,(109&15)<<4,(109&0xF0)+playing_field_offset,
                DoorComboSets[d].bombdoorcombo_r[2],
                DoorComboSets[d].bombdoorcset_r[2]);
      break;
  }
}

void openshutters()
{
  for(int i=0; i<4; i++)
    if(tmpscr->door[i]==dSHUTTER)
    {
      putdoor(scrollbuf,0,i,dOPENSHUTTER);
      tmpscr->door[i]=dOPENSHUTTER;
    }
    sfx(WAV_DOOR,128);
}

void loadscr(int tmp,int scr,int ldir,bool overlay=false)
{
  //  introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
  for (word x=0; x<animated_combos; x++)
  {
    if (combobuf[animated_combo_table4[x][0]].nextcombo!=0)
    {
      animated_combo_table4[x][1]=0;
    }
  }
  for (word x=0; x<animated_combos2; x++)
  {
    if (combobuf[animated_combo_table24[x][0]].nextcombo!=0)
    {
      animated_combo_table24[x][1]=0;
    }
  }
  reset_combo_animations2();


  mapscr ffscr = tmpscr[tmp];
  tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];


  const int _mapsSize = ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth;
  tmpscr[tmp].data = TheMaps[currmap*MAPSCRS+scr].data;
  tmpscr[tmp].sflag = TheMaps[currmap*MAPSCRS+scr].sflag;
  tmpscr[tmp].cset = TheMaps[currmap*MAPSCRS+scr].cset;

  tmpscr[tmp].data.resize( _mapsSize, 0 );
  tmpscr[tmp].sflag.resize( _mapsSize, 0 );
  tmpscr[tmp].cset.resize( _mapsSize, 0 );

  if(overlay)
  {
    for(int c=0; c< ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth; ++c){
      if(tmpscr[tmp].data[c]==0){
        tmpscr[tmp].data[c]=ffscr.data[c];
        tmpscr[tmp].sflag[c]=ffscr.sflag[c];
        tmpscr[tmp].cset[c]=ffscr.cset[c];
      }
    }
       for(int i=0; i<6; i++){
          if(ffscr.layermap[i]>0 && tmpscr[tmp].layermap[i]>0){
			int lm = tmpscr[tmp].layermap[i]*MAPSCRS+tmpscr[tmp].layerscreen[i];
			int fm = ffscr.layermap[i]*MAPSCRS+ffscr.layerscreen[i];
		          
            if(!TheMaps[lm].data.empty() && !TheMaps[fm].data.empty()) {
			  for(int c=0; c< ZCMaps[currmap].tileHeight*ZCMaps[currmap].tileWidth; ++c) {
			    if (TheMaps[lm].data[c]==0) {
				  TheMaps[lm].data[c] = TheMaps[fm].data[c];
				  TheMaps[lm].sflag[c] = TheMaps[fm].sflag[c];
				  TheMaps[lm].cset[c] = TheMaps[fm].cset[c];
				}
			  } 
		    }
        }
      }
  }

  if(tmp==0)
  {
    for(int i=0;i<32;i++)
    {
      if((ffscr.ffflags[i]&ffCARRYOVER)&&!(ffscr.flags5&fNOFFCARRYOVER))
      {
        tmpscr[tmp].ffdata[i] = ffscr.ffdata[i];
        tmpscr[tmp].ffx[i] = ffscr.ffx[i];
        tmpscr[tmp].ffy[i] = ffscr.ffy[i];
        tmpscr[tmp].ffxdelta[i] = ffscr.ffxdelta[i];
        tmpscr[tmp].ffydelta[i] = ffscr.ffydelta[i];
        tmpscr[tmp].ffxdelta2[i] = ffscr.ffxdelta2[i];
        tmpscr[tmp].ffydelta2[i] = ffscr.ffydelta2[i];
        tmpscr[tmp].fflink[i] = ffscr.fflink[i];
        tmpscr[tmp].ffdelay[i] = ffscr.ffdelay[i];
        tmpscr[tmp].ffcset[i] = ffscr.ffcset[i];
        tmpscr[tmp].ffwidth[i] = ffscr.ffwidth[i];
        tmpscr[tmp].ffheight[i] = ffscr.ffheight[i];
        tmpscr[tmp].ffflags[i] = ffscr.ffflags[i];
        tmpscr[tmp].ffscript[i] = ffscr.ffscript[i];
        tmpscr[tmp].scriptflag[i] = ffscr.scriptflag[i];
		for(int j=0;j<16;j++) tmpscr[tmp].ffmisc[i][j] = ffscr.ffmisc[i][j];

        for (int j=0; j<2; ++j)
        {
          tmpscr[tmp].a[i][j] = ffscr.a[i][j];
          tmpscr[tmp].inita[i][j] = ffscr.inita[i][j];
        }
        for (int j=0; j<8; ++j)
        {
          tmpscr[tmp].d[i][j] = ffscr.d[i][j];
          tmpscr[tmp].initd[i][j] = ffscr.initd[i][j];
        }
        if(!(ffscr.ffflags[i]&ffSCRIPTRESET))
        {
          tmpscr[tmp].ffscript[i] = ffscr.ffscript[i]; // Restart script even if it has halted.
          tmpscr[tmp].initialized[i] = ffscr.initialized[i];
          tmpscr[tmp].pc[i] = ffscr.pc[i];
          tmpscr[tmp].ffcref[i] = ffscr.ffcref[i];
          tmpscr[tmp].sp[i] = ffscr.sp[i];
        }
        else
        {
          tmpscr[tmp].initialized[i] = false;
          tmpscr[tmp].pc[i] = 0;
          tmpscr[tmp].sp[i] = 0;
          tmpscr[tmp].ffcref[i] = 0;
        }
      }
      else
      {
	    for(int j=0;j<16;j++) tmpscr[tmp].ffmisc[i][j] = 0;
        for(int j=0; j<256; j++)
        {
          ffstack[i][j] = 0;
        }
      }
    }
  }


  if (tmp==0)
  {
    for (int i=0; i<6; i++)
    {
		mapscr layerscr = tmpscr2[i];
		// Don't delete the old tmpscr2's data yet!
		if(tmpscr[tmp].layermap[i]>0 && (ZCMaps[tmpscr[tmp].layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth)
		   && (ZCMaps[tmpscr[tmp].layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
		{
		 // const int _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);

		  tmpscr2[i]=TheMaps[(tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i]];

		  tmpscr2[i].data.resize( _mapsSize, 0 );
		  tmpscr2[i].sflag.resize( _mapsSize, 0 );
		  tmpscr2[i].cset.resize( _mapsSize, 0 );

		  if(overlay)
		  {
			for(int y=0; y<ZCMaps[currmap].tileHeight; ++y)
			{
			  for(int x=0; x<ZCMaps[currmap].tileWidth; ++x)
			  {
				int c=y*ZCMaps[currmap].tileWidth+x;
				if(tmpscr2[i].data[c]==0)
				{
				  tmpscr2[i].data[c]=layerscr.data[c];
				  tmpscr2[i].sflag[c]=layerscr.sflag[c];
				  tmpscr2[i].cset[c]=layerscr.cset[c];
				}
			  }
			}
		  }
		}
		else
		{
		  (tmpscr2+i)->zero_memory();
		}
    }
  }

  if(!isdungeon()/*||TheMaps[(currmap*MAPSCRS)+currscr].flags6&fTRIGGERFPERM*/)
  {
    if(game->maps[(currmap<<7)+scr]&mSECRET)               // if special stuff done before
    {
      hiddenstair(tmp,false);
      hidden_entrance(tmp,false,false,-3);
    }
  }


  if(game->maps[(currmap<<7)+scr]&mLOCKBLOCK)              // if special stuff done before
  {
    remove_lockblocks(tmp);
  }

  if(game->maps[(currmap<<7)+scr]&mBOSSLOCKBLOCK)          // if special stuff done before
  {
    remove_bosslockblocks(tmp);
  }

  if(game->maps[(currmap<<7)+scr]&mCHEST)              // if special stuff done before
  {
    remove_chests(tmp);
  }

  if(game->maps[(currmap<<7)+scr]&mLOCKEDCHEST)              // if special stuff done before
  {
    remove_lockedchests(tmp);
  }

  if(game->maps[(currmap<<7)+scr]&mBOSSCHEST)              // if special stuff done before
  {
    remove_bosschests(tmp);
  }

  // check doors
  if(isdungeon())
  {
    for(int i=0; i<4; i++)
    {
      int door=tmpscr[tmp].door[i];
      bool putit=true;

      switch(door)
      {
        case d1WAYSHUTTER:
        case dSHUTTER:
        if((ldir^1)==i)
        {
          tmpscr[tmp].door[i]=dOPENSHUTTER;
          //          putit=false;
        }
		opendoors = -4;
        break;

        case dLOCKED:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dUNLOCKED;
          //          putit=false;
        }
        break;

        case dBOSS:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dOPENBOSS;
          //          putit=false;
        }
        break;

        case dBOMB:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dBOMBED;
        }
        break;
      }

      if(putit)
      {
        putdoor(scrollbuf,tmp,i,tmpscr[tmp].door[i],false);
      }
      if (door==dSHUTTER||door==d1WAYSHUTTER)
      {
        tmpscr[tmp].door[i]=door;
      }
    }
  }


  for (int j=-1; j<6; ++j) // j == -1 denotes the current screen
  {
    if (j<0 || ((tmpscr[tmp].layermap[j]>0)&&(ZCMaps[tmpscr[tmp].layermap[j]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[tmpscr[tmp].layermap[j]-1].tileHeight==ZCMaps[currmap].tileHeight)))
    {
      mapscr *layerscreen= (j<0 ? &tmpscr[tmp] : tmpscr2[j].data.empty() ? &tmpscr2[j] :
        &TheMaps[(tmpscr[tmp].layermap[j]-1)*MAPSCRS]+tmpscr[tmp].layerscreen[j]);
      for(int i=0; i<(ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight); ++i)
      {
          int c=layerscreen->data[i];
          int cs=layerscreen->cset[i];

          // New screen flag: Cycle Combos At Screen Init
          if (combobuf[c].nextcombo != 0 && (tmpscr[tmp].flags3 & fCYCLEONINIT) && (j<0 || get_bit(quest_rules,qr_CMBCYCLELAYERS)))
          {
            int r = 0;
			screen_combo_modify_preroutine(layerscreen,i);
            while (combobuf[c].nextcombo != 0 && r++ < 10)
            {
              layerscreen->data[i] = combobuf[c].nextcombo;
              layerscreen->cset[i] = combobuf[c].nextcset;
              c=layerscreen->data[i];
              cs=layerscreen->cset[i];
            }
          }
	      screen_combo_modify_postroutine(layerscreen,i);
      }
    }
  }
}

// Screen is being viewed by the Overworld Map viewer.
void loadscr2(int tmp,int scr,int )
{
  for (word x=0; x<animated_combos; x++)
  {
    if (combobuf[animated_combo_table4[x][0]].nextcombo!=0)
    {
      animated_combo_table4[x][1]=0;
    }
  }
  const int _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);

  tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];

  tmpscr[tmp].data.resize( _mapsSize, 0 );
  tmpscr[tmp].sflag.resize( _mapsSize, 0 );
  tmpscr[tmp].cset.resize( _mapsSize, 0 );

  if (tmp==0)
  {
    for (int i=0; i<6; i++)
    {
      if (tmpscr[tmp].layermap[i]>0)
      {

      if((ZCMaps[tmpscr[tmp].layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[tmpscr[tmp].layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
      {
          tmpscr2[i]=TheMaps[(tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i]];

		  tmpscr2[i].data.resize( _mapsSize, 0 );
		  tmpscr2[i].sflag.resize( _mapsSize, 0 );
		  tmpscr2[i].cset.resize( _mapsSize, 0 );
     }
      else
      {
          (tmpscr2+i)->zero_memory();
      }
    }
    else
    {
        (tmpscr2+i)->zero_memory();
    }
    }
  }
  if(!isdungeon())
  {
    if(game->maps[(currmap<<7)+scr]&mSECRET)               // if special stuff done before
    {
      hiddenstair(tmp,false);
      hidden_entrance(tmp,false,false,-3);
    }
  }

  if(game->maps[(currmap<<7)+scr]&mLOCKBLOCK)              // if special stuff done before
  {
    remove_lockblocks(tmp);
  }

  if(game->maps[(currmap<<7)+scr]&mBOSSLOCKBLOCK)          // if special stuff done before
  {
    remove_bosslockblocks(tmp);
  }

  if(game->maps[(currmap<<7)+scr]&mCHEST)              // if special stuff done before
  {
    remove_chests(tmp);
  }

  if(game->maps[(currmap<<7)+scr]&mLOCKEDCHEST)              // if special stuff done before
  {
    remove_lockedchests(tmp);
  }

  if(game->maps[(currmap<<7)+scr]&mBOSSCHEST)              // if special stuff done before
  {
    remove_bosschests(tmp);
  }

  // check doors
  if(isdungeon())
  {
    for(int i=0; i<4; i++)
    {
      int door=tmpscr[tmp].door[i];
      bool putit=true;

      switch(door)
      {
        case d1WAYSHUTTER:
        case dSHUTTER:
/*
        if((ldir^1)==i)
        {
          tmpscr[tmp].door[i]=dOPENSHUTTER;
          //          putit=false;
        }
*/
		break;

        case dLOCKED:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dUNLOCKED;
          //          putit=false;
        }
        break;

        case dBOSS:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dOPENBOSS;
          //          putit=false;
        }
        break;

        case dBOMB:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dBOMBED;
        }
        break;
      }

      if(putit)
      {
        putdoor(scrollbuf,tmp,i,tmpscr[tmp].door[i],false);
      }
      if (door==dSHUTTER||door==d1WAYSHUTTER)
      {
        tmpscr[tmp].door[i]=door;
      }
    }
  }

  for (int j=-1; j<6; ++j) // j == -1 denotes the current screen
  {
    if (j<0 || ((tmpscr[tmp].layermap[j]>0)&&(ZCMaps[tmpscr[tmp].layermap[j]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[tmpscr[tmp].layermap[j]-1].tileHeight==ZCMaps[currmap].tileHeight)))
    {
      mapscr *layerscreen= (j<0 ? &tmpscr[tmp]
        : &(TheMaps[(tmpscr[tmp].layermap[j]-1)*MAPSCRS+tmpscr[tmp].layerscreen[j]]));
      for(int i=0; i<(ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight); ++i)
      {
          int c=layerscreen->data[i];
          int cs=layerscreen->cset[i];

          // New screen flag: Cycle Combos At Screen Init
          if ((tmpscr[tmp].flags3 & fCYCLEONINIT) && (j<0 || get_bit(quest_rules,qr_CMBCYCLELAYERS)))
          {
            int r = 0;
            while (combobuf[c].nextcombo != 0 && r++ < 10)
            {
              layerscreen->data[i] = combobuf[c].nextcombo;
              layerscreen->cset[i] = combobuf[c].nextcset;
              c=layerscreen->data[i];
              cs=layerscreen->cset[i];
            }
          }
      }
    }
  }

}

void putscr(BITMAP* dest,int x,int y, mapscr* scrn)
{
  if(scrn->valid==0||!show_layer_0)
  {
    rectfill(dest,x,y,x+255,y+175,0);
    return;
  }
  for(int i=0; i<176; i++)
  {
    if(scrn->flags7&fLAYER2BG||scrn->flags7&fLAYER3BG)
    {
      overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,scrn->data[i],scrn->cset[i]);
    }
    else
    {
      putcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,scrn->data[i],scrn->cset[i]);
    }
  }
}

void putscrdoors(BITMAP *dest,int x,int y, mapscr* scrn)
{
  if(scrn->valid==0||!show_layer_0)
  {
    return;
  }
  if(scrn->door[0]==dBOMBED)
  {
    over_door(dest,0,39,up,x,y);
  }
  if(scrn->door[1]==dBOMBED)
  {
    over_door(dest,0,135,down,x,y);
  }
  if(scrn->door[2]==dBOMBED)
  {
    over_door(dest,0,66,left,x,y);
  }
  if(scrn->door[3]==dBOMBED)
  {
    over_door(dest,0,77,right,x,y);
  }
}

bool _walkflag(int x,int y,int cnt)
{
  //  walkflagx=x; walkflagy=y;
  if(get_bit(quest_rules,qr_LTTPWALK))
  {
    if(x<0||y<0) return false;
    if(x>255) return false;
    if(x>247&&cnt==2) return false;
    if(y>175) return false;
  }
  else
  {
    if(x<0||y<0) return false;
    if(x>248) return false;
    if(x>240&&cnt==2) return false;
    if(y>168) return false;
  }
  mapscr *s1, *s2;
  s1=(((*tmpscr).layermap[0]-1)>=0)?tmpscr2:tmpscr;
  s2=(((*tmpscr).layermap[1]-1)>=0)?tmpscr2+1:tmpscr;
  //  s2=TheMaps+((*tmpscr).layermap[1]-1)MAPSCRS+((*tmpscr).layerscreen[1]);

  int bx=(x>>4)+(y&0xF0);
  newcombo c = combobuf[tmpscr->data[bx]];
  newcombo c1 = combobuf[s1->data[bx]];
  newcombo c2 = combobuf[s2->data[bx]];
  bool dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
                 (iswater_type(c2.type))) && (tmpscr->flags7 & fWHISTLEWATER) &&
         (whistleclk>=88));
  int b=1;

  if(x&8) b<<=2;
  if(y&8) b<<=1;
  if(((c.walk&b) || (c1.walk&b) || (c2.walk&b)) && !dried)
    return true;
  if(cnt==1) return false;

  ++bx;
  if(!(x&8))
    b<<=2;
  else
  {
    c  = combobuf[tmpscr->data[bx]];
    c1 = combobuf[s1->data[bx]];
    c2 = combobuf[s2->data[bx]];
    dried = (((iswater_type(c.type)) || (iswater_type(c1.type)) ||
              (iswater_type(c2.type))) && (tmpscr->flags7 & fWHISTLEWATER) &&
        (whistleclk>=88));
    b=1;
    if(y&8) b<<=1;
  }
  return ((c.walk&b)||(c1.walk&b)||(c2.walk&b)) ? !dried : false;
}

bool water_walkflag(int x,int y,int cnt)
{
  if(get_bit(quest_rules,qr_LTTPWALK))
  {
    if(x<0||y<0) return false;
    if(x>255) return false;
    if(x>247&&cnt==2) return false;
    if(y>175) return false;
  }
  else
  {
    if(x<0||y<0) return false;
    if(x>248) return false;
    if(x>240&&cnt==2) return false;
    if(y>168) return false;
  }
  mapscr *s1, *s2;
  /*
    s1=(((*tmpscr).layermap[0]-1)>=0)?
    (TheMaps+((*tmpscr).layermap[0]-1)*MAPSCRS+((*tmpscr).layerscreen[0])):
    tmpscr;
    s2=(((*tmpscr).layermap[1]-1)>=0)?
    (TheMaps+((*tmpscr).layermap[1]-1)*MAPSCRS+((*tmpscr).layerscreen[1])):
    tmpscr;
    */
  s1=(((*tmpscr).layermap[0]-1)>=0)?tmpscr2:tmpscr;
  s2=(((*tmpscr).layermap[1]-1)>=0)?tmpscr2+1:tmpscr;

  int bx=(x>>4)+(y&0xF0);
  newcombo c = combobuf[tmpscr->data[bx]];
  newcombo c1 = combobuf[s1->data[bx]];
  newcombo c2 = combobuf[s2->data[bx]];
  int b=1;

  if(x&8) b<<=2;
  if(y&8) b<<=1;
  if((c.walk&b) && !iswater_type(c.type))
    return true;
  if((c1.walk&b) && !iswater_type(c1.type))
    return true;
  if((c2.walk&b) && !iswater_type(c2.type))
    return true;
  if(cnt==1) return false;

  if(x&8)
    b<<=2;
  else
  {
    c = combobuf[tmpscr->data[++bx]];
    c1 = combobuf[s1->data[bx]];
    c2 = combobuf[s2->data[bx]];
    b=1;
    if(y&8) b<<=1;
  }

  return (c.walk&b) ? !iswater_type(c.type) :
    (c1.walk&b) ? !iswater_type(c1.type) :
    (c2.walk&b) ? !iswater_type(c2.type) :false;
}

bool hit_walkflag(int x,int y,int cnt)
{
  if(dlevel)
    if(x<32 || y<40 || (x+(cnt-1)*8)>=224 || y>=144)
      return true;
  if(blockpath && y<((get_bit(quest_rules,qr_LTTPCOLLISION))?80:88))
    return true;
  if(x<16 || y<16 || (x+(cnt-1)*8)>=240 || y>=160)
    return true;
  //  for(int i=0; i<4; i++)
  if(mblock2.clk && mblock2.hit(x,y,0,cnt*8,1,16))
    return true;
  return _walkflag(x,y,cnt);
}

void map_bkgsfx(bool on)
{
  if(on)
  {
    cont_sfx(tmpscr->oceansfx);
    if(tmpscr->bosssfx && !(game->lvlitems[dlevel]&liBOSS))
        cont_sfx(tmpscr->bosssfx);
  }
  else
  {
    adjust_sfx(tmpscr->oceansfx,128,false);
    adjust_sfx(tmpscr->bosssfx,128,false);
    for(int i=0; i<guys.Count(); i++)
    {
      if(((enemy*)guys.spr(i))->bgsfx)
        stop_sfx(((enemy*)guys.spr(i))->bgsfx);
    }
  }
}

/****  View Map  ****/

//BITMAP *mappic = NULL;
int mapres = 0;

void ViewMap()
{
  mapscr tmpscr_b[2];
  mapscr tmpscr_c[6];
  for (int i=0; i<6; ++i)
  {
    tmpscr_c[i] = tmpscr2[i];
    tmpscr2[i].zero_memory();
    if (i>=2)
    {
      continue;
    }
     tmpscr_b[i] = tmpscr[i];
    tmpscr[i].zero_memory();
  }

  BITMAP* mappic = NULL;
  static double scales[17] =
  {
    0.03125, 0.04419, 0.0625, 0.08839, 0.125, 0.177, 0.25, 0.3535,
    0.50, 0.707, 1.0, 1.414, 2.0, 2.828, 4.0, 5.657, 8.0
  };

  int px = ((8-(currscr&15)) << 9)  - 256;
  int py = ((4-(currscr>>4)) * 352) - 176;
  int lx = ((currscr&15)<<8)  + LinkX()+8;
  int ly = ((currscr>>4)*176) + LinkY()+8;
  int sc = 6;

  bool done=false, redraw=true;

  mappic = create_bitmap_ex(8,(256*16)>>mapres,(176*8)>>mapres);

  if(!mappic)
  {
    system_pal();
    jwin_alert("View Map","Not enough memory.",NULL,NULL,"OK",NULL,13,27,lfont);
    game_pal();
    return;
  }

  // draw the map
  set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
  for(int y=0; y<8; y++)
  {
    for(int x=0; x<16; x++)
    {
      int s = (y<<4) + x;

      if(!(game->maps[(currmap<<7)+s]&mVISITED) ||
        // Don't display if not part of DMap
        ((DMaps[currdmap].flags&dmfDMAPMAP)
        && (DMaps[currdmap].type != dmOVERW)
        && !(x >= DMaps[currdmap].xoff
          && x < DMaps[currdmap].xoff+8
          && DMaps[currdmap].grid[y]&(128>>(x-DMaps[currdmap].xoff)))))
      {
        rectfill(scrollbuf, 256, 0, 511, 223, WHITE);
      }
      else
      {
        loadscr2(1,s,-1);
    for (int i=0; i<6; i++)
        {
          if (tmpscr[1].layermap[i]>0)
          {
            if((ZCMaps[tmpscr[1].layermap[i]-1].tileWidth==ZCMaps[currmap].tileWidth) && (ZCMaps[tmpscr[1].layermap[i]-1].tileHeight==ZCMaps[currmap].tileHeight))
            {
			   const int _mapsSize = (ZCMaps[currmap].tileWidth)*(ZCMaps[currmap].tileHeight);

			   tmpscr2[i]=TheMaps[(tmpscr[1].layermap[i]-1)*MAPSCRS+tmpscr[1].layerscreen[i]];

				tmpscr2[i].data.resize( _mapsSize, 0 );
				tmpscr2[i].sflag.resize( _mapsSize, 0 );
				tmpscr2[i].cset.resize( _mapsSize, 0 );
           }
            else
            {
            }
          }
          else
          {
          }
        }

        if((tmpscr+1)->flags7&fLAYER2BG) do_layer(scrollbuf, 1, tmpscr+1, -256, playing_field_offset, 2);
        if((tmpscr+1)->flags7&fLAYER3BG) do_layer(scrollbuf, 2, tmpscr+1, -256, playing_field_offset, 2);
        putscr(scrollbuf,256,0,tmpscr+1);
        do_layer(scrollbuf, 0, tmpscr+1, -256, playing_field_offset, 2);
        if(!((tmpscr+1)->flags7&fLAYER2BG)) do_layer(scrollbuf, 1, tmpscr+1, -256, playing_field_offset, 2);
        putscrdoors(scrollbuf,256,0,tmpscr+1);
        do_layer(scrollbuf,-2, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf,-3, tmpscr+1, -256, playing_field_offset, 2); // Freeform combos!
        if(!((tmpscr+1)->flags7&fLAYER3BG)) do_layer(scrollbuf, 2, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf, 3, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf,-1, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf, 4, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf, 5, tmpscr+1, -256, playing_field_offset, 2);

      }
      stretch_blit(scrollbuf, mappic, 256, 0, 256, 176, x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
    }
  }

  for (int i=0; i<6; ++i)
  {
    tmpscr2[i]=tmpscr_c[i];
    if (i>=2)
    {
      continue;
    }
    tmpscr[i]=tmpscr_b[i];
  }


  clear_keybuf();
  pause_all_sfx();

  // view it
  int delay = 0;
  static int show  = 3;

  do
  {
    load_control_state();
    int step = int(16.0/scales[sc]);
    step = (step>>1) + (step&1);
    bool r = cRbtn();

    if(cLbtn())
    {
      step <<= 2;
      delay = 0;
    }

    if(r)
    {
      if(rUp())    { py+=step; redraw=true; }
      if(rDown())  { py-=step; redraw=true; }
      if(rLeft())  { px+=step; redraw=true; }
      if(rRight()) { px-=step; redraw=true; }
    }
    else
    {
      if(Up())    { py+=step; redraw=true; }
      if(Down())  { py-=step; redraw=true; }
      if(Left())  { px+=step; redraw=true; }
      if(Right()) { px-=step; redraw=true; }
    }

    if(delay)
      --delay;
    else
    {
      bool a = cAbtn();
      bool b = cBbtn();
      if(a && !b)  { sc=zc_min(sc+1,16); delay=8; redraw=true; }
      if(b && !a)  { sc=zc_max(sc-1,0);  delay=8; redraw=true; }
    }

    if(rPbtn())
      --show;

    px = vbound(px,-4096,4096);
    py = vbound(py,-1408,1408);

    double scale = scales[sc];

    if(!redraw)
    {
      blit(scrollbuf,framebuf,256,0,0,0,256,224);
    }
    else
    {
      clear_to_color(framebuf,BLACK);
      stretch_blit(mappic,framebuf,0,0,mappic->w,mappic->h,
                   int(256+(px-mappic->w)*scale)/2,int(224+(py-mappic->h)*scale)/2,
                   int(mappic->w*scale),int(mappic->h*scale));

      blit(framebuf,scrollbuf,0,0,256,0,256,224);
      redraw=false;
    }

    int x = int(256+(px-((2048-lx)*2))*scale)/2;
    int y = int(224+(py-((704-ly)*2))*scale)/2;

    if(show&1)
    {
      line(framebuf,x-7,y-7,x+7,y+7,(frame&3)+252);
      line(framebuf,x+7,y-7,x-7,y+7,(frame&3)+252);
    }

    //    text_mode(BLACK);

    if(show&2 || r)
      textprintf_ex(framebuf,font,224,216,WHITE,BLACK,"%1.2f",scale);

    if(r)
    {
      textprintf_ex(framebuf,font,0,208,WHITE,BLACK,"m: %d %d",px,py);
      textprintf_ex(framebuf,font,0,216,WHITE,BLACK,"x: %d %d",x,y);
    }

  //since stuff in here accesses tmpscr and tmpscr2... -DD
    advanceframe(false, false);


    if(rSbtn())
      done = true;

  } while(!done && !Quit);

  destroy_bitmap(mappic);

  resume_all_sfx();
}

int onViewMap()
{
  if(Playing && currscr<128 && DMaps[currdmap].flags&dmfVIEWMAP)
  {
    clear_to_color(framebuf,BLACK);
      //      text_mode(BLACK);
      textout_centre_ex(framebuf,font,"Drawing map...",128,108,WHITE,BLACK);
      advanceframe(true);
      ViewMap();
  }
  return D_O_K;
}

bool isGrassType(int type)
{
  switch(type)
  {
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cTALLGRASSTOUCHY:
      return true;
  }
  return false;
}

bool isFlowersType(int type)
{
  switch(type)
  {
    case cFLOWERS:
    case cFLOWERSTOUCHY:
      return true;
  }
  return false;
}

bool isBushType(int type)
{
  switch(type)
  {
    case cBUSH:
    case cBUSHNEXT:
    case cBUSHTOUCHY:
    case cBUSHNEXTTOUCHY:
      return true;
  }
  return false;
}

bool isSlashType(int type)
{
  switch(type)
  {
    case cSLASH:
    case cSLASHITEM:
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
      return true;
  }
  return false;
}

bool isCuttableNextType(int type)
{
  switch(type)
  {
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cTALLGRASSNEXT:
    case cBUSHNEXT:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
      return true;
  }
  return false;
}

bool isTouchyType(int type)
{
  switch(type)
  {
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
      return true;
  }
  return false;
}

bool isCuttableType(int type)
{
  switch(type)
  {
    case cSLASH:
    case cSLASHITEM:
    case cBUSH:
    case cFLOWERS:
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cBUSHNEXT:

    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
      return true;
  }
  return false;
}

bool isCuttableItemType(int type)
{
  switch(type)
  {
    case cSLASHITEM:
    case cBUSH:
    case cFLOWERS:
    case cTALLGRASS:
    case cSLASHNEXTITEM:
    case cBUSHNEXT:

    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
      return true;
  }
  return false;
}


FONT *get_zc_font(int index)
{
  //return getfont(index);
  switch(index)
  {
    default:					return zfont;
    case font_z3font:			return z3font;
    case font_z3smallfont:		return z3smallfont;
    case font_deffont:			return deffont;
    case font_lfont:			return lfont;
    case font_lfont_l:			return lfont_l;
    case font_pfont:			return pfont;
    case font_mfont:			return mfont;
    case font_ztfont:			return ztfont;
    case font_sfont:			return sfont;
    case font_sfont2:			return sfont2;
    case font_spfont:			return spfont;
    case font_ssfont1:			return ssfont1;
    case font_ssfont2:			return ssfont2;
    case font_ssfont3:			return ssfont3;
    case font_ssfont4:			return ssfont4;
    case font_gblafont:			return gblafont;
    case font_goronfont:		return goronfont;
    case font_zoranfont:		return zoranfont;
    case font_hylian1font:		return hylian1font;
    case font_hylian2font:		return hylian2font;
    case font_hylian3font:		return hylian3font;
    case font_hylian4font:		return hylian4font;
    case font_gboraclefont:		return gboraclefont;
    case font_gboraclepfont:	return gboraclepfont;
    case font_dsphantomfont:	return dsphantomfont;
    case font_dsphantompfont:	return dsphantompfont;
  }
}


/*** end of maps.cc ***/

