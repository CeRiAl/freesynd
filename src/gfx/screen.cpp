/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include "common.h"
#include "screen.h"
#include "utils/file.h"

#include "gltiles.h"

Texture::Texture(int width, int height, GLenum target, bool smooth) :
width_(width),
height_(height),
surface_8bpp_(NULL),
surface_32bpp_(NULL),
texture_target_(target)
{
    GLfloat texParam = smooth ? GL_LINEAR : GL_NEAREST;

    glGenTextures(1, &texture_name_);

    glBindTexture(texture_target_, texture_name_);
    glTexImage2D(texture_target_, 0,
                GL_RGBA, width_, height_, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)NULL);

    // For tiles only?
    glTexParameteri(texture_target_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texture_target_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //

    glTexParameterf(texture_target_, GL_TEXTURE_MIN_FILTER, texParam);
    glTexParameterf(texture_target_, GL_TEXTURE_MAG_FILTER, texParam);

    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    surface_32bpp_ = SDL_CreateRGBSurface(SDL_SWSURFACE,
        width_, height_,
        32, rmask, gmask, bmask, amask);

    // SDL_FillRect(surface_32bpp_, NULL, SDL_MapRGBA(surface_32bpp_->format, 0, 0, 0, 255));

    surface_8bpp_ = SDL_CreateRGBSurface(SDL_SWSURFACE,
        width_, height_,
        8, 0, 0, 0, 0);

    /*
    // SDL_BLENDMODE_NONE, SDL_BLENDMODE_BLEND
    SDL_SetSurfaceBlendMode(surface_8bpp_, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceAlphaMod(surface_8bpp_, 255);

    SDL_SetColorKey(surface_8bpp_, SDL_TRUE, SDL_MapRGB(surface_8bpp_->format, 255, 255, 255));

    SDL_FillRect(surface_8bpp_, NULL, SDL_MapRGB(surface_8bpp_->format, 0, 255, 0));
    */

}

Texture::~Texture()
{
    if (texture_name_)
        glDeleteTextures(1, &texture_name_);

    if (surface_32bpp_)
        SDL_FreeSurface(surface_32bpp_);

    if (surface_8bpp_)
        SDL_FreeSurface(surface_8bpp_);
}

void Texture::update(const uint8 * data, bool flipped) {
    SDL_LockSurface(surface_8bpp_);
    if (flipped) {
        for (int row = 0; row < height_; row++) {
            uint8 *src_pixels = (uint8 *)data + (height_ - row - 1) * width_;
            uint8 *dst_pixels = (uint8 *)surface_8bpp_->pixels + row * width_;
            memcpy(dst_pixels, src_pixels, width_);
        }
    } else {
        memcpy(surface_8bpp_->pixels, data, width_ * height_);
    }
    SDL_UnlockSurface(surface_8bpp_);

    update(surface_8bpp_);
}

void Texture::update(SDL_Surface *src_surface) {
    update(src_surface, 0, 0, width_, height_);
}

