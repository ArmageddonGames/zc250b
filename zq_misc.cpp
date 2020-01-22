//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_misc.cc
//
//  Misc. stuff for ZQuest.
//
//--------------------------------------------------------

//INLINE void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }
//INLINE void SCRFIX() {}

#include "zq_misc.h"
#include "zquestdat.h"
#include "zquest.h"
#include "colors.h"
#include "qst.h"
#include "zsys.h"
#include "zq_class.h"
#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#define strupr _strupr
#define stricmp _stricmp
#endif

extern int prv_mode;
extern void dopreview();


const char *imgstr[ftMAX] =
{
  "Not loaded", "Binary/ROM", "Image", "ZC Tiles",
  "ZC Tiles", "ZC Tiles", "ZC Tiles", "ZC Tiles"
};

int filetype(const char *path)
{
  if(path==NULL || strlen(get_filename(path))==0)
    return 0;

  char ext[40];
  strcpy(ext,get_extension(path));
  strupr(ext);

  for (int i=0; i<ssfmtMAX; ++i)
  {
    if(stricmp(ext,snapshotformat_str[i][1])==0) return ftBMP;
  }
  if(stricmp(ext,"til")==0) return ftTIL;
  if(stricmp(ext,"zgp")==0) return ftZGP;
  if(stricmp(ext,"qsu")==0) return ftQSU;
  if(stricmp(ext,"zqt")==0) return ftZQT;
  if(stricmp(ext,"qst")==0) return ftQST;
  if(stricmp(ext,"dat")==0) return 0;
  if(stricmp(ext,"htm")==0) return 0;
  if(stricmp(ext,"html")==0) return 0;
  if(stricmp(ext,"txt")==0) return 0;
  if(stricmp(ext,"zip")==0) return 0;
  return ftBIN;
}

void load_mice()
{
  for(int i=0; i<MOUSE_BMP_MAX; i++)
  {
    for(int j=0; j<4; j++)
    {
      mouse_bmp[i][j] = create_bitmap_ex(8,16,16);
      blit((BITMAP*)zcdata[BMP_MOUSE].dat,mouse_bmp[i][j],i*17+1,j*17+1,0,0,16,16);
    }
  }
}

void load_icons()
{
  for(int i=0; i<ICON_BMP_MAX; i++)
  {
    for(int j=0; j<4; j++)
    {
      icon_bmp[i][j] = create_bitmap_ex(8,16,16);
      blit((BITMAP*)zcdata[BMP_ICONS].dat,icon_bmp[i][j],i*17+1,j*17+1,0,0,16,16);
    }
  }
}

void load_selections()
{
  for(int i=0; i<2; i++)
  {
    select_bmp[i] = create_bitmap_ex(8,16,16);
    //  blit((BITMAP*)zcdata[BMP_SELECT].dat,select_bmp[i],i*17+1,1,0,0,16,16);
    blit((BITMAP*)zcdata[BMP_SELECT].dat,select_bmp[i],i*17+1,1,0,0,16,16);
  }
}

void load_arrows()
{
  for(int i=0; i<MAXARROWS; i++)
  {
    arrow_bmp[i] = create_bitmap_ex(8,16,16);
    blit((BITMAP*)zcdata[BMP_ARROWS].dat,arrow_bmp[i],i*17+1,1,0,0,16,16);
  }
}

void dump_pal()
{
  for(int i=0; i<256; i++)
    rectfill(screen,(i&63)<<2,(i&0xFC0)>>4,((i&63)<<2)+3,((i&0xFC0)>>4)+3,i);
}

int wrap(int x,int low,int high)
{
  if(x<low) x=high;
  if(x>high) x=low;
  return x;
}

bool readfile(const char *path,void *buf,int count)
{
  PACKFILE *f=pack_fopen_password(path,F_READ,"");
  if(!f)
    return 0;
  bool good=pfread(buf,count,f,true);
  pack_fclose(f);
  return good;
}

bool writefile(const char *path,void *buf,int count)
{
  PACKFILE *f=pack_fopen_password(path,F_WRITE,"");
  if(!f)
    return 0;

  bool good=pfwrite(buf,count,f);
  pack_fclose(f);
  return good;
}

/* dotted_rect: (from allegro's guiproc.c)
  *  Draws a dotted rectangle, for showing an object has the input focus.
  */
void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg)
{
  int x = ((x1+y1) & 1) ? 1 : 0;
  int c;

  /* two loops to avoid bank switches */
  for (c=x1; c<=x2; c++)
  {
    putpixel(screen, c, y1, (((c+y1) & 1) == x) ? fg : bg);
  }
  for (c=x1; c<=x2; c++)
  {
    putpixel(screen, c, y2, (((c+y2) & 1) == x) ? fg : bg);
  }

  for (c=y1+1; c<y2; c++)
  {
    putpixel(screen, x1, c, (((c+x1) & 1) == x) ? fg : bg);
    putpixel(screen, x2, c, (((c+x2) & 1) == x) ? fg : bg);
  }
}

RGB _RGB(byte *si)
{
  RGB x;
  x.r = si[0];
  x.g = si[1];
  x.b = si[2];
  return x;
}

RGB _RGB(int r,int g,int b)
{
  RGB x;
  x.r = r;
  x.g = g;
  x.b = b;
  return x;
}

