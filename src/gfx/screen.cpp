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

void Texture::updateRect(const uint8 *data, int x, int y, int width, int height) {
    SDL_LockSurface(surface_8bpp_);
    for (int row = 0; row < height; row++) {

        int target_offset = ((y + row) * width_) + x;
        int source_offset = row * width;

        uint8 *target_pixels = (uint8 *)surface_8bpp_->pixels + target_offset;

        uint8 *source_pixels = (uint8 *)data + source_offset;


        memcpy(target_pixels, source_pixels, width);
        // memset(target_pixels, 0x14, width);

        /*
        Uint32 *target_pixel = (Uint8 *) surface->pixels + y * surface->pitch +
            x * sizeof *target_pixel;
        */
    }
    SDL_UnlockSurface(surface_8bpp_);

    update(surface_8bpp_, x, y, width, height);
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

///

/* SDL interprets each pixel as a 32-bit number, so our masks must depend
    on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;
#else
    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
#endif

///

// only rgba
void textureTreatment(uint8_t *texture, int width, int height) {
    int kern_off[9] = {
        - width - 1, - width, - width + 1,
        -1,          0,       1,
        width - 1,   width,   width + 1
    };
    int kern_weight[9] = {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1
    };

    for (unsigned k = 0; k < height * width; k++) {
        if (texture[3] == 0) {
            unsigned n = 0;
            unsigned r = 0;
            unsigned g = 0;
            unsigned b = 0;

            for (unsigned l = 0; l < sizeof(kern_off) / sizeof(*kern_off); l++) {
                if ((0 <= (k + kern_off[l])) && 
                        ((k + kern_off[l]) < (width * height)) && 
                        texture[kern_off[l] * 4 + 3] == 255) {
                    n ++;
                    r += kern_weight[l] * texture[kern_off[l] * 4 + 0]; 
                    g += kern_weight[l] * texture[kern_off[l] * 4 + 1]; 
                    b += kern_weight[l] * texture[kern_off[l] * 4 + 2]; 
                }
            }
            if (n != 0) {
                texture[0] = r / n;
                texture[1] = g / n;
                texture[2] = b / n;
            }
        }
        texture += 4;
    }
}

///

const int TileTexture::kTextureWidth = 64;
const int TileTexture::kTextureHeight = 64;

TileTexture::TileTexture(uint8_t *tile_data, const SDL_Color *palette, bool smooth) {
    // Create tile texture and fill with transparent black
    tile_texture_ = new uint32_t[kTextureWidth * kTextureHeight];
    memset(tile_texture_, 0, kTextureWidth * kTextureHeight * sizeof(*tile_texture_));

    // Create temporary 8bit and 32bit surfaces and fill with transparent black
    SDL_Surface *surface_8bpp = SDL_CreateRGBSurface(SDL_SWSURFACE,
        kTextureWidth, kTextureHeight, 8,
        0, 0, 0, 0);
    SDL_SetPaletteColors(surface_8bpp->format->palette, palette, 0, 256);
    SDL_FillRect(surface_8bpp, NULL, SDL_MapRGB(surface_8bpp->format, 0, 0, 0));

    SDL_Surface *surface_32bpp = SDL_CreateRGBSurface(SDL_SWSURFACE,
        kTextureWidth, kTextureHeight, 32,
        rmask, gmask, bmask, amask);
    SDL_FillRect(surface_32bpp, NULL, SDL_MapRGBA(surface_32bpp->format, 0, 0, 0, 0));

    // Copy tile_data to surface_8bpp
    SDL_LockSurface(surface_8bpp);
    {
        // memcpy(surface_8bpp->pixels, tile_data, kTextureWidth * kTextureHeight);
        /*
        for (int row = 0; row < kTextureHeight; row++) {
            uint8 *src_pixels = (uint8 *)tile_data + (kTextureHeight - row - 1) * kTextureWidth;
            uint8 *dst_pixels = (uint8 *)surface_8bpp->pixels + (row * kTextureWidth);
            memcpy(dst_pixels, src_pixels, kTextureWidth);
        }
        */
        for (int row = 0; row < 48; row++) {
            uint8 *src_pixels = (uint8 *)tile_data + (48 - row - 1) * kTextureWidth;
            uint8 *dst_pixels = (uint8 *)surface_8bpp->pixels + (row * kTextureWidth);
            memcpy(dst_pixels, src_pixels, kTextureWidth);
        }
    }
    SDL_UnlockSurface(surface_8bpp);

    // Blit surface_8bpp to surface_32bpp (applies palette!)
    SDL_BlitSurface(surface_8bpp, NULL, surface_32bpp, NULL);

    // Finally copy surface_32bpp to tile_texture_
    SDL_LockSurface(surface_32bpp);
    {
        memcpy(tile_texture_, surface_32bpp->pixels, kTextureWidth * kTextureHeight * sizeof(*tile_texture_));
    }
    SDL_UnlockSurface(surface_32bpp);

    // Now free the temporary surfaces
    SDL_FreeSurface(surface_32bpp);
    SDL_FreeSurface(surface_8bpp);

    // /////////////

    // textureTreatment((uint8_t*)tile_texture_, kTextureWidth, kTextureHeight);

    glGenTextures(1, &texture_name_);
    glBindTexture(GL_TEXTURE_2D, texture_name_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (smooth) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    gluBuild2DMipmaps(GL_TEXTURE_2D,
        GL_RGBA, kTextureWidth, kTextureHeight,
        GL_RGBA, GL_UNSIGNED_BYTE, tile_texture_);
}

