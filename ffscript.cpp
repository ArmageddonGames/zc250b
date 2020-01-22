#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include <deque>
//#include <algorithm>
#include <string>
#include <string.h>
#include <sstream>
#include <math.h>
#include <cstdio>

#include "zc_math.h"
#include "zc_array.h"
#include "ffscript.h"
#include "zelda.h"
#include "link.h"
#include "guys.h"
#include "gamedata.h"
#include "zc_init.h"
#include "zsys.h"
#include "title.h"
#include "trapper_keeper.h"

#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))

using std::string;

extern sprite_list particles;
extern zinitdata    zinit;
extern LinkClass Link;
extern int directItem;
extern int directItemA;
extern int directItemB;
extern char *guy_string[];
extern int skipcont;
enemy *script_npc;
item *script_item;
weapon *script_weapon;

int script_type=SCRIPT_GLOBAL;
word scommand;
long sarg1;
long sarg2;

DrawingContainer draw_container;
/*
float *DrawingContainer::depth_buffer = NULL;
int   *DrawingContainer::color_buffer = NULL;
*/

void DrawingContainer::SortContainers()
{
	this->SortDrawString();
	this->SortQuad3D();
	this->SortTriangle3D();
}

void DrawingContainer::SortDrawString()
{
	//std::stable_sort( drawstring.begin(), drawstring.end() );
}

std::list< std::pair <int, std::string> > ::iterator DrawingContainer::getDrawStringIterator( int index )
{
	std::list< std::pair <int, std::string> > ::iterator _it = drawstring.begin();
	for( ; _it != drawstring.end(); ++_it )
		if( (*_it).first == index )
			break;

	return _it;
}

std::deque< quad3Dstruct > ::iterator DrawingContainer::getQuad3dIterator( int index )
{
	std::deque< quad3Dstruct > ::iterator _it = quad3D.begin();
	for( ; _it != quad3D.end(); ++_it )
		if( (*_it).index == index )
			break;

	return _it;
}

std::deque< triangle3Dstruct > ::iterator DrawingContainer::getTriangle3dIterator( int index )
{
	std::deque< triangle3Dstruct > ::iterator _it = triangle3D.begin();
	for( ; _it != triangle3D.end(); ++_it )
		if( (*_it).index == index )
			break;

	return _it;
}

void DrawingContainer::SortQuad3D()
{
	//std::stable_sort( quad3D.begin(), quad3D.end() );
}

void DrawingContainer::SortTriangle3D()
{
	//std::stable_sort( triangle3D.begin(), triangle3D.end() );
}


// global script variables
byte global_ffc=0;
dword global_item=0;
dword global_guy=0;
dword global_ewpn=0;
dword global_lwpn=0;
byte global_scr=0;
byte global_ram=0;
long g_d[8];
dword g_scriptflag=0;
long g_stack[256];
byte g_sp=0;
word g_pc=0;
word g_doscript=1;
byte global_itemclass=0;
byte global_lwpnclass=0;
byte global_ewpnclass=0;
byte global_guyclass=0;
bool global_wait=false;

struct refInfo {
  long *d[8];
  long *a[2];
  byte *sp;
  byte *ffc, *idata, *gclass, *lclass, *eclass, *ramref, global, linkref, scr;
  dword *itemref, *guyref, *lwpn, *ewpn;
};

dword *scriptflag=NULL;
long *na[2];
byte *sp=NULL;
long (*st)[256]=NULL;
word *ffs2=NULL;

//it's not much, but it's a start at some form of organization at least.
class ScriptHelper
{
public:

	enum __Error {
		_NoError,
		_AllocationError, // script array too small
		_Overflow
	};

	//Returns a reference to the correct array based on pointer passed
	static ZScriptArray& getArray(const dword& ptr)
	{
		if(ptr >= MAX_ZCARRAY_SIZE)
			return globalRAM[ptr-MAX_ZCARRAY_SIZE];
		else
			return localRAM[ptr];
	}

	//Can't you get the std::string and then check its length?
	static int strlen(const dword& ptr)
	{
		ZScriptArray& a = getArray(ptr);
		word count;
		for(count = 0; a[count] != 0; count++);
		return count;
	}

	//Returns values of a zscript array as an std::string.
	static string getString(const dword& ptr, word num_chars = 256)
	{
		ZScriptArray& a = getArray(ptr);
		string str;
		for(word i = 0; a[i] != 0 && num_chars != 0; i++)
		{
			str += char(a[i]);
			num_chars--;
		}
		return str;
	}

	//Get element from array
	static INLINE long getElement(const dword& ptr, const word& offset)
	{
		return getArray(ptr)[offset];
	}

	//Set element in array
	static INLINE void setElement(const dword& ptr, const word& offset, const long& value)
	{
		getArray(ptr)[offset] = value;
	}

	/* Puts values of a zscript array into a client <type> array. returns 0 on success. Overloaded*/
template <class T>
	static int getArray( const dword& ptr, const word& size, T *refArray )
	{
		return getArray( ptr, size, 0, 0, 0, refArray );
	}
template <class T>
	static int getArray( const dword& ptr, const word& size, word userOffset, const word& userStride, const word& refArrayOffset, T *refArray )
	{
		ZScriptArray& a = getArray(ptr);
		word j = 0, k = userStride;
		for(word i = 0; j < size; i++)
		{
			if(i >= a.Size())
				return _Overflow;
			if(userOffset-- > 0)
				continue;
			if(k > 0)
				k--;
			else
			{
				refArray[j+refArrayOffset] = T(a[i]);
				k = userStride;
				j++;
			}
		}

		return _NoError;
	}

	/* Puts values of a client <type> array into a zscript array. returns 0 on success. Overloaded*/
template <class T>
	static int SetArray( const dword& ptr, const word& size, T *refArray )
	{
		return SetArray( ptr, size, 0, 0, 0, refArray );
	}
template <class T>
	static int SetArray( const dword& ptr, const word& size, word userOffset, const word& userStride, const word& refArrayOffset, T *refArray )
	{
		ZScriptArray& a = getArray(ptr);
		word j = 0, k = userStride;
		for(word i = 0; j < size; i++)
		{
			if(i >= a.Size())
				return _Overflow; //Resize?
			if(userOffset-- > 0)
				continue;
			if(k > 0)
				k--;
			else
			{
				a[i] = long( refArray[j+refArrayOffset] / T(10000) ) * 10000;
				k = userStride;
				j++;
			}
		}

		return _NoError;
	}

protected:
private:

};


enemy *checkNPC(long eid)
{
  enemy *s = (enemy *)guys.getByUID(eid);
  if(s == NULL)
  {

	Z_eventlog("Script attempted to reference a nonexistent NPC!\n");
	Z_eventlog("You were trying to reference an NPC with UID = %ld; NPCs on screen are UIDs ", eid);
	for(int i=0; i<guys.Count(); i++)
	{
		Z_eventlog("%ld ", guys.spr(i)->getUID());
	}
	Z_eventlog("\n");
    return NULL;
  }
  return s;
}

item *checkItem(long iid)
{
  item *s = (item *)items.getByUID(iid);
  if(s == NULL)
  {
	Z_eventlog("Script attempted to reference a nonexistent item!\n");
	Z_eventlog("You were trying to reference an item with UID = %ld; Items on screen are UIDs ", iid);
	for(int i=0; i<items.Count(); i++)
	{
		Z_eventlog("%ld ", items.spr(i)->getUID());
	}
	Z_eventlog("\n");
    return NULL;
  }
  return s;
}

weapon *checkLWpn(long eid, const char *what)
{
  weapon *s = (weapon *)Lwpns.getByUID(eid);
  if(s == NULL)
  {

	Z_eventlog("Script attempted to reference a nonexistent LWeapon!\n");
	Z_eventlog("You were trying to reference the %s of an LWeapon with UID = %ld; LWeapons on screen are UIDs ", what, eid);
	for(int i=0; i<Lwpns.Count(); i++)
	{
		Z_eventlog("%ld ", Lwpns.spr(i)->getUID());
	}
	Z_eventlog("\n");
    return NULL;
  }
  return s;
}

weapon *checkEWpn(long eid, const char *what)
{
  weapon *s = (weapon *)Ewpns.getByUID(eid);
  if(s == NULL)
  {

	Z_eventlog("Script attempted to reference a nonexistent EWeapon!\n");
	Z_eventlog("You were trying to reference the %s of an EWeapon with UID = %ld; EWeapons on screen are UIDs ", what, eid);
	for(int i=0; i<Ewpns.Count(); i++)
	{
		Z_eventlog("%ld ", Ewpns.spr(i)->getUID());
	}
	Z_eventlog("\n");
    return NULL;
  }
  return s;
}


int get_screen_d(long index1, long index2)
{
  if (index2 < 0 || index2 > 7)
  {
    Z_eventlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", index1);
    return 0;
  }
  return game->screen_d[index1][index2];
}

void set_screen_d(long index1, long index2, int val)
{
  if (index2 < 0 || index2 > 7)
  {
    Z_eventlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", index1);
    return;
  }
  game->screen_d[index1][index2] = val;
}

// If scr is currently being used as a layer, return that layer no.
int whichlayer(long scr)
{
  for (int i=0; i<6; i++)
  {
    if (scr == (tmpscr->layermap[i]-1)*MAPSCRS+tmpscr->layerscreen[i])
      return i;
  }
  return -1;
}

// Is the enemy also carrying Link?
bool HasLink(enemy *s)
{
  if(s->family==eeWALLM)
    return (((eWallM*)s)->haslink);

  if(s->family==eeWALK)
    return (((eStalfos*)s)->haslink);

  return false;
}


long ret=0;
int di, di2;
int mi, mi2;

sprite *s;

int get_screenflags(mapscr*,int);
int get_screeneflags(mapscr*,int);
byte flagpos;
int ornextflag(bool flag);


#ifdef _MSC_VER
	#pragma warning ( disable : 4800 ) //int to bool town. population: lots.
#endif


