/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef MENUS_MAPRENDERER_H_
#define MENUS_MAPRENDERER_H_

#include <vector>
#include <set>

#include "common.h"
#include "model/position.h"

class Mission;
class Map;
class MapObject;
class Vehicle;
class PedInstance;
class WeaponInstance;
class Static;
class SFXObject;
class SquadSelection;

class MapRenderer {
public:
    void init(Mission *pMission, SquadSelection *pSelection);

    void render(const Point2D &worldPos);

private:
    static int fastKey(const TilePoint & tilePos) {
        return tilePos.tx | (tilePos.ty << 8) | (tilePos.tz << 16);
    }

    static int fastKey(MapObject * m);

    void createFastKeys(const Point2D &startPos, const Point2D &endPos);
    int drawObjectsOnTile(const TilePoint & tilePos, const Point2D &screenPos);

private:
    Mission *pMission_;
    Map *pMap_;
    SquadSelection *pSelection_;

    std::vector<Vehicle *> cache_vehicles_;
    std::vector<PedInstance *> cache_peds_;
    std::vector<WeaponInstance *> cache_weapons_;
    std::vector<Static *> cache_statics_;
    std::vector<SFXObject *> cache_sfx_objects_;

    std::set<int> fast_vehicle_cache_, fast_ped_cache_, fast_weapon_cache_,
            fast_statics_cache_, fast_sfx_objects_cache_;
};

#endif  // MENUS_MAPRENDERER_H_
