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
                GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)NULL);
    glTexParameterf(texture_target_, GL_TEXTURE_MIN_FILTER, texParam);
    glTexParameterf(texture_target_, GL_TEXTURE_MAG_FILTER, texParam);

    surface_32bpp_ = SDL_CreateRGBSurface(SDL_SWSURFACE,
        width_, height_,
        32, 0, 0, 0, 0);

    surface_8bpp_ = SDL_CreateRGBSurface(SDL_SWSURFACE,
        width_, height_,
        8, 0, 0, 0, 0);
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

void Texture::update(const uint8 * data)
{
    SDL_LockSurface(surface_8bpp_);
    memcpy(surface_8bpp_->pixels, data, width_ * height_);
    SDL_UnlockSurface(surface_8bpp_);

    SDL_BlitSurface(surface_8bpp_, NULL, surface_32bpp_, NULL);


    glBindTexture(texture_target_, texture_name_);
    glTexSubImage2D(texture_target_,
                    0, 0, 0, width_, height_,
                    GL_BGRA, GL_UNSIGNED_BYTE, surface_32bpp_->pixels);
    glBindTexture(texture_target_, 0);
}

void Texture::setPalette(const uint8 * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        palette[i].r = pal[i * 3 + 0];
        palette[i].g = pal[i * 3 + 1];
        palette[i].b = pal[i * 3 + 2];
    }

    SDL_SetPaletteColors(surface_8bpp_->format->palette, palette, 0, cols);
}


const int Screen::kScreenWidth = 640;
const int Screen::kScreenHeight = 400;
const int Screen::kScreenPanelWidth = 129;

Screen::Screen(int width, int height)
:width_(width)
, height_(height)
, pixels_(NULL)
, dirty_(false)
, gl_dirty_(false)
, data_logo_(NULL), data_logo_copy_(NULL)
, data_mini_logo_(NULL), data_mini_logo_copy_(NULL)
{

    screen_surf_ = NULL;
    tscreen_surf_ = NULL;
    screen_texture_ = 0;
    tscreen_texture_ = 0;
    temp_surf_ = NULL;
    ttemp_surf_ = NULL;

    assert(width_ > 0);
    assert(height_ > 0);

    pixels_ = new uint8[width_ * height_];
}

Screen::~Screen()
{
    delete[] pixels_;
    if (data_logo_)
        delete[] data_logo_;
    if (data_logo_copy_)
        delete[] data_logo_copy_;
    if (data_mini_logo_)
        delete[] data_mini_logo_;
    if (data_mini_logo_copy_)
        delete[] data_mini_logo_copy_;

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
    memset(pixels_, color, width_ * height_);
    dirty_ = true;
}
/*!
 * Blits data to screen
 * @param x position by x coord
 * @param y position by y coord
 * @param width data's width
 * @param height data's height
 * @param pixeldata pointer to data to be blitted
 * @param flipped draw flipped
 * @param stride actual data width (Sprite class related)
 * @sa sprite.h
 */
void Screen::blit(int x, int y, int width, int height,
                  const uint8 * pixeldata, bool flipped, int stride)
{
    if (x + width < 0 || y + height < 0 || x >= width_ || y >= height_)
        return;

    int clipped_x = x < 0 ? 0 : x;
    int clipped_y = y < 0 ? 0 : y;

    int sx = x < 0 ? -x : 0;
    int sy = y < 0 ? -y : 0;

    int w = x < 0 ? x + width : x + width > width_ ? width_ - x : width;
    int h = y < 0
        ? y + height : y + height > height_ ? height_ - y : height;

    stride = (stride == 0 ? width : stride);
    int ofs = (flipped ? w - 1 : 0) + clipped_x;
    uint8 *d = pixels_ + clipped_y * width_ + ofs;

    if (flipped) {
        const uint8 *s = pixeldata + sy * stride + sx + (width - w);
        for (int j = 0; j < h; ++j) {

            const uint8 *cp_s = s;
            s += stride;
            uint8 *cp_d = d;
            d += width_;
            for (int i = 0; i < w; ++i) {
                uint8 c = *cp_s++;

                if (c != 255)
                    *cp_d = c;
                cp_d--;
            }
        }
    } else {
        const uint8 *s = pixeldata + sy * stride + sx;
        for (int j = 0; j < h; ++j) {

            const uint8 *cp_s = s;
            s += stride;
            uint8 *cp_d = d;
            d += width_;
            for (int i = 0; i < w; ++i) {
                uint8 c = *cp_s++;

                if (c != 255)
                    *cp_d = c;
                cp_d++;
            }
        }
    }

    dirty_ = true;
}

