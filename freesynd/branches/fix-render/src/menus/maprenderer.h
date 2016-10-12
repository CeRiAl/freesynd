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

#include <list>
#include <vector>
#include <set>
#include <map>

#include "common.h"
#include "utils/log.h"
#include "gfx/screen.h"
#include "gfx/tile.h"
#include "model/position.h"
#include "mapobject.h"

class Mission;
class Map;
class Vehicle;
class PedInstance;
class WeaponInstance;
class Static;
class SFXObject;
class SquadSelection;

class PoolableResource {
public:
    PoolableResource() { free_ = true; }
    virtual ~PoolableResource() {}

    bool isFree() { return free_; }
    void setFree(bool free) { free_ = free; }

    virtual void handleRelease() {}

private:
    bool free_;
};

class ObjectToDraw : public PoolableResource {
public:
    ObjectToDraw() {
        handleRelease();
    }

    void handleRelease() {
        pObject_ = NULL;
        pNext_ = NULL;
    }

    MapObject * getObject() { return pObject_; }
    ObjectToDraw * getNext() { return pNext_; }

    void setObject(MapObject *pObj) { pObject_ = pObj; }
    void setNext(ObjectToDraw *pNext) {
        pNext_ = pNext;
    }
    void insertNext(ObjectToDraw *pNext) {
        pNext->setNext(pNext_);
        pNext_ = pNext;
    }


private:
    MapObject *pObject_;
    ObjectToDraw *pNext_;
};

template<class T>
class Pool {
public:
    Pool(int nbInitialRes) {
        totalResourceCreated_ = 0;
        for (int nbRes = 0; nbRes < nbInitialRes; nbRes++) {
            pool_.push_back(new T());
            totalResourceCreated_++;
        }
    }
    ~Pool() {
        for (typename std::list < T * >::iterator it = pool_.begin();
         it != pool_.end(); it++) {
             T * pRes = (*it);
            delete pRes;
            totalResourceCreated_--;
        }

        pool_.clear();

        if (totalResourceCreated_ > 0) {
            FSERR(Log::k_FLG_MEM, "Pool", "~Pool", ("There %d unfreed resource(s)", totalResourceCreated_));
        }
    };

    T * getResource() {
        T *pResource = NULL;

        if (pool_.empty()) {
            pResource = new T();
            totalResourceCreated_++;
        } else {
            pResource = pool_.front();
            pool_.pop_front();
        }

        pResource->setFree(false);
        return pResource;
    }

    void releaseResource(T * pResource) {
        if (!pResource->isFree()) {
            pResource->handleRelease();
            pool_.push_back(pResource);
            pResource->setFree(true);
        }
    }

private:
    std::list<T *> pool_;
    int totalResourceCreated_;
};
class MapRenderer {
public:
    MapRenderer() : pool_(10) {}

    void init(Mission *pMission, SquadSelection *pSelection);

    void render(const Point2D &worldPos);

private:
    bool initRenderParams(const Point2D &viewPortPt, TilePoint *pFirstTile, Point2D *pFirstTilePos);
    void listObjectsToDraw(const Point2D &viewport);
    static int fastKey(const TilePoint & tilePos) {
        return tilePos.tx | (tilePos.ty << 8) | (tilePos.tz << 16);
    }

    static int fastKey(MapObject * m);

    void createFastKeys(const Point2D &startPos, const Point2D &endPos);
    int drawObjectsOnTile(const TilePoint & tilePos, const Point2D &screenPos);
    int drawObjectsOnTile2(const TilePoint & tilePos, const Point2D &screenPos);

    bool isTileVisibleOnScreen(const Point2D &screenPos) {
        return screenPos.x >= Screen::kScreenPanelWidth - TILE_WIDTH &&
                screenPos.x <= Screen::kScreenWidth &&
                screenPos.y >= -TILE_HEIGHT &&
                screenPos.y <= Screen::kScreenHeight;
    }

    bool isObjectInsideDrawingArea(MapObject *pObject, const Point2D &viewport);

    void addObjectToDraw(MapObject *pObject);

    void freeUnreleasedResources();

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

    Pool<ObjectToDraw> pool_;
    std::map<int, ObjectToDraw *> objectsByTile_;
};

#endif  // MENUS_MAPRENDERER_H_