long get_arg(long arg, byte, refInfo &ri)
{
  ret=0;

  switch(arg)
  {
    case DATA:
      ret=tmpscr->ffdata[*ri.ffc]*10000; break;
    case FFSCRIPT:
      ret=tmpscr->ffscript[*ri.ffc]*10000; break;
    case FCSET:
      ret=tmpscr->ffcset[*ri.ffc]*10000; break;
    case DELAY:
      ret=tmpscr->ffdelay[*ri.ffc]*10000; break;
    case FX:
      ret=tmpscr->ffx[*ri.ffc]; break;
    case FY:
      ret=tmpscr->ffy[*ri.ffc]; break;
    case XD:
      ret=tmpscr->ffxdelta[*ri.ffc]; break;
    case YD:
      ret=tmpscr->ffydelta[*ri.ffc]; break;
    case XD2:
      ret=tmpscr->ffxdelta2[*ri.ffc]; break;
    case YD2:
      ret=tmpscr->ffydelta2[*ri.ffc]; break;
    case FFFLAGSD:
      ret=((tmpscr->ffflags[*ri.ffc]>>((*(ri.d[0]))/10000))&1)?10000:0; break;
    case FFCWIDTH:
      ret=((tmpscr->ffwidth[*ri.ffc]&63)+1)*10000; break;
    case FFCHEIGHT:
      ret=((tmpscr->ffheight[*ri.ffc]&63)+1)*10000; break;
    case FFTWIDTH:
      ret=((tmpscr->ffwidth[*ri.ffc]>>6)+1)*10000; break;
    case FFTHEIGHT:
      ret=((tmpscr->ffheight[*ri.ffc]>>6)+1)*10000; break;
    case FFLINK:
      ret=(tmpscr->fflink[*ri.ffc])*10000; break;
    case FFMISCD:
	{
     int a = vbound(*(ri.d[0])/10000,0,15);
     ret=(tmpscr->ffmisc[*ri.ffc][a]); break;
    }
    case FFINITDD:
     ret=(tmpscr->initd[*ri.ffc][vbound(*(ri.d[0])/10000,0,7)]); break;
    /*case FFDD:
     ret=(tmpscr->d[*ri.ffc][vbound(*(ri.d[0])/10000,0,7)]); break;*/
    case LINKX:
      ret=(int)(Link.getX())*10000; break;
    case LINKY:
      ret=(int)(Link.getY())*10000; break;
    case LINKZ:
      ret=(int)(Link.getZ())*10000; break;
    case LINKJUMP:
      ret=(int)(-Link.getFall()/100.0)*10000; break;
    case LINKDIR:
      ret=(int)(Link.dir)*10000; break;
    case LINKHP:
      ret=(int)(game->get_life())*10000; break;
    case LINKMP:
      ret=(int)(game->get_magic())*10000; break;
    case LINKMAXHP:
      ret=(int)(game->get_maxlife())*10000; break;
    case LINKMAXMP:
      ret=(int)(game->get_maxmagic())*10000; break;
    case LINKACTION:
      ret=(int)(Link.getAction())*10000; break;
    case LINKHELD:
      ret = (int)(Link.getHeldItem())*10000; break;
    case LINKITEMD:
      ret = game->item[(*ri.d[0])/10000] ? 10000 : 0; break;
    case LINKEQUIP:
	 ret = (Awpn|(Bwpn<<8))*10000; break;
    case LINKINVIS:
	 ret = (int)(Link.getDontDraw())*10000; break;
    case LINKINVINC:
	 ret = (int)(Link.scriptcoldet)*10000; break;
	case LINKLADDERX:
      ret=(int)(Link.getLadderX())*10000; break;
	case LINKLADDERY:
      ret=(int)(Link.getLadderY())*10000; break;
    case LINKSWORDJINX:
      ret = (int)(Link.getSwordClk())*10000; break;
    case LINKITEMJINX:
      ret = (int)(Link.getItemClk())*10000; break;
    case LINKDRUNK:
      ret = (int)(Link.DrunkClock())*10000; break;
    case LINKMISCD:
    	 ret = (int)(Link.miscellaneous[vbound(*(ri.d[0])/10000,0,15)]); break;
    case LINKHXOFS:
      ret = (int)(Link.hxofs)*10000; break;
    case LINKHYOFS:
      ret = (int)(Link.hyofs)*10000; break;
    case LINKXOFS:
      ret = (int)(Link.xofs)*10000; break;
    case LINKYOFS:
      ret = (int)(Link.yofs-playing_field_offset)*10000; break;
    case LINKZOFS:
      ret = (int)(Link.zofs)*10000; break;
    case LINKHXSZ:
      ret = (int)(Link.hxsz)*10000; break;
    case LINKHYSZ:
      ret = (int)(Link.hysz)*10000; break;
    case LINKHZSZ:
      ret = (int)(Link.hzsz)*10000; break;
    case LINKTXSZ:
      ret = (int)(Link.txsz)*10000; break;
    case LINKTYSZ:
      ret = (int)(Link.tysz)*10000; break;
    case INPUTSTART:
      ret=control_state[6]?10000:0; break;
    case INPUTMAP:
      ret=control_state[9]?10000:0; break;
    case INPUTUP:
      ret=control_state[0]?10000:0; break;
    case INPUTDOWN:
      ret=control_state[1]?10000:0; break;
    case INPUTLEFT:
      ret=control_state[2]?10000:0; break;
    case INPUTRIGHT:
      ret=control_state[3]?10000:0; break;
    case INPUTA:
      ret=control_state[4]?10000:0; break;
    case INPUTB:
      ret=control_state[5]?10000:0; break;
    case INPUTL:
      ret=control_state[7]?10000:0; break;
    case INPUTR:
      ret=control_state[8]?10000:0; break;
    case INPUTEX1:
      ret=control_state[10]?10000:0; break;
    case INPUTEX2:
      ret=control_state[11]?10000:0; break;
    case INPUTEX3:
      ret=control_state[12]?10000:0; break;
    case INPUTEX4:
      ret=control_state[13]?10000:0; break;
    case INPUTAXISUP:
      ret=control_state[14]?10000:0; break;
    case INPUTAXISDOWN:
      ret=control_state[15]?10000:0; break;
    case INPUTAXISLEFT:
      ret=control_state[16]?10000:0; break;
    case INPUTAXISRIGHT:
      ret=control_state[17]?10000:0; break;
    case INPUTMOUSEX:
	  ret = (gui_mouse_x() - scrx - 32 + (sbig ? 128 : (sbig2 ? 192 : 0))) / (resx / 320) * 10000;
	  break;
	  //ret=gui_mouse_x() - scrx - 32;
	  //if(sbig) {
	  //	ret += 128;
  	  //} else if(sbig2) {
	  //	ret += 192;
	  //}
	  //ret /= (resx / 320);
	  //ret *= 10000; break;
      //ret=int((gui_mouse_x()-7)/(resx/320))*10000; break;
    case INPUTMOUSEY:
	  ret = ((gui_mouse_y() - scry - 8 + (sbig ? 112 : (sbig2 ? 168 : 0))) / (resy / 240) - passive_subscreen_height) * 10000;
	  break;
	  //ret /= (resy / 240);
	  //ret -= passive_subscreen_height;
	  //ret *= 10000; break;
      //ret=int((gui_mouse_y()-7)/(resx/240))*10000; break;

    case INPUTMOUSEZ:
      ret=(gui_mouse_z())*10000; break;
    case INPUTMOUSEB:
      ret=(gui_mouse_b())*10000; break;
    case INPUTPRESSSTART:
      ret=button_press[6]?10000:0; break;
    case INPUTPRESSUP:
      ret=button_press[0]?10000:0; break;
    case INPUTPRESSDOWN:
      ret=button_press[1]?10000:0; break;
    case INPUTPRESSLEFT:
      ret=button_press[2]?10000:0; break;
    case INPUTPRESSRIGHT:
      ret=button_press[3]?10000:0; break;
    case INPUTPRESSA:
      ret=button_press[4]?10000:0; break;
    case INPUTPRESSB:
      ret=button_press[5]?10000:0; break;
    case INPUTPRESSL:
      ret=button_press[7]?10000:0; break;
    case INPUTPRESSR:
      ret=button_press[8]?10000:0; break;
    case INPUTPRESSEX1:
      ret=button_press[10]?10000:0; break;
    case INPUTPRESSEX2:
      ret=button_press[11]?10000:0; break;
    case INPUTPRESSEX3:
      ret=button_press[12]?10000:0; break;
    case INPUTPRESSEX4:
      ret=button_press[13]?10000:0; break;
    case INPUTPRESSAXISUP:
      ret=button_press[14]?10000:0; break;
    case INPUTPRESSAXISDOWN:
      ret=button_press[15]?10000:0; break;
    case INPUTPRESSAXISLEFT:
      ret=button_press[16]?10000:0; break;
    case INPUTPRESSAXISRIGHT:
      ret=button_press[17]?10000:0; break;

    case ITEMX:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((int)((item*)(s))->x)*10000;
      }
      break;
    case ITEMY:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((int)((item*)(s))->y)*10000;
      }
      break;
    case ITEMZ:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((int)((item*)(s))->z)*10000;
      }
      break;
    case ITEMJUMP:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=(int)(-(((item*)(s))->fall)/100.0)*10000;
      }
      break;
    case ITEMDRAWTYPE:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->drawstyle*10000;
      }
      break;
    case ITEMID:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->id*10000;
      }
      break;
    case ITEMTILE:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->tile*10000;
      }
      break;
    case ITEMOTILE:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->o_tile*10000;
      }
      break;
    case ITEMCSET:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=(((item*)(s))->o_cset&15)*10000;
      }
      break;
    case ITEMFLASHCSET:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=(((item*)(s))->o_cset>>4)*10000;
      }
      break;
    case ITEMFRAMES:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->frames*10000;
      }
      break;
    case ITEMFRAME:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->aframe*10000;
      }
      break;
    case ITEMASPEED:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->o_speed*10000;
      }
      break;
    case ITEMDELAY:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->o_delay*10000;
      }
      break;
    case ITEMFLIP:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->flip*10000;
      }
      break;
    case ITEMFLASH:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->flash*10000;
      }
      break;
    case ITEMHXOFS:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=(((item*)(s))->hxofs)*10000;
	  }
 	  break;
    case ITEMHYOFS:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=(((item*)(s))->hyofs)*10000;
	  }
 	  break;
    case ITEMXOFS:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=((int)(((item*)(s))->xofs))*10000;
	  }
 	  break;
    case ITEMYOFS:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=((int)(((item*)(s))->yofs-playing_field_offset))*10000;
	  }
 	  break;
    case ITEMZOFS:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=((int)(((item*)(s))->zofs))*10000;
	  }
 	  break;
    case ITEMHXSZ:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=(((item*)(s))->hxsz)*10000;
	  }
 	  break;
    case ITEMHYSZ:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=(((item*)(s))->hysz)*10000;
	  }
 	  break;
    case ITEMHZSZ:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=(((item*)(s))->hzsz)*10000;
	  }
 	  break;
    case ITEMTXSZ:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=(((item*)(s))->txsz)*10000;
	  }
 	  break;
    case ITEMTYSZ:
	  if(0!=(s=checkItem(*ri.itemref)))
	  {
		ret=(((item*)(s))->tysz)*10000;
	  }
 	  break;
    case ITEMCOUNT:
      ret=(items.Count())*10000; break;
    case ITEMEXTEND:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->extend*10000;
      }
      break;
    case ITEMPICKUP:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ret=((item*)(s))->pickup*10000;
      }
      break;
    case ITEMMISCD:
     if(0!=(s=checkItem(*ri.itemref))){
       int a = vbound(*(ri.d[0])/10000,0,15);
       ret=(((item*)(s))->miscellaneous[a]); break;
     }
    case IDATAFAMILY:
      ret=(itemsbuf[*ri.idata].family)*10000; break;
    case IDATALEVEL:
      ret=(itemsbuf[*ri.idata].fam_type)*10000; break;
    case IDATAKEEP:
      ret=(itemsbuf[*ri.idata].flags & ITEM_GAMEDATA)?10000:0; break;
    case IDATAAMOUNT:
      ret=(itemsbuf[*ri.idata].amount)*10000; break;
    case IDATASETMAX:
      ret=(itemsbuf[*ri.idata].setmax)*10000; break;
    case IDATAMAX:
      ret=(itemsbuf[*ri.idata].max)*10000; break;
    case IDATACOUNTER:
      ret=(itemsbuf[*ri.idata].count)*10000; break;
    case IDATAUSESOUND:
      ret=(itemsbuf[*ri.idata].usesound)*10000; break;
    case NPCX:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((int)s->x)*10000;
      }
      break;
    case NPCY:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((int)s->y)*10000;
      }
      break;
    case NPCZ:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((int)s->z)*10000;
      }
      break;
    case NPCJUMP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=(int)(-s->fall/100.0)*100000;
      }
      break;
    case NPCDIR:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=(s->dir)*10000;
      }
      break;
    case NPCRATE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->rate*10000;
      }
      break;
    case NPCHOMING:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
	   ret=((enemy*)(s))->homing*10000;
      }
      break;
    case NPCSTEP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret= (int)(((enemy*)(s))->step*100.0)*10000;
      }
      break;
    case NPCFRAMERATE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->frate*10000;
      }
      break;
    case NPCHALTRATE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->hrate*10000;
      }
      break;
    case NPCDRAWTYPE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->drawstyle*10000;
      }
      break;
    case NPCHP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->hp*10000;
      }
      break;
    case NPCID:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=(((enemy*)(s))->id&0xFFF)*10000;
      }
      break;
    case NPCTYPE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=(((enemy*)(s))->family)*10000;
      }
      break;
    case NPCDP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->dp*10000;
      }
      break;
    case NPCWDP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->wdp*10000;
      }
      break;
    case NPCOTILE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->o_tile*10000;
      }
      break;
    case NPCTILE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->tile*10000;
      }
      break;
    case NPCWEAPON:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->wpn*10000;
      }
      break;
    case NPCITEMSET:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->item_set*10000;
      }
      break;
    case NPCCSET:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->cs*10000;
      }
      break;
    case NPCBOSSPAL:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->bosspal*10000;
      }
      break;
    case NPCBGSFX:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->bgsfx*10000;
      }
      break;
    case NPCCOUNT:
      ret=guys.Count()*10000; break;
    case NPCEXTEND:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ret=((enemy*)(s))->extend*10000;
      }
      break;
    case NPCHXOFS:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=(((enemy*)(s))->hxofs)*10000;
	  }
 	  break;
    case NPCHYOFS:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=(((enemy*)(s))->hyofs)*10000;
	  }
 	  break;
    case NPCXOFS:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=((int)(((enemy*)(s))->xofs))*10000;
	  }
 	  break;
    case NPCYOFS:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=((int)(((enemy*)(s))->yofs-playing_field_offset))*10000;
	  }
 	  break;
    case NPCZOFS:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=((int)(((enemy*)(s))->zofs))*10000;
	  }
 	  break;
    case NPCHXSZ:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=(((enemy*)(s))->hxsz)*10000;
	  }
 	  break;
    case NPCHYSZ:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=(((enemy*)(s))->hysz)*10000;
	  }
 	  break;
    case NPCHZSZ:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=(((enemy*)(s))->hzsz)*10000;
	  }
 	  break;
    case NPCTXSZ:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=(((enemy*)(s))->txsz)*10000;
	  }
 	  break;
    case NPCTYSZ:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  {
		ret=(((enemy*)(s))->tysz)*10000;
	  }
 	  break;
    case NPCDEFENSED:
	  if(0!=(s=checkNPC(*ri.guyref))){
          int a = vbound(*(ri.d[0])/10000,0,edefBYRNA);
		ret=(((enemy*)(s))->defense[a]);
	  }
	  break;
    case NPCMISCD:
	  if(0!=(s=checkNPC(*ri.guyref))){
          int a = vbound(*(ri.d[0])/10000,0,15);
		ret=(((enemy*)(s))->miscellaneous[a]);
	  }
	  break;
    case NPCMFLAGS:
	  if(0!=(s=checkNPC(*ri.guyref))){
	  	flagpos = 5;
		// Must be in the same order as in the Enemy Editor pane
	  	int f = (((enemy*)(s))->flags&0x1F)
	  	  | ornextflag(((enemy*)(s))->flags&(lens_only))
	  	  | ornextflag(((enemy*)(s))->flags2&(guy_flashing))
	  	  | ornextflag(((enemy*)(s))->flags2&(guy_blinking))
	  	  | ornextflag(((enemy*)(s))->flags2&(guy_transparent))
	  	  | ornextflag(((enemy*)(s))->flags&(inv_front))
	  	  | ornextflag(((enemy*)(s))->flags&(inv_left))
	  	  | ornextflag(((enemy*)(s))->flags&(inv_right))
	  	  | ornextflag(((enemy*)(s))->flags&(inv_back))
	  	  | ornextflag(((enemy*)(s))->flags&(guy_bkshield));
		ret = f*10000;
	  }
	break;
	case NPCDD:
	  if(0!=(s=checkNPC(*ri.guyref))){
          int a = vbound(*(ri.d[0])/10000,0,11);
	     switch(a){
			case 0: ret = (((enemy*)(s)))->dmisc1; break;
			case 1: ret = (((enemy*)(s)))->dmisc2; break;
			case 2: ret = (((enemy*)(s)))->dmisc3; break;
			case 3: ret = (((enemy*)(s)))->dmisc4; break;
			case 4: ret = (((enemy*)(s)))->dmisc5; break;
			case 5: ret = (((enemy*)(s)))->dmisc6; break;
			case 6: ret = (((enemy*)(s)))->dmisc7; break;
			case 7: ret = (((enemy*)(s)))->dmisc8; break;
			case 8: ret = (((enemy*)(s)))->dmisc9; break;
			case 9: ret = (((enemy*)(s)))->dmisc10; break;
			case 10: ret = (((enemy*)(s)))->dmisc11; break;
			case 11: ret = (((enemy*)(s)))->dmisc12; break;
		}
		ret *= 10000;
	  }
	  break;
    case NPCCOLLDET:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  	ret = (((enemy*)(s)))->scriptcoldet*10000; break;
    case NPCSTUN:
	  if(0!=(s=checkNPC(*ri.guyref)))
	  	ret = (((enemy*)(s)))->stunclk*10000; break;
    case LWPNX:
	  if(0!=(s=checkLWpn(*ri.lwpn,"X")))
		ret=((int)((weapon*)(s))->x)*10000; break;
    case LWPNY:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Y")))
		ret=((int)((weapon*)(s))->y)*10000; break;
    case LWPNZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Z")))
		ret=((int)((weapon*)(s))->z)*10000; break;
    case LWPNJUMP:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Jump")))
		ret=(int)(-((weapon*)(s))->fall/100.0)*100000; break;
    case LWPNDIR:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Dir")))
		ret=((weapon*)(s))->dir*10000; break;
    case LWPNSTEP:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Step")))
		ret=(int)((float)((weapon*)s)->step * 1000000.0); break;
    case LWPNANGLE:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Angle")))
		ret=(int)(((weapon*)(s))->angle*10000); break;
    case LWPNANGULAR:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Angular")))
		ret=((weapon*)(s))->angular*10000; break;
    case LWPNBEHIND:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Behind")))
		ret=((weapon*)(s))->behind*10000; break;
    case LWPNDRAWTYPE:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DrawType")))
		ret=((weapon*)(s))->drawstyle*10000; break;
    case LWPNPOWER:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Damage")))
		ret=((weapon*)(s))->power*10000; break;
    case LWPNDEAD:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DeadState")))
		ret=((weapon*)(s))->dead*10000; break;
    case LWPNID:
	  if(0!=(s=checkLWpn(*ri.lwpn,"ID")))
		ret=((weapon*)(s))->id*10000; break;
    case LWPNTILE:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Tile")))
		ret=((weapon*)(s))->tile*10000; break;
    case LWPNCSET:
	  if(0!=(s=checkLWpn(*ri.lwpn,"CSet")))
		ret=((weapon*)(s))->cs*10000; break;
    case LWPNFLASHCSET:
	  if(0!=(s=checkLWpn(*ri.lwpn,"FlashCSet")))
		ret=(((weapon*)(s))->o_cset>>4)*10000; break;
    case LWPNFRAMES:
	  if(0!=(s=checkLWpn(*ri.lwpn,"NumFrames")))
		ret=((weapon*)(s))->frames*10000; break;
    case LWPNFRAME:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Frame")))
		ret=((weapon*)(s))->aframe*10000; break;
    case LWPNASPEED:
	  if(0!=(s=checkLWpn(*ri.lwpn,"ASpeed")))
		ret=((weapon*)(s))->o_speed*10000; break;
	case LWPNFLASH:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Flash")))
		ret=((weapon*)(s))->flash*10000; break;
    case LWPNFLIP:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Flip")))
		ret=((weapon*)(s))->flip*10000; break;
    case LWPNCOUNT:
	  ret=Lwpns.Count()*10000; break;
    case LWPNEXTEND:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Extend")))
		ret=((weapon*)(s))->extend*10000; break;
    case LWPNOTILE:
	  if(0!=(s=checkLWpn(*ri.lwpn,"OriginalTile")))
		ret=((weapon*)(s))->o_tile*10000; break;
    case LWPNOCSET:
	  if(0!=(s=checkLWpn(*ri.lwpn,"OriginalCSet")))
		ret=(((weapon*)(s))->o_cset&15)*10000; break;
    case LWPNHXOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitXOffset")))
		ret=(((weapon*)(s))->hxofs)*10000; break;
    case LWPNHYOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitYOffset")))
		ret=(((weapon*)(s))->hyofs)*10000; break;
    case LWPNXOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DrawXOffset")))
		ret=((int)(((weapon*)(s))->xofs))*10000; break;
    case LWPNYOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DrawYOffset")))
		ret=((int)(((weapon*)(s))->yofs-playing_field_offset))*10000; break;
    case LWPNZOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DrawZOffset")))
		ret=((int)(((weapon*)(s))->zofs))*10000; break;
    case LWPNHXSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitWidth")))
		ret=(((weapon*)(s))->hxsz)*10000; break;
    case LWPNHYSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitHeight")))
		ret=(((weapon*)(s))->hysz)*10000; break;
    case LWPNHZSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitZHeight")))
		ret=(((weapon*)(s))->hzsz)*10000; break;
    case LWPNTXSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"TileWidth")))
		ret=(((weapon*)(s))->txsz)*10000; break;
    case LWPNTYSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"TileHeight")))
		ret=(((weapon*)(s))->tysz)*10000; break;
    case LWPNMISCD:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Misc"))){
          int a = vbound(*(ri.d[0])/10000,0,15);
		ret=(((weapon*)(s))->miscellaneous[a]);
	  }
	  break;
    case LWPNCOLLDET:
	  if(0!=(s=checkLWpn(*ri.lwpn,"CollDetection")))
          ret=(((weapon*)(s))->scriptcoldet)*10000;
	  break;
    case EWPNX:
	  if(0!=(s=checkEWpn(*ri.ewpn, "X")))
		ret=((int)((weapon*)(s))->x)*10000; break;
    case EWPNY:
	  if(0!=(s=checkEWpn(*ri.ewpn, "Y")))
		ret=((int)((weapon*)(s))->y)*10000; break;
    case EWPNZ:
	  if(0!=(s=checkEWpn(*ri.ewpn, "Z")))
		ret=((int)((weapon*)(s))->z)*10000; break;
    case EWPNJUMP:
	  if(0!=(s=checkEWpn(*ri.ewpn, "Jump")))
		ret=(int)(-((weapon*)(s))->fall/100.0)*100000; break;
    case EWPNDIR:
	  if(0!=(s=checkEWpn(*ri.ewpn, "Dir")))
		ret=((weapon*)(s))->dir*10000; break;
    case EWPNSTEP:
	  if(0!=(s=checkEWpn(*ri.ewpn, "Step")))
		ret=(int)((float)((weapon*)s)->step * 1000000.0); break;
    case EWPNANGLE:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Angle")))
		ret=(int)(((weapon*)(s))->angle*10000); break;
    case EWPNANGULAR:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Angular")))
		ret=((weapon*)(s))->angular*10000; break;
    case EWPNBEHIND:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Behind")))
		ret=((weapon*)(s))->behind*10000; break;
    case EWPNDRAWTYPE:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DrawType")))
		ret=((weapon*)(s))->drawstyle*10000; break;
    case EWPNPOWER:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Damage")))
		ret=((weapon*)(s))->power*10000; break;
    case EWPNDEAD:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DeadState")))
		ret=((weapon*)(s))->dead*10000; break;
    case EWPNID:
	  if(0!=(s=checkEWpn(*ri.ewpn,"ID")))
		ret=((weapon*)(s))->id*10000; break;
    case EWPNTILE:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Tile")))
		ret=((weapon*)(s))->tile*10000; break;
    case EWPNCSET:
	  if(0!=(s=checkEWpn(*ri.ewpn,"CSet")))
		ret=((weapon*)(s))->cs*10000; break;
    case EWPNFLASHCSET:
	  if(0!=(s=checkEWpn(*ri.ewpn,"FlashCSet")))
		ret=(((weapon*)(s))->o_cset>>4)*10000; break;
    case EWPNFRAMES:
	  if(0!=(s=checkEWpn(*ri.ewpn,"NumFrames")))
		ret=((weapon*)(s))->frames*10000; break;
    case EWPNFRAME:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Frame")))
		ret=((weapon*)(s))->aframe*10000; break;
    case EWPNASPEED:
	  if(0!=(s=checkEWpn(*ri.ewpn,"ASpeed")))
		ret=((weapon*)(s))->o_speed*10000; break;
	case EWPNFLASH:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Flash")))
		ret=((weapon*)(s))->flash*10000; break;
    case EWPNFLIP:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Flip")))
		ret=((weapon*)(s))->flip*10000; break;
    case EWPNCOUNT:
	  ret=Ewpns.Count()*10000; break;
    case EWPNEXTEND:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Extend")))
		ret=((weapon*)(s))->extend*10000; break;
    case EWPNOTILE:
	  if(0!=(s=checkEWpn(*ri.ewpn,"OriginalTile")))
		ret=((weapon*)(s))->o_tile*10000; break;
    case EWPNOCSET:
	  if(0!=(s=checkEWpn(*ri.ewpn,"OriginalCSet")))
		ret=(((weapon*)(s))->o_cset&15)*10000; break;
    case EWPNHXOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitXOffset")))
		ret=(((weapon*)(s))->hxofs)*10000; break;
    case EWPNHYOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitYOffset")))
		ret=(((weapon*)(s))->hyofs)*10000; break;
    case EWPNXOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DrawXOffset")))
		ret=((int)(((weapon*)(s))->xofs))*10000; break;
    case EWPNYOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DrawYOffset")))
		ret=((int)(((weapon*)(s))->yofs-playing_field_offset))*10000; break;
    case EWPNZOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DrawZOffset")))
		ret=((int)(((weapon*)(s))->zofs))*10000; break;
    case EWPNHXSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitWidth")))
		ret=(((weapon*)(s))->hxsz)*10000; break;
    case EWPNHYSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitHeight")))
		ret=(((weapon*)(s))->hysz)*10000; break;
    case EWPNHZSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitZHeight")))
		ret=(((weapon*)(s))->hzsz)*10000; break;
    case EWPNTXSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"TileWidth")))
		ret=(((weapon*)(s))->txsz)*10000; break;
    case EWPNTYSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"TileHeight")))
		ret=(((weapon*)(s))->tysz)*10000; break;
    case EWPNMISCD:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Misc"))){
        int a = vbound(*(ri.d[0])/10000,0,15);
		ret=(((weapon*)(s))->miscellaneous[a]);
	  }
	  break;
    case EWPNCOLLDET:
	  if(0!=(s=checkEWpn(*ri.ewpn,"CollDetection")))
          ret=(((weapon*)(s))->scriptcoldet)*10000;
	  break;
    case GAMEDEATHS:
      ret=game->get_deaths()*10000; break;
    case GAMECHEAT:
      ret=game->get_cheat()*10000; break;
    case GAMETIME:
      ret=game->get_time();  break;// Can't multiply by 10000 or the maximum result is too big
    case GAMETIMEVALID:
      ret=game->get_timevalid()?10000:0; break;
    case GAMEHASPLAYED:
      ret=game->get_hasplayed()?10000:0; break;
    case GAMEGUYCOUNT:
      mi = (currmap*MAPSCRSNORMAL)+currscr;
      ret=game->guys[mi]*10000; break;
    case GAMECONTSCR:
      ret=game->get_continue_scrn()*10000; break;
    case GAMECONTDMAP:
      ret=game->get_continue_dmap()*10000; break;
    case GAMECOUNTERD:
      ret=game->get_counter((*(ri.d[0]))/10000)*10000; break;
    case GAMEMCOUNTERD:
      ret=game->get_maxcounter((*(ri.d[0]))/10000)*10000; break;
    case GAMEDCOUNTERD:
      ret=game->get_dcounter((*(ri.d[0]))/10000)*10000; break;
    case GAMEGENERICD:
      ret=game->get_generic((*(ri.d[0]))/10000)*10000; break;
    case GAMEITEMSD:
      ret=(game->item[(*(ri.d[0]))/10000] ? 10000 : 0); break;
    case GAMELITEMSD:
      ret=game->lvlitems[(*(ri.d[0]))/10000]*10000; break;
    case GAMELKEYSD:
      ret=game->lvlkeys[(*(ri.d[0]))/10000]*10000; break;
    case SCREENSTATED:
      mi =(currmap*MAPSCRSNORMAL)+currscr;
      ret=((game->maps[mi]>>((*(ri.d[0]))/10000))&1)?10000:0; break;
    case SCREENSTATEDD:
      ret=((game->maps[(*(ri.d[0]))/10000-8*(((*(ri.d[0]))/10000)>>7)]>>((*(ri.d[1]))/10000))&1)?10000:0; break;
    case GAMEGUYCOUNTD:
      ret=game->guys[*(ri.d[0])/10000]*10000; break;
    case CURMAP:
      ret=(1+currmap)*10000; break;
    case CURSCR:
      ret=currscr*10000; break;
    case GETMIDI:
      ret=(currmidi-(ZC_MIDI_COUNT-1))*10000; break;
    case CURDSCR:
      di = (get_currscr()-DMaps[get_currdmap()].xoff);
      ret=(DMaps[get_currdmap()].type==dmOVERW ? currscr : di)*10000; break;
    case CURDMAP:
      ret=currdmap*10000; break;
    case CURLEVEL:
      ret=DMaps[get_currdmap()].level*10000; break;
    case DMAPFLAGSD:
      ret=DMaps[vbound((*ri.d[0])/10000,0,512)].flags*10000; break;
    case COMBODD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
          ret=tmpscr->data[pos]*10000;
	else
	  ret = 10000;
      }
      break;
    case COMBOCD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
          ret=tmpscr->cset[pos]*10000;
	else
	  ret = 10000;
      }
      break;
    case COMBOFD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
          ret=tmpscr->sflag[pos]*10000;
	else
	  ret = 10000;
      }
      break;
    case COMBOTD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
          ret=combobuf[tmpscr->data[pos]].type*10000;
	else
	  ret = 10000;
      }
      break;
    case COMBOID:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
          ret=combobuf[tmpscr->data[pos]].flag*10000;
	else
	  ret = 10000;
      }
      break;
    case COMBOSD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos < 0 || pos >= 176)
            ret = 10000;
        else
	    ret=(combobuf[tmpscr->data[pos]].walk&15)*10000;
      }
      break;
    case COMBODDM:
      {
        int pos = (*(ri.d[0]))/10000;
		int sc = ((*(ri.d[2]))/10000);
		int m = zc_max(((*(ri.d[1]))/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
		int layr = whichlayer(scr);
        if (pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
		{
	      if (scr==(currmap*MAPSCRS+currscr))
	        ret=tmpscr->data[pos]*10000;
          else if (layr>-1)
		    ret=tmpscr2[layr].data[pos]*10000;
	      else ret=TheMaps[scr].data[pos]*10000;
		}
		else
	      ret = 10000;
      }
      break;
    case COMBOCDM:
      {
        int pos = (*(ri.d[0]))/10000;
		int sc = ((*(ri.d[2]))/10000);
		int m = zc_max(((*(ri.d[1]))/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
		int layr = whichlayer(scr);
        if (pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
		{
	      if (scr==(currmap*MAPSCRS+currscr))
	        ret=tmpscr->cset[pos]*10000;
          else if (layr>-1)
		    ret=tmpscr2[layr].cset[pos]*10000;
	      else ret=TheMaps[scr].cset[pos]*10000;
		}
		else
	      ret = 10000;
      }
      break;
    case COMBOFDM:
      {
        int pos = (*(ri.d[0]))/10000;
		int sc = ((*(ri.d[2]))/10000);
		int m = zc_max(((*(ri.d[1]))/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
		int layr = whichlayer(scr);
        if (pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
		{
	      if (scr==(currmap*MAPSCRS+currscr))
	        ret=tmpscr->sflag[pos]*10000;
          else if (layr>-1)
		    ret=tmpscr2[layr].sflag[pos]*10000;
	      else ret=TheMaps[scr].sflag[pos]*10000;
		}
		else
	      ret = 10000;
      }
      break;
    case COMBOTDM:
      {
        int pos = (*(ri.d[0]))/10000;
		int sc = ((*(ri.d[2]))/10000);
		int m = zc_max(((*(ri.d[1]))/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
		int layr = whichlayer(scr);
        if (pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
		{
	      if (scr==(currmap*MAPSCRS+currscr))
	        ret=combobuf[tmpscr->data[pos]].type*10000;
          else if (layr>-1)
		    ret=combobuf[tmpscr2[layr].data[pos]].type*10000;
	      else ret=combobuf[
			TheMaps[scr]
			.data[pos]].type*10000;
		}
		else
	      ret = 10000;
      }
      break;
    case COMBOIDM:
      {
        int pos = (*(ri.d[0]))/10000;
		int sc = ((*(ri.d[2]))/10000);
		int m = zc_max(((*(ri.d[1]))/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
		int layr = whichlayer(scr);
        if (pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
		{
	      if (scr==(currmap*MAPSCRS+currscr))
	        ret=combobuf[tmpscr->data[pos]].flag*10000;
          else if (layr>-1)
		    ret=combobuf[tmpscr2[layr].data[pos]].flag*10000;
	      else ret=combobuf[TheMaps[scr].data[pos]].flag*10000;
		}
		else
	      ret = 10000;
      }
      break;
    case COMBOSDM:
      {
        int pos = (*(ri.d[0]))/10000;
		int sc = ((*(ri.d[2]))/10000);
		int m = zc_max(((*(ri.d[1]))/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
		int layr = whichlayer(scr);
        if (pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
		{
	      if (scr==(currmap*MAPSCRS+currscr))
	        ret=(combobuf[tmpscr->data[pos]].walk&15)*10000;
          else if (layr>-1)
		    ret=(combobuf[tmpscr2[layr].data[pos]].walk&15)*10000;
	      else ret=(combobuf[TheMaps[scr].data[pos]].walk&15)*10000;
		}
		else
	      ret = 10000;
      }
      break;
    case REFFFC:
      ret=(*ri.ffc)*10000; break;
    case REFITEM:
      ret=(*ri.itemref); break;
    case REFITEMCLASS:
      ret=(*ri.idata);
      break;
    case REFLWPN:
      ret=(*ri.lwpn); break;
    case REFLWPNCLASS:
      ret=(*ri.lclass); break;
    case REFEWPN:
      ret=(*ri.ewpn); break;
    case REFEWPNCLASS:
      ret=(*ri.eclass); break;
    case REFNPC:
      ret=(*ri.guyref); break;
    case REFNPCCLASS:
      ret=(*ri.gclass); break;
    case SDD:
      di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
      ret=get_screen_d(di, *(ri.d[0])/10000); break;
    case GDD:
      ret=game->global_d[*(ri.d[0])/10000]; break;
    case SDDD:
      ret=get_screen_d((*(ri.d[0]))/10000 + ((get_currdmap())<<7), *(ri.d[1])/10000); break;
    case SDDDD:
      ret=get_screen_d((*(ri.d[1]))/10000 + ((*(ri.d[0])/10000)<<7), *(ri.d[2])/10000); break;
    case SCRDOORD:
      ret=tmpscr->door[*(ri.d[0])/10000]*10000; break;
    case LIT:
      ret= darkroom ? 0 : 10000; break;
    case WAVY:
      ret=wavy*10000; break;
    case QUAKE:
      ret=quakeclk*10000; break;
    case SCREENFLAGSD:
	 ret = get_screenflags(tmpscr,vbound(*ri.d[0]/10000,0,9)); break;;
    case SCREENEFLAGSD:
	 ret = get_screeneflags(tmpscr,vbound(*ri.d[0]/10000,0,2)); break;
    case SP:
      ret = (*ri.sp)*10000; break;
	case SCRIPTRAM: case GLOBALRAM:
	  ret = ScriptHelper::getElement(*(ri.d[0])/10000,*(ri.d[1])/10000); break;
	case SCRIPTRAMD: case GLOBALRAMD:
	  ret = ScriptHelper::getElement(*(ri.d[0])/10000,0); break;

	case SAVERAM:
	  if((*(ri.d[0])/10000) < 0 || (*(ri.d[0])/10000) >= 0x2000) break;
	  ret = game->savedata[*(ri.d[0])/10000]; break;
    default:
    {
      int k;
      if(arg>=D(0)&&arg<=D(7))
      {
        k=arg-D(0); ret=*(ri.d[k]); break;
      }
      else if(arg>=A(0)&&arg<=A(1))
      {
        k=arg-A(0); if(script_type!=SCRIPT_GLOBAL) ret=*(ri.a[k]); break;
      }
      else if(arg>=SD(0)&&arg<=SD(7))
      {
        di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
        k=arg-SD(0); ret=get_screen_d(di,k); break;
      }
      else if(arg>=GD(0)&&arg<=GD(255))
      {
        k=arg-GD(0); ret=game->global_d[k]; break;
      }
      else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMECOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
      {
        k=(arg-GAMECOUNTER(0))/3; ret=game->get_counter(k)*10000; //break;
      }
      else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMEMCOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
      {
        k=(arg-GAMECOUNTER(0))/3; ret=game->get_maxcounter(k)*10000; //break;
      }
      else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMEDCOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
      {
        k=(arg-GAMECOUNTER(0))/3; ret=game->get_dcounter(k)*10000; //break;
      }
      else if(arg>=SCREENSTATE(0)&&arg<=SCREENSTATE(31))
      {
        mi = (currmap*MAPSCRSNORMAL)+currscr;
        k=arg-SCREENSTATE(0); ret=((game->maps[mi]>>k)&1)?10000:0; //break;
      }
      else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOD(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
      {
        k=(arg-COMBOD(0))/3; ret=tmpscr->data[k]*10000; //break;
      }
      else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOC(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
      {
        k=(arg-COMBOD(0))/3; ret=tmpscr->cset[k]*10000; //break;
      }
      else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOF(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
      {
        k=(arg-COMBOD(0))/3; ret=tmpscr->sflag[k]*10000; //break;
      }
      else if(arg>=GAMEGENERIC(0)&&arg<=GAMEGENERIC(255))
      {
        k=arg-GAMEGENERIC(0); ret=game->get_generic(k)*10000; //break;
      }
      else if(arg>=GAMEITEMS(0)&&arg<=GAMEITEMS(255))
      {
        k=arg-GAMEITEMS(0); ret=(game->item[k] ? 10000 : 0); //break;
      }
      else if(arg>=GAMELITEMS(0)&&arg<=GAMELITEMS(255))
      {
        k=arg-GAMELITEMS(0); ret=game->lvlitems[k]*10000; //break;
      }
      else if(arg>=GAMELKEYS(0)&&arg<=GAMELKEYS(255))
      {
        k=arg-GAMELKEYS(0); ret=game->lvlkeys[k]*10000; //break;
      }
	 /*else if(arg>=DMAPFLAGS(0)&&arg<=DMAPFLAGS(511))
      {
	   k=arg-DMAPFLAGS(0); ret=DMaps[vbound(k,0,511)].flags*10000; //break;
      }*/
      else if(arg>=SCRDOOR(0)&&arg<=SCRDOOR(3))
      {
        k=arg-SCRDOOR(0); ret=tmpscr->door[k]*10000; //break;
      }
	 else if(arg>=LWPNMISC(0)&&arg<=LWPNMISC(15))
      {
           if(0!=(s=checkLWpn(*ri.lwpn,"Misc"))){
               k=arg-LWPNMISC(0); int a = vbound(k,0,15);
               ret=(((weapon*)(s)))->miscellaneous[a];
           }
           break;
      }
	 else if(arg>=EWPNMISC(0)&&arg<=EWPNMISC(15))
      {
           if(0!=(s=checkEWpn(*ri.ewpn,"Misc"))){
               k=arg-EWPNMISC(0); int a = vbound(k,0,15);
               ret=(((weapon*)(s)))->miscellaneous[a];
           }
           break;
      }
	 else if(arg>=NPCMISC(0)&&arg<=NPCMISC(15))
      {
           if(0!=(s=checkNPC(*ri.guyref))){
               k=arg-NPCMISC(0); int a = vbound(k,0,15);
               ret=(((enemy*)(s)))->miscellaneous[a];
           }
           break;
      }
	 else if(arg>=ITEMMISC(0)&&arg<=ITEMMISC(15))
      {
           if(0!=(s=checkItem(*ri.itemref))){
               k=arg-ITEMMISC(0); int a = vbound(k,0,15);
               ret=(((item*)(s)))->miscellaneous[a];
           }
           break;
      }
      else if(arg>=FFMISC(0)&&arg<=FFMISC(15))
      {
        k=arg-FFMISC(0); int a = vbound(k,0,15);
        ret=(tmpscr->ffmisc[*ri.ffc][a]);
        break;
      }
	 else if(arg>=LINKMISC(0)&&arg<=LINKMISC(15))
      {
		 k = arg-LINKMISC(0);
		 ret=Link.miscellaneous[vbound(k,0,15)];
           break;
      }
      else if(arg>=FFINITD(0)&&arg<=FFINITD(7))
      {
        ret=(tmpscr->initd[*ri.ffc][vbound(arg-FFINITD(0),0,7)])*10000;
        break;
      }/*
      else if(arg>=FFD(0)&&arg<=FFD(7))
      {
        ret=(tmpscr->d[*ri.ffc][vbound(arg-FFD(0),0,7)])*10000;
        break;
      }*/
	  else if(arg>=NPCD(0)&&arg<=NPCD(11))
      {
           if(0!=(s=checkNPC(*ri.guyref))){
               k=arg-NPCD(0); int a = vbound(k,0,11);
               switch(a){
				case 0: ret = (((enemy*)(s)))->dmisc1; break;
				case 1: ret = (((enemy*)(s)))->dmisc2; break;
				case 2: ret = (((enemy*)(s)))->dmisc3; break;
				case 3: ret = (((enemy*)(s)))->dmisc4; break;
				case 4: ret = (((enemy*)(s)))->dmisc5; break;
				case 5: ret = (((enemy*)(s)))->dmisc6; break;
				case 6: ret = (((enemy*)(s)))->dmisc7; break;
				case 7: ret = (((enemy*)(s)))->dmisc8; break;
				case 8: ret = (((enemy*)(s)))->dmisc9; break;
				case 9: ret = (((enemy*)(s)))->dmisc10; break;
				case 10: ret = (((enemy*)(s)))->dmisc11; break;
				case 11: ret = (((enemy*)(s)))->dmisc12; break;
               }
               ret *= 10000;
           }
        break;
      }
      else if(arg>=SCREENFLAGS(0)&&arg<=SCREENFLAGS(9))
      {
      	k = arg-SCREENFLAGS(0);
	     ret = get_screenflags(tmpscr,vbound(k,0,9)); break;
      }
      else if(arg>=SCREENEFLAGS(0)&&arg<=SCREENEFLAGS(2))
      {
      	k = arg-SCREENEFLAGS(0);
	     ret = get_screeneflags(tmpscr,vbound(k,0,2)); break;
      }
      break;
    }
  }
  return ret;
}

int ornextflag(bool flag){
	int f = (flag?1:0)<<flagpos;
	flagpos++;
	return f;
}

int get_screenflags(mapscr *m, int flagset){
	int f=0; flagpos = 0;
	switch(flagset){
		case 0: // Room Type
		f = ornextflag(m->flags6&1)
		  | ornextflag(m->flags6&2)
		  | ornextflag(m->flags7&8);
			break;
		case 1: // View
		f = ornextflag(m->flags3&8)
		  | ornextflag(m->flags7&16)
		  | ornextflag(m->flags3&16)
		  | ornextflag(m->flags3&64)
		  | ornextflag(m->flags7&2)
		  | ornextflag(m->flags7&1)
		  | ornextflag(m->flags&4);
			break;
		case 2: // Secrets
		f = ornextflag(m->flags&1)
		  | ornextflag(m->flags5&16)
		  | ornextflag(m->flags6&4)
		  | ornextflag(m->flags6&32);
			break;
		case 3: // Warp
		f = ornextflag(m->flags5&4)
		  | ornextflag(m->flags5&8)
		  | ornextflag(m->flags&64)
		  | ornextflag(m->flags8&64)
		  | ornextflag(m->flags3&32);
			break;
		case 4: // Item
		f = ornextflag(m->flags3&1)
		  | ornextflag(m->flags7&4);
			break;
		case 5: // Combo
		f = ornextflag((m->flags2>>4)&2)
		  | ornextflag(m->flags3&2)
		  | ornextflag(m->flags5&2)
		  | ornextflag(m->flags6&64);
			break;
		case 6: // Save
		f = ornextflag(m->flags4&64)
		  | ornextflag(m->flags4&128)
		  | ornextflag(m->flags6&8)
		  | ornextflag(m->flags6&16);
			break;
		case 7: // FFC
		f = ornextflag(m->flags6&128)
		  | ornextflag(m->flags5&128);
			break;
		case 8: // Whistle
		f = ornextflag(m->flags&16)
		  | ornextflag(m->flags7&64)
		  | ornextflag(m->flags7&128);
			break;
		case 9: // Misc
		f = ornextflag(m->flags&32)
		  | ornextflag(m->flags5&64)
		  | m->flags8<<2;
			break;
	}
	return f*10000;
}

int get_screeneflags(mapscr *m, int flagset){
	int f=0; flagpos = 0;
	switch(flagset){
		case 0: f = m->enemyflags&0x1F; break;
		case 1:
		f = ornextflag(m->enemyflags&32)
		  | ornextflag(m->enemyflags&64)
		  | ornextflag(m->flags3&4)
		  | ornextflag(m->enemyflags&128)
		  | ornextflag((m->flags2>>4)&4);
			break;
		case 2:
		f = ornextflag(m->flags3&128)
		  | ornextflag(m->flags&2)
		  | ornextflag((m->flags2>>4)&8)
		  | ornextflag(m->flags4&16);
			break;
	}
	return f*10000;
}

void set_variable(int arg, byte, long value, refInfo &ri)
{

  switch(arg)
  {
    case DATA:
      tmpscr->ffdata[*ri.ffc]=vbound(value/10000,0,MAXCOMBOS-1); break;
    case FCSET:
      tmpscr->ffcset[*ri.ffc]=(value/10000)&15; break;
    case DELAY:
      tmpscr->ffdelay[*ri.ffc]=value/10000; break;
    case FX:
      tmpscr->ffx[*ri.ffc]=value; break;
    case FY:
      tmpscr->ffy[*ri.ffc]=value; break;
    case XD:
      tmpscr->ffxdelta[*ri.ffc]=value; break;
    case YD:
      tmpscr->ffydelta[*ri.ffc]=value; break;
    case XD2:
      tmpscr->ffxdelta2[*ri.ffc]=value; break;
    case YD2:
      tmpscr->ffydelta2[*ri.ffc]=value; break;
    case FFFLAGSD:
      value ? tmpscr->ffflags[*ri.ffc] |=   1<<((*(ri.d[0]))/10000)
            : tmpscr->ffflags[*ri.ffc] &= ~(1<<((*(ri.d[0]))/10000)); break;
    case FFCWIDTH:
      tmpscr->ffwidth[*ri.ffc]= (tmpscr->ffwidth[*ri.ffc] & ~63) | (((value/10000)-1)&63); break;
    case FFCHEIGHT:
      tmpscr->ffheight[*ri.ffc]= (tmpscr->ffheight[*ri.ffc] & ~63) | (((value/10000)-1)&63); break;
    case FFTWIDTH:
      tmpscr->ffwidth[*ri.ffc]= (tmpscr->ffwidth[*ri.ffc]&63) | ((((value/10000)-1)&3)<<6); break;
    case FFTHEIGHT:
      tmpscr->ffheight[*ri.ffc]=(tmpscr->ffheight[*ri.ffc]&63) | ((((value/10000)-1)&3)<<6); break;
    case FFLINK:
      (tmpscr->fflink[*ri.ffc])=vbound(value/10000,1,32); break;
    case FFMISCD:{
       int a = vbound(*(ri.d[0])/10000,0,15);
      (tmpscr->ffmisc[*ri.ffc][a])=value;
      break;
    }
    case FFINITDD:
      (tmpscr->initd[*ri.ffc][vbound(*(ri.d[0])/10000,0,7)])=value/10000; break;
    /*case FFDD:
      (tmpscr->d[*ri.ffc][vbound(*(ri.d[0])/10000,0,7)])=value/10000; break;*/
    case LINKX:
      Link.setX(value/10000); break;
    case LINKY:
      Link.setY(value/10000); break;
    case LINKZ:
      Link.setZ(value/10000); break;
    case LINKJUMP:
      Link.setFall(fix((-value*100.0)/10000.0)); break;
    case LINKDIR:
      Link.setDir(value/10000); break;
    case LINKHP:
      game->set_life(zc_max(0, zc_min(value/10000,game->get_maxlife()))); break;
    case LINKMP:
      game->set_magic(zc_max(0, zc_min(value/10000,game->get_maxmagic()))); break;
    case LINKMAXHP:
      game->set_maxlife(value/10000); break;
    case LINKMAXMP:
      game->set_maxmagic(value/10000); break;
    case LINKACTION:
      Link.setAction((actiontype)(value/10000)); break;
    case LINKHELD:
      Link.setHeldItem(vbound(value/10000,0,MAXITEMS-1)); break;
    case LINKITEMD:
      game->set_item(vbound((*ri.d[0])/10000,0,MAXITEMS-1),(value != 0));
      //resetItems(game); - Is this really necessary? ~Joe123
	 if((get_bit(quest_rules,qr_OVERWORLDTUNIC) != 0) || (currscr<128 || dlevel)) ringcolor(false);
      break;
    case LINKEQUIP:
      {
		/*
		int setb = ((value/10000)&0xFF00)>>8, seta = (value/10000)&0xFF;
		if(seta && get_bit(quest_rules,qr_SELECTAWPN) && game->item[seta]){

		}
		if(setb && game->item[setb]){
		}*/
      }
	 break;
    case LINKINVIS:
	 Link.setDontDraw(value/10000); break;
    case LINKINVINC:
	 Link.scriptcoldet=(value/10000); break;
    case LINKSWORDJINX:
      Link.setSwordClk(value/10000); break;
    case LINKITEMJINX:
      Link.setItemClk(value/10000); break;
    case LINKDRUNK:
      Link.setDrunkClock(value/10000); break;
    case LINKMISCD:
	 Link.miscellaneous[vbound(*(ri.d[0])/10000,0,15)] = value; break;
    case LINKHXOFS:
      (Link.hxofs)=(fix)(value/10000); break;
    case LINKHYOFS:
      (Link.hyofs)=(fix)(value/10000); break;
    case LINKXOFS:
      (Link.xofs)=(fix)(value/10000); break;
    case LINKYOFS:
      (Link.yofs)=(fix)(value/10000)+playing_field_offset; break;
    case LINKZOFS:
      (Link.zofs)=(fix)(value/10000); break;
    case LINKHXSZ:
      (Link.hxsz)=(fix)(value/10000); break;
    case LINKHYSZ:
      (Link.hysz)=(fix)(value/10000); break;
    case LINKHZSZ:
      (Link.hzsz)=(fix)(value/10000); break;
    case LINKTXSZ:
      (Link.txsz)=(fix)(value/10000); break;
    case LINKTYSZ:
      (Link.tysz)=(fix)(value/10000); break;
    case INPUTSTART:
      control_state[6]=((value/10000)!=0)?true:false; break;
    case INPUTMAP:
      control_state[9]=((value/10000)!=0)?true:false; break;
    case INPUTUP:
      control_state[0]=((value/10000)!=0)?true:false; break;
    case INPUTDOWN:
      control_state[1]=((value/10000)!=0)?true:false; break;
    case INPUTLEFT:
      control_state[2]=((value/10000)!=0)?true:false; break;
    case INPUTRIGHT:
      control_state[3]=((value/10000)!=0)?true:false; break;
    case INPUTA:
      control_state[4]=((value/10000)!=0)?true:false; break;
    case INPUTB:
      control_state[5]=((value/10000)!=0)?true:false; break;
    case INPUTL:
      control_state[7]=((value/10000)!=0)?true:false; break;
    case INPUTR:
      control_state[8]=((value/10000)!=0)?true:false; break;
	case INPUTEX1:
	  control_state[10]=((value/10000)!=0)?true:false; break;
    case INPUTEX2:
	  control_state[11]=((value/10000)!=0)?true:false; break;
    case INPUTEX3:
	  control_state[12]=((value/10000)!=0)?true:false; break;
    case INPUTEX4:
	  control_state[13]=((value/10000)!=0)?true:false; break;
    case INPUTAXISUP:
	  control_state[14]=((value/10000)!=0)?true:false; break;
    case INPUTAXISDOWN:
	  control_state[15]=((value/10000)!=0)?true:false; break;
    case INPUTAXISLEFT:
	  control_state[16]=((value/10000)!=0)?true:false; break;
	case INPUTAXISRIGHT:
	  control_state[17]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSSTART:
      button_press[6]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSUP:
      button_press[0]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSDOWN:
      button_press[1]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSLEFT:
      button_press[2]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSRIGHT:
      button_press[3]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSA:
      button_press[4]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSB:
      button_press[5]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSL:
      button_press[7]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSR:
      button_press[8]=((value/10000)!=0)?true:false; break;
	case INPUTPRESSEX1:
	  button_press[10]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSEX2:
	  button_press[11]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSEX3:
	  button_press[12]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSEX4:
	  button_press[13]=((value/10000)!=0)?true:false; break;
	case INPUTPRESSAXISUP:
	  button_press[14]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSAXISDOWN:
	  button_press[15]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSAXISLEFT:
	  button_press[16]=((value/10000)!=0)?true:false; break;
    case INPUTPRESSAXISRIGHT:
	  button_press[17]=((value/10000)!=0)?true:false; break;
    case INPUTMOUSEX:
	  // this fixes the origin to the top-left corner of the playing field (tile 0)
	  position_mouse(32 + scrx + (value / 10000 * (resx / 320)) - sbig ? 128 : (sbig2 ? 192 : 0), gui_mouse_y()); break;
	  //position_mouse(((value/10000)+3)*(resx/320), gui_mouse_y()); break;
    case INPUTMOUSEY:
	  position_mouse(gui_mouse_x(), 8 + scry + (value / 10000 * (resy / 240)) - sbig ? 112 : (sbig2 ? 168 : 0)); break;
	  //position_mouse(gui_mouse_x(),((value/10000)+3)*(resy/240)); break;
    case INPUTMOUSEZ:
      position_mouse_z(value/10000); break;
    case ITEMX:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (s->x)=(fix)(value/10000);
        // Move the Fairy enemy as well.
        if (itemsbuf[((item*)(s))->id].family==itype_fairy && itemsbuf[((item*)(s))->id].misc3)
          movefairy2(((item*)(s))->x,((item*)(s))->y,((item*)(s))->misc);
      }
      break;
    case ITEMY:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (s->y)=(fix)(value/10000);
        // Move the Fairy enemy as well.
        if (itemsbuf[((item*)(s))->id].family==itype_fairy && itemsbuf[((item*)(s))->id].misc3)
          movefairy2(((item*)(s))->x,((item*)(s))->y,((item*)(s))->misc);
      }
      break;
    case ITEMZ:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (s->z)=(fix)(value/10000);
        if(s->z < 0)
          s->z = 0;
      }
      break;
    case ITEMJUMP:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->fall)=-value*100.0/10000.0;
      }
      break;
    case ITEMDRAWTYPE:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->drawstyle)=value/10000;
      }
      break;
    case ITEMID:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->id)=value/10000;
        flushItemCache();
      }
      break;
    case ITEMTILE:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->tile)=vbound(value/10000,0,NEWMAXTILES-1);
      }
      break;
    case ITEMOTILE:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->o_tile)=vbound(value/10000,0,NEWMAXTILES-1);
      }
      break;
    case ITEMCSET:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->o_cset) = (((item *)s)->o_cset & ~15) | ((value/10000)&15);
        (((item *)s)->cs) = (((item *)s)->o_cset & 15);
      }
      break;
    case ITEMFLASHCSET:
      if(0!=(s=checkItem(*ri.itemref)))
      {
		  (((item *)s)->o_cset) = ((value/10000)<<4) | (((item *)s)->o_cset & 15);
      }
      break;
    case ITEMFRAMES:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->frames)=value/10000;
      }
      break;
    case ITEMFRAME:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->aframe)=value/10000;
      }
      break;
    case ITEMASPEED:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->o_speed)=value/10000;
      }
      break;
    case ITEMDELAY:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->o_delay)=value/10000;
      }
      break;
    case ITEMFLIP:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->flip)=value/10000;
      }
      break;
    case ITEMFLASH:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->flash)= (value/10000)?1:0;
      }
      break;
    case ITEMEXTEND:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        (((item *)s)->extend)=value/10000;
      }
      break;
    case ITEMHXOFS:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->hxofs=value/10000;
      }
      break;
    case ITEMHYOFS:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->hyofs=value/10000;
      }
      break;
    case ITEMXOFS:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->xofs=(fix)(value/10000);
      }
      break;
    case ITEMYOFS:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->yofs=(fix)(value/10000)+playing_field_offset;
      }
      break;
    case ITEMZOFS:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->zofs=(fix)(value/10000);
      }
      break;
    case ITEMHXSZ:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->hxsz=value/10000;
      }
      break;
    case ITEMHYSZ:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->hysz=value/10000;
      }
      break;
    case ITEMHZSZ:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->hzsz=value/10000;
      }
      break;
    case ITEMTXSZ:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->txsz=vbound((value/10000),1,20);
      }
      break;
    case ITEMTYSZ:
      if(0!=(s=checkItem(*ri.itemref)))
      {
        ((item*)(s))->tysz=vbound((value/10000),1,20);
      }
      break;
    case ITEMPICKUP:
      if(0!=(s=checkItem(*ri.itemref)))
      {
	int newpickup = value/10000;
	// Values that the questmaker should not use, ever
	newpickup &= ~(ipBIGRANGE | ipCHECK | ipMONEY | ipBIGTRI | ipNODRAW | ipFADE);

	// If making an item timeout, set its timer
	if (newpickup & ipFADE)
	{
	  (((item*)(s))->clk2) = 512;
	}

	// If making it a carried item,
	// alter hasitem and set an itemguy.
	if ((((item*)(s))->pickup & ipENEMY) < (newpickup & ipENEMY))
	{
	  hasitem |= 2;
	  bool hasitemguy = false;

	  for(int i=0; i<guys.Count(); i++)
	  {
	    if(((enemy*)guys.spr(i))->itemguy)
	    {
	      hasitemguy = true;
	    }
	  }
	  if (!hasitemguy && guys.Count()>0)
	  {
	    ((enemy*)guys.spr(guys.Count()-1))->itemguy = true;
	  }
	}
	// If unmaking it a carried item,
	// alter hasitem if there are no more carried items.
	else if ((((item*)(s))->pickup & ipENEMY) > (newpickup & ipENEMY))
	{
	  // Move it back onscreen!
	  if (get_bit(quest_rules,qr_HIDECARRIEDITEMS))
	  {
	    for(int i=0; i<guys.Count(); i++)
	    {
	      if(((enemy*)guys.spr(i))->itemguy)
	      {
	        ((item*)(s))->x = ((enemy*)guys.spr(i))->x;
	        ((item*)(s))->y = ((enemy*)guys.spr(i))->y;
	        ((item*)(s))->z = ((enemy*)guys.spr(i))->z;
		break;
	      }
	    }
	  }

	  if (more_carried_items()<=1) // 1 includes this own item.
	  {
	    hasitem &= ~2;
	  }
	}
        ((item*)(s))->pickup=value/10000;
      }
      break;
     case ITEMMISCD:
	  if(0!=(s=checkItem(*ri.itemref))){
          int a = vbound(*(ri.d[0])/10000,0,15);
          (((item*)(s))->miscellaneous[a])=value;
	  }
	  break;
    case IDATAFAMILY:
      (itemsbuf[*ri.idata].family)=value/10000;
      flushItemCache();
      break;
    case IDATALEVEL:
      (itemsbuf[*ri.idata].fam_type)=value/10000;
      flushItemCache();
      break;
    case IDATAKEEP:
      (itemsbuf[*ri.idata].flags)|=(value/10000)?ITEM_GAMEDATA:0; break;
    case IDATAAMOUNT:
      (itemsbuf[*ri.idata].amount)=value/10000; break;
    case IDATASETMAX:
      (itemsbuf[*ri.idata].setmax)=value/10000; break;
    case IDATAMAX:
      (itemsbuf[*ri.idata].max)=value/10000; break;
    case IDATACOUNTER:
      (itemsbuf[*ri.idata].count)=value/10000; break;
    case IDATAUSESOUND:
      (itemsbuf[*ri.idata].usesound)=value/10000; break;
    case LWPNX:
	  if(0!=(s=checkLWpn(*ri.lwpn,"X")))
      ((weapon*)s)->x=(fix)(value/10000); break;
    case LWPNY:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Y")))
      ((weapon*)s)->y=(fix)(value/10000); break;
	case LWPNZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Z")))
      ((weapon*)s)->z=zc_max((fix)(value/10000),(fix)0);
      break;
    case LWPNJUMP:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Jump")))
      ((weapon*)s)->fall=((-value*100.0)/10000.0); break;
	case LWPNDIR:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Dir")))
      ((weapon*)s)->dir=(value/10000); break;
    case LWPNSTEP:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Step")))
      ((weapon*)s)->step=(value/10000)/100.0; break;
	case LWPNANGLE:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Angle")))
      ((weapon*)s)->angle=(double)(value/10000.0); break;
    case LWPNANGULAR:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Angular")))
      ((weapon*)s)->angular=(value/10000) != 0; break;
    case LWPNBEHIND:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Behind")))
      ((weapon*)s)->behind=(value/10000) != 0; break;
    case LWPNDRAWTYPE:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DrawType")))
      ((weapon*)s)->drawstyle=(value/10000); break;
    case LWPNPOWER:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Damage")))
      ((weapon*)s)->power=(value/10000); break;
    case LWPNDEAD:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DeadState")))
      ((weapon*)s)->dead=(value/10000); break;
    case LWPNID:
	  if(0!=(s=checkLWpn(*ri.lwpn,"ID")))
      ((weapon*)s)->id=(value/10000); break;
    case LWPNTILE:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Tile")))
      ((weapon*)s)->tile=(value/10000); break;
	case LWPNCSET:
	  if(0!=(s=checkLWpn(*ri.lwpn,"CSet")))
      ((weapon*)s)->cs=(value/10000)&15; break;
    case LWPNFLASHCSET:
	  if(0!=(s=checkLWpn(*ri.lwpn,"FlashCSet")))
      (((weapon*)s)->o_cset)|=(value/10000)<<4; break;
    case LWPNFRAMES:
	  if(0!=(s=checkLWpn(*ri.lwpn,"NumFrames")))
      ((weapon*)s)->frames=(value/10000); break;
    case LWPNFRAME:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Frame")))
      ((weapon*)s)->aframe=(value/10000); break;
    case LWPNASPEED:
	  if(0!=(s=checkLWpn(*ri.lwpn,"ASpeed")))
      ((weapon*)s)->o_speed=(value/10000); break;
    case LWPNFLASH:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Flash")))
      ((weapon*)s)->flash=(value/10000); break;
    case LWPNFLIP:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Flip")))
      ((weapon*)s)->flip=(value/10000); break;
    case LWPNEXTEND:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Extend")))
      ((weapon*)s)->extend=(value/10000); break;
    case LWPNOTILE:
	  if(0!=(s=checkLWpn(*ri.lwpn,"OriginalTile")))
      ((weapon*)s)->o_tile=(value/10000); break;
    case LWPNOCSET:
	  if(0!=(s=checkLWpn(*ri.lwpn,"OriginalCSet")))
      (((weapon*)s)->o_cset)|=(value/10000)&15; break;
    case LWPNHXOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitXOffset")))
      (((weapon*)s)->hxofs)=(value/10000); break;
    case LWPNHYOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitYOffset")))
      (((weapon*)s)->hyofs)=(value/10000); break;
    case LWPNXOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DrawXOffset")))
      (((weapon*)s)->xofs)=(fix)(value/10000); break;
    case LWPNYOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DrawYOffset")))
      (((weapon*)s)->yofs)=(fix)(value/10000)+playing_field_offset; break;
    case LWPNZOFS:
	  if(0!=(s=checkLWpn(*ri.lwpn,"DrawZOffset")))
      (((weapon*)s)->zofs)=(fix)(value/10000); break;
    case LWPNHXSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitWidth")))
      (((weapon*)s)->hxsz)=(value/10000); break;
    case LWPNHYSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitHeight")))
      (((weapon*)s)->hysz)=(value/10000); break;
    case LWPNHZSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"HitZHeight")))
      (((weapon*)s)->hzsz)=(value/10000); break;
    case LWPNTXSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"TileWidth")))
      (((weapon*)s)->txsz)=vbound((value/10000),1,20); break;
    case LWPNTYSZ:
	  if(0!=(s=checkLWpn(*ri.lwpn,"TileHeight")))
      (((weapon*)s)->tysz)=vbound((value/10000),1,20); break;
    case LWPNMISCD:
	  if(0!=(s=checkLWpn(*ri.lwpn,"Misc"))){
          int a = vbound(*(ri.d[0])/10000,0,15);
          (((weapon*)(s))->miscellaneous[a])=value;
	  }
	  break;
    case LWPNCOLLDET:
	  if(0!=(s=checkLWpn(*ri.lwpn,"CollDetection")))
          (((weapon*)(s))->scriptcoldet)=value/10000;
	  break;
    case EWPNX:
	  if(0!=(s=checkEWpn(*ri.ewpn,"X")))
      ((weapon*)s)->x=(fix)(value/10000); break;
    case EWPNY:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Y")))
      ((weapon*)s)->y=(fix)(value/10000); break;
	case EWPNZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Z")))
      ((weapon*)s)->z=zc_max((fix)(value/10000),(fix)0);
      break;
    case EWPNJUMP:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Jump")))
      ((weapon*)s)->fall=(-value*100.0/10000.0); break;
	case EWPNDIR:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Dir")))
      ((weapon*)s)->dir=(value/10000); break;
    case EWPNSTEP:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Step")))
      ((weapon*)s)->step=(value/10000)/100.0; break;
	case EWPNANGLE:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Angle")))
      ((weapon*)s)->angle=(double)(value/10000.0); break;
    case EWPNANGULAR:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Angular")))
      ((weapon*)s)->angular=(value/10000) != 0; break;
    case EWPNBEHIND:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Behind")))
      ((weapon*)s)->behind=(value/10000) != 0; break;
    case EWPNDRAWTYPE:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DrawType")))
      ((weapon*)s)->drawstyle=(value/10000); break;
    case EWPNPOWER:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Damage")))
      ((weapon*)s)->power=(value/10000); break;
    case EWPNDEAD:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DeadState")))
      ((weapon*)s)->dead=(value/10000);
      break;
    case EWPNID:
	  if(0!=(s=checkEWpn(*ri.ewpn,"ID")))
      ((weapon*)s)->id=(value/10000); break;
    case EWPNTILE:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Tile")))
      ((weapon*)s)->tile=(value/10000); break;
	case EWPNCSET:
	  if(0!=(s=checkEWpn(*ri.ewpn,"CSet")))
      ((weapon*)s)->cs=(value/10000)&15; break;
    case EWPNFLASHCSET:
	  if(0!=(s=checkEWpn(*ri.ewpn,"FlashCSet")))
      (((weapon*)s)->o_cset)|=(value/10000)<<4; break;
    case EWPNFRAMES:
	  if(0!=(s=checkEWpn(*ri.ewpn,"NumFrames")))
      ((weapon*)s)->frames=(value/10000); break;
    case EWPNFRAME:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Frame")))
      ((weapon*)s)->aframe=(value/10000); break;
    case EWPNASPEED:
	  if(0!=(s=checkEWpn(*ri.ewpn,"ASpeed")))
      ((weapon*)s)->o_speed=(value/10000); break;
    case EWPNFLASH:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Flash")))
      ((weapon*)s)->flash=(value/10000); break;
    case EWPNFLIP:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Flip")))
      ((weapon*)s)->flip=(value/10000); break;
    case EWPNEXTEND:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Extend")))
      ((weapon*)s)->extend=(value/10000); break;
    case EWPNOTILE:
	  if(0!=(s=checkEWpn(*ri.ewpn,"OriginalTile")))
      ((weapon*)s)->o_tile=(value/10000); break;
    case EWPNOCSET:
	  if(0!=(s=checkEWpn(*ri.ewpn,"OriginalCSet")))
      (((weapon*)s)->o_cset)|=(value/10000)&15; break;
    case EWPNHXOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitXOffset")))
      (((weapon*)s)->hxofs)=(value/10000); break;
    case EWPNHYOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitYOffset")))
      (((weapon*)s)->hyofs)=(value/10000); break;
    case EWPNXOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DrawXOffset")))
      (((weapon*)s)->xofs)=(fix)(value/10000); break;
    case EWPNYOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DrawYOffset")))
      (((weapon*)s)->yofs)=(fix)(value/10000)+playing_field_offset; break;
    case EWPNZOFS:
	  if(0!=(s=checkEWpn(*ri.ewpn,"DrawZOffset")))
      (((weapon*)s)->zofs)=(fix)(value/10000); break;
    case EWPNHXSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitWidth")))
      (((weapon*)s)->hxsz)=(value/10000); break;
    case EWPNHYSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitHeight")))
      (((weapon*)s)->hysz)=(value/10000); break;
    case EWPNHZSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"HitZHeight")))
      (((weapon*)s)->hzsz)=(value/10000); break;
    case EWPNTXSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"TileWidth")))
      (((weapon*)s)->txsz)=vbound((value/10000),1,20); break;
    case EWPNTYSZ:
	  if(0!=(s=checkEWpn(*ri.ewpn,"TileHeight")))
      (((weapon*)s)->tysz)=vbound((value/10000),1,20); break;
    case EWPNMISCD:
	  if(0!=(s=checkEWpn(*ri.ewpn,"Misc"))){
          int a = vbound(*(ri.d[0])/10000,0,15);
          (((weapon*)(s))->miscellaneous[a])=value;
	  }
	  break;
    case EWPNCOLLDET:
	  if(0!=(s=checkEWpn(*ri.ewpn,"CollDetection")))
          (((weapon*)(s))->scriptcoldet)=value/10000;
	  break;
    case NPCX:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        (s->x)=(fix)(value/10000);
		if (HasLink((enemy*)s))
			Link.setX((fix)(value/10000));
      }
      break;
    case NPCY:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
		fix oldy = (s->y);
        (s->y)=(fix)(value/10000);
        (((enemy *)s)->floor_y)+=((s->y)-oldy);

		if (HasLink((enemy*)s))
			Link.setY((fix)(value/10000));
      }
      break;
    case NPCZ:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        if(canfall(s->id))
        {
          (s->z)=(fix)(value/10000);
          if (s->z<0)
            (s->z)=0;

		  if (HasLink((enemy*)s))
			Link.setZ((fix)(value/10000));
        }
      }
      break;
    case NPCJUMP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        if(canfall(s->id))
          (s->fall)=-(value*100.0/10000.0);
		if (HasLink((enemy*)s))
			Link.setFall((fix)(value/10000.0));
      }
      break;
    case NPCDIR:
      if(0!=(s=checkNPC(*ri.guyref)))
        (s->dir)=value/10000;
      break;
    case NPCRATE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->rate=value/10000;
      }
      break;
    case NPCHOMING:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->homing=value/10000;
      }
      break;
    case NPCSTEP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->step= ((fix)(value/10000))/100.0;
      }
      break;
    case NPCFRAMERATE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->frate=value/10000;
      }
      break;
    case NPCHALTRATE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->hrate=value/10000;
      }
      break;
    case NPCDRAWTYPE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->drawstyle=value/10000;
      }
      break;
    case NPCHP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->hp=value/10000;
      }
      break;
    /*case NPCID:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
	// No way! Who knows what chaos this could unleash?!
	// (It'd break item fairies, for one thing...)
        //((enemy*)(s))->id=value/10000;
      }
      break;*/
    case NPCDP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->dp=value/10000;
      }
      break;
    case NPCWDP:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->wdp=value/10000;
      }
      break;
    case NPCOTILE:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->o_tile=vbound(value/10000,0,NEWMAXTILES-1);
      }
      break;
    case NPCTILE: // This will usually not work.
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->tile=vbound(value/10000,0,NEWMAXTILES-1);
      }
      break;
    case NPCWEAPON:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->wpn=vbound(value/10000,0,MAXWPNS-1);
      }
      break;
    case NPCITEMSET:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->item_set=value/10000;
      }
      break;
    case NPCCSET:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->cs=(value/10000)&15;
      }
      break;
    case NPCBOSSPAL:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->bosspal=value/10000;
      }
      break;
    case NPCBGSFX:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->bgsfx=value/10000;
      }
      break;
    case NPCEXTEND:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->extend=value/10000;
      }
      break;
    case NPCHXOFS:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->hxofs=value/10000;
      }
      break;
    case NPCHYOFS:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->hyofs=value/10000;
      }
      break;
    case NPCXOFS:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->xofs=(fix)(value/10000);
      }
      break;
    case NPCYOFS:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->yofs=(fix)(value/10000)+playing_field_offset;
      }
      break;
    case NPCZOFS:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->zofs=(fix)(value/10000);
      }
      break;
    case NPCHXSZ:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->hxsz=value/10000;
      }
      break;
    case NPCHYSZ:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->hysz=value/10000;
      }
      break;
    case NPCHZSZ:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->hzsz=value/10000;
      }
      break;
    case NPCTXSZ:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->txsz=vbound((value/10000),1,20);
      }
      break;
    case NPCTYSZ:
      if(0!=(s=checkNPC(*ri.guyref)))
      {
        ((enemy*)(s))->tysz=vbound((value/10000),1,20);
      }
      break;
    case NPCDEFENSED:
	if(0!=(s=checkNPC(*ri.guyref)))
	{
          int a = vbound(*(ri.d[0])/10000,0,edefBYRNA);
          (((enemy*)s)->defense[a])=value/10000;
	}
	break;
    case NPCMISCD:
	if(0!=(s=checkNPC(*ri.guyref)))
	{
          int a = vbound(*(ri.d[0])/10000,0,15);
          (((enemy*)s)->miscellaneous[a])=value;
	}
	break;
    case NPCCOLLDET:
	  if(0!=(s=checkNPC(*ri.guyref))) ((enemy*)(s))->scriptcoldet=(value/10000); break;
    case NPCSTUN:
	  if(0!=(s=checkNPC(*ri.guyref))) ((enemy*)(s))->stunclk=(value/10000); break;
    case GAMEDEATHS:
      game->set_deaths(value/10000); break;
    case GAMECHEAT:
      game->set_cheat(value/10000); cheat=(value/10000); break;
    case GAMETIME:
      game->set_time(value); break; // Can't multiply by 10000 or the maximum result is too big
    case GAMETIMEVALID:
      game->set_timevalid((value/10000)?1:0); break;
    case GAMEHASPLAYED:
      game->set_hasplayed((value/10000)?1:0); break;
    case GAMEGUYCOUNT:
      mi2 = (currmap*MAPSCRSNORMAL)+currscr;
      game->guys[mi2]=value/10000; break;
    case GAMECONTSCR:
      game->set_continue_scrn(value/10000); break;
    case GAMECONTDMAP:
      game->set_continue_dmap(value/10000); break;
    case GAMECOUNTERD:
      game->set_counter(value/10000, (*(ri.d[0]))/10000); break;
    case GAMEMCOUNTERD:
      game->set_maxcounter(value/10000, (*(ri.d[0]))/10000); break;
    case GAMEDCOUNTERD:
      game->set_dcounter(value/10000, (*(ri.d[0]))/10000); break;
    case GAMEGENERICD:
      game->set_generic(value/10000, (*(ri.d[0]))/10000);
      break;
    case GAMEITEMSD:
      game->set_item((*(ri.d[0]))/10000,(value!=0)); break;
    case GAMELITEMSD:
      game->lvlitems[(*(ri.d[0]))/10000]=value/10000; break;
    case GAMELKEYSD:
      game->lvlkeys[(*(ri.d[0]))/10000]=value/10000; break;
    case SCREENSTATED:
      mi2 = (currmap*MAPSCRSNORMAL)+currscr;
      (value)?setmapflag(mi2,(1)<<((*(ri.d[0]))/10000)) : unsetmapflag(mi2,(1)<<((*(ri.d[0]))/10000));
      break;
    case SCREENSTATEDD:
      mi2 = *(ri.d[0])/10000;
      mi2 -= 8*(mi2>>7);
      (value)?setmapflag(mi2,(1)<<((*(ri.d[1]))/10000)) : unsetmapflag(mi2,(1)<<((*(ri.d[1]))/10000));
      break;
    case GAMEGUYCOUNTD:
      game->guys[*(ri.d[0])/10000]=value/10000; break;
    case DMAPFLAGSD:
	 //if(vbound((*ri.d[0])/10000,0,512) == get_currdmap()) break; //Don't allow setting on current DMap
      DMaps[vbound((*ri.d[0])/10000,0,512)].flags=value/10000; break;
    case COMBODD:
      {
        int pos = (*(ri.d[0]))/10000;
        if(pos >= 0 && pos < 176){
		   screen_combo_modify_preroutine(tmpscr,pos);
		   tmpscr->data[pos]=(value/10000);
		   screen_combo_modify_postroutine(tmpscr,pos);
        }
      }
      break;
    case COMBOCD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
		{
		  screen_combo_modify_preroutine(tmpscr,pos);
	      tmpscr->cset[pos]=(value/10000)&15;
		  screen_combo_modify_postroutine(tmpscr,pos);
		}
      }
      break;
    case COMBOFD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
	  tmpscr->sflag[pos]=(value/10000);
      }
      break;
    case COMBOTD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
		{
		  // Preprocess each instance of the combo on the screen
		  for (int i = 0; i < 176; i++)
		  {
		    if (tmpscr->data[i] == tmpscr->data[pos])
			{
			  screen_combo_modify_preroutine(tmpscr,i);
			}
		  }
	      combobuf[tmpscr->data[pos]].type=value/10000;
		  for (int i = 0; i < 176; i++)
		  {
		    if (tmpscr->data[i] == tmpscr->data[pos])
			{
			  screen_combo_modify_postroutine(tmpscr,i);
			}
		  }
		}
      }
      break;
    case COMBOID:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
	  combobuf[tmpscr->data[pos]].flag=value/10000;
      }
      break;
    case COMBOSD:
      {
        int pos = (*(ri.d[0]))/10000;
        if (pos >= 0 && pos < 176)
	  combobuf[tmpscr->data[pos]].walk=(value/10000)&15;
      }
      break;
    case COMBODDM:
    {
      int pos = (*(ri.d[0]))/10000;
	  int sc = ((*(ri.d[2]))/10000);
	  int m = zc_max(((*(ri.d[1]))/10000)-1,0);
      long scr = zc_max(m*MAPSCRS+sc,0);
      if (!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;

	  if (scr==(currmap*MAPSCRS+currscr))
	    screen_combo_modify_preroutine(tmpscr,pos);
      TheMaps[scr].data[pos]=value/10000;
	  if (scr==(currmap*MAPSCRS+currscr))
	  {
	    tmpscr->data[pos] = value/10000;
	    screen_combo_modify_postroutine(tmpscr,pos);
      }
      int layr = whichlayer(scr);
      if (layr>-1)
	  {
		//if (layr==(currmap*MAPSCRS+currscr))
	    //  screen_combo_modify_preroutine(tmpscr,pos);
        tmpscr2[layr].data[pos]=value/10000;
		//if (layr==(currmap*MAPSCRS+currscr))
	    //  screen_combo_modify_postroutine(tmpscr,pos);
	  }
    }
    break;
    case COMBOCDM:
    {
      int pos = (*(ri.d[0]))/10000;
	  int sc = ((*(ri.d[2]))/10000);
	  int m = zc_max(((*(ri.d[1]))/10000)-1,0);
      long scr = zc_max(m*MAPSCRS+sc,0);
      if (!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;

      TheMaps[scr].cset[pos]=(value/10000)&15;
	  if (scr==(currmap*MAPSCRS+currscr))
	    tmpscr->cset[pos] = value/10000;

      int layr = whichlayer(scr);
      if (layr>-1)
        tmpscr2[layr].cset[pos]=(value/10000)&15;
    }
    break;
    case COMBOFDM:
    {
      int pos = (*(ri.d[0]))/10000;
	  int sc = ((*(ri.d[2]))/10000);
	  int m = zc_max(((*(ri.d[1]))/10000)-1,0);
      long scr = zc_max(m*MAPSCRS+sc,0);
      if (!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;

      TheMaps[scr].sflag[pos]=value/10000;
	  if (scr==(currmap*MAPSCRS+currscr))
	    tmpscr->sflag[pos] = value/10000;

      int layr = whichlayer(scr);
      if (layr>-1)
        tmpscr2[layr].sflag[pos]=value/10000;
    }
    break;
    case COMBOTDM:
      {
      int pos = (*(ri.d[0]))/10000;
	  int sc = ((*(ri.d[2]))/10000);
	  int m = zc_max(((*(ri.d[1]))/10000)-1,0);
      long scr = zc_max(m*MAPSCRS+sc,0);
      if (!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count))
	    break;
	  int cdata = TheMaps[scr].data[pos];
	  // Preprocess the screen's combos in case the combo changed is present on the screen. -L
	  for (int i = 0; i < 176; i++)
	  {
		if (tmpscr->data[i] == cdata)
		{
		  screen_combo_modify_preroutine(tmpscr,i);
		}
	  }
	  combobuf[cdata].type=value/10000;
	  for (int i = 0; i < 176; i++)
	  {
		if (tmpscr->data[i] == cdata)
		{
		  screen_combo_modify_postroutine(tmpscr,i);
		}
	  }
      }
      break;
    case COMBOIDM:
      {
      int pos = (*(ri.d[0]))/10000;
	  int sc = ((*(ri.d[2]))/10000);
	  int m = zc_max(((*(ri.d[1]))/10000)-1,0);
      long scr = zc_max(m*MAPSCRS+sc,0);
      if (!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count))
	    break;

	  combobuf[TheMaps[scr].data[pos]].flag=value/10000;
      }
      break;
    case COMBOSDM:
      {
      int pos = (*(ri.d[0]))/10000;
      long scr = ((*(ri.d[1]))/10000)*MAPSCRS+((*(ri.d[2]))/10000);
	  if (pos < 0 || pos >= 176 || scr < 0) break;

	  combobuf[TheMaps[scr].data[pos]].walk=(value/10000)&15;
      }
      break;
    case REFFFC:
      *ri.ffc = vbound(value/10000,0,MAXFFCS-1); break;
    case REFITEM:
      *ri.itemref = value; break;
    case REFITEMCLASS:
      *ri.idata = value; break;
    case REFLWPN:
      *ri.lwpn = value; break;
    case REFLWPNCLASS:
      *ri.lclass = value; break;
    case REFEWPN:
      *ri.ewpn = value; break;
    case REFEWPNCLASS:
      *ri.eclass = value; break;
    case REFNPC:
      *ri.guyref = value; break;
    case REFNPCCLASS:
      *ri.gclass = value; break;
    case SDD:
      di2 = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
      set_screen_d(di2, *(ri.d[0])/10000, value); break;
    case GDD:
      game->global_d[*(ri.d[0])/10000]=value; break;
    case SDDD:
      set_screen_d((*(ri.d[0]))/10000 + ((get_currdmap())<<7), *(ri.d[1])/10000, value); break;
    case SDDDD:
      set_screen_d((*(ri.d[1]))/10000 + ((*(ri.d[0])/10000)<<7), *(ri.d[2])/10000, value); break;
    case SCRDOORD:
      tmpscr->door[*(ri.d[0])/10000]=value/10000;
      putdoor(scrollbuf,0,*(ri.d[0])/10000,value/10000,true,true);
      break;
    case LIT:
      naturaldark = !value;
      lighting(false, false); break;
    case WAVY:
      wavy=value/10000; break;
    case QUAKE:
      quakeclk=value/10000; break;
    case SP:
      *ri.sp = value/10000; break;
    case SCRIPTRAM: case GLOBALRAM:
	  ScriptHelper::setElement(*(ri.d[0])/10000, *(ri.d[1])/10000, value); break;
	case SCRIPTRAMD: case GLOBALRAMD:
	  ScriptHelper::setElement(*(ri.d[0])/10000, 0, value); break;

	case SAVERAM:
	  if((*(ri.d[0])/10000) < 0 || (*(ri.d[0])/10000) >= 0x2000) break;
	  game->savedata[*(ri.d[0])/10000] = value; break;
    default:
    {
      int k;
      if(arg>=D(0)&&arg<=D(7))
      {
        k=arg-D(0); *(ri.d[k])=value; break;
      }
      else if(arg>=A(0)&&arg<=A(1))
      {
        k=arg-A(0); if(script_type!=SCRIPT_GLOBAL) *(ri.a[k])=value; break;
      }
      else if(arg>=SD(0)&&arg<=SD(7))
      {
        di2 = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
        k=arg-SD(0); set_screen_d(di2, k, value); break;
      }
      else if(arg>=GD(0)&&arg<=GD(255))
      {
        k=arg-GD(0); game->global_d[k]=value; break;
      }
      else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMECOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
      {
        k=(arg-GAMECOUNTER(0))/3; game->set_counter(value/10000, k); //break;
      }
      else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMEMCOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
      {
        k=(arg-GAMECOUNTER(0))/3; game->set_maxcounter(value/10000, k); //break;
      }
      else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMEDCOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
      {
        k=(arg-GAMECOUNTER(0))/3; game->set_dcounter(value/10000, k); //break;
      }
      else if(arg>=SCREENSTATE(0)&&arg<=SCREENSTATE(31))
      {
        mi2 = currmap*MAPSCRSNORMAL+currscr;
        k=arg-SCREENSTATE(0);
        (value)?setmapflag(mi2,1<<k):unsetmapflag(mi2,1<<k);
        //(value/10000)?game->maps[mi2]|=((value/10000)?1:0)<<(k):game->maps[mi2]&=~(((value/10000)?1:0)<<(k)); //break;
      }
      else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOD(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
      {
        k=(arg-COMBOD(0))/3; tmpscr->data[k]=(value/10000); //break;
      }
      else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOC(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
      {
        k=(arg-COMBOD(0))/3; tmpscr->cset[k]=(value/10000)&15; //break;
      }
      else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOF(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
      {
        k=(arg-COMBOD(0))/3; tmpscr->sflag[k]=(value/10000); //break;
      }
      else if(arg>=GAMEGENERIC(0)&&arg<=GAMEGENERIC(255))
      {
        k=arg-GAMEGENERIC(0); game->set_generic(value/10000, k); //break;
      }
      else if(arg>=GAMEITEMS(0)&&arg<=GAMEITEMS(255))
      {
        k=arg-GAMEITEMS(0); game->item[k]= (value!=0); //break;
      }
      else if(arg>=GAMELITEMS(0)&&arg<=GAMELITEMS(255))
      {
        k=arg-GAMELITEMS(0); game->lvlitems[k]=value/10000; //break;
      }
      else if(arg>=GAMELKEYS(0)&&arg<=GAMELKEYS(255))
      {
        k=arg-GAMELKEYS(0); game->lvlkeys[k]=value/10000; //break;
      }
      else if(arg>=SCRDOOR(0)&&arg<=SCRDOOR(3))
      {
        k=arg-SCRDOOR(0); tmpscr->door[k]=value/10000;
        putdoor(scrollbuf,0,k,value/10000,true,true);
      }
      else if(arg>=LWPNMISC(0)&&arg<=LWPNMISC(15))
      {
          if(0!=(s=checkLWpn(*ri.lwpn,"Misc"))){
               k=arg-LWPNMISC(0); int a = vbound(k,0,15);
               (((weapon*)(s)))->miscellaneous[a]=value;
          }
          break;
      }
      else if(arg>=EWPNMISC(0)&&arg<=EWPNMISC(15))
      {
          if(0!=(s=checkEWpn(*ri.ewpn,"Misc"))){
               k=arg-EWPNMISC(0); int a = vbound(k,0,15);
               (((weapon*)(s)))->miscellaneous[a]=value;
          }
          break;
      }
      else if(arg>=NPCMISC(0)&&arg<=NPCMISC(15))
      {
          if(0!=(s=checkItem(*ri.guyref))){
               k=arg-NPCMISC(0); int a = vbound(k,0,15);
               (((enemy*)(s)))->miscellaneous[a]=value;
          }
          break;
      }
      else if(arg>=ITEMMISC(0)&&arg<=ITEMMISC(15))
      {
          if(0!=(s=checkItem(*ri.itemref))){
               k=arg-ITEMMISC(0); int a = vbound(k,0,15);
               (((item*)(s)))->miscellaneous[a]=value;
          }
          break;
      }
      else if(arg>=FFMISC(0)&&arg<=FFMISC(15))
      {
        k=arg-FFMISC(0); int a = vbound(k,0,15);
        (tmpscr->ffmisc[*ri.ffc][a])=value;
        break;
      }
	 else if(arg>=LINKMISC(0)&&arg<=LINKMISC(15))
      {
      	k = arg-LINKMISC(0);
      	Link.miscellaneous[vbound(k,0,15)] = value;
          break;
      }
      else if(arg>=FFINITD(0)&&arg<=FFINITD(7))
      {
        (tmpscr->initd[*ri.ffc][vbound(arg-FFINITD(0),0,7)])=value/10000;
        break;
      }
      /*else if(arg>=FFD(0)&&arg<=FFD(7))
      {
        (tmpscr->d[*ri.ffc][vbound(arg-FFD(0),0,7)])=value/10000;
        break;
      }*/
      break;
    }
  }
}

void do_set(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  set_variable(sarg1,i,temp,ri);
}


void do_trig(int, word *, byte i, bool v, int type, refInfo &ri)
{
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  if(type==0)
  {
    double rangle = (temp/10000.0) * PI / 180.0;
    temp = (long)(sin(rangle)*10000.0);
  }
  else if(type==1)
  {
    double rangle = (temp/10000.0) * PI / 180.0;
    temp = (long)(cos(rangle)*10000.0);
  }
  else if(type==2)
  {
    double rangle = (temp/10000.0) * PI / 180.0;
    temp = (long)(tan(rangle)*10000.0);
  }
  set_variable(sarg1,i,temp,ri);
}

void do_asin(int, word *, byte i, bool v, refInfo &ri)
{
  double temp = (v?sarg2:get_arg(sarg2,i,ri))/10000.0;

  if(temp >= -1 && temp <= 1)
      set_variable(sarg1,i,long(asin(temp)*10000.0),ri);
  else{
      Z_eventlog("Script attempted to pass %ld into ArcSin!\n",temp);
      set_variable(sarg1,i,-100000,ri);
  }
}

void do_acos(int, word *, byte i, bool v, refInfo &ri)
{
  double temp = (v ? sarg2: get_arg(sarg2, i, ri) ) / 10000.0;

  if(temp >= -1 && temp <= 1)
      set_variable(sarg1,i,long(acos(temp)*10000.0),ri);
  else
  {
      Z_eventlog("Script attempted to pass %ld into ArcCos!\n",temp);
      set_variable(sarg1,i,-100000,ri);
  }
}

void do_arctan(int, word*, byte i, bool, refInfo &ri)
{
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  double xpos = (*(ri.d[0]))/10000.0;
  double ypos = (*(ri.d[1]))/10000.0;
  temp = (long)(atan2(ypos, xpos)*10000.0);
  set_variable(sarg1,i,temp,ri);
}

void do_add(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp2=get_arg(sarg1,i,ri);
  set_variable(sarg1,i,temp2+temp,ri);
}

void do_sub(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp2=get_arg(sarg1,i,ri);
  set_variable(sarg1,i,temp2-temp,ri);
}

void do_mult(int, word *, byte i, bool v, refInfo &ri)
{

  long long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  //temp = temp/10000.0;
  temp2=get_arg(sarg1,i,ri);
  temp *= temp2;
  //set_variable(sarg1,i,(long)(temp2*temp),ri);
  set_variable(sarg1,i,(long)(temp/10000),ri);
}

void do_div(int, word *, byte i, bool v, refInfo &ri)
{

  long long temp;
  long long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp2=get_arg(sarg1,i,ri);
  //temp = temp/10000.0;
  if(temp==0){
    Z_eventlog("Script attempted to divide %ld by zero!\n",temp2);
	temp2 = 0;
	temp = 1;
  }
  temp = (temp2*10000)/temp;
  //set_variable(sarg1,i,(long)(temp2/temp),ri);
  set_variable(sarg1,i,(long)(temp),ri);
}

void do_mod(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  //temp = temp/10000;
  //temp2=get_arg(sarg1,i,ri)/10000;
  temp2=get_arg(sarg1,i,ri);
  if (temp==0) {
    Z_eventlog("Script attempted to modulo %ld by zero!\n",temp2);
    temp = 1; //n%1 == 0
  }
  //set_variable(sarg1,i,(temp2%temp)*10000,ri);
  set_variable(sarg1,i,temp2%temp,ri);
}


void do_comp(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  /*switch(script_type)
  {
  case SCRIPT_FFC:
    scriptflag = &(tmpscr->scriptflag[i]);
    break;
  case SCRIPT_ITEM:
    scriptflag = &(items.spr(i)->scriptflag);
    break;
  case SCRIPT_GLOBAL:
    scriptflag = &g_scriptflag;
    break;
  }*/

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp2=get_arg(sarg1,i,ri);

  if(temp2 >= temp) *scriptflag |= MOREFLAG; else *scriptflag &= ~MOREFLAG;
  if(temp2 == temp) *scriptflag |= TRUEFLAG; else *scriptflag &= ~TRUEFLAG;
}

void do_loada(int, word *, byte i, int a, refInfo &ri)
{
  long temp;
  int j;

  /*switch(script_type)
  {
    case SCRIPT_FFC:
      na[0] = &(tmpscr->a[i][0]);
      na[1] = &(tmpscr->a[i][1]);
      break;
    case SCRIPT_ITEM:
        na[0] = &(items.spr(i)->a[0]);
      na[1] = &(items.spr(i)->a[1]);
      break;
    case SCRIPT_GLOBAL:
      na[0] = NULL;
      na[1] = NULL;
      break;
  }*/

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(a) j = (*(na[1])/10000)-1;
  else j = (*(na[0])/10000)-1;

  temp=get_arg(sarg2,j,ri);
  set_variable(sarg1,i,temp,ri);
}

void do_seta(int, word *, byte i, int a, refInfo &ri)
{
  long temp;
  int j;
  //long *na[2];

  /*switch(script_type)
  {
    case SCRIPT_FFC:
      na[0] = &(tmpscr->a[i][0]);
      na[1] = &(tmpscr->a[i][1]);
      break;
    case SCRIPT_ITEM:
      na[0] = &(items.spr(i)->a[0]);
      na[1] = &(items.spr(i)->a[1]);
      break;
     case SCRIPT_GLOBAL:
      na[0] = NULL;
      na[1] = NULL;
      break;
  }*/

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(a) j = (*(na[1])/10000)-1;
  else j = (*(na[0])/10000)-1;

  temp=get_arg(sarg2,i,ri);
  set_variable(sarg1,j,temp,ri);
}

void do_abs(int, word *, byte i, bool, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  temp=get_arg(sarg1,i,ri);
  set_variable(sarg1,i,abs(temp),ri);
}

void do_log10(int, word *, byte i, bool, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp;
  long double temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;
  temp = get_arg(sarg1,i,ri);
  if(temp > 0){
    temp2 = temp/10000.0;
    temp = (long)(log10(temp2)*10000);
  }else temp = 0;

  set_variable(sarg1,i,temp,ri);
}

void do_naturallog(int, word *, byte i, bool, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp;
  long double temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;
  temp = get_arg(sarg1,i,ri);
  if(temp > 0){
    temp2 = temp/10000.0;
    temp = (long)(log(temp2)*10000);
  }else temp = 0;

  set_variable(sarg1,i,temp,ri);
}

void do_min(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp2=get_arg(sarg1,i,ri);
  set_variable(sarg1,i,zc_min(temp2,temp),ri);
}

void do_max(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp2=get_arg(sarg1,i,ri);
  set_variable(sarg1,i,zc_max(temp2,temp),ri);
}


void do_rnd(int , word *, byte i, bool v, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp2=get_arg(sarg1,i,ri);
  set_variable(sarg1,i,(rand()%(temp/10000))*10000,ri);
}

void do_factorial(int, word *, byte i, bool v, refInfo &ri)
{
  //long arg1;
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    return;  //must factorial a register, not a value
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
    if (temp<2)
    {
      set_variable(sarg1,i,temp>=0?10000:00000,ri);
      return;
    }
  }
  temp2=1;
  for (long temp3=temp; temp>1; temp--)
  {
    temp2*=temp3;
  }
  set_variable(sarg1,i,temp2*10000,ri);
}

void do_power(int, word *, byte i, bool v, refInfo &ri)
{
  double temp;
  double temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp = temp/10000.0;
  temp2=get_arg(sarg1,i,ri)/10000.0;

  if(temp == 0 && temp2 == 0){
  	Z_eventlog("Script attempted to calculate 0 to the power 0!\n");
	set_variable(sarg1,i,0,ri);
	return;
  }

  double power = pow((double)temp2,(double)temp);

#ifdef _DEBUGSCRIPTPOWER
  al_trace( "arg1:%09.9f \n",temp2 );
  al_trace( "arg2:%09.9f \n",temp );
  al_trace( "result:%09.9f \n \n",power );
#endif

  set_variable(sarg1,i,long( power*10000.0 ),ri);
}

void do_ipower(int, word *, byte i, bool v, refInfo &ri)
{
  double temp;
  double temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp = 10000.0/temp;
  temp2=get_arg(sarg1,i,ri)/10000.0;
  set_variable(sarg1,i,((long)pow(temp2,temp))*10000,ri);
}

void do_and(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000;
  temp2=get_arg(sarg1,i,ri)/10000;
  set_variable(sarg1,i,(temp2&temp)*10000,ri);
}

void do_or(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000;
  temp2=get_arg(sarg1,i,ri)/10000;
  set_variable(sarg1,i,(temp2|temp)*10000,ri);
}

void do_xor(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000;
  temp2=get_arg(sarg1,i,ri)/10000;
  set_variable(sarg1,i,(temp2^temp)*10000,ri);
}

void do_nand(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000;
  temp2=get_arg(sarg1,i,ri)/10000;
  set_variable(sarg1,i,(!(temp2&temp))*10000,ri);
}

void do_nor(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  ////arg1 = ffs2cripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000;
  temp2=get_arg(sarg1,i,ri)/10000;
  set_variable(sarg1,i,(!(temp2|temp))*10000,ri);
}

void do_xnor(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000;
  temp2=get_arg(sarg1,i,ri)/10000;
  set_variable(sarg1,i,(!(temp2^temp))*10000,ri);
}

void do_not(int, word *, byte i, bool v, refInfo &ri)
{
  //long arg1;
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  temp=temp/10000;
  set_variable(sarg1,i,(!temp)*10000,ri);
}

void do_bitwisenot(int, word *, byte i, bool v, refInfo &ri)
{
  //long arg1;
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  temp=temp/10000;
  set_variable(sarg1,i,(~temp)*10000,ri);
}

void do_lshift(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000;
  temp2=get_arg(sarg1,i,ri)/10000;
  set_variable(sarg1,i,(temp2<<temp)*10000,ri);
}

void do_rshift(int, word *, byte i, bool v, refInfo &ri)
{
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000;
  temp2=get_arg(sarg1,i,ri)/10000;
  set_variable(sarg1,i,(temp2>>temp)*10000,ri);
}

void do_sqroot(int, word *, byte i, bool v, refInfo &ri)
{
  double temp;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp=temp/10000.0;
  set_variable(sarg1,i,int((sqrt(temp))*10000),ri);
}

void do_message(int, word *, byte i, bool v, refInfo &ri)
{
  word temp = (v ? sarg1: get_arg(sarg1, i, ri)) / 10000;

  if (temp == 0)
  {
    dismissmsg();
    msgfont=zfont;
    blockpath=false;
    Link.finishedmsg();
  }
  else
	donewmsg(temp);
}

void do_issolid(int, word *, byte i, refInfo &ri)
{
  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  int x = int((*(ri.d[0]))/10000);
  int y = int((*(ri.d[1]))/10000);

  long temp = (_walkflag(x,y,1)) ? 10000 : 0;
  set_variable(sarg1,i,temp,ri);
}

void do_setsidewarp(int script, word *, byte i, refInfo)
{
  long warp = read_stack(script,i,(*sp)+3)/10000;
  long scr = vbound(read_stack(script,i,(*sp)+2)/10000,-1,0x87);
  long dmap = vbound(read_stack(script,i,(*sp)+1)/10000,-1,511);
  long type = vbound(read_stack(script,i,*sp)/10000,-1,wtMAX-1);
  if(warp < 0 || warp > 3) return;
  if(scr > -1)  tmpscr->sidewarpscr[warp] = scr;
  if(dmap > -1) tmpscr->sidewarpdmap[warp] = dmap;
  if(type > -1) tmpscr->sidewarptype[warp] = type;
}


void do_settilewarp(int script, word *, byte i, refInfo)
{
  long warp = read_stack(script,i,(*sp)+3)/10000;
  long scr = vbound(read_stack(script,i,(*sp)+2)/10000,-1,0x87);
  long dmap = vbound(read_stack(script,i,(*sp)+1)/10000,-1,511);
  long type = vbound(read_stack(script,i,*sp)/10000,-1,wtMAX-1);
  if(warp < 0 || warp > 3) return;
  if(scr > -1)  tmpscr->tilewarpscr[warp] = scr;
  if(dmap > -1) tmpscr->tilewarpdmap[warp] = dmap;
  if(type > -1) tmpscr->tilewarptype[warp] = type;
}

void do_layerscreen(int, word *, byte i, refInfo &ri)
{
  int layer = (get_arg(sarg2,i,ri)/10000)-1;
  long temp;
  if(layer > 5 || layer < 0 || tmpscr->layermap[layer] == 0) temp = -10000;
  else temp = tmpscr->layerscreen[layer]*10000;
  set_variable(sarg1,i,temp,ri);
}

void do_layermap(int, word *, byte i, refInfo &ri)
{
  int layer = (get_arg(sarg2,i,ri)/10000)-1;
  long temp;
  if(layer > 5 || layer < 0 || tmpscr->layermap[layer] == 0) temp = -10000;
  else temp = tmpscr->layermap[layer]*10000;
  set_variable(sarg1,i,temp,ri);
}

void do_triggersecrets(int,word*,byte,refInfo)
{
    hidden_entrance(0,true,false,-4);
}

void do_combotile(int, word*, byte i, refInfo &ri)
{
	dword combo = get_arg(sarg2,i,ri) / 10000;
	set_variable(sarg1,i,combobuf[combo].tile*10000,ri);
}

void do_isvaliditem(int, word *, byte i, refInfo &ri)
{
  long temp =get_arg(sarg1, i,ri);
  bool found = false;
  for(int j=0; j<items.Count(); j++)
  {
    if(items.spr(j)->getUID() == temp)
    {
      found = true;
      break;
    }
  }
  temp = found ? 10000 : 0;
  set_variable(sarg1,i,temp,ri);
}

void do_isvalidnpc(int, word *, byte i, refInfo &ri)
{
  long temp = get_arg(sarg1, i,ri);
  bool found = false;
  for(int j=0; j<guys.Count(); j++)
  {
    if(guys.spr(j)->getUID() == temp)
    {
      found = true;
      break;
    }
  }
  temp = found ? 10000 : 0;
  set_variable(sarg1,i,temp,ri);
}

void do_isvalidlwpn(int, word *, byte i, refInfo &ri)
{
  long temp = get_arg(sarg1, i,ri);
  bool found = false;
  for(int j=0; j<Lwpns.Count(); j++)
  {
    if(Lwpns.spr(j)->getUID() == temp)
    {
      found = true;
      break;
    }
  }
  temp = found ? 10000 : 0;
  set_variable(sarg1,i,temp,ri);
}

void do_isvalidewpn(int, word *, byte i, refInfo &ri)
{
  long temp = get_arg(sarg1, i,ri);
  bool found = false;
  for(int j=0; j<Ewpns.Count(); j++)
  {
    if(Ewpns.spr(j)->getUID() == temp)
    {
      found = true;
      break;
    }
  }
  temp = found ? 10000 : 0;
  set_variable(sarg1,i,temp,ri);
}

void do_lwpnusesprite(int, word *, byte i, bool v, refInfo &ri)
{
  weapon *s2;
  double temp;
  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp = get_arg(sarg1, i,ri);
  }
  temp/=10000.0;
  if ((int)temp >= MAXWPNS || (int)temp<0)
    return;

  if(0 != (s2=checkLWpn(*ri.lwpn,"UseSprite() method")))
    s2->LOADGFX((int)temp);
}