/*!
 * Blits a portion of the source data to the screen a given position.
 */
void Screen::blitRect(int x, int y, int width, int height,
                  const uint8 * pixeldata, bool flipped, int stride)
{
    if (x + width < 0 || y + height < 0 || x >= width_ || y >= height_)
        return;

    int dest_x = x < 0 ? 0 : x;
    int dest_y = y < 0 ? 0 : y;

    int clipped_w = x < 0 ? x + width : x + width > width_ ? width_ - x : width;
    int clipped_h = y < 0
        ? y + height : y + height > height_ ? height_ - y : height;

    stride = (stride == 0 ? width : stride);
    int ofs = (flipped ? clipped_w - 1 : 0) + dest_x;
    uint8 *d = pixels_ + dest_y * width_ + ofs;

    if (flipped) {
        const uint8 *s = pixeldata + y * stride + x + (width - clipped_w);
        for (int j = 0; j < clipped_h; ++j) {

            const uint8 *cp_s = s;
            s += stride;
            uint8 *cp_d = d;
            d += width_;
            for (int i = 0; i < clipped_w; ++i) {
                uint8 c = *cp_s++;

                if (c != 255)
                    *cp_d = c;
                cp_d--;
            }
        }
    } else {
        const uint8 *s = pixeldata + y * stride + x;
        for (int j = 0; j < clipped_h; ++j) {

            const uint8 *cp_s = s;
            s += stride;
            uint8 *cp_d = d;
            d += width_;
            for (int i = 0; i < clipped_w; ++i) {
                uint8 c = *cp_s++;

                if (c != 255)
                    *cp_d = c;
                cp_d++;
            }
        }
    }

    dirty_ = true;
}

void Screen::scale2x(int x, int y, int width, int height,
                     const uint8 * pixeldata, int stride, bool transp)
{
    stride = (stride == 0 ? width : stride);

    for (int j = 0; j < height; ++j) {
        uint8 *d = pixels_ + (y + j * 2) * width_ + x;

        for (int i = 0; i < width; ++i, d += 2) {
            uint8 c = *(pixeldata + i);
            if (c != 255 || !transp) {
                *(d + 0) = c;
                *(d + 1) = c;
                *(d + 0 + width_) = c;
                *(d + 1 + width_) = c;
            }
        }

        pixeldata += stride;
    }

    dirty_ = true;
}

void Screen::drawVLine(int x, int y, int length, uint8 color)
{
    if (x < 0 || x >= width_ || y + length < 0 || y >= height_)
        return;

    length = y + length >= height_ ? height_ - y : length;
    if (y < 0) {
        length += y;
        y = 0;
    }
    if (length < 1)
        return;

    uint8 *pixel = pixels_ + y * width_ + x;
    while (length--) {
        *pixel = color;
        pixel += width_;
    }

    glBegin(GL_LINES);
        glPointSize(3.0);  
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(50.0f, 50.0f);

        glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();

    glFlush();

    dirty_ = true;
}

void Screen::drawHLine(int x, int y, int length, uint8 color)
{
    if ((x + length) < 0 || x >= width_ || y < 0 || y >= height_)
        return;

    length = x + length >= width_ ? width_ - x : length;
    if (x < 0) {
        length += x;
        x = 0;
    }
    if (length < 1)
        return;

    uint8 *pixel_ptr = pixels_ + y * width_ + x;
    while (length--)
        *pixel_ptr++ = color;

    dirty_ = true;
}

int Screen::numLogos()
{
    return size_logo_ / (32 * 32);
}