RGB invRGB(RGB s)
{
  RGB x;
  x.r = 63-s.r;
  x.g = 63-s.g;
  x.b = 63-s.b;
  return x;
}

/*
  INLINE RGB NESpal(int i)
  {
  return _RGB(nes_pal+(i*3));
  }
  */

RGB mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio)
{
  RGB x;
  x.r = ( r1*(64-ratio) + r2*ratio ) >> 6;
  x.g = ( g1*(64-ratio) + g2*ratio ) >> 6;
  x.b = ( b1*(64-ratio) + b2*ratio ) >> 6;
  return x;
}

void reset_pal_cycling();
void cycle_palette();

void load_cset(RGB *pal,int cset_index,int dataset)
{
  byte *si = colordata + CSET(dataset)*3;
  for(int i=0; i<16; i++)
  {
    pal[CSET(cset_index)+i] = _RGB(si);
    si+=3;
  }
}

void set_pal()
{
  set_palette_range(RAMpal,0,192,true);
}

void loadlvlpal(int level)
{
  Color=level;
  // full pal
  for(int i=0; i<192; i++)
    RAMpal[i] = _RGB(colordata+i*3);

  // level pal
  byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;

  for(int i=0; i<16*3; i++)
  {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
  }

  for(int i=0; i<16; i++)
  {
    RAMpal[CSET(9)+i] = _RGB(si);
    si+=3;
  }

  reset_pal_cycling();
  set_pal();
}

void loadfadepal(int dataset)
{
  byte *si = colordata + CSET(dataset)*3;

  for(int i=0; i<16*3; i++)
  {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
  }

  set_pal();
}

void setup_lcolors()
{
  for(int i=0; i<14; i++)
  {
    RAMpal[lc1(i)] = _RGB(colordata+(CSET(i*pdLEVEL+poLEVEL)+2)*3);
    RAMpal[lc2(i)] = _RGB(colordata+(CSET(i*pdLEVEL+poLEVEL)+16+1)*3);
  }
  set_palette(RAMpal);
}

void refresh_pal()
{
  loadlvlpal(Color);
  setup_lcolors();
}

char ns_string[4];

const char *roomtype_string[MAXROOMTYPES] =
{
  "(None)","Special Item","Pay for Info","Secret Money","Gamble",
  "Door Repair","Red Potion or Heart Container","Feed the Goriya","Level 9 Entrance",
  "Potion Shop","Shop","More Bombs","Leave Money or Life","10 Rupees",
  "3-Stair Warp","Ganon","Zelda", "-<item pond>", "1/2 Magic Upgrade", "Learn Slash", "More Arrows","Take One Item"
};

const char *catchall_string[MAXROOMTYPES] =
{
  " ","Special Item","Info Type","Amount"," ","Repair Fee"," "," "," ","Shop Type",
  "Shop Type","Price","Price"," ","Warp Ring"," "," ", " ", " ",
  " ", "Price","Shop Type"
};

const char *warptype_string[MAXWARPTYPES] =
{
  "Cave/Item Cellar","Passageway","Entrance/Exit","Scrolling Warp","Insta-Warp","Insta-Warp with Blackout","Insta-Warp with Opening Wipe","Insta-Warp with Zap Effects", "Insta-Warp with Wave Effects", "Cancel Warp"
  //  "Cave/Item Cellar","Passageway","Entrance/Exit","Scrolling Warp","Standard","Cancel Warp"," "," ", " ", " "
};

const char *warpeffect_string[MAXWARPEFFECTS] =
{
  "Instant", "Circle", "Oval", "Triangle", "Super Mario All-Stars", "Curtains (Smooth)", "Curtains (Stepped)", "Mosaic",
  "Wave White", "Wave Black", "Fade White", "Fade Black", "Global Opening/Closing", "Destination Default"
};

