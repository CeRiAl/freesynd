#include "font.h"
#include <stdlib.h>

const uint32_t RCGL_PALETTE_VGA1[256] = {
0x00000000, 0x000000aa, 0x0000aa00, 0x0000aaaa, 0x00aa0000, 0x00aa00aa, 
0x00aa5500, 0x00aaaaaa, 0x00555555, 0x005555ff, 0x0055ff55, 0x0055ffff, 
0x00ff5555, 0x00ff55ff, 0x00ffff55, 0x00ffffff, 0x00000000, 0x00141414, 
0x00202020, 0x002c2c2c, 0x00383838, 0x00454545, 0x00515151, 0x00616161, 
0x00717171, 0x00828282, 0x00929292, 0x00a2a2a2, 0x00b6b6b6, 0x00cbcbcb, 
0x00e3e3e3, 0x00ffffff, 0x000000ff, 0x004100ff, 0x007d00ff, 0x00be00ff, 
0x00ff00ff, 0x00ff00be, 0x00ff007d, 0x00ff0041, 0x00ff0000, 0x00ff4100, 
0x00ff7d00, 0x00ffbe00, 0x00ffff00, 0x00beff00, 0x007dff00, 0x0041ff00, 
0x0000ff00, 0x0000ff41, 0x0000ff7d, 0x0000ffbe, 0x0000ffff, 0x0000beff, 
0x00007dff, 0x000041ff, 0x007d7dff, 0x009e7dff, 0x00be7dff, 0x00df7dff, 
0x00ff7dff, 0x00ff7ddf, 0x00ff7dbe, 0x00ff7d9e, 0x00ff7d7d, 0x00ff9e7d, 
0x00ffbe7d, 0x00ffdf7d, 0x00ffff7d, 0x00dfff7d, 0x00beff7d, 0x009eff7d, 
0x007dff7d, 0x007dff9e, 0x007dffbe, 0x007dffdf, 0x007dffff, 0x007ddfff, 
0x007dbeff, 0x007d9eff, 0x00b6b6ff, 0x00c7b6ff, 0x00dbb6ff, 0x00ebb6ff, 
0x00ffb6ff, 0x00ffb6eb, 0x00ffb6db, 0x00ffb6c7, 0x00ffb6b6, 0x00ffc7b6, 
0x00ffdbb6, 0x00ffebb6, 0x00ffffb6, 0x00ebffb6, 0x00dbffb6, 0x00c7ffb6, 
0x00b6ffb6, 0x00b6ffc7, 0x00b6ffdb, 0x00b6ffeb, 0x00b6ffff, 0x00b6ebff, 
0x00b6dbff, 0x00b6c7ff, 0x00000071, 0x001c0071, 0x00380071, 0x00550071, 
0x00710071, 0x00710055, 0x00710038, 0x0071001c, 0x00710000, 0x00711c00, 
0x00713800, 0x00715500, 0x00717100, 0x00557100, 0x00387100, 0x001c7100, 
0x00007100, 0x0000711c, 0x00007138, 0x00007155, 0x00007171, 0x00005571, 
0x00003871, 0x00001c71, 0x00383871, 0x00453871, 0x00553871, 0x00613871, 
0x00713871, 0x00713861, 0x00713855, 0x00713845, 0x00713838, 0x00714538, 
0x00715538, 0x00716138, 0x00717138, 0x00617138, 0x00557138, 0x00457138, 
0x00387138, 0x00387145, 0x00387155, 0x00387161, 0x00387171, 0x00386171, 
0x00385571, 0x00384571, 0x00515171, 0x00595171, 0x00615171, 0x00695171, 
0x00715171, 0x00715169, 0x00715161, 0x00715159, 0x00715151, 0x00715951, 
0x00716151, 0x00716951, 0x00717151, 0x00697151, 0x00617151, 0x00597151, 
0x00517151, 0x00517159, 0x00517161, 0x00517169, 0x00517171, 0x00516971, 
0x00516171, 0x00515971, 0x00000041, 0x00100041, 0x00200041, 0x00300041, 
0x00410041, 0x00410030, 0x00410020, 0x00410010, 0x00410000, 0x00411000, 
0x00412000, 0x00413000, 0x00414100, 0x00304100, 0x00204100, 0x00104100, 
0x00004100, 0x00004110, 0x00004120, 0x00004130, 0x00004141, 0x00003041, 
0x00002041, 0x00001041, 0x00202041, 0x00282041, 0x00302041, 0x00382041, 
0x00412041, 0x00412038, 0x00412030, 0x00412028, 0x00412020, 0x00412820, 
0x00413020, 0x00413820, 0x00414120, 0x00384120, 0x00304120, 0x00284120, 
0x00204120, 0x00204128, 0x00204130, 0x00204138, 0x00204141, 0x00203841, 
0x00203041, 0x00202841, 0x002c2c41, 0x00302c41, 0x00342c41, 0x003c2c41, 
0x00412c41, 0x00412c3c, 0x00412c34, 0x00412c30, 0x00412c2c, 0x0041302c, 
0x0041342c, 0x00413c2c, 0x0041412c, 0x003c412c, 0x0034412c, 0x0030412c, 
0x002c412c, 0x002c4130, 0x002c4134, 0x002c413c, 0x002c4141, 0x002c3c41, 
0x002c3441, 0x002c3041, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
0x00000000, 0x00000000, 0x00000000, 0x00000000 };

