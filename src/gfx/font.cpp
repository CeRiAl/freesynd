/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
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

#include "utils/file.h"
#include "font.h"
#include "screen.h"
#include "cp437.h"

#include <stdlib.h>

FontRange::FontRange()
{
    memset(char_present_, 0, sizeof(char_present_));
}

FontRange::FontRange(const std::string& valid_chars)
{
    // turn description valid_chars into bitfield char_present_
    memset(char_present_, 0, sizeof(char_present_));
    size_t pos = 0;
    while (pos < valid_chars.size()) {
        // skip commas if we are pointing to any.
        while (pos < valid_chars.size() && valid_chars[pos] == ',') pos++;

        size_t it = valid_chars.find(',', pos);
        size_t next_pos;
        if (it == std::string::npos) {
            // there is no next section.
            next_pos = valid_chars.size();
        } else {
            // there is a next section, and we'll point to
            // the comma on the start of the next loop.
            next_pos = it;
        }

        // each section can be a single number, or a range of numbers.
        // numbers can be in decimal or hex with the prefix 0x.
        std::string section(valid_chars, pos, next_pos - pos);

        it = section.find('-');
        if (it == std::string::npos) {
            // single value.
            char *endptr = NULL;
            unsigned char value = (uint8)strtol(section.c_str(), &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                fprintf(stderr, "Invalid font range specified: %s\n", valid_chars.c_str());
                return;
            }
            char_present_[value / 32] |= 1 << (value % 32);
        } else {
            // range of values.
            char *endptr = NULL;
            unsigned char start = (uint8)strtol(section.c_str(), &endptr, 0);
            if (endptr == NULL || *endptr != '-') {
                fprintf(stderr, "Invalid font range specified: %s\n", valid_chars.c_str());
                return;
            }
            unsigned char end = (uint8)strtol(section.c_str() + it + 1, &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                fprintf(stderr, "Invalid font range specified: %s\n", valid_chars.c_str());
                return;
            }
            unsigned char value;
            for (value = start; value <= end; value++) {
                char_present_[value / 32] |= 1 << (value % 32);
            }
        }

        pos = next_pos;
    }
}

unsigned char Font::decode(const unsigned char * &c, bool dos)
{
    if (dos) {
        if (*c == '\0') return 0;
        unsigned char value = *c;
        c++;
        return value;
    }

    int wc = decodeUTF8(c);
    if (wc > (int)sizeof(cp437)) return 0xff; // out-of-range
    if (wc == 0) return 0;
    unsigned char value = cp437[wc];
    if (value == 0) return 0xff;
    return value;
}

// a negative return value is invalid.
// note that the character pointer is passed by reference and advanced
// as bytes from the input stream are consumed.
int Font::decodeUTF8(const unsigned char * &c)
{
    if (*c == '\0') return 0;

    // count the number of contiguous high bits in x.
    unsigned char tmp = *c;
    int count = 0;
    while (tmp & 0x80) {
        tmp <<= 1;
        count++;
    }

    if (count == 0) {
        // this byte is the entire character.
        unsigned int wc = *c;
        c++;
        return wc;
    } else if (count < 2 || count > 6) {
        // this byte is not a valid start for a utf8 sequence.
        c++;
        return -1;
    } else {
        // compute how many of the low bits to keep.
        int bits = 8 - count - 1;
        // start the wide character with these bits.
        unsigned int wc = *c & ((1 << bits) - 1);
        c++;

        // consume the rest of the bytes and add them to this character.
        count--;
        while (count) {
            if (*c == 0) {
                // the input stream ends mid-character.
                return -1;
            }
            if ((*c & 0xc0) != 0x80) {
                // this byte is not a valid continuation of a utf8 sequence.
                return -1;
            }
            wc <<= 6;
            wc |= (*c) & 0x3f;
            c++;
            count--;
        }

        return wc;
    }
}

Sprite *Font::getSprite(unsigned char dos_char) {
    if (range_.in_range(dos_char) == false) {
        // use '?' as default character.
        if (range_.in_range('?') == true) {
            return sprites_->sprite('?' + offset_);
        } else {
            // NULL causes the missing glyph to be skipped.
            // no space will be consumed on-screen.
            return NULL;
        }
    }
    return sprites_->sprite(dos_char + offset_);
}

#if 0
void Font::initTextureSize(unsigned &texture_width, unsigned &texture_height) {
    int char_count = 256;
    int char_width = 0;
    int char_height = textHeight(false);

    for (int cc = 0; cc < char_count; cc++) {
        if (range_.in_range(cc) == true) {            
            int curr_width = textWidth((char *)&cc, false, false);
            if (char_width < curr_width) char_width = curr_width;
        }
    }

    // Increment char width and height by one (safe margin for possible texture filtering)
    char_width++;
    char_height++;

    char_width_ = char_width;
    char_height_ = char_height;

    int char_cols = 32;
    int char_rows = char_count / char_cols + 1;

    // FIXME: check against GL_MAX_TEXTURE_SIZE;
    while(texture_width < char_cols * char_width_) texture_width <<= 1;
    while(texture_height < char_rows * char_height_) texture_height <<= 1;

    // Recalculate actual number of cols and rows
    char_cols = texture_width / char_width_;
    char_rows = char_count / char_cols + 1;

    char_cols_ = char_cols;
    char_rows_ = char_rows;
}

