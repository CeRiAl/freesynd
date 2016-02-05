/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2016  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef MODEL_POSITION_H_
#define MODEL_POSITION_H_

/*!
 * This a convenient structure to store a position
 * in map tile coordinates.
 */
class TilePoint {
public:
    /*! The tile's X coord.*/
    int tx;
    /*! The tile's Y coord.*/
    int ty;
    /*! The tile's Z coord.*/
    int tz;
    /*! X Offset inside the tile. (varies between 0 and 256)*/
    int ox;
    /*! Y Offset inside the tile. (varies between 0 and 256)*/
    int oy;
    /*! Z Offset inside the tile. (varies between 0 and 128)*/
    int oz;

    TilePoint() {
        tx = 0;
        ty = 0;
        tz = 0;
        ox = 0;
        oy = 0;
        oz = 0;
    }

    TilePoint(const TilePoint &tp) {
        tx = tp.tx;
        ty = tp.ty;
        tz = tp.tz;
        ox = tp.ox;
        oy = tp.oy;
        oz = tp.oz;
    }

    /*!
     *
     */
    void initFrom(const TilePoint &tp) {
        tx = tp.tx;
        ty = tp.ty;
        tz = tp.tz;
        ox = tp.ox;
        oy = tp.oy;
        oz = tp.oz;
    }

    /*!
     * Return true if this point matches the other
     */
    bool equals(const TilePoint &otherTp) {
        return otherTp.tx == tx
                && otherTp.ty == ty
                && otherTp.tz == tz
                && otherTp.oy == oy
                && otherTp.oz == oz;
    }
};

/*!
 * This structure stores a position
 * in absolute coordinates.
 */
class WorldPoint {
public:
    int x;
    int y;
    int z;

    WorldPoint() {
        x = 0;
        y = 0;
        z = 0;
    }

    WorldPoint(const TilePoint &tp) {
        x = tp.tx * 256 + tp.ox;
        y = tp.ty * 256 + tp.oy;
        z = tp.tz * 128 + tp.oz;
    }
};

#endif // MODEL_POSITION_H_
