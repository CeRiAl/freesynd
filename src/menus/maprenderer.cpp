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

#include "menus/maprenderer.h"
#include "mission.h"
#include "model/vehicle.h"
#include "agentmanager.h"
#include "mapobject.h"
#include "core/squad.h"
#include "gfx/screen.h"
#include "gfx/tile.h"
#include "system.h"
#include "menus/squadselection.h"

void MapRenderer::init(Mission *pMission, SquadSelection *pSelection) {
    pMission_ = pMission;
    pMap_ = pMission->get_map();
    pSelection_ = pSelection;
}

/**
 * Draw tiles and map objects.
 */
void MapRenderer::render(const Point2D &worldPos) {
    // TODO: after a lot of attempts to fix this, map drawing remains buggy
    TilePoint worldTilePos = pMap_->screenToTilePoint(worldPos.x, worldPos.y);

    int boardSize = g_Screen.gameScreenWidth() / (TILE_WIDTH / 2) + 2
        + g_Screen.gameScreenHeight() / (TILE_HEIGHT / 3) + pMap_->maxZ() * 2;
    //int chk = 1;

    Point2D enclosingStartPos = {worldTilePos.tx, worldTilePos.ty - 8};
    pMap_->clip(&enclosingStartPos);

    Point2D enclosingEndPos = {enclosingStartPos.x + boardSize, enclosingStartPos.y + boardSize};
    pMap_->clip(&enclosingEndPos);

    DEBUG_SPEED_INIT

    createFastKeys(enclosingStartPos, enclosingEndPos);
    int cmw = worldPos.x + g_Screen.gameScreenWidth() -
                g_Screen.gameScreenLeftMargin() + 128;
    int cmh = worldPos.y + g_Screen.gameScreenHeight() + 128;
    int cmx = worldPos.x - g_Screen.gameScreenLeftMargin();
     //  z = 0 - is minimap data and mapdata
    int zr = enclosingEndPos.y + pMap_->maxZ() + 1;

    int nbobject = 0;

    for (int inc = 0; inc < zr; ++inc) {
        int ye = enclosingStartPos.y + inc;
        int ys = ye - pMap_->maxZ() - 2;
        int tile_z = pMap_->maxZ() + 1;  // the Z coord of the next tile to draw
        for (int yb = ys; yb < ye; ++yb) {
            if (yb < 0 || yb < enclosingStartPos.y || yb >= enclosingEndPos.y) {
                --tile_z;
                continue;
            }
            int tile_y = yb;  // The Y coord of the tile to draw
            for (int tile_x = enclosingStartPos.x;
                    tile_y >= enclosingStartPos.y && tile_x < pMap_->maxX();
                    ++tile_x) {
                if (tile_x < 0 || tile_y >= pMap_->maxY()) {
                    --tile_y;
                    continue;
                }

                int screen_w = (pMap_->maxX() + (tile_x - tile_y)) * (TILE_WIDTH / 2);
                // int screen_h = (max_z_ + w + h) * (TILE_HEIGHT / 3);
                int coord_h = ((pMap_->maxZ() + tile_x + tile_y) - (tile_z - 1)) * (TILE_HEIGHT / 3);
                if (screen_w >= worldPos.x - TILE_WIDTH * 2
                    && screen_w + TILE_WIDTH * 2 < cmw
                    && coord_h >= worldPos.y - TILE_HEIGHT * 2
                    && coord_h + TILE_HEIGHT * 2 < cmh) {
#if 0
                    if (z > 2)
                        continue;
#endif
                    // draw a tile
                    if (tile_z < pMap_->maxZ()) {
                        Tile *p_tile = pMap_->getTileAt(tile_x, tile_y, tile_z);
                        if (p_tile->notTransparent()) {
                            int dx = 0, dy = 0;
                            if (screen_w - worldPos.x < 0)
                                dx = -(screen_w - worldPos.x);
                            if (coord_h - worldPos.y < 0)
                                dy = -(coord_h - worldPos.y);
                            if (dx < TILE_WIDTH && dy < TILE_HEIGHT) {
                                p_tile->drawToScreen(screen_w - cmx, coord_h - worldPos.y);
                            }
                        }
                    }

                    // draw everything that's on the tile
                    if (tile_z - 1 >= 0) {
                        TilePoint currentTile(tile_x, tile_y, tile_z - 1);

                        Point2D screenPos = {
                            screen_w - cmx + TILE_WIDTH / 2,
                            coord_h - worldPos.y + TILE_HEIGHT / 3 * 2};

                        nbobject += drawObjectsOnTile(currentTile, screenPos);
                    }
                }
                --tile_y;
            }
            --tile_z;
        }
    }

#ifdef _DEBUG
    if (g_System.getKeyModState() & KMD_LALT) {
        for (SquadSelection::Iterator it = pSelection_->begin();
            it != pSelection_->end(); ++it) {
            (*it)->showPath(worldPos.x, worldPos.y);
        }
    }
#endif

    DEBUG_SPEED_LOG("MapRenderer::render")
}