const char *flag_string[MAXFLAGS] =
{
  "  0 (None)",
  "  1 Push Block (Vertical, Trigger)",
  "  2 Push Block (4-Way, Trigger)",
  "  3 Whistle Trigger",
  "  4 Burn Trigger (Any)",
  "  5 Arrow Trigger (Any)",
  "  6 Bomb Trigger (Any)",
  "  7 Fairy Ring (Life)",
  "  8 Raft Path",
  "  9 Armos -> Secret",
  " 10 Armos/Chest -> Item",
  " 11 Bomb (Super)",
  " 12 Raft Branch",
  " 13 Dive -> Item",
  " 14 Lens Marker",
  " 15 Zelda (Win Game)",
  " 16 Secret Tile 0",
  " 17 Secret Tile 1",
  " 18 Secret Tile 2",
  " 19 Secret Tile 3",
  " 20 Secret Tile 4",
  " 21 Secret Tile 5",
  " 22 Secret Tile 6",
  " 23 Secret Tile 7",
  " 24 Secret Tile 8",
  " 25 Secret Tile 9",
  " 26 Secret Tile 10",
  " 27 Secret Tile 11",
  " 28 Secret Tile 12",
  " 29 Secret Tile 13",
  " 30 Secret Tile 14",
  " 31 Secret Tile 15",
  " 32 Trap (Horizontal, Line of Sight)",
  " 33 Trap (Vertical, Line of Sight)",
  " 34 Trap (4-Way, Line of Sight)",
  " 35 Trap (Horizontal, Constant)",
  " 36 Trap (Vertical, Constant)",
  " 37 Enemy 0",
  " 38 Enemy 1",
  " 39 Enemy 2",
  " 40 Enemy 3",
  " 41 Enemy 4",
  " 42 Enemy 5",
  " 43 Enemy 6",
  " 44 Enemy 7",
  " 45 Enemy 8",
  " 46 Enemy 9",
  " 47 Push Block (Horiz, Once, Trigger)",
  " 48 Push Block (Up, Once, Trigger)",
  " 49 Push Block (Down, Once, Trigger)",
  " 50 Push Block (Left, Once, Trigger)",
  " 51 Push Block (Right, Once, Trigger)",
  " 52 Push Block (Vert, Once)",
  " 53 Push Block (Horizontal, Once)",
  " 54 Push Block (4-Way, Once)",
  " 55 Push Block (Up, Once)",
  " 56 Push Block (Down, Once)",
  " 57 Push Block (Left, Once)",
  " 58 Push Block (Right, Once)",
  " 59 Push Block (Vertical, Many)",
  " 60 Push Block (Horizontal, Many)",
  " 61 Push Block (4-Way, Many)",
  " 62 Push Block (Up, Many)",
  " 63 Push Block (Down, Many)",
  " 64 Push Block (Left, Many)",
  " 65 Push Block (Right, Many)",
  " 66 Block Trigger",
  " 67 No Push Blocks",
  " 68 Boomerang Trigger (Any)",
  " 69 Boomerang Trigger (Magic +)",
  " 70 Boomerang Trigger (Fire)",
  " 71 Arrow Trigger (Silver +)",
  " 72 Arrow Trigger (Golden)",
  " 73 Burn Trigger (Red Candle +)",
  " 74 Burn Trigger (Wand Fire)",
  " 75 Burn Trigger (Din's Fire)",
  " 76 Magic Trigger (Wand)",
  " 77 Magic Trigger (Reflected)",
  " 78 Fireball Trigger (Reflected)",
  " 79 Sword Trigger (Any)",
  " 80 Sword Trigger (White +)",
  " 81 Sword Trigger (Magic +)",
  " 82 Sword Trigger (Master)",
  " 83 Sword Beam Trigger (Any)",
  " 84 Sword Beam Trigger (White +)",
  " 85 Sword Beam Trigger (Magic +)",
  " 86 Sword Beam Trigger (Master)",
  " 87 Hookshot Trigger",
  " 88 Wand Trigger",
  " 89 Hammer Trigger",
  " 90 Strike Trigger",
  " 91 Block Hole (Block -> Next)",
  " 92 Fairy Ring (Magic)",
  " 93 Fairy Ring (All)",
  " 94 Trigger -> Self Only",
  " 95 Trigger -> Self, Secret Tiles",
  " 96 No Enemies",
  " 97 Unused",
  " 98 General Purpose 1 (Scripts)",
  " 99 General Purpose 2 (Scripts)",
  "100 General Purpose 3 (Scripts)",
  "101 General Purpose 4 (Scripts)",
  "102 General Purpose 5 (Scripts)",
  "103 Raft Bounce"
};