void Font::initChar(Texture &font_texture, uint8 *data, unsigned char cc, int width, int height) {
    int x = (cc % char_cols_) * char_width_;
    int y = (cc / char_cols_) * char_height_;

    printf("  XXX '%c': %i / %i  %ix%i\n", cc, x, y, width, height);

    /*
    int pos = 0;
    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            // %#08x
            printf("%02x ", data[pos]);
            pos++;
        }
        printf("\n");
    }
    */

    font_texture.update(data, x, y, width, height);
}

void Font::generateTexture() {
    int char_count = 256;

    unsigned texture_width = 1;
    unsigned texture_height = 1;

    initTextureSize(texture_width, texture_height);

    printf("XXX CHR: %ix%i\n", char_width_, char_height_);
    printf("XXX TEX: %ix%i\n", texture_width, texture_height);
    printf("XXX FIN: %i / %i\n", char_rows_, char_cols_);

    font_texture_ = new Texture(texture_width, texture_height);

    // font_texture_->setPalette(palette);
    font_texture_->setPalette(g_Screen.palette());

    uint32_t texture[texture_width * texture_height];
    memset(texture, 0, texture_width * texture_height * sizeof(*texture));

    for (int cc = 0; cc < char_count; cc++) {
        if (range_.in_range(cc) == true) {
            Sprite *s = getSprite(cc);
            if (s) {
                int width = s->width();
                int height = s->height();

                uint8 *data = new uint8[width * height];
                s->data(data);

                initChar(*font_texture_, data, cc, width, height);
            }
        }
    }

    /*
    for(unsigned id = 0; id < char_count; id++) {

        // _data->_req->get(id, pixels, texture_width * sizeof(*texture), _colors);
    }
    */    
}
#endif

void Font::setSpriteManager(SpriteManager *sprites, int offset, char base, const FontRange& range) {
    sprites_ = sprites;
    offset_ = offset - base;
    range_ = range;
}

void Font::setSpriteManager(SpriteManager *sprites, int offset, char base, const std::string& valid_chars) {
    setSpriteManager(sprites, offset, base, FontRange(valid_chars));
}

void Font::drawText(int x, int y, const char *text, bool dos, bool x2) {
    int sc = x2 ? 2 : 1;
    int ox = x;
    const unsigned char *c = (const unsigned char *)text;
    for (unsigned char cc = decode(c, dos); cc; cc = decode(c, dos)) {
        if (cc == 0xff) {
            // invalid utf8 code, skip it.
            continue;
        }
        if (cc == ' ') {
            x += getSprite('A')->width() * sc - sc;
            continue;
        }
        if (cc == '\n') {
            x = ox;
            y += textHeight() - sc;
            continue;
        }
        Sprite *s = getSprite(cc);
        if (s) {
            int y_offset = 0;
            if (cc == ':')
                y_offset = sc;
            else if (cc == '.' || cc == ',')
                y_offset = 4 * sc;
            else if (cc == '-')
                y_offset = 2 * sc;

            s->draw(x, y + y_offset, 0, false, x2);
            
            x += s->width() * sc - sc;
        }
    }
}

int Font::textWidth(const char *text, bool dos, bool x2) {
    int sc = x2 ? 2 : 1;
    int x = 0;
    const unsigned char *c = (const unsigned char *)text;
    for (unsigned char cc = decode(c, dos); cc; cc = decode(c, dos)) {
        if (cc == 0xff) {
            // invalid utf8 code, skip it.
            continue;
        }
        if (cc == ' ') {
            x += getSprite('A')->width() * sc - sc;
            continue;
        }
        Sprite *s = getSprite(cc);
        if (s) {
            x += s->width() * sc - sc;
        }
    }
    return x;
}

int Font::textHeight(bool x2) {
    int sc = x2 ? 2 : 1;
    return getSprite('A')->height() * sc;
}

// returns true if given code point is printable with the font
bool Font::isPrintable(uint16 unicode) {
    return unicode != 0;
}

MenuFont::MenuFont() : Font() {
}

Sprite *MenuFont::getSprite(unsigned char dos_char, bool highlighted) {
    if (range_.in_range(dos_char) == false) {
        // use '?' as default character.
        if (range_.in_range('?') == true) {
            return sprites_->sprite('?' + (highlighted ? lightOffset_ : offset_));
        } else {
            // NULL causes the missing glyph to be skipped.
            // no space will be consumed on-screen.
            return NULL;
        }
    }
    return sprites_->sprite(dos_char + (highlighted ? lightOffset_ : offset_));
}