int MapRenderer::fastKey(MapObject * m) {
    return fastKey(m->position());
}

void MapRenderer::createFastKeys(const Point2D &startPos, const Point2D &endPos) {
    int nbobject = 0;

    cache_vehicles_.clear();
    cache_peds_.clear();
    cache_weapons_.clear();
    cache_statics_.clear();
    cache_sfx_objects_.clear();

    fast_vehicle_cache_.clear();
    fast_ped_cache_.clear();
    fast_weapon_cache_.clear();
    fast_statics_cache_.clear();
    fast_sfx_objects_cache_.clear();

    // updating position for visual markers
    for (size_t i = 0; i < AgentManager::kMaxSlot; i++) {
        PedInstance *p = pMission_->getSquad()->member(i);
        if (p != NULL && p->isAlive()) {
            if (p->tileX() >= startPos.x && p->tileX() < endPos.x
                && p->tileY() >= startPos.y && p->tileY() < endPos.y) {
                // sfx_objects_[i]->setPosition(p->tileX(), p->tileY(), p->tileZ(),
                    // p->offX(), p->offY(), p->offZ() + 320);
                pMission_->sfxObjects(i + 4)->setPosition(p->tileX(), p->tileY(),
                    p->tileZ(), p->offX() - 16, p->offY(), p->offZ() + 256);
            }
        } else {
             // sfx_objects_[i]->setMap(-1);
             pMission_->sfxObjects(i + 4)->setMap(-1);
        }
    }

    // vehicles
    for (unsigned int i = 0; i < pMission_->numVehicles(); i++) {
        Vehicle *v = pMission_->vehicle(i);
        if (v->tileX() >= startPos.x && v->tileX() < endPos.x
            && v->tileY() >= startPos.y && v->tileY() < endPos.y) {
            // NOTE: a trick to make vehicles be drawn correctly z+1
            TilePoint tilePos( v->position());
            tilePos.tz += 1;
            fast_vehicle_cache_.insert(fastKey(tilePos));
            cache_vehicles_.push_back(v);
            nbobject++;
        }
    }

    // peds
    for (size_t i = 0; i < AgentManager::kMaxSlot; i++) {
        PedInstance *p = pMission_->getSquad()->member(i);
        if (p != NULL && p->map() != -1) {
            if (p->tileX() >= startPos.x && p->tileX() < endPos.x
                && p->tileY() >= startPos.y && p->tileY() < endPos.y) {
                fast_ped_cache_.insert(fastKey(p));
                cache_peds_.push_back(p);

                nbobject++;
            }
        }
    }
    for (size_t i = pMission_->getSquad()->size(); i < pMission_->numPeds(); i++) {
        PedInstance *p = pMission_->ped(i);
        if (p->map() != -1) {
            if (p->tileX() >= startPos.x && p->tileX() < endPos.x
                && p->tileY() >= startPos.y && p->tileY() < endPos.y) {
                fast_ped_cache_.insert(fastKey(p));
                cache_peds_.push_back(p);
                nbobject++;
            }
        }
    }

    // weapons
    for (unsigned int i = 0; i < pMission_->numWeapons(); i++) {
        WeaponInstance *w = pMission_->weapon(i);
        if (w->map() != -1 && w->tileX() >= startPos.x && w->tileX() < endPos.x
            && w->tileY() >= startPos.y && w->tileY() < endPos.y) {
            fast_weapon_cache_.insert(fastKey(w));
            cache_weapons_.push_back(w);
            nbobject++;
        }
    }

    // statics
    for (unsigned int i = 0; i < pMission_->numStatics(); i++) {
        Static *s = pMission_->statics(i);
        if (s->tileX() >= startPos.x && s->tileX() < endPos.x
            && s->tileY() >= startPos.y && s->tileY() < endPos.y) {
            fast_statics_cache_.insert(fastKey(s));
            cache_statics_.push_back(s);
            nbobject++;
        }
    }

    // sfx objects
    for (unsigned int i = 0; i < pMission_->numSfxObjects(); i++) {
        SFXObject *so = pMission_->sfxObjects(i);
        if (so->map() != -1 && so->tileX() >= startPos.x && so->tileX() < endPos.x
            && so->tileY() >= startPos.y && so->tileY() < endPos.y) {
            fast_sfx_objects_cache_.insert(fastKey(so));
            cache_sfx_objects_.push_back(so);
            nbobject++;
        }
    }
}