const char *flag_help_string[(mfMAX)*3] =
{
  "","","",
  "Allows Link to push the combo up or down once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
  "Allows Link to push the combo in any direction once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
  "Triggers Screen Secrets when Link plays the Whistle on it.","Is replaced with the 'Whistle' Secret Combo.","Doesn't interfere with Whistle related Screen Flags.",
  "Triggers Screen Secrets when Link touches it with", "fire from any source (Candle, Wand, Din's Fire, etc.)", "Is replaced with the 'Blue Candle' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with one of his Arrows.", "Is replaced with the 'Wooden Arrow' Secret Combo.",
  "Triggers Screen Secrets when the middle", "part of a Bomb explosion touches it.", "Is replaced with the 'Bomb' Secret Combo.",
  "Makes a heart circle appear on screen", "when Link steps on it, and refills his life.", "See also the Heart Circle-related Quest Rules.",
  "Place in paths to define the path Link travels", "when using the Raft. Use with Dock-type combos.", "If a path branches, Link takes the clockwise-most path.",
  "When placed on an Armos-type combo, causes the 'Stairs'"," Secret Combo to appear when the Armos is triggered,","instead of the screen's Under Combo.",
  "When placed on an Armos or treasure chest, causes","the room's Special Item to appear when the combo is activated.","Requires the 'Special Item' Room Type.",
  "Triggers Screen Secrets when the middle", "part of a Super Bomb explosion touches it.", "Is replaced with the 'Super Bomb' Secret Combo.",
  "Place at intersections of Raft flag paths to define", "points where the player may change directions.","Change directions by holding down a directional key.",
  "When Link dives on a flagged water-type combo","he will recieve the screen's Special Item.","Requires the 'Special Item' Room Type.",
  "Combos with this flag will flash white when", "viewed with the Lens of Truth item.","",
  "When Link steps on this flag, the quest will", "end, and the credits will roll.","",
  // 16-31
  "","","",
  "","","",
  "","","",//18
  "","","",
  "","","",
  "","","",//21
  "","","",
  "","","",
  "","","",//24
  "","","",
  "","","",
  "","","",//27
  "","","",
  "","","",
  "","","",//30
  "","","",
  // Anyway...
  "Creates the lowest-numbered enemy with the","'Spawned by 'Horz Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
  "Creates the lowest-numbered enemy with the","'Spawned by 'Vert Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
  "Creates the lowest-numbered enemy with the","'Spawned by '4-Way Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
  "Creates the lowest-numbered enemy with the","'Spawned by 'LR Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
  "Creates the lowest-numbered enemy with the","'Spawned by 'UD Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
  // Enemy 0-9
  "","","",
  "","","",
  "","","",//2
  "","","",
  "","","",
  "","","",//5
  "","","",
  "","","",
  "","","",//8
  "","","",
  //Anyway...
  "Allows Link to push the combo left or right once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
  "Allows Link to push the combo up once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
  "Allows Link to push the combo down once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
  "Allows Link to push the combo left once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
  "Allows Link to push the combo right once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
  // Push Silent
  "","","",//52
  "","","",
  "","","",
  "","","",
  "","","",
  "","","",
  "","","",
  "","","",//59
  "","","",
  "","","",
  "","","",
  "","","",
  "","","",
  "","","",
  //Anyway...
  "Pushing blocks onto ALL Block Triggers will trigger","Screen Secrets (or just the 'Stairs' secret combo)","as well as Block->Shutters.",
  "Prevents push blocks from being pushed onto the","flagged combo, even if it is not solid.","",
  "Triggers Screen Secrets when Link", "touches it with one of his Boomerangs.", "Is replaced with the 'Wooden Boomerang' Secret Combo.",
  "Triggers Screen Secrets when Link touches", "it with a level 2 or higher Boomerang.", "Is replaced with the 'Magic Boomerang' Secret Combo.",
  "Triggers Screen Secrets when Link touches", "it with a level 3 or higher Boomerang.", "Is replaced with the 'Fire Boomerang' Secret Combo.",
  "Triggers Screen Secrets when Link touches", "it with a level 2 or higher Arrow.", "Is replaced with the 'Silver Arrow' Secret Combo.",
  "Triggers Screen Secrets when Link touches", "it with a level 3 or higher Arrow.", "Is replaced with the 'Golden Arrow' Secret Combo.",
  "Triggers Screen Secrets when Link touches it with", "fire from a level 2 Candle, a Wand, or Din's Fire.", "Is replaced with the 'Red Candle' Secret Combo.",
  "Triggers Screen Secrets when Link touches it with", "fire from a Wand, or Din's Fire.", "Is replaced with the 'Wand Fire' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with Din's Fire. Is replaced", "with the 'Din's Fire' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with Wand magic, be it fire or not.", "Is replaced with the 'Wand Magic' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with reflected Wand magic.", "Is replaced with the 'Reflected Magic' Secret Combo.",
  "Triggers Screen Secrets when Link touches", "it with a Shield-reflected fireball.", "Is replaced with the 'Reflected Fireball' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with one of his Swords.", "Is replaced with the 'Wooden Sword' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with a level 2 or higher Sword.", "Is replaced with the 'White Sword' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with a level 3 or higher Sword.", "Is replaced with the 'Magic Sword' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with a level 4 or higher Sword.", "Is replaced with the 'Master Sword' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with one of his Sword beams.", "Is replaced with the 'Sword Beam' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with a level 2 or higher Sword's beam.", "Is replaced with the 'White Sword Beam' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with a level 3 or higher Sword's beam.", "Is replaced with the 'Magic Sword Beam' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with a level 4 or higher Sword's beam.", "Is replaced with the 'Master Sword Beam' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with one of his Hookshot hooks.", "Is replaced with the 'Hookshot' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with one of his Wands.", "Is replaced with the 'Wand' Secret Combo.",
  "Triggers Screen Secrets when Link", "pounds it with one of his Hammers.", "Is replaced with the 'Hammer' Secret Combo.",
  "Triggers Screen Secrets when Link", "touches it with any weapon or projectile.", "Is replaced with the 'Any Weapon' Secret Combo.",
  "A push block pushed onto this flag","will cycle to the next combo in the list,","and lose the Push flag that was presumably on it.",
  "Makes a heart circle appear on screen", "when Link steps on it, and refills his magic.", "See also the Heart Circle-related Quest Rules.",
  "Makes a heart circle appear on screen when", "Link steps on it, and refills his life and magic.", "See also the Heart Circle-related Quest Rules.",
  "When stacked with a Trigger Combo Flag, it","prevents the triggered Secrets process from","changing all other flagged combos on-screen.",
  "Similar to 'Trigger->Self Only', but the","Secret Tile (16-31) flagged combos will still change.","(The 'Hit All Triggers->16-31' Screen Flag overrides this.)",
  "Enemies cannot enter or appear","on the flagged combo.","",
  "This flag currently has no use.","Do not use it in your quests, as","its purpose may change in future versions.",
  //Only Script Flags follow.
  "", "", "",
  "", "", "",
  "", "", "",
  "", "", "",
  "", "", "",
  //Raft bounce flag! ^_^
  "When Link is rafting, and hits","this flag, he will be turned around.", ""
};

