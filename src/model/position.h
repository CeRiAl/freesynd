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

struct toDefineXYZ {
        int x;
        int y;
        int z;
    };

/*!
 * Path node class.
 */
class PathNode {
public:
    PathNode() :
        tile_x_(0), tile_y_(0), tile_z_(0), off_x_(0), off_y_(0), off_z_(0) {}
    PathNode(int tile_x, int tile_y, int tile_z, int off_x = 128,
            int off_y = 128, int off_z = 0) :
        tile_x_(tile_x), tile_y_(tile_y), tile_z_(tile_z),
        off_x_(off_x), off_y_(off_y), off_z_(off_z) {}

    int tileX() const { return tile_x_; }
    int tileY() const { return tile_y_; }
    int tileZ() const { return tile_z_; }

    int offX() const { return off_x_; }
    int offY() const { return off_y_; }
    int offZ() const { return off_z_; }

    bool operator<(const PathNode &other) const {
        int a = tile_x_ | (tile_y_ << 16);
        int b = other.tile_x_ | (other.tile_y_ << 16);
        return a < b;
    }

    void setOffX(int x) { off_x_ = x; }
    void setOffY(int y) { off_y_ = y; }
    void setOffZ(int z) { off_z_ = z; }
    void setOffXY(int x, int y) { off_x_ = x; off_y_ = y; }
    void setOffXYZ(int x, int y, int z) {
        off_x_ = x;
        off_y_ = y;
        off_z_ = z;
    }

    void setTileX(int x) { tile_x_ = x; }
    void setTileY(int y) { tile_y_ = y; }
    void setTileZ(int z) { tile_z_ = z; }
    void setTileXYZ(int x, int y, int z) {
        tile_x_ = x;
        tile_y_ = y;
        tile_z_ = z;
    }

    void convertPosToXYZ(toDefineXYZ *xyz) const {
        xyz->x = tile_x_ * 256 + off_x_;
        xyz->y = tile_y_ * 256 + off_y_;
        xyz->z = tile_z_ * 128 + off_z_;
    }

    void convertPosToXY(int *x, int *y) {
        *x = tile_x_ * 256 + off_x_;
        *y = tile_y_ * 256 + off_y_;
    }

protected:
    int tile_x_, tile_y_, tile_z_, off_x_, off_y_, off_z_;
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
        convertFromTilePoint(tp);
    }

    WorldPoint(const PathNode &pn) {
        x = pn.tileX() * 256 + pn.offX();
        y = pn.tileY() * 256 + pn.offY();
        z = pn.tileZ() * 128 + pn.offZ();
    }

    void convertFromTilePoint(const TilePoint &tp) {
        x = tp.tx * 256 + tp.ox;
        y = tp.ty * 256 + tp.oy;
        z = tp.tz * 128 + tp.oz;
    }
};

#endif // MODEL_POSITION_H_