const uint32_t RCGL_PALETTE_VGA[256] = {
0xff000000, 0xff0000aa, 0xff00aa00, 0xff00aaaa, 0xffaa0000, 0xffaa00aa, 
0xffaa5500, 0xffaaaaaa, 0xff555555, 0xff5555ff, 0xff55ff55, 0xff55ffff, 
0xffff5555, 0xffff55ff, 0xffffff55, 0xffffffff, 0xff000000, 0xff141414, 
0xff202020, 0xff2c2c2c, 0xff383838, 0xff454545, 0xff515151, 0xff616161, 
0xff717171, 0xff828282, 0xff929292, 0xffa2a2a2, 0xffb6b6b6, 0xffcbcbcb, 
0xffe3e3e3, 0xffffffff, 0xff0000ff, 0xff4100ff, 0xff7d00ff, 0xffbe00ff, 
0xffff00ff, 0xffff00be, 0xffff007d, 0xffff0041, 0xffff0000, 0xffff4100, 
0xffff7d00, 0xffffbe00, 0xffffff00, 0xffbeff00, 0xff7dff00, 0xff41ff00, 
0xff00ff00, 0xff00ff41, 0xff00ff7d, 0xff00ffbe, 0xff00ffff, 0xff00beff, 
0xff007dff, 0xff0041ff, 0xff7d7dff, 0xff9e7dff, 0xffbe7dff, 0xffdf7dff, 
0xffff7dff, 0xffff7ddf, 0xffff7dbe, 0xffff7d9e, 0xffff7d7d, 0xffff9e7d, 
0xffffbe7d, 0xffffdf7d, 0xffffff7d, 0xffdfff7d, 0xffbeff7d, 0xff9eff7d, 
0xff7dff7d, 0xff7dff9e, 0xff7dffbe, 0xff7dffdf, 0xff7dffff, 0xff7ddfff, 
0xff7dbeff, 0xff7d9eff, 0xffb6b6ff, 0xffc7b6ff, 0xffdbb6ff, 0xffebb6ff, 
0xffffb6ff, 0xffffb6eb, 0xffffb6db, 0xffffb6c7, 0xffffb6b6, 0xffffc7b6, 
0xffffdbb6, 0xffffebb6, 0xffffffb6, 0xffebffb6, 0xffdbffb6, 0xffc7ffb6, 
0xffb6ffb6, 0xffb6ffc7, 0xffb6ffdb, 0xffb6ffeb, 0xffb6ffff, 0xffb6ebff, 
0xffb6dbff, 0xffb6c7ff, 0xff000071, 0xff1c0071, 0xff380071, 0xff550071, 
0xff710071, 0xff710055, 0xff710038, 0xff71001c, 0xff710000, 0xff711c00, 
0xff713800, 0xff715500, 0xff717100, 0xff557100, 0xff387100, 0xff1c7100, 
0xff007100, 0xff00711c, 0xff007138, 0xff007155, 0xff007171, 0xff005571, 
0xff003871, 0xff001c71, 0xff383871, 0xff453871, 0xff553871, 0xff613871, 
0xff713871, 0xff713861, 0xff713855, 0xff713845, 0xff713838, 0xff714538, 
0xff715538, 0xff716138, 0xff717138, 0xff617138, 0xff557138, 0xff457138, 
0xff387138, 0xff387145, 0xff387155, 0xff387161, 0xff387171, 0xff386171, 
0xff385571, 0xff384571, 0xff515171, 0xff595171, 0xff615171, 0xff695171, 
0xff715171, 0xff715169, 0xff715161, 0xff715159, 0xff715151, 0xff715951, 
0xff716151, 0xff716951, 0xff717151, 0xff697151, 0xff617151, 0xff597151, 
0xff517151, 0xff517159, 0xff517161, 0xff517169, 0xff517171, 0xff516971, 
0xff516171, 0xff515971, 0xff000041, 0xff100041, 0xff200041, 0xff300041, 
0xff410041, 0xff410030, 0xff410020, 0xff410010, 0xff410000, 0xff411000, 
0xff412000, 0xff413000, 0xff414100, 0xff304100, 0xff204100, 0xff104100, 
0xff004100, 0xff004110, 0xff004120, 0xff004130, 0xff004141, 0xff003041, 
0xff002041, 0xff001041, 0xff202041, 0xff282041, 0xff302041, 0xff382041, 
0xff412041, 0xff412038, 0xff412030, 0xff412028, 0xff412020, 0xff412820, 
0xff413020, 0xff413820, 0xff414120, 0xff384120, 0xff304120, 0xff284120, 
0xff204120, 0xff204128, 0xff204130, 0xff204138, 0xff204141, 0xff203841, 
0xff203041, 0xff202841, 0xff2c2c41, 0xff302c41, 0xff342c41, 0xff3c2c41, 
0xff412c41, 0xff412c3c, 0xff412c34, 0xff412c30, 0xff412c2c, 0xff41302c, 
0xff41342c, 0xff413c2c, 0xff41412c, 0xff3c412c, 0xff34412c, 0xff30412c, 
0xff2c412c, 0xff2c4130, 0xff2c4134, 0xff2c413c, 0xff2c4141, 0xff2c3c41, 
0xff2c3441, 0xff2c3041, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 
0xff000000, 0xff000000, 0xff000000, 0xff000000 };

#if 0
    static SDL_Color palette[256];

    Uint8 r[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };
    Uint8 g[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };
    Uint8 b[4] = { 0, 85, 170, 255 };
    int curColor = 0;
    for( unsigned int i = 0; i < 8; ++i )
    for( unsigned int j = 0; j < 8; ++j )
    for( unsigned int k = 0; k < 4; ++k ) {
        SDL_Color color = { r[i], g[j], b[k], 255 };
        palette[curColor] = color;
        // SDL_SetPaletteColors( surface->format->palette, &color, curColor, 1 );
        curColor++;
    }
    /*
    for (int i = 0; i < cols; ++i) {
        palette[i].r = pal[i * 3 + 0];
        palette[i].g = pal[i * 3 + 1];
        palette[i].b = pal[i * 3 + 2];
        palette[i].a = 255;
    }
    SDL_SetPaletteColors(surface_8bpp_->format->palette, palette, 0, cols);
    */
#endif