void do_ewpnusesprite(int, word *, byte i, bool v, refInfo &ri)
{
  weapon *s2;
  double temp;
  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp = get_arg(sarg1, i,ri);
  }
  temp=temp/10000.0;
  if (temp >= MAXWPNS || temp<0)
    return;
  if(0!=(s2=checkEWpn(*ri.ewpn, "UseSprite() method")))
    s2->LOADGFX((int)temp);
}

void do_clearsprites(int , word *, byte i, bool v, refInfo &ri)
{
  word temp = (v ? sarg1: get_arg(sarg1, i, ri)) / 10000;

  switch (temp)
  {
    case 0:
      guys.clear();
      break;
    case 1:
      items.clear();
      break;
    case 2:
      Ewpns.clear();
      break;
    case 3:
      Lwpns.clear();
	 Link.reset_hookshot();
      break;
    case 4:
      decorations.clear();
      break;
    case 5:
      particles.clear();
      break;
  }
}

void do_drawing_command(int script, word *, byte i, int script_command, refInfo &ri)
{
  //byte *sp=NULL;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
      sp = &(tmpscr->sp[i]);
      break;
    case SCRIPT_ITEM:
      sp = &(items.spr(i)->sp);
      break;
    case SCRIPT_GLOBAL:
      sp = &g_sp;
      break;
  }*/