void Texture::update(SDL_Surface *src_surface, int x, int y, int width, int height) {
    SDL_Rect rect = {x, y, width, height};
    // SDL_Rect rect{x, y, width, height}; // for C++11 & up

    SDL_BlitSurface(src_surface, &rect, surface_32bpp_, &rect);

    /*
    if (src_surface->format->BitsPerPixel == 8) {
        SDL_LockSurface(surface_32bpp_);
        for (unsigned int pos = 0; pos < width * height; pos++) {
            uint8 *pixels = (uint8 *)surface_32bpp_->pixels +
                (pos * 4) + 3;
            
            *pixels = 255;
        }
        SDL_UnlockSurface(surface_32bpp_);
    }
    */

    uint8 *pixels = (uint8 *)surface_32bpp_->pixels;

    glBindTexture(texture_target_, texture_name_);
    glTexSubImage2D(texture_target_, 0,
                    // x, y,
                    // width, height,
                    0, 0,
                    width_, height_,
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(texture_target_, 0);
}


void Texture::xupdate(const uint8 *data, int x, int y, int width, int height) {
    SDL_Rect rect = { x, y, width, height };

    SDL_FillRect(surface_8bpp_, &rect, SDL_MapRGB(surface_8bpp_->format, 0, 255, 0));

    update(surface_8bpp_, x, y, width, height);
    // update(surface_8bpp_, 0, 0, width_, height_);
    // update(surface_8bpp_);
}

void Texture::update(const uint8 *data, int x, int y, int width, int height) {
    SDL_LockSurface(surface_8bpp_);
    for (int row = 0; row < height; row++) {

        uint8 *pixels = (uint8 *)surface_8bpp_->pixels +
            (y + row) * width_ +
            // (y + row) * surface_8bpp_->pitch +
            x * sizeof *pixels;

        /*
        Uint32 *target_pixel = (Uint8 *) surface->pixels + y * surface->pitch +
            x * sizeof *target_pixel;
        */

        // memcpy(surface_8bpp_->pixels, data, width * height);
        memcpy(pixels, data + row * width, width);

        /*
        uint32_t *pixels = texture +
                                (cc / char_cols) * texture_width * char_height +
                                (cc % char_cols) * char_width;
        */
    }

    SDL_UnlockSurface(surface_8bpp_);

    update(surface_8bpp_, x, y, width, height);
}

void Texture::updateFromFB(int x, int y, int width, int height) {
    glBindTexture(texture_target_, texture_name_);
    glCopyTexImage2D(texture_target_, 0,
                    GL_RGBA, x, y, width, height, 0);
    glBindTexture(texture_target_, 0);
}

void Texture::setPalette(const uint8 * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        palette[i].r = pal[i * 3 + 0];
        palette[i].g = pal[i * 3 + 1];
        palette[i].b = pal[i * 3 + 2];
        palette[i].a = 255;
    }

    SDL_SetPaletteColors(surface_8bpp_->format->palette, palette, 0, cols);
}

void Texture::setPalette(const SDL_Color *pal, int cols) {
    SDL_SetPaletteColors(surface_8bpp_->format->palette, pal, 0, cols);
}




// /////////////

const int Screen::kScreenWidth = 800; // 640;
const int Screen::kScreenHeight = 600; // 400;
const int Screen::kScreenPanelWidth = 129;

const GLfloat Screen::planeValueS_[] = {
    0.5, 
    -0.5,
    0.0,
    0.5
};
const GLfloat Screen::planeValueT_[] = {
    0.25,
    0.25,
    0.5,
    0.25
};

Screen::Screen(int width, int height)
:width_(width)
, height_(height)
, data_logo_(NULL)
, data_mini_logo_(NULL),
background_texture_(NULL),
logo_texture_(NULL),
logo_mini_texture_(NULL),

  distance_(8000),
  zenith_(-60),
  azimuth_(45),
  pan_x_(-2000),
  pan_y_(-18000),
  zoom_(1.0),
  perspective_(false),
  smooth_(false),
  change_size_(true),
  change_zoom_(true),
  change_pan_(true),
  on_screen_mode_(false)
{

    screen_surf_ = NULL;
    tscreen_surf_ = NULL;
    screen_texture_ = 0;
    tscreen_texture_ = 0;
    temp_surf_ = NULL;
    ttemp_surf_ = NULL;

    assert(width_ > 0);
    assert(height_ > 0);
}

Screen::~Screen()
{
    if (data_logo_)
        delete[] data_logo_;
    if (data_mini_logo_)
        delete[] data_mini_logo_;

    if (logo_texture_)
        delete logo_texture_;
    if (logo_mini_texture_)
        delete logo_mini_texture_;

    if (background_texture_)
        delete background_texture_;

    if (screen_texture_)
        glDeleteTextures(1, &screen_texture_);

    if (tscreen_texture_)
        glDeleteTextures(1, &tscreen_texture_);

    if (temp_surf_)
        SDL_FreeSurface(temp_surf_);
    if (ttemp_surf_)
        SDL_FreeSurface(ttemp_surf_);

    if (tscreen_surf_)
        SDL_FreeSurface(tscreen_surf_);
    if (screen_surf_)
        SDL_FreeSurface(screen_surf_);
}

void Screen::clear(uint8 color)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Screen::drawVLine(int x, int y, int length, uint8 color) {
    if (x < 0 || x >= width_ || y + length < 0 || y >= height_)
        return;

    length = y + length >= height_ ? height_ - y : length;
    if (y < 0) {
        length += y;
        y = 0;
    }
    if (length < 1)
        return;

    drawLine(x, y, x, y + length, color);
}