/*!
 * Draw on screen all objects that have their position on the given tile.
 * \param tilePos const TilePoint& Tile's position
 * \param screenPos const Point2D& Screen position to draw the objects
 * \return int
 *
 */
int MapRenderer::drawObjectsOnTile(const TilePoint & tilePos, const Point2D &screenPos) {
    int key = fastKey(tilePos);
    int nbobject = 0;

    if (fast_vehicle_cache_.find(key) != fast_vehicle_cache_.end()) {
        // draw vehicles
        for (unsigned int i = 0; i < cache_vehicles_.size(); i++)
            if (cache_vehicles_[i]->tileX() == tilePos.tx
                && cache_vehicles_[i]->tileY() == tilePos.ty
                // NOTE: a trick to make vehicles be drawn correctly z+1
                && (cache_vehicles_[i]->tileZ() + 1) == tilePos.tz)
                cache_vehicles_[i]->draw(screenPos.x, screenPos.y);
                nbobject++;
    }

    if (fast_ped_cache_.find(key) != fast_ped_cache_.end()) {
        // draw peds
        for (unsigned int i = 0; i < cache_peds_.size(); i++)
            if (cache_peds_[i]->sameTile(tilePos)) {
                cache_peds_[i]->draw(screenPos.x, screenPos.y);

                nbobject++;

#if 0
                g_Screen.drawLine(x - TILE_WIDTH / 2, y,
                                  x + TILE_WIDTH / 2, y, 11);
                g_Screen.drawLine(x + TILE_WIDTH / 2, y,
                                  x + TILE_WIDTH / 2, y + TILE_HEIGHT,
                                  11);
                g_Screen.drawLine(x + TILE_WIDTH / 2, y + TILE_HEIGHT,
                                  x - TILE_WIDTH / 2, y + TILE_HEIGHT,
                                  11);
                g_Screen.drawLine(x - TILE_WIDTH / 2, y + TILE_HEIGHT,
                                  x - TILE_WIDTH / 2, y, 11);
#endif
            }
    }

    if (fast_weapon_cache_.find(key) != fast_weapon_cache_.end()) {
        // draw weapons
        for (unsigned int i = 0; i < cache_weapons_.size(); i++)
            if (cache_weapons_[i]->map() != -1
                && cache_weapons_[i]->sameTile(tilePos)) {
                cache_weapons_[i]->draw(screenPos.x, screenPos.y);
                nbobject++;
            }
    }

    if (fast_statics_cache_.find(key) != fast_statics_cache_.end()) {
        // draw statics
        for (unsigned int i = 0; i < cache_statics_.size(); i++)
            if (cache_statics_[i]->sameTile(tilePos)) {
                cache_statics_[i]->draw(screenPos.x, screenPos.y);
                nbobject++;
            }
    }

    if (fast_sfx_objects_cache_.find(key) != fast_sfx_objects_cache_.end()) {
        // draw sfx objects
        for (unsigned int i = 0; i < cache_sfx_objects_.size(); i++)
            if (cache_sfx_objects_[i]->sameTile(tilePos)) {
                cache_sfx_objects_[i]->draw(screenPos.x, screenPos.y);
                nbobject++;
            }
    }

    return nbobject;
}