#define MAX_DRAWSTRING_SIZE		256

  long temp = get_arg(sarg2,i,ri);
  long temp2 = get_arg(sarg1,i,ri);

  int j=0;
  for (; j<MAX_SCRIPT_DRAWING_COMMANDS; ++j)
  {
    if (script_drawing_commands[j][0]==0)
    {
      break;
    }
  }
  if (j>=MAX_SCRIPT_DRAWING_COMMANDS)  //out of drawing command space
  {
    al_trace("Max draw primitive limit reached\n");
    return;
  }
  script_drawing_commands[j][0]=script_command;
  /*
  script_drawing_commands[j][1]=read_stack(script, i, (*sp)+8);//get_arg(sarg1,i); //xy
  script_drawing_commands[j][2]=read_stack(script, i, (*sp)+7);//get_arg(sarg2,i); //color or tile/combo+cset for DRAWTILER and DRAWCOMBOR
  script_drawing_commands[j][3]=read_stack(script, i, (*sp)+6);
  script_drawing_commands[j][4]=read_stack(script, i, (*sp)+5);
  script_drawing_commands[j][5]=read_stack(script, i, (*sp)+4);
  script_drawing_commands[j][6]=read_stack(script, i, (*sp)+3);
  script_drawing_commands[j][7]=read_stack(script, i, (*sp)+2);
  script_drawing_commands[j][8]=read_stack(script, i, (*sp)+1);
  script_drawing_commands[j][9]=read_stack(script, i, *sp);
  */
  switch(script_command)
  {
    case RECTR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+11);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+10);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+9);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+8);       //x2
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+7);       //y2
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+6);       //color
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+5);       //scale factor
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+4);       //rotation anchor x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+3);       //rotation anchor y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+2);      //rotation angle
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+1);      //fill
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case CIRCLER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+10);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+9);       //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+8);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+7);       //radius
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+6);       //color
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+5);       //scale factor
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+4);       //rotation anchor x
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+3);       //rotation anchor y
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+2);       //rotation angle
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+1);      //fill
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case ARCR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+13);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+12);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+11);      //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+10);      //radius
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+9);       //start angle
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+8);       //end angle
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+7);       //color
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+6);       //scale factor
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+5);       //rotation anchor x
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+4);      //rotation anchor y
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+3);      //rotation angle
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+2);      //closed
      script_drawing_commands[j][13]=read_stack(script, i, (*sp)+1);      //fill
      script_drawing_commands[j][14]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case ELLIPSER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+11);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+10);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+9);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+8);       //radiusx
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+7);       //radiusy
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+6);       //color
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+5);       //scale factor
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+4);       //rotation anchor x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+3);       //rotation anchor y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+2);      //rotation angle
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+1);      //fill
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case LINER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+10);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+9);       //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+8);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+7);       //x2
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+6);       //y2
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+5);       //color
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+4);       //scale factor
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+3);       //rotation anchor x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+2);       //rotation anchor y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+1);      //rotation angle
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case PUTPIXELR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+7);       //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+6);       //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+5);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+4);       //color
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+3);       //rotation anchor x
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+2);       //rotation anchor y
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+1);       //rotation angle
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+0);       //opacity
      break;
    case DRAWTILER:
	  script_drawing_commands[j][1]=read_stack(script, i, (*sp)+14);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+13);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+12);      //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+11);      //tile
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+10);      //tile width
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+9);       //tile height
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+8);       //color (cset)
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+7);       //scale x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+6);       //scale y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+5);      //rotation anchor x
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+4);      //rotation anchor y
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+3);      //rotation angle
      script_drawing_commands[j][13]=read_stack(script, i, (*sp)+2);      //flip
      script_drawing_commands[j][14]=read_stack(script, i, (*sp)+1);      //transparency
      script_drawing_commands[j][15]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case DRAWCOMBOR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+15);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+14);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+13);      //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+12);      //combo
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+11);      //combo width
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+10);      //combo height
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+9);       //color (cset)
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+8);       //scale x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+7);       //scale y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+6);      //rotation anchor x
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+5);      //rotation anchor y
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+4);      //rotation
      script_drawing_commands[j][13]=read_stack(script, i, (*sp)+3);      //frame
      script_drawing_commands[j][14]=read_stack(script, i, (*sp)+2);      //flip
      script_drawing_commands[j][15]=read_stack(script, i, (*sp)+1);      //transparency
	  script_drawing_commands[j][16]=read_stack(script, i, (*sp)+0);      //opacity
      break;
	case DRAWCHARR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+9);      //float 1
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+8);       //float 2
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+7);       //float 3
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+6);       //float 4
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+5);       //float 5
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+4);      //float 6
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+3);      //float 7
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+2);      //float 8
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+1);      //float 9
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+0);      //float 10
	  break;
    case DRAWINTR:
	  script_drawing_commands[j][1]=read_stack(script, i, (*sp)+10);      //float 1
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+9);      //float 2
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+8);       //float 3
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+7);       //float 4
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+6);       //float 5
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+5);       //float 6
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+4);      //float 7
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+3);      //float 8
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+2);      //float 9
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+1);      //float 10
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+0);      //float 11
	  break;
	case QUADR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+14);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+13);      //x1
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+12);      //y1
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+11);      //x
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+10);      //y
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+9);       //x
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+8);       //y
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+7);       //x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+6);       //y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+5);      //w
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+4);      //h
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+3);      //
      script_drawing_commands[j][13]=read_stack(script, i, (*sp)+2);      //
      script_drawing_commands[j][14]=read_stack(script, i, (*sp)+1);      //
	  script_drawing_commands[j][15]=read_stack(script, i, (*sp)+0);      //
      break;
	case TRIANGLER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+12);      //x
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+11);      //y
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+10);      //x
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+9);       //y
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+8);       //x
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+7);       //y
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+6);       //w
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+5);      //h
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+4);      //
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+3);      //
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+2);      //
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+1);      //
	  script_drawing_commands[j][13]=read_stack(script, i, (*sp)+0);      //
      break;
	case QUAD3DR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+7);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+6);      //pos[12] x,y,z
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+5);       //uv[8]
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+4);       //size[2]w,h
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+3);      //color[4]
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+2);      //flip
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+1);      //tile
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+0);      //render mode
	  {
		quad3Dstruct *q = new quad3Dstruct;
		q->index = script_drawing_commands[j][19] = j;
		if( ::ScriptHelper::getArray( script_drawing_commands[j][2], 12, &q->pos[0] ) != ScriptHelper::_NoError ||
			::ScriptHelper::getArray( script_drawing_commands[j][3], 8, &q->uv[0] ) != ScriptHelper::_NoError ||
			::ScriptHelper::getArray( script_drawing_commands[j][4], 2, &q->size[0] ) != ScriptHelper::_NoError ||
			::ScriptHelper::getArray( script_drawing_commands[j][5], 4, &q->color[0] ) != ScriptHelper::_NoError )
		{	//poop. nothing to do but ship out a dud. we'll handle it from the other end.
			memset( q, 0, sizeof(quad3Dstruct) );
			al_trace( "Invalid array pointer used for Quad3D. \n" );
		}
		draw_container.quad3D.push_back( *q );
		delete q;
	  }
      break;
	case TRIANGLE3DR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+7);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+6);      //pos[9] x,y,z
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+5);       //uv[6]
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+4);       //size[2]w,h
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+3);      //color[3]
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+2);      //flip
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+1);      //tile
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+0);      //render mode
	  {
		triangle3Dstruct *q = new triangle3Dstruct;
		q->index = script_drawing_commands[j][19] = j;
		if( ScriptHelper::getArray( script_drawing_commands[j][2], 9, &q->pos[0] ) != ScriptHelper::_NoError ||
			ScriptHelper::getArray( script_drawing_commands[j][3], 6, &q->uv[0] ) != ScriptHelper::_NoError ||
			ScriptHelper::getArray( script_drawing_commands[j][4], 2, &q->size[0] ) != ScriptHelper::_NoError ||
			ScriptHelper::getArray( script_drawing_commands[j][5], 3, &q->color[0] ) != ScriptHelper::_NoError )
		{	// oh well.
			memset( q, 0, sizeof(triangle3Dstruct) );
			al_trace( "Invalid array pointer used for Triangle3D. \n" );
		}
		draw_container.triangle3D.push_back( *q );
		delete q;
	  }
      break;
    case FASTTILER:
	  script_drawing_commands[j][1]=read_stack(script, i, (*sp)+5);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+4);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+3);      //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+2);      //tile
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+1);       //color (cset)
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case FASTCOMBOR:
	  script_drawing_commands[j][1]=read_stack(script, i, (*sp)+5);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+4);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+3);      //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+2);      //combo
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+1);       //color (cset)
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+0);      //opacity
      break;
	case DRAWSTRINGR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+8);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+7);       //float x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+6);       //float y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+5);       //float font
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+4);       //float color
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+3);      //float bg_color
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+2);      //float format
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+1);      //string *ptr
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+0);      //float opacity
      {
		//
		const int index = script_drawing_commands[j][19] = j;
		const std::string str = ::ScriptHelper::getString(script_drawing_commands[j][8]);
		draw_container.drawstring.push_back( std::make_pair( index, str ) );
	  }
	  break;
    case SPLINER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+10);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+9);      //x1
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+8);      //y1
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+7);       //x2
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+6);       //y2
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+5);       //x3
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+4);       //y3
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+3);      //x4
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+2);      //y4
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+1);      //color
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+0);      //opacity
      break;
	case 111111: //for random testing :)
	  break;
  }
}