void Screen::drawHLine(int x, int y, int length, uint8 color) {
    if ((x + length) < 0 || x >= width_ || y < 0 || y >= height_)
        return;

    length = x + length >= width_ ? width_ - x : length;
    if (x < 0) {
        length += x;
        x = 0;
    }
    if (length < 1)
        return;

    drawLine(x, y, x + length, y, color);    
}

int Screen::numLogos()
{
    return size_logo_ / (32 * 32);
}

void Screen::drawLogo(int x, int y, int logo, int colour, bool mini)
{
    if (data_logo_ == NULL) {
        data_logo_ = File::loadOriginalFile("mlogos.dat", size_logo_);

        for (int i = 0; i < size_logo_; i++)
            if (data_logo_[i] == 0xFE)
                data_logo_[i] = 0xFF;

        logo_texture_ = new Texture(32, 32);
        logo_texture_->setPalette(current_palette_);
    }
    if (data_mini_logo_ == NULL) {
        data_mini_logo_ = File::loadOriginalFile("mminlogo.dat", size_mini_logo_);

        for (int i = 0; i < size_mini_logo_; i++)
            if (data_mini_logo_[i] == 0xFE)
                data_mini_logo_[i] = 0xFF;

        logo_mini_texture_ = new Texture(16, 16);
        logo_mini_texture_->setPalette(current_palette_);
    }

    if (mini) {
        logo_mini_texture_->update(data_mini_logo_ + logo * 16 * 16);
        drawRect(x, y, 16 * 2, 16 * 2, colour);
        renderTexture2x(logo_mini_texture_, x, y, 16, 16);
    } else {
        logo_texture_->update(data_logo_ + logo * 32 * 32);
        drawRect(x, y, 32 * 2, 32 * 2, colour);
        renderTexture2x(logo_texture_, x, y, 32, 32);
    }
}

// Taken from SDL_gfx
void Screen::drawLine(int x1, int y1, int x2, int y2, uint8 color,
                      int skip, int off) {
#if 0
    int pixx, pixy;
    int x, y;
    int dx, dy;
    int sx, sy;
    int swaptmp;
    uint8 *pixel;

    /*
     * Variable setup
     */
    dx = x2 - x1;
    dy = y2 - y1;
    sx = (dx >= 0) ? 1 : -1;
    sy = (dy >= 0) ? 1 : -1;

    /*
     * No alpha blending - use fast pixel routines
     */

    /*
     * More variable setup
     */
    dx = sx * dx + 1;
    dy = sy * dy + 1;
    pixx = 1;
    pixy = GAME_SCREEN_WIDTH;
    pixel = pixels_ + pixx * (int) x1 + pixy * (int) y1;
    pixx *= sx;
    pixy *= sy;
    if (dx < dy) {
        swaptmp = dx;
        dx = dy;
        dy = swaptmp;
        swaptmp = pixx;
        pixx = pixy;
        pixy = swaptmp;
    }

    /*
     * Draw
     */
    x = 0;
    y = 0;
    int count = 0;
    for (; x < dx; x++, pixel += pixx) {
        if (skip == 0 || !(((off + count++) / skip) & 1))
            if (pixel >= pixels_ && pixel < pixels_ + width_ * height_)
                *pixel = color;
        y += dy;
        if (y >= dx) {
            y -= dx;
            pixel += pixy;
        }
    }
#endif

    uint8 _r = 0;
    uint8 _g = 0;
    uint8 _b = 0;    
    getColor(color, _r, _g, _b);

    glBegin(GL_LINES);
        // glLineWidth(5.0);  
        glColor3f((float)_r / 255.0f, (float)_g / 255.0f, (float)_b / 255.0f);

        glVertex2f(x1, y1);
        glVertex2f(x2, y2);

        glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();
}

void Screen::setPixel(int x, int y, uint8 color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_)
        return;

    uint8 _r = 0;
    uint8 _g = 0;
    uint8 _b = 0;    
    getColor(color, _r, _g, _b);

    glBegin(GL_POINTS);
        glColor3f((float)_r / 255.0f, (float)_g / 255.0f, (float)_b / 255.0f);

        glVertex2f(x, y);

        glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();
}

