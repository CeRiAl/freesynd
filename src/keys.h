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

#ifndef KEYS_H
#define KEYS_H

#include "common.h"

/*! 
 * All available key codes.
 */
enum KeyFunc {
    KFC_UNKNOWN = 0,
    KFC_ESCAPE = 1,
    KFC_BACKSPACE = 2,
    KFC_RETURN = 4,

    // Arrows + Home/End pad
    KFC_UP = 11,
    KFC_DOWN = 12,
    KFC_RIGHT = 13,
    KFC_LEFT = 14,
    KFC_INSERT = 15,
    KFC_HOME = 16,
    KFC_END = 17,
    KFC_PAGEUP = 18,
    KFC_PAGEDOWN = 19,
    KFC_DELETE = 20,

    // Function keys
    KFC_F1 = 21,
    KFC_F2 = 22,
    KFC_F3 = 23,
    KFC_F4 = 24,
    KFC_F5 = 25,
    KFC_F6 = 26,
    KFC_F7 = 27,
    KFC_F8 = 28,
    KFC_F9 = 29,
    KFC_F10 = 30,
    KFC_F11 = 31,
    KFC_F12 = 32
};

//! Valid key modifiers
enum KeyMod {
    KMD_NONE = 0x0000,
    KMD_LSHIFT = 0x0001,
    KMD_RSHIFT = 0x0002,
    KMD_LCTRL = 0x0040,
    KMD_RCTRL = 0x0080,
    KMD_LALT = 0x0100,
    KMD_RALT = 0x0200
};

const int KMD_CTRL = KMD_LCTRL | KMD_RCTRL;
const int KMD_SHIFT = KMD_LSHIFT | KMD_RSHIFT;
const int KMD_ALT = KMD_LALT | KMD_RALT;

/*! 
 * Virtual key represents the physical key on a keyboard.
 */
enum KeyVirtual {
    KVT_UNKNOWN = 0,
    KVT_NUMPAD0 = 1,
    KVT_NUMPAD1 = 2,
    KVT_NUMPAD2 = 3,
    KVT_NUMPAD3 = 4,
    KVT_NUMPAD4 = 5,
    KVT_NUMPAD5 = 6,
    KVT_NUMPAD6 = 7,
    KVT_NUMPAD7 = 8,
    KVT_NUMPAD8 = 9,
    KVT_NUMPAD9 = 10
    
};

/*! 
 * All available key codes.
 */
typedef struct Key {
    KeyFunc keyFunc;
    KeyVirtual keyVirt;        /**< Virtual key : KVT_UNKNOWN if not set. */
    uint16 unicode;            /**< Unicode for printable characters. */
    uint32 keySym;             /**< Symbol for printable characters. */
} Key;

// A list of macros to ease keysym comparisons
#define isLetterA(keySym) keySym == 0x61
#define isLetterD(keySym) keySym == 0x64
#define isLetterG(keySym) keySym == 0x67
#define isLetterH(keySym) keySym == 0x68
#define isLetterQ(keySym) keySym == 0x71
#define isLetterP(keySym) keySym == 0x70

#define K_PLUS      0x2B
#define K_MINUS     0x2D
#define K_SPACE     0x20

#define K_DGT_0     0x30
#define K_DGT_1     0x31
#define K_DGT_2     0x32
#define K_DGT_3     0x33
#define K_DGT_4     0x34
#define K_DGT_5     0x35
#define K_DGT_6     0x36
#define K_DGT_7     0x37
#define K_DGT_8     0x38
#define K_DGT_9     0x39

#endif