// eMAXGUYS is defined in zdefs.h
const char *old_guy_string[OLDMAXGUYS] =
{
  "(None)","Abei","Ama","Merchant","Moblin","Fire","Fairy","Goriya","Zelda","Abei 2","Empty","","","","","","","","","",
  // 020
  "Octorok (L1, Slow)","Octorok (L2, Slow)","Octorok (L1, Fast)","Octorok (L2, Fast)","Tektite (L1)",
  // 025
  "Tektite (L2)","Leever (L1)","Leever (L2)","Moblin (L1)","Moblin (L2)",
  // 030
  "Lynel (L1)","Lynel (L2)","Peahat (L1)","Zora","Rock",
  // 035
  "Ghini (L1, Normal)","Ghini (L1, Phantom)","Armos","Keese (CSet 7)","Keese (CSet 8)",
  // 040
  "Keese (CSet 9)","Stalfos (L1)","Gel (L1, Normal)","Zol (L1, Normal)","Rope (L1)",
  // 045
  "Goriya (L1)","Goriya (L2)","Trap (4-Way)","Wall Master","Darknut (L1)",
  // 050
  "Darknut (L2)","Bubble (Sword, Temporary Disabling)","Vire (Normal)","Like Like","Gibdo",
  // 055
  "Pols Voice (Arrow)","Wizzrobe (Teleporting)","Wizzrobe (Floating)","Aquamentus (Facing Left)","Moldorm",
  // 060
  "Dodongo","Manhandla (L1)","Gleeok (1 Head)","Gleeok (2 Heads)","Gleeok (3 Heads)",
  // 065
  "Gleeok (4 Heads)","Digdogger (1 Kid)","Digdogger (3 Kids)","Digdogger Kid (1)","Digdogger Kid (2)",
  // 070
  "Digdogger Kid (3)","Digdogger Kid (4)","Gohma (L1)","Gohma (L2)","Lanmola (L1)",
  // 075
  "Lanmola (L2)","Patra (Big Circle)","Patra (Oval)","Ganon","Stalfos (L2)",
  // 080
  "Rope (L2)","Bubble (Sword, Permanent Disabling)","Bubble (Sword, Re-enabling)","Shooter (Fireball)","Item Fairy ",
  // 085
  "Fire","Octorok (Magic)", "Darknut (Death Knight)", "Gel (L1, Tribble)", "Zol (L1, Tribble)",
  // 090
  "Keese (Tribble)", "Vire (Tribble)", "Darknut (Splitting)", "Aquamentus (Facing Right)", "Manhandla (L2)",
  // 095
  "Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (Horizontal, Constant)", "Trap (Vertical, Constant)", "Wizzrobe (Fire)",
  // 100
  "Wizzrobe (Wind)", "Ceiling Master ", "Floor Master ", "Patra (BS Zelda)", "Patra (L2)",
  // 105
  "Patra (L3)", "Bat", "Wizzrobe (Bat)", "Wizzrobe (Bat 2) ", "Gleeok (Fire, 1 Head)",
  // 110
  "Gleeok (Fire, 2 Heads)",  "Gleeok (Fire, 3 Heads)","Gleeok (Fire, 4 Heads)", "Wizzrobe (Mirror)", "Dodongo (BS Zelda)",
  // 115
  "Dodongo (Fire) ","Trigger", "Bubble (Item, Temporary Disabling)", "Bubble (Item, Permanent Disabling)", "Bubble (Item, Re-enabling)",
  // 120
  "Stalfos (L3)", "Gohma (L3)", "Gohma (L4)", "NPC 1 (Standing) ", "NPC 2 (Standing) ",
  // 125
  "NPC 3 (Standing) ", "NPC 4 (Standing) ", "NPC 5 (Standing) ", "NPC 6 (Standing) ", "NPC 1 (Walking) ",
  // 130
  "NPC 2 (Walking) ", "NPC 3 (Walking) ", "NPC 4 (Walking) ", "NPC 5 (Walking) ", "NPC 6 (Walking) ",
  // 135
  "Boulder", "Goriya (L3)", "Leever (L3)", "Octorok (L3, Slow)", "Octorok (L3, Fast)",
  // 140
  "Octorok (L4, Slow)", "Octorok (L4, Fast)", "Trap (8-Way) ", "Trap (Diagonal) ", "Trap (/, Constant) ",
  // 145
  "Trap (/, Line of Sight) ", "Trap (\\, Constant) ", "Trap (\\, Line of Sight) ", "Trap (CW, Constant) ", "Trap (CW, Line of Sight) ",
  // 150
  "Trap (CCW, Constant) ", "Trap (CCW, Line of Sight) ", "Summoner", "Wizzrobe (Ice) ", "Shooter (Magic)",
  // 155
  "Shooter (Rock)", "Shooter (Spear)", "Shooter (Sword)", "Shooter (Fire)", "Shooter (Fire 2)",
  // 160
  "Bombchu", "Gel (L2, Normal)", "Zol (L2, Normal)", "Gel (L2, Tribble)", "Zol (L2, Tribble)",
  // 165
  "Tektite (L3) ", "Spinning Tile (Combo)", "Spinning Tile (Enemy Sprite)", "Lynel (L3) ", "Peahat (L2) ",
  // 170
  "Pols Voice (Magic)", "Pols Voice (Whistle) ", "Darknut (Mirror) ", "Ghini (L2, Fire) ", "Ghini (L2, Magic) ",
  // 175
  "Grappler Bug (HP) ", "Grappler Bug (MP) "
};