void do_push(int script, word *, byte i, bool v, refInfo &ri) {
  /*byte *sp=NULL;
  switch(script_type)
  {
    case SCRIPT_FFC:
      sp = &(tmpscr->sp[i]);
      break;
    case SCRIPT_ITEM:
      sp = &(items.spr(i)->sp);
      break;
    case SCRIPT_GLOBAL:
      sp = &g_sp;
      break;
  }*/
  long temp;
  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp = get_arg(sarg1,i,ri);
  }
  (*sp)--;
  write_stack(script,i,*sp,temp);
}

void do_pop(int script, word *, byte i, bool, refInfo &ri) {
  /*byte *sp=NULL;
  switch(script_type)
  {
  case SCRIPT_FFC:
    sp = &(tmpscr->sp[i]);
    break;
  case SCRIPT_ITEM:
    sp = &(items.spr(i)->sp);
    break;
  case SCRIPT_GLOBAL:
    sp = &g_sp;
    break;
  }*/
  long temp = read_stack(script,i,*sp);
  (*sp)++;
  set_variable(sarg1,i,temp,ri);
}

void do_loadi(int script, word *, byte i, bool, refInfo &ri) {
  //long arg1 = ffscripts[script][*pc].arg1;
  //long arg2 = ffscripts[script][*pc].arg2;
  long sp2 = get_arg(sarg2,i,ri)/10000;
  long val = read_stack(script,i,sp2);
  set_variable(sarg1,i,val,ri);
}

void do_storei(int script, word *, byte i, bool, refInfo &ri) {
  //long arg1 = ffscripts[script][*pc].arg1;
  //long arg2 = ffscripts[script][*pc].arg2;
  long sp2 = get_arg(sarg2,i,ri)/10000;
  long val = get_arg(sarg1,i,ri);
  write_stack(script,i,sp2,val);
}

void do_enqueue(int, word *, byte, bool)
{
}
void do_dequeue(int, word *, byte, bool)
{
}

void do_sfx(int, word *, byte i, bool v, refInfo &ri)
{
  //long arg1;
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = (sarg1)/10000;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  sfx(temp);
}

void do_midi(int , word *, byte i, bool v, refInfo &ri)
{
  long temp;

  if(v)
  {
    temp = (sarg1)/10000;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }

  if (temp==0)
    music_stop();
  else
    jukebox(temp+(ZC_MIDI_COUNT-1));
}

void do_loadlweapon(int , word *, byte i, bool v, refInfo &ri)
{
  long temp;
  if(v)
  {
    temp=sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  //temp is now an index into the item buffer
  if(temp >= Lwpns.Count() || temp < 0)
    temp = -1;
  else
    temp = Lwpns.spr(temp)->getUID();
  *ri.lwpn=temp;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
    (tmpscr->lwpnref[i])=temp;
    break;
    case SCRIPT_ITEM:
    (items.spr(i)->lwpnref)=temp;
    break;
    case SCRIPT_GLOBAL:
    (global_lwpn)=temp;
    break;
  }*/
}

void do_loadeweapon(int , word *, byte i, bool v, refInfo &ri)
{
  long temp;
  if(v)
  {
    temp=sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  //temp is now an index into the item buffer
  if(temp >= Ewpns.Count() || temp < 0)
    temp = -1;
  else
    temp = Ewpns.spr(temp)->getUID();
  *ri.ewpn=temp;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
    (tmpscr->ewpnref[i])=temp;
    break;
    case SCRIPT_ITEM:
    (items.spr(i)->ewpnref)=temp;
    break;
    case SCRIPT_GLOBAL:
    (global_ewpn)=temp;
    break;
  }*/
}

void do_loaditem(int , word *, byte i, bool v, refInfo &ri)
{
  long temp;
  if(v)
  {
    temp=sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  //temp is now an index into the item buffer
  if(temp >= items.Count() || temp < 0)
    temp = -1;
  else
    temp = items.spr(temp)->getUID();
  *ri.itemref=temp;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
    (tmpscr->itemref[i])=temp;
    break;
    case SCRIPT_ITEM:
    (items.spr(i)->itemref)=temp;
    break;
    case SCRIPT_GLOBAL:
    (global_item)=temp;
    break;
  }*/
}

void do_loaditemdata(int , word *, byte i, bool v, refInfo &ri)
{
  long temp;
  if(v)
  {
    temp=sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  *ri.idata=temp;

  /*switch(script_type)
  {
    case SCRIPT_FFC:
    (tmpscr->itemclass[i])=temp;
    break;
    case SCRIPT_ITEM:
    (items.spr(i)->itemclass)=temp;
    break;
    case SCRIPT_GLOBAL:
    (global_itemclass)=temp;
    break;
  }*/
}

void do_loadnpc(int , word *, byte i, bool v, refInfo &ri)
{
  long temp;
  if(v)
  {
    temp=sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }

  //temp is now an index into the guys buffer
  if(temp >= guys.Count() || temp < 0)
    temp = -1;
  else
    temp = guys.spr(temp)->getUID();
  *ri.guyref=temp;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
    (tmpscr->guyref[i])=temp;
    break;
    case SCRIPT_ITEM:
    (items.spr(i)->guyref)=temp;
    break;
    case SCRIPT_GLOBAL:
    (global_guy)=temp;
    break;
  }*/
}

void do_createlweapon(int, word *, byte i, bool v, refInfo &ri)
{

  /*dword *wpnref=NULL;
  switch(script_type)
  {
    case SCRIPT_FFC:
      wpnref=&(tmpscr->lwpnref[i]);
      break;
    case SCRIPT_ITEM:
      wpnref=&(items.spr(i)->lwpnref);
      break;
    case SCRIPT_GLOBAL:
      wpnref=&(global_lwpn);
      break;
  }*/
  //long arg1;
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  addLwpn(0,0,0,temp,0,0,0,Link.getUID());
  if(Lwpns.Count() >= 1)
  *ri.lwpn=Lwpns.spr(Lwpns.Count()-1)->getUID();
  else
  {
    Z_eventlog("Script exceeded screen LWeapon limit!\n");
//    *itemref = -1;
    //the 2 lines below are to bypass the error that the above line generates (warning: assignment of negative value `-1' to `long unsigned int')
    *ri.lwpn = 0;
    *ri.lwpn-=1;
  }
}

void do_createeweapon(int, word *, byte i, bool v, refInfo &ri)
{

  /*dword *wpnref=NULL;
  switch(script_type)
  {
    case SCRIPT_FFC:
      wpnref=&(tmpscr->ewpnref[i]);
      break;
    case SCRIPT_ITEM:
      wpnref=&(items.spr(i)->ewpnref);
      break;
    case SCRIPT_GLOBAL:
      wpnref=&(global_ewpn);
      break;
  }*/
  //long arg1;
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  addEwpn(0,0,0,temp,0,0,0,-1);
  if(Ewpns.Count() >= 1)
  *ri.ewpn=Ewpns.spr(Ewpns.Count()-1)->getUID();
  else
  {
    al_trace("Script exceeded screen EWeapon limit!\n");
//    *itemref = -1;
    //the 2 lines below are to bypass the error that the above line generates (warning: assignment of negative value `-1' to `long unsigned int')
    *ri.ewpn = 0;
    *ri.ewpn-=1;
  }
}

void do_createitem(int , word *, byte i, bool v, refInfo &ri)
{
  /*dword *itemref=NULL;
  switch(script_type)
  {
    case SCRIPT_FFC:
      itemref=&(tmpscr->itemref[i]);
      break;
    case SCRIPT_ITEM:
      itemref=&(items.spr(i)->itemref);
      break;
    case SCRIPT_GLOBAL:
      itemref=&(global_item);
      break;
  }*/
  //long arg1;
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  additem(0,(get_bit(quest_rules, qr_NOITEMOFFSET))?1:0,temp,0);
  if(items.Count() >= 1)
  *ri.itemref=items.spr(items.Count()-1)->getUID();
  else
  {
    al_trace("Script exceeded screen item limit!\n");
//    *itemref = -1;
    //the 2 lines below are to bypass the error that the above line generates (warning: assignment of negative value `-1' to `long unsigned int')
    *ri.itemref = 0;
    *ri.itemref-=1;
  }
}

void do_createnpc(int, word *, byte i, bool v, refInfo &ri)
{
  /*dword *guyref=NULL;
  switch(script_type)
  {
    case SCRIPT_FFC:
      guyref=&(tmpscr->guyref[i]);
      break;
    case SCRIPT_ITEM:
      guyref=&(items.spr(i)->guyref);
      break;
    case SCRIPT_GLOBAL:
      guyref=&(global_guy);
      break;
  }*/
  //long arg1;
  long temp;

  //arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri)/10000;
  }
  int numcreated = addenemy(0,0,temp,-10);
  if(!numcreated)
  {
    al_trace("Couldn't add NPC %ld!\n", temp);
    *ri.guyref = 0;
  }
  else
    *ri.guyref=guys.spr(guys.Count()-numcreated)->getUID(); // Get the main enemy - not a segment!!

  Z_eventlog("Script created NPC \"%s\" with UID = %ld\n", guy_string[temp], *ri.guyref);
}

void do_trace(int , word *, byte i, bool v, refInfo &ri)
{
	//long arg1;
	long temp;

	//arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  char tmp[100];
  sprintf(tmp,(temp < 0 ? "%06ld" : "%05ld"), temp);
  string s2 = tmp;
  s2 = s2.substr(0, s2.size()-4) + "." + s2.substr(s2.size()-4,4);
  al_trace("%s\n", s2.c_str());
}

void do_trace2(int , word *, byte i, bool v, refInfo &ri)
{
  long temp = sarg1;
  if(!v) temp = get_arg(sarg1,i,ri);
  al_trace("%s\n",temp?"true":"false");
}

void do_tracestring(int , word *, byte i, bool v, refInfo &ri)
{
  long temp = sarg1;
  if(!v) temp = get_arg(sarg1,i,ri);
  string str = ::ScriptHelper::getString(temp,512);
  al_trace("%s",str.c_str());
}

void do_tracenl(int,word*,byte,bool,refInfo)
{
  al_trace("\n");
}

void do_cleartrace(int,word*,byte,bool,refInfo)
{
	//Fun fact: Allegro used to be in control of allegro.log. This caused
	//problems, because it would hold on to a file handle. Even if we blank
	//the contents of the log, it will still write to the end, causing
	//lots of nulls.

	//No more!

	zc_trace_clear();
}