TileTexture::~TileTexture() {
    if (tile_texture_)
        delete[] tile_texture_;

    if (texture_name_)
        glDeleteTextures(1, &texture_name_);
}

// /////////////

const int Screen::kScreenWidth = 1024; // 800; // 640;
const int Screen::kScreenHeight = 768; // 600; // 400;
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

Screen::Screen(int width, int height) :
  width_(width),
  height_(height),
  render_offset_({ 0, 0 }),
  num_logos_(0),
  logos_bitmap_(NULL),
  logos_mini_bitmap_(NULL),
  render_width_(kScreenWidth),
  render_height_(kScreenHeight),
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
  world_mode_(true)
{
    assert(width_ > 0);
    assert(height_ > 0);
}

Screen::~Screen() {
    if (logos_bitmap_)
        delete logos_bitmap_;

    if (logos_mini_bitmap_)
        delete logos_mini_bitmap_;
}

void Screen::clear(uint8 color) {
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

void Screen::initLogos(const std::string& filename, Texture *&logos_bitmap, int logo_size) {
    int data_size = 0;
    uint8 *data_logo = File::loadOriginalFile(filename, data_size);

    int num_logos = data_size / (logo_size * logo_size);
    if (num_logos_ == 0) {
        num_logos_ = num_logos;
    }
    assert(num_logos_ == num_logos);

    for (int i = 0; i < data_size; i++)
        if (data_logo[i] == 0xFE)
            data_logo[i] = 0xFF;

    logos_bitmap = new Texture(logo_size * 8, logo_size * 8);
    logos_bitmap->setPalette(current_palette_);
    
    for (int cc = 0; cc < num_logos; cc++) {
        int posX = (cc % 8) * logo_size;
        int posY = (cc / 8) * logo_size;
        // printf("Pos: %i x %i\n", posX, posY);
        logos_bitmap->update(data_logo + cc * logo_size * logo_size, posX, posY, logo_size, logo_size);
    }

    delete[] data_logo;
}

void Screen::drawLogo(int x, int y, int logo, int color, bool mini) {
    int logo_size = 32;
    int mini_logo_size = 16;

    if (logos_bitmap_ == NULL) {
        initLogos("mlogos.dat", logos_bitmap_, logo_size);
    }

    if (logos_mini_bitmap_ == NULL) {
        initLogos("mminlogo.dat", logos_mini_bitmap_, mini_logo_size);
    }

    if (mini) {
        drawRect(x, y, mini_logo_size * 2, mini_logo_size * 2, color);
        int posX = (logo % 8) * mini_logo_size;
        int posY = (logo / 8) * mini_logo_size;
        renderBitmap2x(logos_mini_bitmap_, x, y, mini_logo_size, mini_logo_size,
            posX, posY, mini_logo_size, mini_logo_size);
    } else {
        drawRect(x, y, logo_size * 2, logo_size * 2, color);
        int posX = (logo % 8) * logo_size;
        int posY = (logo / 8) * logo_size;
        renderBitmap2x(logos_bitmap_, x, y, logo_size, logo_size,
            posX, posY, logo_size, logo_size);
    }
}

void Screen::drawLine(int x1, int y1, int x2, int y2, uint8 color,
                      int skip, int off) {
    x1 += render_offset_.x;
    y1 += render_offset_.y;
    x2 += render_offset_.x;
    y2 += render_offset_.y;

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
    x += render_offset_.x;
    y += render_offset_.y;

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

bool Screen::enterWorldMode(void) {
    if (world_mode_) return false;

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();

    world_mode_ = true;

    return true;
}

bool Screen::leaveWorldMode(void) {
    if (!world_mode_) return false;

    glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT) ; 

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0.0, gameScreenWidth(), gameScreenHeight(), 0.0, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    world_mode_ = false;

    return true;
}

bool Screen::initTileVertices(void) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer (3, GL_FLOAT, 0, tiles_vertices);

    return true;
}

