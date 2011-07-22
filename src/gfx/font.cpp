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
            if (cc == ':')
                s->draw(x, y + sc, 0, false, x2);
            else if (cc == '.' || cc == ',')
                s->draw(x, y + 4 * sc, 0, false, x2);
            else if (cc == '-')
                s->draw(x, y + 2 * sc, 0, false, x2);
            else
                s->draw(x, y, 0, false, x2);
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

HChar::HChar():width_(0), height_(0), bits_(0) {
}

HChar::~HChar() {
    if (bits_)
        delete[] bits_;
}

void HChar::set(int w, int h, uint8 *data) {
    width_ = w;
    height_ = h;
    if (data) {
        int stride = w < 9 ? 1 : 2;
        bits_ = new bool[width_ * height_];
        for (int y = 0; y < height_; y++) {
            for (int x = 0; x < width_; x++) {
                if (data[y * stride + (x / 8)] & (128 >> (x % 8)))
                    bits_[width_ * y + x] = true;
                else
                    bits_[width_ * y + x] = false;
            }
        }
    }
}

int HChar::draw(int x, int y, uint8 color) {
    if (bits_) {
        for (int j = 0; j < height_; j++) {
            for (int i = 0; i < width_; i++) {
                if (bits_[width_ * j + i])
                    g_Screen.setPixel(x + i, y + j, color);
            }
        }
    }
    return width_;
}

HFont::HFont() {

}

HFont::~HFont() {

}

void HFont::load() {
    int size;
    uint8 *data = File::loadOriginalFile("hfnt01.dat", size);
    for (int i = 0; i < 128; i++) {
        if (data[i * 5] || data[i * 5 + 1]) {
            if (data[i * 5] == 0xff && data[i * 5 + 1] == 0xff)
                characters[i].set(data[i * 5 + 2], data[i * 5 + 3], NULL);
            else
                characters[i].set(data[i * 5 + 2], data[i * 5 + 3],
                                  data +
                                  (data[i * 5] | (data[i * 5 + 1] << 8)));
        }
    }
    delete[] data;
}

void HFont::drawText(int x, int y, const char *str, uint8 color) {
    while (*str) {
        if (characters.find(*str) != characters.end())
            x += characters[*str].draw(x, y, color);

        str++;
    }
}