char *guy_string[eMAXGUYS];

const char *enetype_string[eeMAX] =
{
	"-Guy",
	"Walking Enemy",
	"-Unused",
	"Tektite",
	"Leever",
	"Peahat",
	"Zora",
	"Rock",
	"Ghini",
	"-Unused",
	"Keese",
	"-Unused",
	"-Unused",
	"-Unused",
	"-Unused",
	"Trap",
	"Wall Master",
	"-Unused",
	"-Unused",
	"-Unused",
	"-Unused",
	"Wizzrobe",
	"Aquamentus",
	"Moldorm",
	"Dodongo",
	"Manhandla",
	"Gleeok",
	"Digdogger",
	"Gohma",
	"Lanmola",
	"Patra",
	"Ganon",
	"Projectile Shooter",
	"-Unused",
	"-Unused",
	"-Unused",
	"Keese Tribble",
	"Spin Tile",
	"(None)",
	"-Fairy",
	"Other (Floating)",
	"Other"
};

const char *eneanim_string[aMAX] =
{
	"(None)",
	"Flip",
	"Unused",
	"2-Frame",
	"Unused",
	"Octorok (NES)",
	"Tektite (NES)",
	"Leever (NES)",
	"Walker",
	"Zora (NES)",
	"Zora (4-Frame)",
	"Ghini",
	"Armos (NES)",
	"Rope",
	"Wall Master (NES)",
	"Wall Master (4-Frame)",
	"Darknut (NES)",
	"Vire",
	"3-Frame",
	"Wizzrobe (NES)",
	"Aquamentus",
	"Dodongo (NES)",
	"Manhandla",
	"Gleeok",
	"Digdogger",
	"Gohma",
	"Lanmola",
	"2-Frame Flying",
	"4-Frame 4-Dir + Tracking",
	"4-Frame 8-Dir + Tracking",
	"4-Frame 4-Dir + Firing",
	"4-Frame 4-Dir",
	"4-Frame 8-Dir + Firing",
	"Armos (4-Frame)",
	"4-Frame Flying 4-Dir",
	"4-Frame Flying 8-Dir",
	"Unused",
	"4-Frame 8-Dir Big",
	"Tektite (4-Frame)",
	"3-Frame 4-Dir",
	"2-Frame 4-Dir",
	"Leever (4-Frame)",
	"2-Frame 4-Dir + Tracking",
	"Wizzrobe (4-Frame)",
	"Dodongo (4-Frame)",
	"Dodongo BS (4-Frame)",
	"4-Frame Flying 8-Dir + Firing",
	"4-Frame Flying 4-Dir + Firing",
	"4-Frame",
	"Ganon",
	"2-Frame Big"
};

/*
char *itemset_string[isMAX] =
{
	"(None)",
	"Default",
	"Bombs",
	"Rupees",
	"Life",
	"Bombs 100%",
	"Super Bombs 100%",
	"Magic",
	"Magic + Bombs",
	"Magic + Rupees",
	"Magic + Life",
	"Magic 2"
};
*/

const char *eweapon_string[wMax-wEnemyWeapons] =
{
	"(None)",
	"Fireball",
	"Arrow",
	"Boomerang",
	"Sword",
	"Rock",
	"Magic",
	"Bomb Blast",
	"Super Bomb Blast",
	"Lit Bomb",
	"Lit Super Bomb",
	"Fire Trail",
	"Flame",
	"Wind",
	"Flame 2",
	"Flame 2 Trail <unused>",
	"Ice <unused>",
	"Fireball (Rising)"
};

const char *walkmisc1_string[e1tLAST] =
{
	"1 Shot", "1 (End-Halt)", "Rapid-Fire", "1 (Fast)", "1 (Slanted)", "3 Shots", "4 Shots", "5 Shots", "3 (Fast)", "Breath", "8 Shots", "Summon", "Summon (Layer)"
};
const char *walkmisc2_string[e2tTRIBBLE+1] =
{
	"Normal", "Split On Hit", "Split On Death", "8 Shots", "Explode", "Tribble"
};

const char *walkmisc7_string[e7tEATHURT+1] =
{
	"None", "Temp. Jinx", "Perm. Jinx", "Cure Jinx", "Lose Magic", "Lose Rupees", "Drunk", "Eat (Items)", "Eat (Magic)", "Eat (Rupees)", "Eat (Damage)"
};

const char *walkmisc9_string[e9tARMOS+1] =
{
	"Normal", "Rope", "Vire", "Pols Voice", "Armos"
};

const char *pattern_string[MAXPATTERNS] =
{
  "Spawn (Classic)", "Enter from Sides (Consecutive)", "Enter from Sides (Random)", "Fall From Ceiling (Classic)", "Fall From Ceiling (Random)", "Spawn (Random)"
};

const char *short_pattern_string[MAXPATTERNS] =
{
  "Spawn (C)", "Sides", "Sides (R)", "Ceiling (C)", "Ceiling (R)", "Spawn (R)"
};

const char *midi_string[MAXCUSTOMMIDIS_ZQ] =
{
  "(None)",
  "Overworld",
  "Dungeon",
  "Level 9",
};