string inttobase(const word& base, const long& x, const word& mindigits)
{
	char coeff[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	string s2;
	for(int i=zc_max(mindigits-1,int(floor(log((double)x)/log((double)base))));i>=0;i--)
		s2 += coeff[int(floor(x/pow((double)base,i)))%base];
	return s2;
}

void do_tracetobase(int script, word *, byte i, bool, refInfo)
{
  long x = read_stack(script,i,(*sp)+2)/10000;
  unsigned long base = vbound(read_stack(script,i,(*sp)+1)/10000,2,36);
  unsigned long mindigits = zc_max(1,read_stack(script,i,*sp)/10000);
  string s2 = x<0 ? "-": "";

  switch(base){
	case 8: s2 = '0'; break;
	case 16: s2 = "0x"; break;
  }

  s2 += inttobase(base,int(fabs((double)x)),mindigits);
  switch(base){
  	case 8: case 10: case 16: break;
  	case 2: s2 += 'b'; break;
  	default:
	std::stringstream ss;
	ss << " (Base " << base << ')';
	s2 += ss.str();
		break;
  }
  al_trace("%s\n", s2.c_str());
}

void do_getscreenflags(int script, word *, byte i, bool, refInfo ri)
{
  int map = vbound(int((*(ri.d[2]))/10000),0,map_count-1);
  int scr = vbound(int((*(ri.d[1]))/10000),0,0x87);
  int flagset = vbound(int((*(ri.d[0]))/10000),0,9);

  set_variable(sarg1,i,get_screenflags(&TheMaps[map*MAPSCRS+scr],flagset),ri);
}

void do_getscreeneflags(int script, word *, byte i, bool, refInfo ri)
{
  int map = vbound(int((*(ri.d[2]))/10000),0,map_count-1);
  int scr = vbound(int((*(ri.d[1]))/10000),0,0x87);
  int flagset = vbound(int((*(ri.d[0]))/10000),0,2);

  set_variable(sarg1,i,get_screeneflags(&TheMaps[map*MAPSCRS+scr],flagset),ri);
}


/*
void do_calculate_spline(int script, word *, byte i, bool, refInfo)
{
}
*/

void do_copytile(int, word *, byte i, bool v, bool v2, refInfo &ri)
{

  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  if(v2)
  {
    temp2 = sarg2;
  }
  else
  {
    temp2=get_arg(sarg2,i,ri);
  }
  temp/=10000;
  temp2/=10000;
  copy_tile(newtilebuf, temp, temp2, false);
}

void do_swaptile(int, word *, byte i, bool v, bool v2, refInfo &ri)
{

  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  if(v2)
  {
    temp2 = sarg2;
  }
  else
  {
    temp2=get_arg(sarg2,i,ri);
  }
  temp/=10000;
  temp2/=10000;
  copy_tile(newtilebuf, temp, temp2, true);
}

void do_overlaytile(int, word *, byte i, bool v, bool v2, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  if(v2)
  {
    temp2 = sarg2;
  }
  else
  {
    temp2=get_arg(sarg2,i,ri);
  }
  temp/=10000;
  temp2/=10000;
  //overlaytile
  //fix the 0 below!
  overlay_tile(newtilebuf,temp,temp2,0,false);
}

void do_fliprotatetile(int, word *, byte i, bool v, bool v2, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp=0;
  long temp2=0;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  if(v2)
  {
    temp2 = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp/=10000;
  temp2/=10000;
  //fliprotatetile
}

void do_settilepixel(int, word *, byte i, bool v, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp=0;
  //long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  temp/=10000;
  //settilepixel
}

void do_gettilepixel(int, word *, byte i, bool v, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp=0;
  //long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  temp/=10000;
  //gettilepixel
}

void do_shifttile(int, word *, byte i, bool v, bool v2, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp=0;
  long temp2=0;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  if(v2)
  {
    temp2 = sarg2;
  }
  else
  {
    temp=get_arg(sarg2,i,ri);
  }
  temp/=10000;
  temp2/=10000;
  //shifttile
}

void do_cleartile(int, word *, byte i, bool v, refInfo &ri)
{
  //long arg1;
  //long arg2;
  long temp;
  //long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = sarg1;
  }
  else
  {
    temp=get_arg(sarg1,i,ri);
  }
  temp/=10000;
  //cleartile
  reset_tile(newtilebuf, temp, newtilebuf[temp].format);
}

void do_allocatemem(int, word *, byte i, bool v, refInfo &ri, bool local)
{
	const dword size = (v ? sarg2: get_arg(sarg2,i,ri)) / 10000;
	word ptrval;

	if(local)
	{
				/* v- 0 should be NULL pointer in ZScript. We can handle wasting one container*/
		for(ptrval = 1; localRAM[ptrval].Size() != 0; ptrval++);

		if(ptrval >= MAX_ZCARRAY_SIZE)
		{
			al_trace("%d local arrays already in use, no more can be allocated\n", MAX_ZCARRAY_SIZE-1);
			ptrval = 0;
		}
		else
			localRAM[ptrval].Allocate(size);
	}
	else
	{
		for(ptrval = 0; globalRAM[ptrval].Size() != 0; ptrval++);

		if(ptrval >= globalRAM.size())
		{
			al_trace("Invalid pointer value of %d passed to global allocate\n", ptrval);
			throw "Not enough global array containers allocated";
		}

		globalRAM[ptrval].Allocate(size);
		ptrval += MAX_ZCARRAY_SIZE; //so each pointer has a unique value
	}

	set_variable(sarg1, i, ptrval*10000, ri);

#ifdef _DEBUGARRAYALLOC
	al_trace("Allocated %s array of size %d, pointer address %d\n",
			local ? "local": "global", size, ptrval);
#endif
}

void do_deallocatemem(int , word* , byte i, bool v, refInfo &ri)
{
	word ptrval = (v ? sarg1: get_arg(sarg1,i,ri)) / 10000;

	if(localRAM[ptrval].Size() == 0 || ptrval >= MAX_ZCARRAY_SIZE)
	{
#ifdef _DEBUGARRAYALLOC
		al_trace("Tried to deallocate array with address %i\n",ptrval);
#else
		al_trace("Script tried to deallocate invalid memory\n");
#endif
	}
	else
	{
#ifdef _DEBUGARRAYALLOC
		word size = localRAM[ptrval].Size();
#endif

		localRAM[ptrval].Clear();

#ifdef _DEBUGARRAYALLOC
		al_trace("Deallocated array with address %i, size %i\n",ptrval,size);
#endif
	}
}

script_command command_list[NUMCOMMANDS+1]=
{
    //name                args arg1 arg2 more
  { "SETV",                2,   0,   1,   0},
  { "SETR",                2,   0,   0,   0},
  { "ADDR",                2,   0,   0,   0},
  { "ADDV",                2,   0,   1,   0},
  { "SUBR",                2,   0,   0,   0},
  { "SUBV",                2,   0,   1,   0},
  { "MULTR",               2,   0,   0,   0},
  { "MULTV",               2,   0,   1,   0},
  { "DIVR",                2,   0,   0,   0},
  { "DIVV",                2,   0,   1,   0},
  { "WAITFRAME",           0,   0,   0,   0},
  { "GOTO",                1,   1,   0,   0},
  { "CHECKTRIG",           0,   0,   0,   0},
  { "WARP",                2,   1,   1,   0},
  { "COMPARER",            2,   0,   0,   0},
  { "COMPAREV",            2,   0,   1,   0},
  { "GOTOTRUE",            2,   0,   0,   0},
  { "GOTOFALSE",           2,   0,   0,   0},
  { "GOTOLESS",            2,   0,   0,   0},
  { "GOTOMORE",            2,   0,   0,   0},
  { "LOAD1",               2,   0,   0,   0},
  { "LOAD2",               2,   0,   0,   0},
  { "SETA1",               2,   0,   0,   0},
  { "SETA2",               2,   0,   0,   0},
  { "QUIT",                0,   0,   0,   0},
  { "SINR",                2,   0,   0,   0},
  { "SINV",                2,   0,   1,   0},
  { "COSR",                2,   0,   0,   0},
  { "COSV",                2,   0,   1,   0},
  { "TANR",                2,   0,   0,   0},
  { "TANV",                2,   0,   1,   0},
  { "MODR",                2,   0,   0,   0},
  { "MODV",                2,   0,   1,   0},
  { "ABS",                 1,   0,   0,   0},
  { "MINR",                2,   0,   0,   0},
  { "MINV",                2,   0,   1,   0},
  { "MAXR",                2,   0,   0,   0},
  { "MAXV",                2,   0,   1,   0},
  { "RNDR",                2,   0,   0,   0},
  { "RNDV",                2,   0,   1,   0},
  { "FACTORIAL",           1,   0,   0,   0},
  { "POWERR",              2,   0,   0,   0},
  { "POWERV",              2,   0,   1,   0},
  { "IPOWERR",             2,   0,   0,   0},
  { "IPOWERV",             2,   0,   1,   0},
  { "ANDR",                2,   0,   0,   0},
  { "ANDV",                2,   0,   1,   0},
  { "ORR",                 2,   0,   0,   0},
  { "ORV",                 2,   0,   1,   0},
  { "XORR",                2,   0,   0,   0},
  { "XORV",                2,   0,   1,   0},
  { "NANDR",               2,   0,   0,   0},
  { "NANDV",               2,   0,   1,   0},
  { "NORR",                2,   0,   0,   0},
  { "NORV",                2,   0,   1,   0},
  { "XNORR",               2,   0,   0,   0},
  { "XNORV",               2,   0,   1,   0},
  { "NOT",                 1,   0,   0,   0},
  { "LSHIFTR",             2,   0,   0,   0},
  { "LSHIFTV",             2,   0,   1,   0},
  { "RSHIFTR",             2,   0,   0,   0},
  { "RSHIFTV",             2,   0,   1,   0},
  { "TRACER",              1,   0,   0,   0},
  { "TRACEV",              1,   1,   0,   0},
  { "TRACE3",              0,   0,   0,   0},
  { "LOOP",                2,   1,   0,   0},
  { "PUSHR",               1,   0,   0,   0},
  { "PUSHV",               1,   1,   0,   0},
  { "POP",                 1,   0,   0,   0},
  { "ENQUEUER",            2,   0,   0,   0},
  { "ENQUEUEV",            2,   0,   1,   0},
  { "DEQUEUE",             1,   0,   0,   0},
  { "PLAYSOUNDR",          1,   0,   0,   0},
  { "PLAYSOUNDV",          1,   1,   0,   0},
  { "LOADLWEAPONR",        1,   0,   0,   0},
  { "LOADLWEAPONV",        1,   1,   0,   0},
  { "LOADITEMR",           1,   0,   0,   0},
  { "LOADITEMV",           1,   1,   0,   0},
  { "LOADNPCR",            1,   0,   0,   0},
  { "LOADNPCV",            1,   1,   0,   0},
  { "CREATELWEAPONR",      1,   0,   0,   0},
  { "CREATELWEAPONV",      1,   1,   0,   0},
  { "CREATEITEMR",         1,   0,   0,   0},
  { "CREATEITEMV",         1,   1,   0,   0},
  { "CREATENPCR",          1,   0,   0,   0},
  { "CREATENPCV",          1,   1,   0,   0},
  { "LOADI",               2,   0,   0,   0},
  { "STOREI",              2,   0,   0,   0},
  { "GOTOR",               1,   0,   0,   0},
  { "SQROOTV",             2,   0,   1,   0},
  { "SQROOTR",             2,   0,   0,   0},
  { "CREATEEWEAPONR",      1,   0,   0,   0},
  { "CREATEEWEAPONV",      1,   1,   0,   0},
  { "PITWARP",             2,   1,   1,   0},
  { "WARPR",               2,   0,   0,   0},
  { "PITWARPR",            2,   0,   0,   0},
  { "CLEARSPRITESR",       1,   0,   0,   0},
  { "CLEARSPRITESV",       1,   1,   0,   0},
  { "RECT",                0,   0,   0,   0},
  { "CIRCLE",              0,   0,   0,   0},
  { "ARC",                 0,   0,   0,   0},
  { "ELLIPSE",             0,   0,   0,   0},
  { "LINE",                0,   0,   0,   0},
  { "PUTPIXEL",            0,   0,   0,   0},
  { "DRAWTILE",            0,   0,   0,   0},
  { "DRAWCOMBO",           0,   0,   0,   0},
  { "ELLIPSE2",            0,   0,   0,   0},
  { "SPLINE",              0,   0,   0,   0},
  { "FLOODFILL",           0,   0,   0,   0},
  { "COMPOUNDR",           1,   0,   0,   0},
  { "COMPOUNDV",           1,   1,   0,   0},
  { "MSGSTRR",             1,   0,   0,   0},
  { "MSGSTRV",             1,   1,   0,   0},
  { "ISVALIDITEM",         1,   0,   0,   0},
  { "ISVALIDNPC",          1,   0,   0,   0},
  { "PLAYMIDIR",           1,   0,   0,   0},
  { "PLAYMIDIV",           1,   1,   0,   0},
  { "COPYTILEVV",          2,   1,   1,   0},
  { "COPYTILEVR",          2,   1,   0,   0},
  { "COPYTILERV",          2,   0,   1,   0},
  { "COPYTILERR",          2,   0,   0,   0},
  { "SWAPTILEVV",          2,   1,   1,   0},
  { "SWAPTILEVR",          2,   1,   0,   0},
  { "SWAPTILERV",          2,   0,   1,   0},
  { "SWAPTILERR",          2,   0,   0,   0},
  { "CLEARTILEV",          1,   1,   0,   0},
  { "CLEARTILER",          1,   0,   0,   0},
  { "OVERLAYTILEVV",       2,   1,   1,   0},
  { "OVERLAYTILEVR",       2,   1,   0,   0},
  { "OVERLAYTILERV",       2,   0,   1,   0},
  { "OVERLAYTILERR",       2,   0,   0,   0},
  { "FLIPROTTILEVV",       2,   1,   1,   0},
  { "FLIPROTTILEVR",       2,   1,   0,   0},
  { "FLIPROTTILERV",       2,   0,   1,   0},
  { "FLIPROTTILERR",       2,   0,   0,   0},
  { "GETTILEPIXELV",       1,   1,   0,   0},
  { "GETTILEPIXELR",       1,   0,   0,   0},
  { "SETTILEPIXELV",       1,   1,   0,   0},
  { "SETTILEPIXELR",       1,   0,   0,   0},
  { "SHIFTTILEVV",         2,   1,   1,   0},
  { "SHIFTTILEVR",         2,   1,   0,   0},
  { "SHIFTTILERV",         2,   0,   1,   0},
  { "SHIFTTILERR",         2,   0,   0,   0},
  { "ISVALIDLWPN",         1,   0,   0,   0},
  { "ISVALIDEWPN",         1,   0,   0,   0},
  { "LOADEWEAPONR",        1,   0,   0,   0},
  { "LOADEWEAPONV",        1,   1,   0,   0},
  { "ALLOCATEMEMR",        2,   0,   0,   0},
  { "ALLOCATEMEMV",        2,   0,   1,   0},
  { "DECLARE",             0,   0,   0,   0},
  { "DEALLOCATEMEMR",      1,   0,   0,   0},
  { "DEALLOCATEMEMV",      1,   1,   0,   0},
  { "WAITDRAW",		  0,   0,   0,   0},
  { "ARCTANR",		       1,   0,   0,   0},
  { "LWPNUSESPRITER",      1,   0,   0,   0},
  { "LWPNUSESPRITEV",      1,   1,   0,   0},
  { "EWPNUSESPRITER",      1,   0,   0,   0},
  { "EWPNUSESPRITEV",      1,   1,   0,   0},
  { "LOADITEMDATAR",       1,   0,   0,   0},
  { "LOADITEMDATAV",       1,   1,   0,   0},
  { "BITNOT",              1,   0,   0,   0},
  { "LOG10",               1,   0,   0,   0},
  { "LOGE",                1,   0,   0,   0},
  { "ISSOLID",             1,   0,   0,   0},
  { "LAYERSCREEN",         2,   0,   0,   0},
  { "LAYERMAP",            2,   0,   0,   0},
  { "TRACE2R",             1,   0,   0,   0},
  { "TRACE2V",             1,   1,   0,   0},
  { "TRACE4",              0,   0,   0,   0},
  { "TRACE5",              0,   0,   0,   0},
  { "SECRETS",			  0,   0,   0,   0},
  { "DRAWCHAR",            0,   0,   0,   0},
  { "GETSCREENFLAGS",      0,   0,   0,   0},
  { "QUAD",                0,   0,   0,   0},
  { "TRIANGLE",            0,   0,   0,   0},
  { "QUAD3D",              0,   0,   0,   0},
  { "TRIANGLE3D",          0,   0,   0,   0},
  { "ARCSINR",             2,   0,   0,   0},
  { "ARCSINV",             2,   1,   0,   0},
  { "ARCCOSR",             2,   0,   0,   0},
  { "ARCCOSV",             2,   1,   0,   0},
  { "DRAWINT",             0,   0,   0,   0},
  { "SETTRUE",             1,   0,   0,   0},
  { "SETFALSE",            1,   0,   0,   0},
  { "SETMORE",             1,   0,   0,   0},
  { "SETLESS",             1,   0,   0,   0},
  { "FASTTILE",            0,   0,   0,   0},
  { "FASTCOMBO",           0,   0,   0,   0},
  { "DRAWSTRING",          0,   0,   0,   0},
  { "SETSIDEWARP",         0,   0,   0,   0},
  { "SAVE",                0,   0,   0,   0},
  { "TRACE6",              0,   0,   0,   0},
  { "PTROFF",              1,   0,   0,   0},
  { "SETCOLORB",           0,   0,   0,   0},
  { "SETDEPTHB",           0,   0,   0,   0},
  { "GETCOLORB",           0,   0,   0,   0},
  { "GETDEPTHB",           0,   0,   0,   0},
  { "",                    0,   0,   0,   0}
};


// Let's do this
int run_ff_script(int script, byte i)
{
  //word scommand;
#ifdef _SCRIPT_COUNTER
  unsigned long script_timer[NUMCOMMANDS];
  unsigned long script_execount[NUMCOMMANDS];
  for(int j=0; j<NUMCOMMANDS; j++) { script_timer[j]=0; script_execount[j]=0; }
  unsigned long start_time, end_time;

  script_counter = 0;
#endif

  word pc = tmpscr->pc[i];
  scommand = ffscripts[script][pc].command;
  sarg1 = ffscripts[script][pc].arg1;
  sarg2 = ffscripts[script][pc].arg2;
  word ffs = tmpscr->ffscript[i];
  dword sflag = tmpscr->scriptflag[i];

  //scommand = 0;  //to get gcc to stop complaining aout unused variables
  //long arg1=0;
  //long arg2=0;
  //arg1=arg2; //to avoid unused variables warnings
  //word *pc=NULL;
  //word *ffs=NULL;
  //dword *sflag=NULL;
  //script_type = stype;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
      pc = &(tmpscr->pc[i]);
      command = &(ffscripts[script][*pc].command);
      arg1 = &(ffscripts[script][*pc].arg1);
      arg2 = &(ffscripts[script][*pc].arg2);
      ffs = &(tmpscr->ffscript[i]);
      sflag = &(tmpscr->scriptflag[i]);
      tmpscr->ffcref[i]=i;
	  st = &(ffstack[i]);
      ffs2 = &(tmpscr->ffscript[i]);
	  sp = &(tmpscr->sp[i]);
	  na[0] = &(tmpscr->a[i][0]);
      na[1] = &(tmpscr->a[i][1]);
	  scriptflag = &(tmpscr->scriptflag[i]);
      break;
    case SCRIPT_ITEM:
      pc = &(pc);
      command = &(itemscripts[script][*pc].command);
      arg1 = &(itemscripts[script][*pc].arg1);
      arg2 = &(itemscripts[script][*pc].arg2);
      ffs = &(doscript);
      sflag = &(scriptflag);
      items.spr(i)->itemref = i;
	  st = &(items.spr(i)->stack);
      ffs2 = &(doscript);
	  sp = &(items.spr(i)->sp);
	  na[0] = &(items.spr(i)->a[0]);
      na[1] = &(items.spr(i)->a[1]);
	  scriptflag = &(scriptflag);
      break;
    case SCRIPT_GLOBAL:
      pc = &pc;
      command = &(itemscripts[script][*pc].command);
      arg1 = &(itemscripts[script][*pc].arg1);
      arg2 = &(itemscripts[script][*pc].arg2);
      ffs = &doscript;
      sflag = &scriptflag;
	  st = &g_stack;
      ffs2 = &doscript;
	  sp = &g_sp;
	  na[0] = NULL;
      na[1] = NULL;
	  scriptflag = &scriptflag;
      break;
  }*/

  //these don't change during the execution of a script
  //so there's no need to recompute them every time we set or get a var
  //(which happens A LOT in big scripts)
  refInfo ri;

      ri.ffc = &(tmpscr->ffcref[i]);
      ri.itemref = &(tmpscr->itemref[i]);
      ri.idata = &(tmpscr->itemclass[i]);
      ri.lwpn = &(tmpscr->lwpnref[i]);
      ri.ewpn = &(tmpscr->ewpnref[i]);
      ri.guyref = &(tmpscr->guyref[i]);
	 ri.ramref = &(tmpscr->ramref[i]);
      ri.gclass = &(tmpscr->guyclass[i]);
      ri.lclass = &(tmpscr->lwpnclass[i]);
      ri.eclass = &(tmpscr->ewpnclass[i]);
      ri.sp = &(tmpscr->sp[i]);
      for(int j=0;j<8;j++)
      {
        ri.d[j] = &(tmpscr->d[i][j]);
      }
      for(int j=0; j<2; j++)
      {
        ri.a[j] = &(tmpscr->a[i][j]);
      }
	  //pc = &(tmpscr->pc[i]);
      //command = &(ffscripts[script][pc].command);
      //arg1 = &(ffscripts[script][pc].arg1);
      //arg2 = &(ffscripts[script][pc].arg2);
      //ffs = &(tmpscr->ffscript[i]);
      //sflag = &(tmpscr->scriptflag[i]);
      tmpscr->ffcref[i]=i;
	  st = &(ffstack[i]);
      ffs2 = &(tmpscr->ffscript[i]);
	  sp = &(tmpscr->sp[i]);
	  na[0] = &(tmpscr->a[i][0]);
      na[1] = &(tmpscr->a[i][1]);
	  scriptflag = &sflag;//&(tmpscr->scriptflag[i]);


#ifdef _DEBUGPRINTSCOMMAND
	al_trace("\nStart of FFC script processing:\n");
#endif

  bool increment = true;

  while( (scommand!=0xFFFF) && (scommand!=WAITFRAME) &&
	  (ffs != 0) //-Can ffs change during the execution of a script frame? -gleeok
			   //-Yeah, QUIT sets it to 0 ~Joe123
	  )
  {
	if( key[KEY_F4] && (key[KEY_ALT]||key[KEY_ALTGR]) )
	{
		quit_game();
		exit(101);
	}

#ifdef _DEBUGPRINTSCOMMAND
	al_trace("scommand: %i\n",scommand);
#endif

#ifdef _SCRIPT_COUNTER
	start_time = script_counter;
#endif

    switch(scommand)
    {
	  case SETTRUE:
	    set_variable(sarg1,i,(sflag & TRUEFLAG?1:0), ri); break;
	  case SETFALSE:
	    set_variable(sarg1,i,(sflag & TRUEFLAG?0:1), ri); break;
	  case SETMORE:
	    set_variable(sarg1,i,(sflag & MOREFLAG?1:0), ri); break;
	  case SETLESS:
	    set_variable(sarg1,i,(!(sflag & MOREFLAG) || (sflag & TRUEFLAG))?1:0, ri); break;
      case SETV:
        do_set(script, &pc, i, true,ri); break;
      case SETR:
        do_set(script, &pc, i, false,ri); break;
      case ADDV:
        do_add(script, &pc, i, true,ri); break;
      case ADDR:
        do_add(script, &pc, i, false,ri); break;
      case SUBV:
        do_sub(script, &pc, i, true,ri); break;
      case SUBR:
        do_sub(script, &pc, i, false,ri); break;
      case MULTV:
        do_mult(script, &pc, i, true,ri); break;
      case MULTR:
        do_mult(script, &pc, i, false,ri); break;
      case DIVV:
        do_div(script, &pc, i, true,ri); break;
      case DIVR:
        do_div(script, &pc, i, false,ri); break;
      case MODV:
        do_mod(script, &pc, i, true,ri); break;
      case MODR:
        do_mod(script, &pc, i, false,ri); break;
      case GOTO:
        pc = sarg1; increment = false; break;
      case CHECKTRIG:
        break;
      case WARP:
        tmpscr->sidewarpdmap[0] = (sarg1)/10000;
        tmpscr->sidewarpscr[0] = (sarg2)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
      case WARPR:
        tmpscr->sidewarpdmap[0] = get_arg(sarg1,i,ri)/10000;
        tmpscr->sidewarpscr[0] = get_arg(sarg2,i,ri)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
      case PITWARP:
        tmpscr->sidewarpdmap[0] = (sarg1)/10000;
        tmpscr->sidewarpscr[0] = (sarg2)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
      case PITWARPR:
        tmpscr->sidewarpdmap[0] = get_arg(sarg1,i,ri)/10000;
        tmpscr->sidewarpscr[0] = get_arg(sarg2,i,ri)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
      case COMPAREV:
        do_comp(script, &pc, i, true,ri); break;
      case COMPARER:
        do_comp(script, &pc, i, false,ri); break;
      case GOTOTRUE:
        if(sflag & TRUEFLAG)
        {pc = sarg1; increment = false;} break;
      case GOTOFALSE:
        if(!(sflag & TRUEFLAG))
        {pc = sarg1; increment = false;} break;
      case GOTOMORE:
        if(sflag & MOREFLAG)
        {pc = sarg1; increment = false;} break;
      case GOTOLESS:
        if(!(sflag & MOREFLAG) || (!get_bit(quest_rules,qr_GOTOLESSNOTEQUAL) && (sflag & TRUEFLAG)))
        {pc = sarg1; increment = false;} break;
      case LOAD1:
        do_loada(script, &pc, i, 0,ri); break;
      case LOAD2:
        do_loada(script, &pc, i, 1,ri); break;
      case SETA1:
        do_seta(script, &pc, i, 0,ri); break;
      case SETA2:
        do_seta(script, &pc, i, 1,ri); break;
	  case GAMEEND:
	    Quit = qQUIT; skipcont=1; //fallthrough
      case QUIT:
        ffs = 0; break;
      case SINV:
        do_trig(script, &pc, i, true, 0,ri); break;
      case SINR:
        do_trig(script, &pc, i, false, 0,ri); break;
      case COSV:
        do_trig(script, &pc, i, true, 1,ri); break;
      case COSR:
        do_trig(script, &pc, i, false, 1,ri); break;
      case TANV:
        do_trig(script, &pc, i, true, 2,ri); break;
      case TANR:
        do_trig(script, &pc, i, false, 2,ri); break;
      case ARCSINR:
        do_asin(script, &pc, i, false,ri); break;
      case ARCCOSR:
        do_acos(script, &pc, i, false,ri); break;
      case ABSR:
        do_abs(script, &pc, i, false,ri); break;
      case LOG10:
        do_log10(script, &pc, i, false,ri); break;
      case LOGE:
        do_naturallog(script, &pc, i, false,ri); break;
      case MINR:
        do_min(script, &pc, i, false,ri); break;
      case MINV:
        do_min(script, &pc, i, true,ri); break;
      case MAXR:
        do_max(script, &pc, i, false,ri); break;
      case MAXV:
        do_max(script, &pc, i, true,ri); break;
      case RNDR:
        do_rnd(script, &pc, i, false,ri); break;
      case RNDV:
        do_rnd(script, &pc, i, true,ri); break;
      case FACTORIAL:
        do_factorial(script, &pc, i, false,ri); break;
      case POWERR:
        do_power(script, &pc, i, false,ri); break;
      case POWERV:
        do_power(script, &pc, i, true,ri); break;
      case IPOWERR:
        do_ipower(script, &pc, i, false,ri); break;
      case IPOWERV:
        do_ipower(script, &pc, i, true,ri); break;
      case ANDR:
        do_and(script, &pc, i, false,ri); break;
      case ANDV:
        do_and(script, &pc, i, true,ri); break;
      case ORR:
        do_or(script, &pc, i, false,ri); break;
      case ORV:
        do_or(script, &pc, i, true,ri); break;
      case XORR:
        do_xor(script, &pc, i, false,ri); break;
      case XORV:
        do_xor(script, &pc, i, true,ri); break;
      case NANDR:
        do_nand(script, &pc, i, false,ri); break;
      case NANDV:
        do_nand(script, &pc, i, true,ri); break;
      case NORR:
        do_nor(script, &pc, i, false,ri); break;
      case NORV:
        do_nor(script, &pc, i, true,ri); break;
      case XNORR:
        do_xnor(script, &pc, i, false,ri); break;
      case XNORV:
        do_xnor(script, &pc, i, true,ri); break;
      case NOT:
        do_not(script, &pc, i, false,ri); break;
      case BITNOT:
        do_bitwisenot(script, &pc, i, false,ri); break;
      case LSHIFTR:
        do_lshift(script, &pc, i, false,ri); break;
      case LSHIFTV:
        do_lshift(script, &pc, i, true,ri); break;
      case RSHIFTR:
        do_rshift(script, &pc, i, false,ri); break;
      case RSHIFTV:
        do_rshift(script, &pc, i, true,ri); break;
      case TRACER:
        do_trace(script, &pc, i, false,ri); break;
      case TRACEV:
        do_trace(script, &pc, i, true,ri); break;
      case TRACE2R:
        do_trace2(script, &pc, i, false,ri); break;
      case TRACE2V:
        do_trace2(script, &pc, i, true,ri); break;
      case TRACE3:
        do_tracenl(script, &pc, i, false,ri); break;
      case TRACE4:
        do_cleartrace(script, &pc, i, false,ri); break;
      case TRACE5:
        do_tracetobase(script, &pc, i, false,ri); break;
      case TRACE6:
        do_tracestring(script, &pc, i, false,ri); break;
      case LOOP:
        if(get_arg(sarg2,i,ri)>0)
        {
          pc = sarg1;
          increment = false;
        }
        else
        {
          set_variable(sarg1,i,sarg1-1,ri);
        }
        break;
      case PUSHR:
        do_push(script, &pc, i, false,ri); break;
      case PUSHV:
        do_push(script, &pc, i, true,ri); break;
      case POP:
        do_pop(script, &pc, i, false,ri); break;
      case ENQUEUER:
        do_enqueue(script, &pc, i, false); break;
      case ENQUEUEV:
        do_enqueue(script, &pc, i, true); break;
      case DEQUEUE:
        do_dequeue(script, &pc, i, false); break;
      case PLAYSOUNDR:
        do_sfx(script, &pc, i, false,ri); break;
      case PLAYSOUNDV:
        do_sfx(script, &pc, i, true,ri); break;
      case PLAYMIDIR:
        do_midi(script, &pc, i, false,ri); break;
      case PLAYMIDIV:
        do_midi(script, &pc, i, true,ri); break;
      case LOADLWEAPONR:
        do_loadlweapon(script, &pc, i, false,ri); break;
      case LOADLWEAPONV:
        do_loadlweapon(script, &pc, i, true,ri); break;
      case LOADEWEAPONR:
        do_loadeweapon(script, &pc, i, false,ri); break;
      case LOADEWEAPONV:
        do_loadeweapon(script, &pc, i, true,ri); break;
      case LOADITEMR:
        do_loaditem(script, &pc, i, false,ri); break;
      case LOADITEMV:
        do_loaditem(script, &pc, i, true,ri); break;
      case LOADITEMDATAR:
        do_loaditemdata(script, &pc, i, false,ri); break;
      case LOADITEMDATAV:
        do_loaditemdata(script, &pc, i, true,ri); break;
      case LOADNPCR:
        do_loadnpc(script, &pc, i, false,ri); break;
      case LOADNPCV:
        do_loadnpc(script, &pc, i, true,ri); break;
      case CREATELWEAPONR:
        do_createlweapon(script, &pc, i, false,ri); break;
      case CREATELWEAPONV:
        do_createlweapon(script, &pc, i, true,ri); break;
      case CREATEEWEAPONR:
        do_createeweapon(script, &pc, i, false,ri); break;
      case CREATEEWEAPONV:
        do_createeweapon(script, &pc, i, true,ri); break;
      case CREATEITEMR:
        do_createitem(script, &pc, i, false,ri); break;
      case CREATEITEMV:
        do_createitem(script, &pc, i, true,ri); break;
      case CREATENPCR:
        do_createnpc(script, &pc, i, false,ri); break;
      case CREATENPCV:
        do_createnpc(script, &pc, i, true,ri); break;
      case LOADI:
        do_loadi(script,&pc,i,true,ri); break;
      case STOREI:
        do_storei(script,&pc,i,true,ri); break;
      case GOTOR:
        {
          int tmp2 = (get_arg(sarg1,i,ri)/10000)-1;
          pc = tmp2;
          increment = false;
        }
        break;
      case SQROOTV:
        do_sqroot(script,&pc,i,true,ri); break;
      case SQROOTR:
        do_sqroot(script,&pc,i,false,ri); break;
      case CLEARSPRITESR:
        do_clearsprites(script, &pc, i, false,ri); break;
      case CLEARSPRITESV:
        do_clearsprites(script, &pc, i, true,ri); break;
      case MSGSTRR:
        do_message(script, &pc, i, false,ri); break;
      case MSGSTRV:
        do_message(script, &pc, i, true,ri); break;
      case ISSOLID:
        do_issolid(script, &pc, i, ri); break;
	 case SETSIDEWARP:
	   do_setsidewarp(script, &pc, i, ri); break;
	 case SETTILEWARP:
	   do_settilewarp(script, &pc, i, ri); break;
      case LAYERSCREEN:
        do_layerscreen(script, &pc, i, ri); break;
      case LAYERMAP:
        do_layermap(script, &pc, i, ri); break;
	 case SECRETS:
	   do_triggersecrets(script, &pc, i, ri); break;
      case ISVALIDITEM:
        do_isvaliditem(script, &pc, i, ri); break;
      case ISVALIDNPC:
        do_isvalidnpc(script, &pc, i, ri); break;
    case ISVALIDLWPN:
      do_isvalidlwpn(script, &pc, i, ri); break;
    case ISVALIDEWPN:
      do_isvalidewpn(script, &pc, i, ri); break;
    case LWPNUSESPRITER:
      do_lwpnusesprite(script, &pc, i, false, ri); break;
    case LWPNUSESPRITEV:
      do_lwpnusesprite(script, &pc, i, true, ri); break;
    case EWPNUSESPRITER:
      do_ewpnusesprite(script, &pc, i, false, ri); break;
    case EWPNUSESPRITEV:
      do_ewpnusesprite(script, &pc, i, true, ri); break;
      case RECTR:
      case CIRCLER:
      case ARCR:
      case ELLIPSER:
      case LINER:
      case PUTPIXELR:
      case DRAWTILER:
      case DRAWCOMBOR:
      case DRAWCHARR:
	  case DRAWINTR:
	  case QUADR:
	  case TRIANGLER:
	  case QUAD3DR:
	  case TRIANGLE3DR:
	  case FASTTILER:
	  case FASTCOMBOR:
	  case DRAWSTRINGR:
	  case SPLINER:
        do_drawing_command(script, &pc, i, scommand,ri); break;
      case COPYTILEVV:
        do_copytile(script, &pc, i, true, true, ri); break;
      case COPYTILEVR:
        do_copytile(script, &pc, i, true, false, ri); break;
      case COPYTILERV:
        do_copytile(script, &pc, i, false, true, ri); break;
      case COPYTILERR:
        do_copytile(script, &pc, i, false, false, ri); break;
      case SWAPTILEVV:
        do_swaptile(script, &pc, i, true, true, ri); break;
      case SWAPTILEVR:
        do_swaptile(script, &pc, i, true, false, ri); break;
      case SWAPTILERV:
        do_swaptile(script, &pc, i, false, true, ri); break;
      case SWAPTILERR:
        do_swaptile(script, &pc, i, false, false, ri); break;
      case CLEARTILEV:
        do_cleartile(script, &pc, i, true, ri); break;
      case CLEARTILER:
        do_cleartile(script, &pc, i, false, ri); break;
      case OVERLAYTILEVV:
        do_overlaytile(script, &pc, i, true, true, ri); break;
      case OVERLAYTILEVR:
        do_overlaytile(script, &pc, i, true, false, ri); break;
      case OVERLAYTILERV:
        do_overlaytile(script, &pc, i, false, true, ri); break;
      case OVERLAYTILERR:
        do_overlaytile(script, &pc, i, false, false, ri); break;
      case FLIPROTTILEVV:
        do_fliprotatetile(script, &pc, i, true, true, ri); break;
      case FLIPROTTILEVR:
        do_fliprotatetile(script, &pc, i, true, false, ri); break;
      case FLIPROTTILERV:
        do_fliprotatetile(script, &pc, i, false, true, ri); break;
      case FLIPROTTILERR:
        do_fliprotatetile(script, &pc, i, false, false, ri); break;
      case GETTILEPIXELV:
        do_gettilepixel(script, &pc, i, true, ri); break;
      case GETTILEPIXELR:
        do_gettilepixel(script, &pc, i, false, ri); break;
      case SETTILEPIXELV:
        do_settilepixel(script, &pc, i, true, ri); break;
      case SETTILEPIXELR:
        do_settilepixel(script, &pc, i, false, ri); break;
      case SHIFTTILEVV:
        do_shifttile(script, &pc, i, true, true, ri); break;
      case SHIFTTILEVR:
        do_shifttile(script, &pc, i, true, false, ri); break;
      case SHIFTTILERV:
        do_shifttile(script, &pc, i, false, true, ri); break;
      case SHIFTTILERR:
        do_shifttile(script, &pc, i, false, false, ri); break;
	  case ALLOCATEMEMR:
        do_allocatemem(script, &pc, i, false, ri, true); break;
	  case ALLOCATEMEMV:
        do_allocatemem(script, &pc, i, true, ri, true); break;
	  case DEALLOCATEMEMR:
        do_deallocatemem(script, &pc, i, false, ri); break;
	  case DEALLOCATEMEMV:
        do_deallocatemem(script, &pc, i, true, ri); break;
	  case SAVE:
	    save_game(false); break;
	  case ARCTANR:
		do_arctan(script, &pc, i, false, ri); break;
	  case SETCOLORB:
	  case SETDEPTHB:
	  case GETCOLORB:
	  case GETDEPTHB:
		  break;
	  case GETSCREENFLAGS:
		do_getscreenflags(script, &pc, i, false, ri); break;
		  break;
	  case GETSCREENEFLAGS:
		do_getscreeneflags(script, &pc, i, false, ri); break;
	  case COMBOTILE:
		do_combotile(script, &pc, i, ri); break;
    }

#ifdef _SCRIPT_COUNTER
	end_time=script_counter;
	script_timer[*command] += end_time-start_time;
	++script_execount[*command];
#endif

	if(increment)
		pc++;
	else
		increment = true;

	scommand = ffscripts[script][pc].command;
     sarg1 = ffscripts[script][pc].arg1;
     sarg2 = ffscripts[script][pc].arg2;
    /*switch(script_type)
    {
      case SCRIPT_FFC:
        command = &(ffscripts[script][*pc].command);
        arg1 = &(ffscripts[script][*pc].arg1);
        arg2 = &(ffscripts[script][*pc].arg2);
        break;
      case SCRIPT_ITEM:
        command = &(itemscripts[script][*pc].command);
        arg1 = &(itemscripts[script][*pc].arg1);
        arg2 = &(itemscripts[script][*pc].arg2);
        break;
      case SCRIPT_GLOBAL:
        command = &(itemscripts[script][*pc].command);
        arg1 = &(itemscripts[script][*pc].arg1);
        arg2 = &(itemscripts[script][*pc].arg2);
        break;
    }*/
  }
  tmpscr->pc[i] = pc;
  tmpscr->ffscript[i] = ffs;
  if(scommand==0xFFFF)
	tmpscr->ffscript[i] = 0;
  else
	tmpscr->pc[i]++;

#ifdef _SCRIPT_COUNTER
  for(int j=0; j<NUMCOMMANDS; j++)
  {
	  if(script_execount[j] != 0)
		  al_trace("Command %s took %ld ticks in all to complete in %ld executions.\n", command_list[j].name, script_timer[j], script_execount[j]);
  }
  remove_int(update_script_counter);
#endif

  return 0;
}