void Screen::drawLogo(int x, int y, int logo, int colour, bool mini)
{
    if (data_logo_ == NULL) {
        data_logo_ = File::loadOriginalFile("mlogos.dat", size_logo_);
        data_logo_copy_ = new uint8[size_logo_];
    }
    if (data_mini_logo_ == NULL) {
        data_mini_logo_ = File::loadOriginalFile("mminlogo.dat", size_mini_logo_);
        data_mini_logo_copy_ = new uint8[size_mini_logo_];
    }

    for (int i = 0; i < size_logo_; i++)
        if (data_logo_[i] == 0xFE)
            data_logo_copy_[i] = colour;
        else
            data_logo_copy_[i] = data_logo_[i];
    for (int i = 0; i < size_mini_logo_; i++)
        if (data_mini_logo_[i] == 0xFE)
            data_mini_logo_copy_[i] = colour;
        else
            data_mini_logo_copy_[i] = data_mini_logo_[i];
    if (mini)
        scale2x(x, y, 16, 16, data_mini_logo_copy_ + logo * 16 * 16, 16);
    else
        scale2x(x, y, 32, 32, data_logo_copy_ + logo * 32 * 32, 32);

    dirty_ = true;
}

// Taken from SDL_gfx
#define ABS(a) (((a)<0) ? -(a) : (a))
void Screen::drawLine(int x1, int y1, int x2, int y2, uint8 color,
                      int skip, int off)
{
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

    dirty_ = true;
}

void Screen::setPixel(int x, int y, uint8 color)
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_)
        return;
    pixels_[y * width_ + x] = color;
    dirty_ = true;
}


void Screen::drawRect(int x, int y, int width, int height, uint8 color)
{
    if (x < 0 || y < 0 || (x + width) > width_
        || (y + height) > height_ || width <= 0 || height <= 0)
        return;
    // NOTE: we don't handle properly clipping by (x,y), do we need it?


    for (int i = 0; i < height; i++) {
        uint8 *p_pixels = pixels_ + x + width_ * (y + i);
        for (int w = 0; w != width; w++)
            *p_pixels++ = color;
    }
    dirty_ = true;
}

int Screen::gameScreenHeight()
{
    return GAME_SCREEN_HEIGHT;
}

int Screen::gameScreenWidth()
{
    return GAME_SCREEN_WIDTH;
}

int Screen::gameScreenLeftMargin()
{
    return 129;
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

void Screen::initScreen(void) {
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
    // glEnable(GL_ALPHA_TEST);
    // glAlphaFunc(GL_GREATER, 0.5);
    glViewport(0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);



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

bool Screen::renderScreen(void)
{
    SDL_LockSurface(temp_surf_);

    memcpy(temp_surf_->pixels, pixels_,
            GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT);

    SDL_UnlockSurface(temp_surf_);

    clearDirty();

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

    enterOnScreenMode();

    {
        // glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 
        // Set our loaded texture as the current 2D texture (this isn't actually technically necessary since our
        // texture was never unselected from above, but this is most clear)
        glBindTexture(GL_TEXTURE_2D, screen_texture_);

        // Tell OpenGL that all subsequent drawing operations should try to use the current 2D texture
        glEnable(GL_TEXTURE_2D);

        glTexImage2D(GL_TEXTURE_2D,
                0, GL_RGBA, screen_surf_->w, screen_surf_->h,
                0, GL_BGRA, GL_UNSIGNED_BYTE, screen_surf_->pixels);
    
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

    leaveOnScreenMode();

    return true;
}

void Screen::renderTexture(Texture *texture, int x, int y, int width, int height)
{
    GLuint texture_name = texture->textureName();

    enterOnScreenMode();
    {
        glBindTexture(GL_TEXTURE_2D, texture_name);
        glEnable(GL_TEXTURE_2D);
    
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); 
            glVertex2f(0.0f, 0.0f);

            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(width, 0.0f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(width, height);

            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(0.0f, height);
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    leaveOnScreenMode();

    gl_dirty_ = true;
}