const char *screen_midi_string[MAXCUSTOMMIDIS_ZQ+1] =
{
  "Use DMap MIDI",
  "(None)",
  "Overworld",
  "Dungeon",
  "Level 9",
};

void refresh(int flags);
void domouse();
void init_doorcombosets();

int onNew();
int onOpen();
int onOpen2();
int onRevert();
int onSave();
int onSaveAs();
int onQuestTemplates();

int onUndo();
int onCopy();
int onPaste();
int onPasteAll();
int onPasteToAll();
int onPasteAllToAll();
int onDelete();
int onDeleteMap();

int onTemplate();
int onDoors();
int onCSetFix();
int onFlags();
int onShowPal();
int onReTemplate();

int playTune();
int playMIDI();
int stopMIDI();
int onKeyFile();

int onUp();
int onDown();
int onLeft();
int onRight();
int onPgUp();
int onPgDn();
int onIncreaseCSet();
int onDecreaseCSet();

int  onHelp();
void doHelp(int bg,int fg);

int onScrData();
int onGuy();
int onEndString();
int onString();
int onRType();
int onCatchall();
int onItem();
int onWarp();
int onWarp2();
int onPath();
int onEnemies();
int onEnemyFlags();
int onUnderCombo();
int onSecretCombo();

int onHeader();
int onAnimationRules();
int onComboRules();
int onItemRules();
int onEnemyRules();
int onFixesRules();
int onMiscRules();
int onRules2();
int onCheats();
int onStrings();
int onDmaps();
int onTiles();
int onCombos();
int onMidis();
int onShopTypes();
int onInfoTypes();
int onWarpRings();
int onWhistle();
int onMiscColors();
int onMapStyles();
int onTemplates();
int onDoorCombos();
int onTriPieces();
int onIcons();
int onInit();
int onLayers();
int onScreenPalette();
int xtoi(char *hexstr);

int onColors_Main();
int onColors_Levels();
int onColors_Sprites();

int onImport_Map();
int onImport_DMaps();
int onImport_Msgs();
int onImport_Combos();
int onImport_Tiles();
int onImport_Subscreen();
int onImport_Pals();
int onImport_ZGP();
int onImport_ZQT();
int onImport_UnencodedQuest();

int onExport_Map();
int onExport_DMaps();
int onExport_Msgs();
int onExport_MsgsText();
int onExport_Combos();
int onExport_Tiles();
int onExport_Subscreen();
int onExport_Pals();
int onExport_ZGP();
int onExport_ZQT();
int onExport_UnencodedQuest();

int onGotoMap();
int onMapCount();

int onViewPic();
int onViewMap();
int onComboPage();

int onDefault_Pals();
int onDefault_Tiles();
int onDefault_Combos();
int onDefault_Sprites();
int onDefault_MapStyles();

int onCustomItems();
int onCustomWpns();
int onCustomLink();
int onCustomGuys();

int onTest();
int onTestOptions();

int onOptions();

void draw_checkbox(BITMAP *dest,int x,int y,int bg,int fg,bool value);
void draw_layerradio(BITMAP *dest,int x,int y,int bg,int fg,int value);
void KeyFileName(char *kfname);


extern int draw_mode;
extern int alias_origin;

int onSpacebar()
{
  if(draw_mode==3)
  {
    alias_origin=(alias_origin+1)%4;
    return D_O_K;
  }
  combo_cols=!combo_cols;
  return D_O_K;
}

int onSnapshot()
{
  char buf[20];
  int num=0;
  do
  {
    sprintf(buf, "zelda%03d.%s", ++num, snapshotformat_str[SnapshotFormat][1]);
  } while(num<999 && exists(buf));

  blit(screen,screen2,0,0,0,0,zq_screen_w,zq_screen_h);
  PALETTE RAMpal2;
  get_palette(RAMpal2);
  save_bitmap(buf,screen2,RAMpal2);
  return D_O_K;
}

int gocnt=0;

void go()
{
  switch(gocnt)
  {
    case 0:
    scare_mouse();
    blit(screen,menu1,0,0,0,0,zq_screen_w,zq_screen_h);
    unscare_mouse();
    break;
    case 1:
    scare_mouse();
    blit(screen,menu3,0,0,0,0,zq_screen_w,zq_screen_h);
    unscare_mouse();
    break;
    default: return;
  }
  ++gocnt;
}

void comeback()
{
  switch(gocnt)
  {
    case 1:
    scare_mouse();
    blit(menu1,screen,0,0,0,0,zq_screen_w,zq_screen_h);
    unscare_mouse();
    break;
    case 2:
    scare_mouse();
    blit(menu3,screen,0,0,0,0,zq_screen_w,zq_screen_h);
    unscare_mouse();
    break;
    default: return;
  }
  --gocnt;
}

int checksave()
{
  if(saved)
    return 1;
  char buf[80];
  char *name = get_filename(filepath);
  if(name[0]==0)
    sprintf(buf,"Save this quest file?");
  else
    sprintf(buf,"Save changes to %s?",name);
  switch(jwin_alert3("ZQuest",buf,NULL,NULL,"&Yes","&No","Cancel",'y','n',27,lfont))
  {
    case 1:
    onSave();
    return 1;
    case 2:
    return 1;
  }
  return 0;
}