void Screen::drawRect(int x, int y, int width, int height, uint8 color) {
    if (x < 0 || y < 0 || (x + width) > width_
        || (y + height) > height_ || width <= 0 || height <= 0)
        return;

    // NOTE: we don't handle properly clipping by (x,y), do we need it?

    uint8 _r = 0;
    uint8 _g = 0;
    uint8 _b = 0;    
    getColor(color, _r, _g, _b);

    glBegin(GL_QUADS);
        glColor3f((float)_r / 255.0f, (float)_g / 255.0f, (float)_b / 255.0f);

        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);

        glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();
}

int Screen::gameScreenHeight() {
    return kScreenHeight;
}

int Screen::gameScreenWidth() {
    return kScreenWidth;
}

int Screen::gameScreenLeftMargin() {
    return kScreenPanelWidth;
}

void Screen::getColor(uint8 color, uint8 &r, uint8 &g, uint8 &b) {
    SDL_Color col = current_palette_[color];

    r = col.r;
    g = col.g;
    b = col.b;
}

void Screen::setPalette(SDL_Color *pal, int cols) {
    for(int i = 0; i < 256; i++) {
        current_palette_[i].r = pal[i].r;
        current_palette_[i].g = pal[i].g;
        current_palette_[i].b = pal[i].b;
        current_palette_[i].a = pal[i].a;
    }
}


bool Screen::enterOnScreenMode(void) {
  glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT) ; 

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glOrtho(0.0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 0.0, 0.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  on_screen_mode_ = true;

  return true;
}

bool Screen::leaveOnScreenMode(void) {
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glPopAttrib();

  on_screen_mode_ = false;

  return true;
}

bool Screen::initTileVertices(void) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer (3, GL_FLOAT, 0, tiles_vertices);

    return true;
}

void Screen::initScreen(void) {
    background_texture_ = new Texture(MENU_SCREEN_WIDTH, MENU_SCREEN_HEIGHT);

    // Construct a surface that's in a format close to the texture
    screen_surf_ = SDL_CreateRGBSurface(SDL_SWSURFACE,
        GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT,
        32, 0, 0, 0, 0);

    tscreen_surf_ = SDL_CreateRGBSurface(SDL_SWSURFACE,
        GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT,
        32, 0, 0, 0, 0);

    temp_surf_ = SDL_CreateRGBSurface(SDL_SWSURFACE,
        GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT,
        8, 0, 0, 0, 0);

    ttemp_surf_ = SDL_CreateRGBSurface(SDL_SWSURFACE,
        GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT,
        8, 0, 0, 0, 0);

    glClearColor(0.8, 0.8, 0.8, 1.0);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f);
    // glAlphaFunc(GL_GREATER, 0.0f);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, gameScreenWidth(), gameScreenHeight());

    // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    glGenTextures(1, &screen_texture_);
    glBindTexture(GL_TEXTURE_2D, screen_texture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    if(true) {  // _smooth
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT,
                GL_RGBA, GL_UNSIGNED_BYTE, screen_surf_);
}

void Screen::renderStart(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (change_size_) {
        glViewport (0, 0, gameScreenWidth(), gameScreenHeight());
    }


    /**********************/
    /** render in world  **/
    /**********************/
    glMatrixMode(GL_PROJECTION);

    if(change_size_ || change_zoom_) {
        glLoadIdentity();

        if(perspective_) {
            gluPerspective(45.0 * zoom_, (float)gameScreenWidth() / (float)gameScreenHeight(), 0.0, 10000.0);
        } else {
            GLdouble horizontal_clip = 1800.0 * zoom_;
            GLdouble vertical_clip = horizontal_clip * (float)gameScreenHeight() / (float)gameScreenWidth();
            glOrtho(-horizontal_clip, horizontal_clip, 
                -vertical_clip, vertical_clip, 
                -10000.0, 10000.0);
        }
    }
    
    glMatrixMode(GL_MODELVIEW);

    if(change_size_ || change_zoom_ || change_pan_) {
        glLoadIdentity();
        glRotatef(180.0, 1.0, 0.0, 0.0);
        
        if(perspective_) {
            glTranslatef(0.0, 0.0, distance_ * zoom_);
        }
        glRotatef(zenith_, 1.0, 0.0, 0.0);
        glTranslatef(pan_x_, pan_y_, 0);
        glRotatef(azimuth_, 0.0, 0.0, 1.0);
    }

    change_size_ = false;
    change_zoom_ = false;
    change_pan_ = false;
}