void MenuFont::setSpriteManager(SpriteManager *sprites, int darkOffset, int lightOffset, char base,
            const std::string& valid_chars) {
    sprites_ = sprites;
    offset_ = darkOffset - base;
    lightOffset_ = lightOffset - base;
    range_ = FontRange(valid_chars);
}

void MenuFont::drawText(int x, int y, bool dos, const char *text, bool highlighted, bool x2) {
    int sc = x2 ? 2 : 1;
    int ox = x;
    const unsigned char *c = (const unsigned char *)text;
    Sprite *pDef = getSprite('A', false);
    for (unsigned char cc = decode(c, dos); cc; cc = decode(c, dos)) {
        if (cc == 0xff) {
            // invalid utf8 code, skip it.
            continue;
        }
        if (cc == ' ') {
            x += pDef->width() * sc - sc;
            continue;
        }
        if (cc == '\n') {
            x = ox;
            y += textHeight() - sc;
            continue;
        }
        Sprite *s = getSprite(cc, highlighted);
        if (s) {
            int y_offset = 0;
            if (cc == ':')
                y_offset = sc;
            else if (cc == '.' || cc == ',' || cc == '-' || cc == '_')
                y_offset = pDef->height() *sc - getSprite(cc, false)->height() * sc;
            else if (cc == '/') {
                y_offset = (pDef->height() *sc)/2 - (getSprite('/', false)->height() * sc) / 2;
            }

            s->draw(x, y + y_offset, 0, false, x2);
            x += s->width() * sc - sc;
        }
    }
}

GameFont::GameFont() : Font() {}

Sprite *GameFont::getSprite(unsigned char dos_char, uint8 color) {
    Sprite *color_sprites = NULL;

    for (int col_idx = 0; col_idx < sizeof(font_colors_); col_idx++) {
        if (font_colors_[col_idx] == color) {
            color_sprites = colored_sprites_[col_idx];
            break;
        }
    }

    if (color_sprites == NULL) {
        return NULL;
    }

    if (range_.in_range(dos_char) == false) {
        // use '?' as default character.
        if (range_.in_range('?') == true) {
            return &color_sprites[(int)'?'];
        } else {
            // NULL causes the missing glyph to be skipped.
            // no space will be consumed on-screen.
            return NULL;
        }
    }

    return &color_sprites[dos_char];
}

void GameFont::setSpriteManager(SpriteManager *sprites, int offset, char base, const std::string& valid_chars) {
    sprites_ = sprites;
    offset_ = offset - base;
    range_ = FontRange(valid_chars);

    int char_count = 256;
    uint8 from_color = fs_cmn::kColorUnknown;

    for (int color = 0; color < sizeof(font_colors_); color++) {
        colored_sprites_[color] = new Sprite[char_count];;
        assert(colored_sprites_[color]);
    }

    for (int cc = 0; cc < char_count; cc++) {
        if (range_.in_range(cc)) {
            Sprite *s = getSprite(cc);
            if (s) {
                int data_size = s->width() * s->height();
                uint8 *data = new uint8[data_size];

                for (int color = 0; color < sizeof(font_colors_); color++) {
                    s->data(data);

                    // Change original color to the specified color
                    for (int i = 0; i < data_size; i++)
                        data[i] = (data[i] == from_color ? font_colors_[color] : 255);

                    Sprite *color_s = &colored_sprites_[color][cc];

                    color_s->loadSprite(s);
                    color_s->update(data);
                }

                delete[] data;
            }
        }
    }
}

/*!
 * Draw text at the given position. Text will have the specified color.
 * \param x X location
 * \param y Y location
 * \param text The text to draw. It must be in UTF-8.
 * \param color The color used to draw the text.
 */
void GameFont::drawText(int x, int y, const char *text, uint8 color) {
    assert(color == fs_cmn::kColorBlack ||
           color == fs_cmn::kColorYellow ||
           color == fs_cmn::kColorLightRed);

    int sc = 1;
    int ox = x;
    const unsigned char *c = (const unsigned char *)text;
    Sprite *pDef = getSprite('A');
    for (unsigned char cc = decode(c, false); cc; cc = decode(c, false)) {
        if (cc == 0xff) {
            // invalid utf8 code, skip it.
            continue;
        }
        if (cc == ' ') {
            // If char is a space, only move the drawing origin to the left
            x += pDef->width() * sc - sc;
            continue;
        }
        if (cc == '\n') {
            // If char is a space, only move the drawing origin to the next line
            x = ox;
            y += textHeight() - sc;
            continue;
        }

        // get the colored sprite for the character
        Sprite *s = getSprite(cc, color);
        if (s) {
            int y_offset = 0;
            // Add some offset correct for special caracters as ':' '.' ',' '-'
            if (cc == ':')
                y_offset = sc;
            else if (cc == '.' || cc == ',' || cc == '-')
                y_offset = pDef->height() *sc - getSprite(cc)->height() * sc;
            else if (cc == '/') {
                y_offset = (pDef->height() *sc)/2 - (getSprite('/')->height() * sc) / 2;
            }

            s->draw(x, y + y_offset, 0);
            x += s->width() * sc - sc;
        }
    }
}
