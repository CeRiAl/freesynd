/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef SCREEN_H
#define SCREEN_H

#include <SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "common.h"

class Texture {
public:
    Texture(int width, int height, GLenum target = GL_TEXTURE_2D, bool smooth = false);
    ~Texture();

    GLuint textureName() { return texture_name_; }

    int width() { return width_; }
    int height() { return height_; }

    void update(const uint8 *data, bool flipped = false);
    void update(SDL_Surface *src_surface);
    void update(SDL_Surface *src_surface, int x, int y, int width, int height);
    void xupdate(const uint8 *data, int x, int y, int width, int height);
    void update(const uint8 *data, int x, int y, int width, int height);
    void updateRect(const uint8 *data, int x, int y, int width, int height);

    void setPalette(const uint8 *pal, int cols = 256);
    void setPalette(const SDL_Color *pal, int cols = 256);

protected:
    int width_;
    int height_;

    SDL_Surface *surface_8bpp_;
    SDL_Surface *surface_32bpp_;

    GLenum texture_target_ = GL_TEXTURE_2D;
    GLuint texture_name_;
};


class TileTexture {
public:
    static const int kTextureWidth;
    static const int kTextureHeight;

    TileTexture(uint8_t *texture, const SDL_Color *palette, bool smooth = false);
    ~TileTexture();

    GLuint textureName() { return texture_name_; }

protected:
    uint32_t *tile_texture_;

    GLuint texture_name_;
};


/*!
 * Screen class.
 */
class Screen : public Singleton<Screen> {
public:
    /*! Width of the screen in pixels.*/
    static const int kScreenWidth;
    /*! Height of the screen in pixels.*/
    static const int kScreenHeight;
    /*! Width of the left control panel*/
    static const int kScreenPanelWidth;

    explicit Screen(int width, int height);
    ~Screen();

    void initScreen(void);

    void clear(uint8 color = fs_cmn::kColorBlack);

    void setPalette(SDL_Color *pal, int cols = 256);
    void getColor(uint8 color, uint8 &r, uint8 &g, uint8 &b);

    SDL_Color *palette() { return current_palette_; }

    bool enterWorldMode(void);
    bool leaveWorldMode(void);

    bool initTileVertices(void);

    void renderStart(void);
    bool renderScreen(void);

    void renderTile(TileTexture *tile_texture, uint8 texture_id, int dst_x, int dst_y, int dst_z, bool smooth = false);

    void renderTexture(Texture *texture, int dst_x, int dst_y, int dst_width, int dst_height,
            int src_x, int src_y, int src_width, int src_height, bool flipped = false);
    void renderTexture(Texture *texture, int dst_x, int dst_y, int dst_width, int dst_height);

    void renderBitmap(Texture *texture, int x, int y, int width, int height,
            int stride = 0, bool flipped = false, bool transp = true);
    void renderBitmap2x(Texture *texture, int x, int y, int width, int height,
            int stride = 0, bool flipped = false, bool transp = true);
    void renderBitmap2x(Texture *texture, int dst_x, int dst_y, int dst_width, int dst_height,
                        int src_x, int src_y, int src_width, int src_height,
                        int stride = 0, bool flipped = false, bool transp = true);

    void drawVLine(int x, int y, int length, uint8 color);
    void drawHLine(int x, int y, int length, uint8 color);

    int numLogos() { return num_logos_; }
    void initLogos(const std::string& filename, Texture *&logos_bitmap, int logo_size);
    void drawLogo(int x, int y, int logo, int color, bool mini = false);
    void drawLine(int x1, int y1, int x2, int y2, uint8 color, int skip = 0,
            int off = 0);

    void setPixel(int x, int y, uint8 color);
    void drawRect(int x, int y, int width, int height, uint8 color = fs_cmn::kColorBlack);

    void startTextInput() {
        if (!SDL_IsTextInputActive())
            SDL_StartTextInput();
    }

    void stopTextInput() {
        if (SDL_IsTextInputActive())
            SDL_StopTextInput();
    }

    void resize(unsigned width, unsigned height) {
        width_ = width;
        height_ = height;
        change_size_ = true;
    }

    void panZoom(int delta_x, int delta_y, int delta_z) {
        if (delta_x != 0) {
            pan_x_ += (delta_x * delta_pan_x_);
            change_pan_ = true;
        }
        if (delta_y != 0) {
            pan_y_ += (delta_y * delta_pan_y_);
            change_pan_ = true;
        }
        if (delta_z != 0) {
            zoom_ += (delta_z * delta_zoom_);
            change_zoom_ = true;
        }

        printf("panZooom: %f x %f x %f\n", pan_x_, pan_y_, zoom_);
    }

    void setPanZoom(int pan_x, int pan_y, int zoom) {
        pan_x_ = pan_x;
        pan_y_ = pan_y;
        zoom_ = zoom;

        change_pan_ = true;
        change_zoom_ = true;
    }

    void perspective(bool enable) {
        perspective_ = enable;
        change_zoom_ = true;
    }

    void smooth(bool enable) {
        smooth_ = enable;
    }

    int gameScreenHeight();
    int gameScreenWidth();
    int gameScreenLeftMargin();

    void setRenderSize(int width, int height) {
        render_width_ = width;
        render_height_ = height;

        if (width > 0 && height > 0) {
            int x = (gameScreenWidth() - width) / 2;
            int y = (gameScreenHeight() - height) / 2;
            if (x >= 0 && y >= 0) {
                render_offset_ = { x, y };
            }
        } else {
            render_offset_ = { 0, 0 };
        }
    }

    Point2D renderOffset() { return render_offset_; }

protected:
    int width_;
    int height_;

    SDL_Color current_palette_[256];

    Point2D render_offset_;
    int num_logos_;
    Texture *logos_bitmap_;
    Texture *logos_mini_bitmap_;

    Screen();

private:
    int render_width_;
    int render_height_;
    float distance_;
    float zenith_;
    float azimuth_;
    float pan_x_;
    float pan_y_;
    float zoom_;

    bool perspective_;
    bool smooth_;

    bool change_size_;
    bool change_zoom_;
    bool change_pan_;

    bool world_mode_;

    static constexpr float tile_side_ = 256.0;
    static constexpr float delta_pan_x_ = 256.0;
    static constexpr float delta_pan_y_ = 256.0;
    static constexpr float delta_zoom_ = 0.2;

    static const GLfloat planeValueS_[];
    static const GLfloat planeValueT_[];
};

#define g_Screen    Screen::singleton()

#endif