void Screen::initScreen(void) {
    glClearColor(0.8, 0.8, 0.8, 1.0);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f);
    // glAlphaFunc(GL_GREATER, 0.0f);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, gameScreenWidth(), gameScreenHeight());

    // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void Screen::renderStart(void) {
    enterWorldMode();

    world_mode_ = true;

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

bool Screen::renderScreen(void) {
    // clearDirty();

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

    return true;
}

void Screen::renderTile(TileTexture *tile_texture, uint8 texture_id, int dst_x, int dst_y, int dst_z, bool smooth) {
    ////////
    GLfloat transX = (tile_side_) * dst_x;
    GLfloat transY = (tile_side_) * dst_y;
    GLfloat transZ = (-tile_side_ / 2.0) * dst_z;

    glPushMatrix();
        glTranslatef(transX, transY, transZ);
    ////////

    GLuint texture_name = tile_texture->textureName();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    glBindTexture(GL_TEXTURE_2D, texture_name);

    if (smooth) {
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
#if 1
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
    dst_x += render_offset_.x;
    dst_y += render_offset_.y;

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

void Screen::renderBitmap(Texture *texture,
                          int x, int y, int width, int height,
                          int stride, bool flipped, bool transp) {
    if (!transp) {
        printf("!transp: %i   Width: %i\n", stride, width);
    }
    stride = (stride == 0 ? width : stride);

    renderTexture(texture, x, y, stride, height, 0, 0, stride, height, flipped);
}

void Screen::renderBitmap2x(Texture *texture,
                            int x, int y, int width, int height,
                            int stride, bool flipped, bool transp) {
    stride = (stride == 0 ? width : stride);

    // renderBitmap(texture, x, y, stride * 2, height * 2, stride, flipped, transp);
    renderTexture(texture, x, y, stride * 2, height * 2, 0, 0, stride, height, flipped);
}

void Screen::renderBitmap2x(Texture *texture, int dst_x, int dst_y, int dst_width, int dst_height,
                            int src_x, int src_y, int src_width, int src_height,
                            int stride, bool flipped, bool transp) {
    stride = (stride == 0 ? dst_width : stride);

    // renderBitmap(texture, dst_x, dst_y, stride * 2, height * 2, stride, flipped, transp);
    renderTexture(texture, dst_x, dst_y, stride * 2, dst_height * 2,
                  src_x, src_y, src_width, src_height, flipped);
}