int run_item_script(int script, byte i)
{
  word scommand;

#ifdef _SCRIPT_COUNTER
  unsigned long script_timer[NUMCOMMANDS];
  unsigned long script_execount[NUMCOMMANDS];
  for(int j=0; j<NUMCOMMANDS; j++) { script_timer[j]=0; script_execount[j]=0; }
  unsigned long start_time, end_time;

  script_counter = 0;
#endif

  word pc = items.spr(i)->pc;
  scommand = itemscripts[script][pc].command;
  sarg1 = itemscripts[script][pc].arg1;
  sarg2 = itemscripts[script][pc].arg2;
  word ffs = items.spr(i)->doscript;
  dword sflag = items.spr(i)->scriptflag;

  //scommand = 0;  //to get gcc to stop complaining aout unused variables
  //long arg1=0;
  //long arg2=0;
  //arg1=arg2; //to avoid unused variables warnings
  //word *pc=NULL;
  //word *ffs=NULL;
  //dword *sflag=NULL;
  //script_type = stype;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
      pc = &(tmpscr->pc[i]);
      command = &(ffscripts[script][*pc].command);
      arg1 = &(ffscripts[script][*pc].arg1);
      arg2 = &(ffscripts[script][*pc].arg2);
      ffs = &(tmpscr->ffscript[i]);
      sflag = &(tmpscr->scriptflag[i]);
      tmpscr->ffcref[i]=i;
	  st = &(ffstack[i]);
      ffs2 = &(tmpscr->ffscript[i]);
	  sp = &(tmpscr->sp[i]);
	  na[0] = &(tmpscr->a[i][0]);
      na[1] = &(tmpscr->a[i][1]);
	  scriptflag = &(tmpscr->scriptflag[i]);
      break;
    case SCRIPT_ITEM:
      pc = &(pc);
      command = &(itemscripts[script][*pc].command);
      arg1 = &(itemscripts[script][*pc].arg1);
      arg2 = &(itemscripts[script][*pc].arg2);
      ffs = &(doscript);
      sflag = &(scriptflag);
      items.spr(i)->itemref = i;
	  st = &(items.spr(i)->stack);
      ffs2 = &(doscript);
	  sp = &(items.spr(i)->sp);
	  na[0] = &(items.spr(i)->a[0]);
      na[1] = &(items.spr(i)->a[1]);
	  scriptflag = &(scriptflag);
      break;
    case SCRIPT_GLOBAL:
      pc = &pc;
      command = &(itemscripts[script][*pc].command);
      arg1 = &(itemscripts[script][*pc].arg1);
      arg2 = &(itemscripts[script][*pc].arg2);
      ffs = &doscript;
      sflag = &scriptflag;
	  st = &g_stack;
      ffs2 = &doscript;
	  sp = &g_sp;
	  na[0] = NULL;
      na[1] = NULL;
	  scriptflag = &scriptflag;
      break;
  }*/

  //these don't change during the execution of a script
  //so there's no need to recompute them every time we set or get a var
  //(which happens A LOT in big scripts)
  refInfo ri;

      ri.ffc = &(items.spr(i)->ffcref);
      ri.itemref = &(items.spr(i)->itemref);
      ri.idata = &(items.spr(i)->itemclass);
      ri.lwpn = &(items.spr(i)->lwpnref);
      ri.ewpn = &(items.spr(i)->ewpnref);
      ri.guyref = &(items.spr(i)->guyref);
	  ri.ramref = &(items.spr(i)->ramref);
      ri.gclass = &(items.spr(i)->guyclass);
      ri.lclass = &(items.spr(i)->lwpnclass);
      ri.eclass = &(items.spr(i)->ewpnclass);
      ri.sp = &(items.spr(i)->sp);
      for(int j=0;j<8;j++)
      {
        ri.d[j] = &(items.spr(i)->d[j]);
      }
      for(int j=0;j<2;j++)
      {
        ri.a[j] = &(items.spr(i)->a[j]);
      }
	  //pc = &(items.spr(i)->pc);
      //command = &(itemscripts[script][pc].command);
     // arg1 = &(itemscripts[script][pc].arg1);
      //arg2 = &(itemscripts[script][pc].arg2);
      //ffs = &(items.spr(i)->doscript);
      //sflag = &(items.spr(i)->scriptflag);
      items.spr(i)->itemref = i;
	  st = &(items.spr(i)->stack);
      ffs2 = &(items.spr(i)->doscript);
	  sp = &(items.spr(i)->sp);
	  na[0] = &(items.spr(i)->a[0]);
      na[1] = &(items.spr(i)->a[1]);
	  scriptflag = &sflag;//&(items.spr(i)->scriptflag);

#ifdef _DEBUGPRINTSCOMMAND
	al_trace("\nStart of Item script processing:\n");
#endif

  bool increment = true;

  while(ffs != 0 && (scommand!=0xFFFF)&&(scommand!=WAITFRAME))
  {

    if((key[KEY_ALT]||key[KEY_ALTGR])&&key[KEY_F4])
    {
      quit_game();
      exit(101);
    }

#ifdef _DEBUGPRINTSCOMMAND
	al_trace("scommand: %i\n",scommand);
#endif

#ifdef _SCRIPT_COUNTER
	start_time = script_counter;
#endif

    switch(scommand)
    {
	  case SETTRUE:
	    set_variable(sarg1,i,(sflag & TRUEFLAG?1:0), ri); break;
	  case SETFALSE:
	    set_variable(sarg1,i,(sflag & TRUEFLAG?0:1), ri); break;
	  case SETMORE:
	    set_variable(sarg1,i,(sflag & MOREFLAG?1:0), ri); break;
	  case SETLESS:
	    set_variable(sarg1,i,(!(sflag & MOREFLAG) || (sflag & TRUEFLAG))?1:0, ri); break;
      case SETV:
        do_set(script, &pc, i, true,ri); break;
      case SETR:
        do_set(script, &pc, i, false,ri); break;
      case ADDV:
        do_add(script, &pc, i, true,ri); break;
      case ADDR:
        do_add(script, &pc, i, false,ri); break;
      case SUBV:
        do_sub(script, &pc, i, true,ri); break;
      case SUBR:
        do_sub(script, &pc, i, false,ri); break;
      case MULTV:
        do_mult(script, &pc, i, true,ri); break;
      case MULTR:
        do_mult(script, &pc, i, false,ri); break;
      case DIVV:
        do_div(script, &pc, i, true,ri); break;
      case DIVR:
        do_div(script, &pc, i, false,ri); break;
      case MODV:
        do_mod(script, &pc, i, true,ri); break;
      case MODR:
        do_mod(script, &pc, i, false,ri); break;
      case GOTO:
        pc = sarg1; increment = false; break;
      case CHECKTRIG:
        break;
      case WARP:
        tmpscr->sidewarpdmap[0] = (sarg1)/10000;
        tmpscr->sidewarpscr[0] = (sarg2)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
      case WARPR:
        tmpscr->sidewarpdmap[0] = get_arg(sarg1,i,ri)/10000;
        tmpscr->sidewarpscr[0] = get_arg(sarg2,i,ri)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
      case PITWARP:
        tmpscr->sidewarpdmap[0] = (sarg1)/10000;
        tmpscr->sidewarpscr[0] = (sarg2)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
      case PITWARPR:
        tmpscr->sidewarpdmap[0] = get_arg(sarg1,i,ri)/10000;
        tmpscr->sidewarpscr[0] = get_arg(sarg2,i,ri)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
      case COMPAREV:
        do_comp(script, &pc, i, true,ri); break;
      case COMPARER:
        do_comp(script, &pc, i, false,ri); break;
      case GOTOTRUE:
        if(sflag & TRUEFLAG)
        {pc = sarg1; increment = false;} break;
      case GOTOFALSE:
        if(!(sflag & TRUEFLAG))
        {pc = sarg1; increment = false;} break;
      case GOTOMORE:
        if(sflag & MOREFLAG)
        {pc = sarg1; increment = false;} break;
      case GOTOLESS:
        if(!(sflag & MOREFLAG) || (sflag & TRUEFLAG))
        {pc = sarg1; increment = false;} break;
      case LOAD1:
        do_loada(script, &pc, i, 0,ri); break;
      case LOAD2:
        do_loada(script, &pc, i, 1,ri); break;
      case SETA1:
        do_seta(script, &pc, i, 0,ri); break;
      case SETA2:
        do_seta(script, &pc, i, 1,ri); break;
	  case GAMEEND:
	    Quit = qQUIT; skipcont=1; //fallthrough
      case QUIT:
        ffs = 0; break;
      case SINV:
        do_trig(script, &pc, i, true, 0,ri); break;
      case SINR:
        do_trig(script, &pc, i, false, 0,ri); break;
      case COSV:
        do_trig(script, &pc, i, true, 1,ri); break;
      case COSR:
        do_trig(script, &pc, i, false, 1,ri); break;
      case TANV:
        do_trig(script, &pc, i, true, 2,ri); break;
      case TANR:
        do_trig(script, &pc, i, false, 2,ri); break;
	 case ARCSINR:
        do_asin(script, &pc, i, false,ri); break;
      case ARCCOSR:
        do_acos(script, &pc, i, false,ri); break;
      case ABSR:
        do_abs(script, &pc, i, false,ri); break;
      case LOG10:
        do_log10(script, &pc, i, false,ri); break;
      case LOGE:
        do_naturallog(script, &pc, i, false,ri); break;
      case MINR:
        do_min(script, &pc, i, false,ri); break;
      case MINV:
        do_min(script, &pc, i, true,ri); break;
      case MAXR:
        do_max(script, &pc, i, false,ri); break;
      case MAXV:
        do_max(script, &pc, i, true,ri); break;
      case RNDR:
        do_rnd(script, &pc, i, false,ri); break;
      case RNDV:
        do_rnd(script, &pc, i, true,ri); break;
      case FACTORIAL:
        do_factorial(script, &pc, i, false,ri); break;
      case POWERR:
        do_power(script, &pc, i, false,ri); break;
      case POWERV:
        do_power(script, &pc, i, true,ri); break;
      case IPOWERR:
        do_ipower(script, &pc, i, false,ri); break;
      case IPOWERV:
        do_ipower(script, &pc, i, true,ri); break;
      case ANDR:
        do_and(script, &pc, i, false,ri); break;
      case ANDV:
        do_and(script, &pc, i, true,ri); break;
      case ORR:
        do_or(script, &pc, i, false,ri); break;
      case ORV:
        do_or(script, &pc, i, true,ri); break;
      case XORR:
        do_xor(script, &pc, i, false,ri); break;
      case XORV:
        do_xor(script, &pc, i, true,ri); break;
      case NANDR:
        do_nand(script, &pc, i, false,ri); break;
      case NANDV:
        do_nand(script, &pc, i, true,ri); break;
      case NORR:
        do_nor(script, &pc, i, false,ri); break;
      case NORV:
        do_nor(script, &pc, i, true,ri); break;
      case XNORR:
        do_xnor(script, &pc, i, false,ri); break;
      case XNORV:
        do_xnor(script, &pc, i, true,ri); break;
      case NOT:
        do_not(script, &pc, i, false,ri); break;
      case BITNOT:
        do_bitwisenot(script, &pc, i, false,ri); break;
      case LSHIFTR:
        do_lshift(script, &pc, i, false,ri); break;
      case LSHIFTV:
        do_lshift(script, &pc, i, true,ri); break;
      case RSHIFTR:
        do_rshift(script, &pc, i, false,ri); break;
      case RSHIFTV:
        do_rshift(script, &pc, i, true,ri); break;
      case TRACER:
        do_trace(script, &pc, i, false,ri); break;
      case TRACEV:
        do_trace(script, &pc, i, true,ri); break;
      case TRACE2R:
        do_trace2(script, &pc, i, false,ri); break;
      case TRACE2V:
        do_trace2(script, &pc, i, true,ri); break;
      case TRACE3:
        do_tracenl(script, &pc, i, false,ri); break;
      case TRACE4:
        do_cleartrace(script, &pc, i, false,ri); break;
      case TRACE5:
        do_tracetobase(script, &pc, i, false,ri); break;;
      case TRACE6:
        do_tracestring(script, &pc, i, false,ri); break;
      case LOOP:
        if(get_arg(sarg2,i,ri)>0)
        {
          pc = sarg1;
          increment = false;
        }
        else
        {
          set_variable(sarg1,i,sarg1-1,ri);
        }
        break;
      case PUSHR:
        do_push(script, &pc, i, false,ri); break;
      case PUSHV:
        do_push(script, &pc, i, true,ri); break;
      case POP:
        do_pop(script, &pc, i, false,ri); break;
      case ENQUEUER:
        do_enqueue(script, &pc, i, false); break;
      case ENQUEUEV:
        do_enqueue(script, &pc, i, true); break;
      case DEQUEUE:
        do_dequeue(script, &pc, i, false); break;
      case PLAYSOUNDR:
        do_sfx(script, &pc, i, false,ri); break;
      case PLAYSOUNDV:
        do_sfx(script, &pc, i, true,ri); break;
      case PLAYMIDIR:
        do_midi(script, &pc, i, false,ri); break;
      case PLAYMIDIV:
        do_midi(script, &pc, i, true,ri); break;
      case LOADLWEAPONR:
        do_loadlweapon(script, &pc, i, false,ri); break;
      case LOADLWEAPONV:
        do_loadlweapon(script, &pc, i, true,ri); break;
      case LOADEWEAPONR:
        do_loadeweapon(script, &pc, i, false,ri); break;
      case LOADEWEAPONV:
        do_loadeweapon(script, &pc, i, true,ri); break;
      case LOADITEMR:
        do_loaditem(script, &pc, i, false,ri); break;
      case LOADITEMV:
        do_loaditem(script, &pc, i, true,ri); break;
      case LOADITEMDATAR:
        do_loaditemdata(script, &pc, i, false,ri); break;
      case LOADITEMDATAV:
        do_loaditemdata(script, &pc, i, true,ri); break;
      case LOADNPCR:
        do_loadnpc(script, &pc, i, false,ri); break;
      case LOADNPCV:
        do_loadnpc(script, &pc, i, true,ri); break;
      case CREATELWEAPONR:
        do_createlweapon(script, &pc, i, false,ri); break;
      case CREATELWEAPONV:
        do_createlweapon(script, &pc, i, true,ri); break;
      case CREATEEWEAPONR:
        do_createeweapon(script, &pc, i, false,ri); break;
      case CREATEEWEAPONV:
        do_createeweapon(script, &pc, i, true,ri); break;
      case CREATEITEMR:
        do_createitem(script, &pc, i, false,ri); break;
      case CREATEITEMV:
        do_createitem(script, &pc, i, true,ri); break;
      case CREATENPCR:
        do_createnpc(script, &pc, i, false,ri); break;
      case CREATENPCV:
        do_createnpc(script, &pc, i, true,ri); break;
      case LOADI:
        do_loadi(script,&pc,i,true,ri); break;
      case STOREI:
        do_storei(script,&pc,i,true,ri); break;
      case GOTOR:
        {
          long temp = sarg1;
          int tmp2 = (get_arg(temp,i,ri)/10000)-1;
          pc = tmp2;
          increment = false;
        }
        break;
      case SQROOTV:
        do_sqroot(script,&pc,i,true,ri); break;
      case SQROOTR:
        do_sqroot(script,&pc,i,false,ri); break;
      case CLEARSPRITESR:
        do_clearsprites(script, &pc, i, false,ri); break;
      case CLEARSPRITESV:
        do_clearsprites(script, &pc, i, true,ri); break;
      case MSGSTRR:
        do_message(script, &pc, i, false,ri); break;
      case MSGSTRV:
        do_message(script, &pc, i, true,ri); break;
      case ISSOLID:
        do_issolid(script, &pc, i, ri); break;
	 case SETSIDEWARP:
	   do_setsidewarp(script, &pc, i, ri); break;
	 case SETTILEWARP:
	   do_settilewarp(script, &pc, i, ri); break;
      case LAYERSCREEN:
        do_layerscreen(script, &pc, i, ri); break;
      case LAYERMAP:
        do_layermap(script, &pc, i, ri); break;
	 case SECRETS:
	   do_triggersecrets(script, &pc, i, ri); break;
      case ISVALIDITEM:
        do_isvaliditem(script, &pc, i, ri); break;
      case ISVALIDNPC:
        do_isvalidnpc(script, &pc, i, ri); break;
    case ISVALIDLWPN:
      do_isvalidlwpn(script, &pc, i, ri); break;
    case ISVALIDEWPN:
      do_isvalidewpn(script, &pc, i, ri); break;
    case LWPNUSESPRITER:
      do_lwpnusesprite(script, &pc, i, false, ri); break;
    case LWPNUSESPRITEV:
      do_lwpnusesprite(script, &pc, i, true, ri); break;
    case EWPNUSESPRITER:
      do_ewpnusesprite(script, &pc, i, false, ri); break;
    case EWPNUSESPRITEV:
      do_ewpnusesprite(script, &pc, i, true, ri); break;
      case RECTR:
      case CIRCLER:
      case ARCR:
      case ELLIPSER:
      case LINER:
      case PUTPIXELR:
      case DRAWTILER:
      case DRAWCOMBOR:
      case DRAWCHARR:
	  case DRAWINTR:
	  case QUADR:
	  case TRIANGLER:
	  case QUAD3DR:
	  case TRIANGLE3DR:
	  case FASTTILER:
	  case FASTCOMBOR:
	  case DRAWSTRINGR:
	  case SPLINER:
        do_drawing_command(script, &pc, i, scommand,ri); break;
      case COPYTILEVV:
        do_copytile(script, &pc, i, true, true, ri); break;
      case COPYTILEVR:
        do_copytile(script, &pc, i, true, false, ri); break;
      case COPYTILERV:
        do_copytile(script, &pc, i, false, true, ri); break;
      case COPYTILERR:
        do_copytile(script, &pc, i, false, false, ri); break;
      case SWAPTILEVV:
        do_swaptile(script, &pc, i, true, true, ri); break;
      case SWAPTILEVR:
        do_swaptile(script, &pc, i, true, false, ri); break;
      case SWAPTILERV:
        do_swaptile(script, &pc, i, false, true, ri); break;
      case SWAPTILERR:
        do_swaptile(script, &pc, i, false, false, ri); break;
      case CLEARTILEV:
        do_cleartile(script, &pc, i, true, ri); break;
      case CLEARTILER:
        do_cleartile(script, &pc, i, false, ri); break;
      case OVERLAYTILEVV:
        do_overlaytile(script, &pc, i, true, true, ri); break;
      case OVERLAYTILEVR:
        do_overlaytile(script, &pc, i, true, false, ri); break;
      case OVERLAYTILERV:
        do_overlaytile(script, &pc, i, false, true, ri); break;
      case OVERLAYTILERR:
        do_overlaytile(script, &pc, i, false, false, ri); break;
      case FLIPROTTILEVV:
        do_fliprotatetile(script, &pc, i, true, true, ri); break;
      case FLIPROTTILEVR:
        do_fliprotatetile(script, &pc, i, true, false, ri); break;
      case FLIPROTTILERV:
        do_fliprotatetile(script, &pc, i, false, true, ri); break;
      case FLIPROTTILERR:
        do_fliprotatetile(script, &pc, i, false, false, ri); break;
      case GETTILEPIXELV:
        do_gettilepixel(script, &pc, i, true, ri); break;
      case GETTILEPIXELR:
        do_gettilepixel(script, &pc, i, false, ri); break;
      case SETTILEPIXELV:
        do_settilepixel(script, &pc, i, true, ri); break;
      case SETTILEPIXELR:
        do_settilepixel(script, &pc, i, false, ri); break;
      case SHIFTTILEVV:
        do_shifttile(script, &pc, i, true, true, ri); break;
      case SHIFTTILEVR:
        do_shifttile(script, &pc, i, true, false, ri); break;
      case SHIFTTILERV:
        do_shifttile(script, &pc, i, false, true, ri); break;
      case SHIFTTILERR:
        do_shifttile(script, &pc, i, false, false, ri); break;
	  case ALLOCATEMEMR:
        do_allocatemem(script, &pc, i, false, ri, true); break;
	  case ALLOCATEMEMV:
        do_allocatemem(script, &pc, i, true, ri, true); break;
	  case DEALLOCATEMEMR:
        do_deallocatemem(script, &pc, i, false, ri); break;
	  case DEALLOCATEMEMV:
        do_deallocatemem(script, &pc, i, true, ri); break;
	  case SAVE:
	    save_game(false); break;
	  case ARCTANR:
		do_arctan(script, &pc, i, false, ri); break;
	  case SETCOLORB:
	  case SETDEPTHB:
	  case GETCOLORB:
	  case GETDEPTHB:
		  break;
	  case GETSCREENFLAGS:
		do_getscreenflags(script, &pc, i, false, ri); break;
	  case GETSCREENEFLAGS:
		do_getscreeneflags(script, &pc, i, false, ri); break;
	  case COMBOTILE:
		do_combotile(script,&pc,i,ri); break;
    }

#ifdef _SCRIPT_COUNTER
	end_time=script_counter;
	script_timer[*command] += end_time-start_time;
	++script_execount[*command];
#endif

    if(increment)
		pc++;
    else
		increment = true;

    scommand = itemscripts[script][pc].command;
    sarg1 = itemscripts[script][pc].arg1;
    sarg2 = itemscripts[script][pc].arg2;

    /*switch(script_type)
    {
      case SCRIPT_FFC:
        command = &(ffscripts[script][*pc].command);
        arg1 = &(ffscripts[script][*pc].arg1);
        arg2 = &(ffscripts[script][*pc].arg2);
        break;
      case SCRIPT_ITEM:
        command = &(itemscripts[script][*pc].command);
        arg1 = &(itemscripts[script][*pc].arg1);
        arg2 = &(itemscripts[script][*pc].arg2);
        break;
      case SCRIPT_GLOBAL:
        command = &(itemscripts[script][*pc].command);
        arg1 = &(itemscripts[script][*pc].arg1);
        arg2 = &(itemscripts[script][*pc].arg2);
        break;
    }*/
  }
  items.spr(i)->pc = pc;
  items.spr(i)->doscript = ffs;
  if(scommand==0xFFFF)
  {
    items.spr(i)->doscript = 0;
  }
  else
  {
    items.spr(i)->pc+=1;
  }