void Screen::renderBackground(void) {
    renderBackground(0, 0, kScreenWidth, kScreenHeight);
}

void Screen::renderBackground(int x, int y, int width, int height) {
    enterOnScreenMode();
        renderTexture(background_texture_, x, y, width, height);
    leaveOnScreenMode();
}

bool Screen::renderScreen(void) {
    SDL_LockSurface(temp_surf_);

    SDL_UnlockSurface(temp_surf_);

    // clearDirty();

    //SDL_UpdateTexture(screen_texture_, NULL, temp_surf_->pixels, temp_surf_->pitch);

    /*
    * Blit 8-bit palette surface onto the window surface that's
    * closer to the texture's format
    */
    SDL_BlitSurface(temp_surf_, NULL, screen_surf_, NULL);

    /*
    // Modify the texture's pixels
    void *pixels;
    int pitch;

    SDL_LockTexture(screen_texture_, NULL, &pixels, &pitch);
    SDL_ConvertPixels(screen_surf_->w, screen_surf_->h,
        screen_surf_->format->format,
        screen_surf_->pixels, screen_surf_->pitch,
        SDL_PIXELFORMAT_RGBA8888,
        pixels, pitch);
    SDL_UnlockTexture(screen_texture_);
    */

    // SDL_RenderClear(sdlRenderer);

    { 
        // Set our loaded texture as the current 2D texture (this isn't actually technically necessary since our
        // texture was never unselected from above, but this is most clear)
        glBindTexture(GL_TEXTURE_2D, screen_texture_);

        // Tell OpenGL that all subsequent drawing operations should try to use the current 2D texture
        glEnable(GL_TEXTURE_2D);

        glTexImage2D(GL_TEXTURE_2D,
                0, GL_RGBA, screen_surf_->w, screen_surf_->h,
                0, GL_RGBA, GL_UNSIGNED_BYTE, screen_surf_->pixels);
    
        glBegin(GL_QUADS);
            glTexCoord2f(0.0, 0.0); 
            glVertex2f(0, 0);

            glTexCoord2f(1, 0.0);
            glVertex2f(screen_surf_->w, 0.0);

            glTexCoord2f(1, 1);
            glVertex2f(screen_surf_->w, screen_surf_->h);

            glTexCoord2f(0, 1);
            glVertex2f(0, screen_surf_->h);
        glEnd();


        // Tell OpenGL that all subsequent drawing operations should NOT try to use the current 2D texture
        glDisable(GL_TEXTURE_2D);

        // **************************************

#if 0
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, screen_texture_);

        if(true) {     // smooth?
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        errorcheck();

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        errorcheck();

        static const float mult = 4.0 * sqrtf(2.0);
        if(on_screen_mode_) {
            // glClear( GL_COLOR_BUFFER_BIT );

            /*
            glBegin( GL_QUADS );
                glVertex2f( -0.5f, -0.5f );
                glVertex2f(  0.5f, -0.5f );
                glVertex2f(  0.5f,  0.5f );
                glVertex2f( -0.5f,  0.5f );
            glEnd();
            */

            // glCallList(fullscreen);

            GLfloat w = (float)screen_surf_->w;
            GLfloat h = (float)screen_surf_->h;

            glTexImage2D(GL_TEXTURE_2D,
                0, GL_RGBA, screen_surf_->w, screen_surf_->h,
                0, GL_RGBA, GL_UNSIGNED_BYTE, screen_surf_->pixels);


            // w = 100.0f;
            // h = 100.0f;

            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); 
                glVertex2f(0, 0);

                glTexCoord2f(1, 0.0);
                glVertex2f(w, 0.0);

                glTexCoord2f(1, 1);
                glVertex2f(w, h);

                glTexCoord2f(0, 1);
                glVertex2f(0, h);
            glEnd();

            errorcheck();
        } else {
            glPushMatrix();
            glScalef(mult, mult, mult);
            // glCallList(_frame_list_base + id);
            glPopMatrix();
        }

        glDisable(GL_TEXTURE_2D);
#endif

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return true;
}