int onExit()
{
  restore_mouse();
  if(checksave()==0)
    return D_O_K;
  if(jwin_alert("ZQuest","Really want to quit?", NULL, NULL, "&Yes", "&No", 'y', 'n', lfont) == 2)
    return D_O_K;
  return D_CLOSE;
}

int onAbout()
{
  char buf1[80];
  char buf2[80];
  char buf3[80];
  if(get_debug())
  {
#if IS_BETA
    {
      sprintf(buf1,"ZQuest %s Beta Build %d - DEBUG",VerStr(ZELDA_VERSION), VERSION_BUILD);
    }
#else
    {
      sprintf(buf1,"ZQuest %s Build %d - DEBUG",VerStr(ZELDA_VERSION), VERSION_BUILD);
    }
#endif
    sprintf(buf2,"ZQuest Editor: %04X",INTERNAL_VERSION);
    sprintf(buf3,"This qst file: %04X",header.internal&0xFFFF);
    jwin_alert("About ZQuest",buf1,buf2,buf3,"OK", NULL, 13, 27, lfont);
  }
  else
  {
    switch (IS_BETA)
    {
      case -1:
        sprintf(buf1,"ZQuest %s Alpha Build %d",VerStr(ZELDA_VERSION), VERSION_BUILD);
        break;
      case 1:
        sprintf(buf1,"ZQuest %s Beta Build %d",VerStr(ZELDA_VERSION), VERSION_BUILD);
        break;
      case 0:
      default:
        sprintf(buf1,"ZQuest %s Build %d",VerStr(ZELDA_VERSION), VERSION_BUILD);
        break;
    }
    sprintf(buf3,"'The Travels of Link' sequenced by Jeff Glenen.");
    jwin_alert("About ZQuest",buf1,NULL,buf3,"OK", NULL, 13, 27, lfont);
  }
  return D_O_K;
}

int onShowWalkability()
{
  Flags^=cWALK;
  refresh(rMAP+rMENU);
  return D_O_K;
}

int onPreviewMode()
{
  prv_mode=(prv_mode+1)%2;
  if(prv_mode)
  {
    Map.set_prvscr(Map.getCurrMap(),Map.getCurrScr());
  }
  bool tempcb=ComboBrush!=0;
  ComboBrush=0;
  restore_mouse();
  dopreview();
  ComboBrush=tempcb;
  return D_O_K;
}

int onShowFlags()
{
  Flags^=cFLAGS;
  refresh(rMAP);
  return D_O_K;
}

int onP()
{
  if(prv_mode)
  {
    Map.set_prvfreeze(((Map.get_prvfreeze()+1)%2));
  }
  return D_O_K;
}

int onShowComboInfoCSet()
{
  if(Flags&cCSET)
  {
    Flags ^= cCSET;
    Flags |= cCTYPE;
  }
  else if(Flags&cCTYPE)
    {
      Flags ^= cCTYPE;
    }
    else
    {
      Flags |= cCSET;
    }

  refresh(rMAP);
  return D_O_K;
}

int onShowCSet()
{
  Flags^=cCSET;
  Flags&=~cCTYPE;
  refresh(rMAP);
  return D_O_K;
}

int onShowCType()
{
  Flags^=cCTYPE;
  Flags&=~cCSET;
  refresh(rMAP);
  return D_O_K;
}

int onShowDarkness()
{
  if(get_bit(quest_rules,qr_FADE))
  {
    int last = CSET(5)-1;

    if(get_bit(quest_rules,qr_FADECS5))
      last += 16;

    byte *si = colordata + CSET(Color*pdLEVEL+poFADE1)*3;
    for(int i=0; i<16; i++)
    {
      int light = si[0]+si[1]+si[2];
      si+=3;
      fade_interpolate(RAMpal,black_palette,RAMpal,light?32:64,CSET(2)+i,CSET(2)+i);
    }
    fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(3),last);
    set_palette(RAMpal);

    readkey();

    load_cset(RAMpal,5,5);
    loadlvlpal(Color);
  }
  else
  {
    loadfadepal(Color*pdLEVEL+poFADE3);
    readkey();
    loadlvlpal(Color);
  }
  return D_O_K;
}

int onM()
{
  return D_O_K;
}

int onJ()
{
  return D_O_K;
}

void setFlagColor()
{
  RAMpal[dvc(0)]=RAMpal[vc(Flag%16)];
  set_palette_range(RAMpal,dvc(0),dvc(0),false);
}

int onIncreaseFlag()
{
  Flag=(Flag+1);
  if (Flag==mfMAX)
  {
    Flag=0;
  }
  setFlagColor();
  refresh(rMENU);
  return D_O_K;
}

int onDecreaseFlag()
{
  if (Flag==0)
  {
    Flag=mfMAX;
  }
  Flag=(Flag-1);
  setFlagColor();
  refresh(rMENU);
  return D_O_K;
}

int on0();
int on1();
int on2();
int on3();
int on4();
int on5();
int on6();
int on7();
int on8();
int on9();
int on10();
int on11();
int on12();
int on13();
int on14();

int onToggleDarkness();
int onIncMap();
int onDecMap();

int onDumpScr();

// these are here so that copy_dialog won't choke when compiling zquest
int d_jbutton_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_kbutton_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_listen_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_savemidi_proc(int, DIALOG*, int)
{
  return D_O_K;
}