#ifdef _SCRIPT_COUNTER
  for(int j=0; j<NUMCOMMANDS; j++)
  {
	  if(script_execount[j] != 0)
		  al_trace("Command %s took %ld ticks in all to complete in %ld executions.\n", command_list[j].name, script_timer[j], script_execount[j]);
  }
  remove_int(update_script_counter);
#endif

  return 0;
}

int run_global_script(int script)
{
  byte i=0;
  //word scommand;

#ifdef _SCRIPT_COUNTER
  unsigned long script_timer[NUMCOMMANDS];
  unsigned long script_execount[NUMCOMMANDS];
  for(int j=0; j<NUMCOMMANDS; j++) { script_timer[j]=0; script_execount[j]=0; }
  unsigned long start_time, end_time;

  script_counter = 0;
#endif

  // fuck..this happens
#ifdef _DEBUG //TODO: remove me
	  if(g_pc>=0xffff)
	  {
		  al_trace( "g_pc increment past 0xFFFE. val: (%d)\n", g_pc );
		  al_trace( "scommand: %d \nscript: %d \n", scommand, script );
		  al_trace( "Fatal\n" );
		  return 1; //No recovery
	  }
#endif

  scommand = globalscripts[script][g_pc].command;
  sarg1 = globalscripts[script][g_pc].arg1;
  sarg2 = globalscripts[script][g_pc].arg2;

  //scommand = 0;  //to get gcc to stop complaining aout unused variables
  //long arg1=0;
  //long arg2=0;
  //arg1=arg2; //to avoid unused variables warnings
  //word *pc=NULL;
  //word *ffs=NULL;
  //dword *sflag=NULL;
  //script_type = stype;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
      pc = &(tmpscr->pc[i]);
      command = &(ffscripts[script][*pc].command);
      arg1 = &(ffscripts[script][*pc].arg1);
      arg2 = &(ffscripts[script][*pc].arg2);
      ffs = &(tmpscr->ffscript[i]);
      sflag = &(tmpscr->scriptflag[i]);
      tmpscr->ffcref[i]=i;
	  st = &(ffstack[i]);
      ffs2 = &(tmpscr->ffscript[i]);
	  sp = &(tmpscr->sp[i]);
	  na[0] = &(tmpscr->a[i][0]);
      na[1] = &(tmpscr->a[i][1]);
	  scriptflag = &(tmpscr->scriptflag[i]);
      break;
    case SCRIPT_ITEM:
      pc = &(items.spr(i)->pc);
      command = &(itemscripts[script][*pc].command);
      arg1 = &(itemscripts[script][*pc].arg1);
      arg2 = &(itemscripts[script][*pc].arg2);
      ffs = &(items.spr(i)->doscript);
      sflag = &(items.spr(i)->scriptflag);
      items.spr(i)->itemref = i;
	  st = &(items.spr(i)->stack);
      ffs2 = &(items.spr(i)->doscript);
	  sp = &(items.spr(i)->sp);
	  na[0] = &(items.spr(i)->a[0]);
      na[1] = &(items.spr(i)->a[1]);
	  scriptflag = &(items.spr(i)->scriptflag);
      break;
    case SCRIPT_GLOBAL:
      pc = &g_pc;
      command = &(globalscripts[script][*pc].command);
      arg1 = &(globalscripts[script][*pc].arg1);
      arg2 = &(globalscripts[script][*pc].arg2);
      ffs = &g_doscript;
      sflag = &g_scriptflag;
	  st = &g_stack;
      ffs2 = &g_doscript;
	  sp = &g_sp;
	  na[0] = NULL;
      na[1] = NULL;
	  scriptflag = &g_scriptflag;
      break;
  }*/

  //these don't change during the execution of a script
  //so there's no need to recompute them every time we set or get a var
  //(which happens A LOT in big scripts)
  refInfo ri;

      ri.ffc = &(global_ffc);
      ri.itemref = &(global_item);
      ri.idata = &(global_itemclass);
      ri.lwpn = &(global_lwpn);
      ri.ewpn = &(global_ewpn);
      ri.guyref = &(global_guy);
	  ri.ramref = &(global_ram);
      ri.gclass = &(global_guyclass);
      ri.lclass = &(global_lwpnclass);
      ri.eclass = &(global_ewpnclass);
      ri.sp = &g_sp;
      for(int j=0;j<8;j++)
      {
        ri.d[j] = &g_d[j];
      }
      for(int j=0;j<2;j++)
      {
        ri.a[j] = NULL;
      }
	  //pc = &g_pc;
      //command = &(globalscripts[script][g_pc].command);
      //arg1 = &(globalscripts[script][g_pc].arg1);
      //arg2 = &(globalscripts[script][g_pc].arg2);
      //ffs = &g_doscript;
      //sflag = &g_scriptflag;
	  st = &g_stack;
      ffs2 = &g_doscript;
	  sp = &g_sp;
	  na[0] = NULL;
      na[1] = NULL;
	  scriptflag = &g_scriptflag;

#ifdef _DEBUGPRINTSCOMMAND
	al_trace("\nStart of Global script processing:\n");
#endif

  bool increment = true;
  global_wait=false;

  while( g_doscript != 0 && (scommand!=0xFFFF) && (scommand!=WAITFRAME) && (scommand!=WAITDRAW))
  {
	if( key[KEY_F4] && (key[KEY_ALT]||key[KEY_ALTGR]) )
	{
		quit_game();
		exit(101);
	}

#ifdef _DEBUGPRINTSCOMMAND
	al_trace("scommand: %i\n",scommand);
#endif

#ifdef _SCRIPT_COUNTER
	start_time = script_counter;
#endif

    switch(scommand)
    {
	  case SETTRUE:
	    set_variable(sarg1,i,(g_scriptflag & TRUEFLAG?1:0), ri); break;
	  case SETFALSE:
	    set_variable(sarg1,i,(g_scriptflag & TRUEFLAG?0:1), ri); break;
	  case SETMORE:
	    set_variable(sarg1,i,(g_scriptflag & MOREFLAG?1:0), ri); break;
	  case SETLESS:
	    set_variable(sarg1,i,(!(g_scriptflag & MOREFLAG) || (g_scriptflag & TRUEFLAG))?1:0, ri); break;
      case SETV:
        do_set(script, &g_pc, i, true,ri); break;
      case SETR:
        do_set(script, &g_pc, i, false,ri); break;
      case ADDV:
        do_add(script, &g_pc, i, true,ri); break;
      case ADDR:
        do_add(script, &g_pc, i, false,ri); break;
      case SUBV:
        do_sub(script, &g_pc, i, true,ri); break;
      case SUBR:
        do_sub(script, &g_pc, i, false,ri); break;
      case MULTV:
        do_mult(script, &g_pc, i, true,ri); break;
      case MULTR:
        do_mult(script, &g_pc, i, false,ri); break;
      case DIVV:
        do_div(script, &g_pc, i, true,ri); break;
      case DIVR:
        do_div(script, &g_pc, i, false,ri); break;
      case MODV:
        do_mod(script, &g_pc, i, true,ri); break;
      case MODR:
        do_mod(script, &g_pc, i, false,ri); break;
      case GOTO:
        g_pc = sarg1; increment = false; break;
      case CHECKTRIG:
        break;
      case WARP:
        tmpscr->sidewarpdmap[0] = (sarg1)/10000;
        tmpscr->sidewarpscr[0] = (sarg2)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
      case WARPR:
        tmpscr->sidewarpdmap[0] = get_arg(sarg1,i,ri)/10000;
        tmpscr->sidewarpscr[0] = get_arg(sarg2,i,ri)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
      case PITWARP:
        tmpscr->sidewarpdmap[0] = (sarg1)/10000;
        tmpscr->sidewarpscr[0] = (sarg2)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
      case PITWARPR:
        tmpscr->sidewarpdmap[0] = get_arg(sarg1,i,ri)/10000;
        tmpscr->sidewarpscr[0] = get_arg(sarg2,i,ri)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
      case COMPAREV:
        do_comp(script, &g_pc, i, true,ri); break;
      case COMPARER:
        do_comp(script, &g_pc, i, false,ri); break;
      case GOTOTRUE:
        if(g_scriptflag & TRUEFLAG)
        {g_pc = sarg1; increment = false;} break;
      case GOTOFALSE:
        if(!(g_scriptflag & TRUEFLAG))
        {g_pc = sarg1; increment = false;} break;
      case GOTOMORE:
        if(g_scriptflag & MOREFLAG)
        {g_pc = sarg1; increment = false;} break;
      case GOTOLESS:
        if(!(g_scriptflag & MOREFLAG) || (g_scriptflag & TRUEFLAG))
        {g_pc = sarg1; increment = false;} break;
      case LOAD1:
        do_loada(script, &g_pc, i, 0,ri); break;
      case LOAD2:
        do_loada(script, &g_pc, i, 1,ri); break;
      case SETA1:
        do_seta(script, &g_pc, i, 0,ri); break;
      case SETA2:
        do_seta(script, &g_pc, i, 1,ri); break;
	  case GAMEEND:
	    Quit = qQUIT; skipcont=1; //fallthrough
      case QUIT:
        g_doscript = 0; break;
      case SINV:
        do_trig(script, &g_pc, i, true, 0,ri); break;
      case SINR:
        do_trig(script, &g_pc, i, false, 0,ri); break;
      case COSV:
        do_trig(script, &g_pc, i, true, 1,ri); break;
      case COSR:
        do_trig(script, &g_pc, i, false, 1,ri); break;
      case TANV:
        do_trig(script, &g_pc, i, true, 2,ri); break;
      case TANR:
        do_trig(script, &g_pc, i, false, 2,ri); break;
	 case ARCSINR:
        do_asin(script, &g_pc, i, false,ri); break;
      case ARCCOSR:
        do_acos(script, &g_pc, i, false,ri); break;
      case ABSR:
        do_abs(script, &g_pc, i, false,ri); break;
      case LOG10:
        do_log10(script, &g_pc, i, false,ri); break;
      case LOGE:
        do_naturallog(script, &g_pc, i, false,ri); break;
      case MINR:
        do_min(script, &g_pc, i, false,ri); break;
      case MINV:
        do_min(script, &g_pc, i, true,ri); break;
      case MAXR:
        do_max(script, &g_pc, i, false,ri); break;
      case MAXV:
        do_max(script, &g_pc, i, true,ri); break;
      case RNDR:
        do_rnd(script, &g_pc, i, false,ri); break;
      case RNDV:
        do_rnd(script, &g_pc, i, true,ri); break;
      case FACTORIAL:
        do_factorial(script, &g_pc, i, false,ri); break;
      case POWERR:
        do_power(script, &g_pc, i, false,ri); break;
      case POWERV:
        do_power(script, &g_pc, i, true,ri); break;
      case IPOWERR:
        do_ipower(script, &g_pc, i, false,ri); break;
      case IPOWERV:
        do_ipower(script, &g_pc, i, true,ri); break;
      case ANDR:
        do_and(script, &g_pc, i, false,ri); break;
      case ANDV:
        do_and(script, &g_pc, i, true,ri); break;
      case ORR:
        do_or(script, &g_pc, i, false,ri); break;
      case ORV:
        do_or(script, &g_pc, i, true,ri); break;
      case XORR:
        do_xor(script, &g_pc, i, false,ri); break;
      case XORV:
        do_xor(script, &g_pc, i, true,ri); break;
      case NANDR:
        do_nand(script, &g_pc, i, false,ri); break;
      case NANDV:
        do_nand(script, &g_pc, i, true,ri); break;
      case NORR:
        do_nor(script, &g_pc, i, false,ri); break;
      case NORV:
        do_nor(script, &g_pc, i, true,ri); break;
      case XNORR:
        do_xnor(script, &g_pc, i, false,ri); break;
      case XNORV:
        do_xnor(script, &g_pc, i, true,ri); break;
      case NOT:
        do_not(script, &g_pc, i, false,ri); break;
      case BITNOT:
        do_bitwisenot(script, &g_pc, i, false,ri); break;
      case LSHIFTR:
        do_lshift(script, &g_pc, i, false,ri); break;
      case LSHIFTV:
        do_lshift(script, &g_pc, i, true,ri); break;
      case RSHIFTR:
        do_rshift(script, &g_pc, i, false,ri); break;
      case RSHIFTV:
        do_rshift(script, &g_pc, i, true,ri); break;
      case TRACER:
        do_trace(script, &g_pc, i, false,ri); break;
      case TRACEV:
        do_trace(script, &g_pc, i, true,ri); break;
      case TRACE2R:
        do_trace2(script, &g_pc, i, false,ri); break;
      case TRACE2V:
        do_trace2(script, &g_pc, i, true,ri); break;
	 case TRACE3:
        do_tracenl(script, &g_pc, i, false,ri); break;
      case TRACE4:
        do_cleartrace(script, &g_pc, i, false,ri); break;
      case TRACE5:
        do_tracetobase(script, &g_pc, i, false,ri); break;
      case TRACE6:
        do_tracestring(script, &g_pc, i, false,ri); break;
      case LOOP:
        if(get_arg(sarg2,i,ri)>0)
        {
          g_pc = sarg1;
          increment = false;
        }
        else
        {
          set_variable(sarg1,i,sarg1-1,ri);
        }
        break;
      case PUSHR:
        do_push(script, &g_pc, i, false,ri); break;
      case PUSHV:
        do_push(script, &g_pc, i, true,ri); break;
      case POP:
        do_pop(script, &g_pc, i, false,ri); break;
      case ENQUEUER:
        do_enqueue(script, &g_pc, i, false); break;
      case ENQUEUEV:
        do_enqueue(script, &g_pc, i, true); break;
      case DEQUEUE:
        do_dequeue(script, &g_pc, i, false); break;
      case PLAYSOUNDR:
        do_sfx(script, &g_pc, i, false,ri); break;
      case PLAYSOUNDV:
        do_sfx(script, &g_pc, i, true,ri); break;
      case PLAYMIDIR:
        do_midi(script, &g_pc, i, false,ri); break;
      case PLAYMIDIV:
        do_midi(script, &g_pc, i, true,ri); break;
      case LOADLWEAPONR:
        do_loadlweapon(script, &g_pc, i, false,ri); break;
      case LOADLWEAPONV:
        do_loadlweapon(script, &g_pc, i, true,ri); break;
      case LOADEWEAPONR:
        do_loadeweapon(script, &g_pc, i, false,ri); break;
      case LOADEWEAPONV:
        do_loadeweapon(script, &g_pc, i, true,ri); break;
      case LOADITEMR:
        do_loaditem(script, &g_pc, i, false,ri); break;
      case LOADITEMV:
        do_loaditem(script, &g_pc, i, true,ri); break;
      case LOADITEMDATAR:
        do_loaditemdata(script, &g_pc, i, false,ri); break;
      case LOADITEMDATAV:
        do_loaditemdata(script, &g_pc, i, true,ri); break;
      case LOADNPCR:
        do_loadnpc(script, &g_pc, i, false,ri); break;
      case LOADNPCV:
        do_loadnpc(script, &g_pc, i, true,ri); break;
      case CREATELWEAPONR:
        do_createlweapon(script, &g_pc, i, false,ri); break;
      case CREATELWEAPONV:
        do_createlweapon(script, &g_pc, i, true,ri); break;
      case CREATEEWEAPONR:
        do_createeweapon(script, &g_pc, i, false,ri); break;
      case CREATEEWEAPONV:
        do_createeweapon(script, &g_pc, i, true,ri); break;
      case CREATEITEMR:
        do_createitem(script, &g_pc, i, false,ri); break;
      case CREATEITEMV:
        do_createitem(script, &g_pc, i, true,ri); break;
      case CREATENPCR:
        do_createnpc(script, &g_pc, i, false,ri); break;
      case CREATENPCV:
        do_createnpc(script, &g_pc, i, true,ri); break;
      case LOADI:
        do_loadi(script,&g_pc,i,true,ri); break;
      case STOREI:
        do_storei(script,&g_pc,i,true,ri); break;
      case GOTOR:
        {
          long temp = sarg1;
          int tmp2 = (get_arg(temp,i,ri)/10000)-1;
          g_pc = tmp2;
          increment = false;
        }
        break;
      case SQROOTV:
        do_sqroot(script,&g_pc,i,true,ri); break;
      case SQROOTR:
        do_sqroot(script,&g_pc,i,false,ri); break;
      case CLEARSPRITESR:
        do_clearsprites(script, &g_pc, i, false,ri); break;
      case CLEARSPRITESV:
        do_clearsprites(script, &g_pc, i, true,ri); break;
      case MSGSTRR:
        do_message(script, &g_pc, i, false,ri); break;
      case MSGSTRV:
        do_message(script, &g_pc, i, true,ri); break;
      case ISSOLID:
        do_issolid(script, &g_pc, i, ri); break;
	 case SETSIDEWARP:
	   do_setsidewarp(script, &g_pc, i, ri); break;
	 case SETTILEWARP:
	   do_settilewarp(script, &g_pc, i, ri); break;
      case LAYERSCREEN:
        do_layerscreen(script, &g_pc, i, ri); break;
      case LAYERMAP:
        do_layermap(script, &g_pc, i, ri); break;
	 case SECRETS:
	   do_triggersecrets(script, &g_pc, i, ri); break;
      case ISVALIDITEM:
        do_isvaliditem(script, &g_pc, i, ri); break;
      case ISVALIDNPC:
        do_isvalidnpc(script, &g_pc, i, ri); break;
    case ISVALIDLWPN:
      do_isvalidlwpn(script, &g_pc, i, ri); break;
    case ISVALIDEWPN:
      do_isvalidewpn(script, &g_pc, i, ri); break;
    case LWPNUSESPRITER:
      do_lwpnusesprite(script, &g_pc, i, false, ri); break;
    case LWPNUSESPRITEV:
      do_lwpnusesprite(script, &g_pc, i, true, ri); break;
    case EWPNUSESPRITER:
      do_ewpnusesprite(script, &g_pc, i, false, ri); break;
    case EWPNUSESPRITEV:
      do_ewpnusesprite(script, &g_pc, i, true, ri); break;
      case RECTR:
      case CIRCLER:
      case ARCR:
      case ELLIPSER:
      case LINER:
      case PUTPIXELR:
      case DRAWTILER:
      case DRAWCOMBOR:
      case DRAWCHARR:
	  case DRAWINTR:
	  case QUADR:
	  case TRIANGLER:
	  case QUAD3DR:
	  case TRIANGLE3DR:
  	  case FASTTILER:
	  case FASTCOMBOR:
	  case DRAWSTRINGR:
	  case SPLINER:
        do_drawing_command(script, &g_pc, i, scommand,ri); break;
      case COPYTILEVV:
        do_copytile(script, &g_pc, i, true, true, ri); break;
      case COPYTILEVR:
        do_copytile(script, &g_pc, i, true, false, ri); break;
      case COPYTILERV:
        do_copytile(script, &g_pc, i, false, true, ri); break;
      case COPYTILERR:
        do_copytile(script, &g_pc, i, false, false, ri); break;
      case SWAPTILEVV:
        do_swaptile(script, &g_pc, i, true, true, ri); break;
      case SWAPTILEVR:
        do_swaptile(script, &g_pc, i, true, false, ri); break;
      case SWAPTILERV:
        do_swaptile(script, &g_pc, i, false, true, ri); break;
      case SWAPTILERR:
        do_swaptile(script, &g_pc, i, false, false, ri); break;
      case CLEARTILEV:
        do_cleartile(script, &g_pc, i, true, ri); break;
      case CLEARTILER:
        do_cleartile(script, &g_pc, i, false, ri); break;
      case OVERLAYTILEVV:
        do_overlaytile(script, &g_pc, i, true, true, ri); break;
      case OVERLAYTILEVR:
        do_overlaytile(script, &g_pc, i, true, false, ri); break;
      case OVERLAYTILERV:
        do_overlaytile(script, &g_pc, i, false, true, ri); break;
      case OVERLAYTILERR:
        do_overlaytile(script, &g_pc, i, false, false, ri); break;
      case FLIPROTTILEVV:
        do_fliprotatetile(script, &g_pc, i, true, true, ri); break;
      case FLIPROTTILEVR:
        do_fliprotatetile(script, &g_pc, i, true, false, ri); break;
      case FLIPROTTILERV:
        do_fliprotatetile(script, &g_pc, i, false, true, ri); break;
      case FLIPROTTILERR:
        do_fliprotatetile(script, &g_pc, i, false, false, ri); break;
      case GETTILEPIXELV:
        do_gettilepixel(script, &g_pc, i, true, ri); break;
      case GETTILEPIXELR:
        do_gettilepixel(script, &g_pc, i, false, ri); break;
      case SETTILEPIXELV:
        do_settilepixel(script, &g_pc, i, true, ri); break;
      case SETTILEPIXELR:
        do_settilepixel(script, &g_pc, i, false, ri); break;
      case SHIFTTILEVV:
        do_shifttile(script, &g_pc, i, true, true, ri); break;
      case SHIFTTILEVR:
        do_shifttile(script, &g_pc, i, true, false, ri); break;
      case SHIFTTILERV:
        do_shifttile(script, &g_pc, i, false, true, ri); break;
      case SHIFTTILERR:
        do_shifttile(script, &g_pc, i, false, false, ri); break;
	  case ALLOCATEMEMR:
        do_allocatemem(script, &g_pc, i, false, ri, true); break;
	  case ALLOCATEMEMV:
        do_allocatemem(script, &g_pc, i, true, ri, true); break;
	  case ALLOCATEGMEM:
	   do_allocatemem(script, &g_pc, i, true, ri, false); break;
	  case DEALLOCATEMEMR:
        do_deallocatemem(script, &g_pc, i, false, ri); break;
	  case DEALLOCATEMEMV:
        do_deallocatemem(script, &g_pc, i, true, ri); break;
	  case SAVE:
	    save_game(false); break;
	  case ARCTANR:
		do_arctan(script, &g_pc, i, false, ri); break;
	  /*case PTROFF:
		do_offsetpointer(script, &g_pc, i, false, ri); break;*/
	  case SETCOLORB:
	  case SETDEPTHB:
	  case GETCOLORB:
	  case GETDEPTHB:
		  break;
	   case GETSCREENFLAGS:
		do_getscreenflags(script, &g_pc, i, false, ri); break;
	   case GETSCREENEFLAGS:
		do_getscreeneflags(script, &g_pc, i, false, ri); break;
	  case COMBOTILE:
		do_combotile(script,&g_pc,i,ri); break;
    }

#ifdef _SCRIPT_COUNTER
	end_time=script_counter;
	script_timer[*command] += end_time-start_time;
	++script_execount[*command];
#endif

	if(increment)
		g_pc++;
	else
		increment = true;

	scommand = globalscripts[script][g_pc].command;
	sarg1 = globalscripts[script][g_pc].arg1;
	sarg2 = globalscripts[script][g_pc].arg2;
    /*switch(script_type)
    {
      case SCRIPT_FFC:
        command = &(ffscripts[script][*pc].command);
        arg1 = &(ffscripts[script][*pc].arg1);
        arg2 = &(ffscripts[script][*pc].arg2);
        break;
      case SCRIPT_ITEM:
        command = &(itemscripts[script][*pc].command);
        arg1 = &(itemscripts[script][*pc].arg1);
        arg2 = &(itemscripts[script][*pc].arg2);
        break;
      case SCRIPT_GLOBAL:
        command = &(globalscripts[script][*pc].command);
        arg1 = &(globalscripts[script][*pc].arg1);
        arg2 = &(globalscripts[script][*pc].arg2);
        break;
    }*/
  }

  if(scommand==0xFFFF)
    g_doscript = 0;
  else
    g_pc++;

  if(scommand==WAITDRAW) global_wait=true;

#ifdef _SCRIPT_COUNTER
  for(int j=0; j<NUMCOMMANDS; j++)
  {
	  if(script_execount[j] != 0)
		  al_trace("Command %s took %ld ticks to complete in %ld executions.\n", command_list[j].name, script_timer[j], script_execount[j]);
  }
  remove_int(update_script_counter);
#endif

  return 0;
}

int ffscript_engine(bool preload)
{
  for(byte i=0;i<32;i++)
  {
    if(!tmpscr->initialized[i])
    {
      tmpscr->initialized[i] = true;
      for(int j=0; j<8; j++)
        tmpscr->d[i][j] = tmpscr->initd[i][j];
      for(int j=0; j<2; j++)
        tmpscr->a[i][j] = tmpscr->inita[i][j];
    }
    if(tmpscr->ffscript[i] && !(preload && !(tmpscr->ffflags[i]&ffPRELOAD)))
    {
      run_ff_script(tmpscr->ffscript[i], i);
    }
  }
  return 0;
}

void write_stack(int script, byte, int sp2, long value)
{
  //long (*st)[256] = NULL;
  //word *ffs=NULL;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
      st = &(ffstack[i]);
      ffs = &(tmpscr->ffscript[i]);
      break;
    case SCRIPT_ITEM:
      st = &(items.spr(i)->stack);
      ffs = &(items.spr(i)->doscript);
      break;
    case SCRIPT_GLOBAL:
      st = &g_stack;
      ffs = &g_doscript;
      break;
  }*/

  if(sp2 == 0)
  {
    char tmp[200];
    sprintf(tmp, "Stack over or underflow: script %d\n", script);
    al_trace(tmp);
    *ffs2=0;
  }
  (*st)[sp2]=value;
}

int read_stack(int script, byte, int sp2)
{
  //long (*st)[256]=NULL;
  //word *ffs=NULL;
  /*switch(script_type)
  {
    case SCRIPT_FFC:
      st = &(ffstack[i]);
      ffs = &(tmpscr->ffscript[i]);
      break;
    case SCRIPT_ITEM:
      st = &(items.spr(i)->stack);
      ffs = &(items.spr(i)->doscript);
      break;
    case SCRIPT_GLOBAL:
      st = &g_stack;
      ffs = &g_doscript;
      break;
  }*/
  if(sp2 == 0)
  {
    char tmp[200];
    sprintf(tmp, "Stack over or underflow: script %d\n", script);
    al_trace(tmp);
    *ffs2=0;
  }
  return (*st)[sp2];
}