void Screen::renderTile(Texture *texture, uint8 texture_id, int dst_x, int dst_y, int dst_z, bool smooth) {
    ////////
    GLfloat transX = (tile_side_) * dst_x;
    GLfloat transY = (tile_side_) * dst_y;
    GLfloat transZ = (-tile_side_ / 2.0) * dst_z;

    glPushMatrix();
        glTranslatef(transX, transY, transZ);
    ////////

    GLuint texture_name = texture->textureName();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    glBindTexture(GL_TEXTURE_2D, texture_name);

    if(smooth) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_S, GL_OBJECT_PLANE, planeValueS_);

    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, planeValueT_);

    glPushMatrix();
        glScalef(tile_side_, tile_side_, tile_side_);
        glDrawElements(GL_QUADS, gl_tiles[texture_id].nb_vertices_, GL_UNSIGNED_BYTE, gl_tiles[texture_id].faces_);
#if 0
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0, 0.0, 0);
        glDrawElements(GL_QUADS, gl_tiles[texture_id].nb_vertices_, GL_UNSIGNED_BYTE, gl_tiles[texture_id].faces_);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glBindTexture(GL_TEXTURE_2D, 0);

    ////////
    glPopMatrix();
    ////////
}

void Screen::renderTexture(Texture *texture, int dst_x, int dst_y, int dst_width, int dst_height,
        int src_x, int src_y, int src_width, int src_height, bool flipped) {
    GLfloat texLeft = GLfloat(src_x) / GLfloat(texture->width());   // 0.0f
    GLfloat texTop = GLfloat(src_y) / GLfloat(texture->height());   // 0.0f
    GLfloat texRight = (GLfloat(src_x) + GLfloat(src_width)) / GLfloat(texture->width());       // 1.0f
    GLfloat texBottom = (GLfloat(src_y) + GLfloat(src_height)) / GLfloat(texture->height());    // 1.0f

    if (flipped) {
        texLeft = (GLfloat(src_x) + GLfloat(src_width)) / GLfloat(texture->width());
        texRight = GLfloat(src_x) / GLfloat(texture->width());
    }

    GLuint texture_name = texture->textureName();

    glBindTexture(GL_TEXTURE_2D, texture_name);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
        glTexCoord2f(texLeft, texTop);
        glVertex3f(GLfloat(dst_x), GLfloat(dst_y), 0.0f);

        glTexCoord2f(texRight, texTop);
        glVertex3f(GLfloat(dst_x + dst_width), GLfloat(dst_y), 0.0f);

        glTexCoord2f(texRight, texBottom);
        glVertex3f(GLfloat(dst_x + dst_width), GLfloat(dst_y + dst_height), 0.0f);

        glTexCoord2f(texLeft, texBottom);
        glVertex3f(GLfloat(dst_x), GLfloat(dst_y + dst_height), 0.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    /*
    glBegin(GL_LINES);
        glColor3f(1.0f, 0.0f, 0.0f);

        glVertex2f(GLfloat(dst_x + dst_width), GLfloat(dst_y + dst_height));
        glVertex2f(GLfloat(dst_x), GLfloat(dst_y));
        glVertex2f(GLfloat(dst_x + dst_width), GLfloat(dst_y));
        glVertex2f(GLfloat(dst_x + dst_width), GLfloat(dst_y + dst_height));
        glVertex2f(GLfloat(dst_x), GLfloat(dst_y + dst_height));
        glVertex2f(GLfloat(dst_x + dst_width), GLfloat(dst_y));

        glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();
    */

    /*
    glBegin(GL_LINES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);

        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);

        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();
    */
}

void Screen::renderTexture(Texture *texture, int dst_x, int dst_y, int dst_width, int dst_height) {
    renderTexture(texture, dst_x, dst_y, dst_width, dst_height, 0, 0, texture->width(), texture->height());
}

void Screen::renderTextureB(Texture *texture, int x, int y, int width, int height, bool flipped, int stride) {
    stride = (stride == 0 ? width : stride);

    renderTexture(texture, x, y, stride, height, 0, 0, stride, height, flipped);
}

void Screen::renderTexture2x(Texture *texture,
        int x, int y, int width, int height, int stride, bool transp) {
    if (!transp) {
        printf("!transp: %i   Width: %i\n", stride, width);
    }
    stride = (stride == 0 ? width : stride);

    renderTexture(texture, x, y, stride * 2, height * 2, 0, 0, stride, height);
}
