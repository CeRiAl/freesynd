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
#include "system.h"
#include "menus/squadselection.h"

void MapRenderer::init(Mission *pMission, SquadSelection *pSelection) {
    pMission_ = pMission;
    pMap_ = pMission->get_map();
    pSelection_ = pSelection;
}

/**
 *
 * \param viewPortPt const Point2D&
 * \param viewPortTile const TilePoint&
 * \param pFirstTile TilePoint*
 * \param pFirstTilePos Point2D*
 * \return bool
 *
 */
bool MapRenderer::initRenderParams(const Point2D &viewPortPt, TilePoint *pFirstTile, Point2D *pFirstTilePos) {
    TilePoint viewPortTile;
    pMap_->viewportToTileCoordsWithAltitudeZero(viewPortPt, &viewPortTile);
    TilePoint vpTile0 (viewPortTile);
    vpTile0.ox = 0;
    vpTile0.oy = 0;
    vpTile0.oz = 0;

    Point2D viewportOriPt;
    pMap_->tileToViewportCoords(vpTile0, &viewportOriPt);

    Point2D screenPos = {
        Screen::kScreenPanelWidth - (viewPortPt.x - viewportOriPt.x + Tile::kHalfTileWidth),
        - viewPortPt.y + viewportOriPt.y};

    bool nextLineLeft = false;

    if (viewPortPt.x < viewportOriPt.x) {
        if (viewPortPt.y < viewportOriPt.y + Tile::kThirdTileHeight) {
            pFirstTile->tx = viewPortTile.tx - 1;
            pFirstTile->ty = viewPortTile.ty;
            pFirstTilePos->x = screenPos.x - Tile::kHalfTileWidth;
            pFirstTilePos->y = screenPos.y - Tile::kThirdTileHeight;
            nextLineLeft = false;

        } else {
            pFirstTile->tx = viewPortTile.tx;
            pFirstTile->ty = viewPortTile.ty;
            pFirstTilePos->x = screenPos.x;
            pFirstTilePos->y = screenPos.y;
            nextLineLeft = true;
        }
    } else {
        if (viewPortPt.y < viewportOriPt.y + Tile::kThirdTileHeight) {
            pFirstTile->tx = viewPortTile.tx - 1;
            pFirstTile->ty = viewPortTile.ty;
            pFirstTilePos->x = screenPos.x - Tile::kHalfTileWidth;
            pFirstTilePos->y = screenPos.y - Tile::kThirdTileHeight;
            nextLineLeft = true;
        } else {
            pFirstTile->tx = viewPortTile.tx;
            pFirstTile->ty = viewPortTile.ty;
            pFirstTilePos->x = screenPos.x;
            pFirstTilePos->y = screenPos.y;
            nextLineLeft = false;
        }
    }

    return nextLineLeft;
}

/**
 * Rendering is made
 * \param viewPortPt const Point2D&
 * \return void
 *
 */
void MapRenderer::render(const Point2D &viewPortPt) {
    DEBUG_SPEED_INIT

    TilePoint tileToDraw;
    // when drawing a line of tiles, position of the tile to draw is computed
    // based on the first tile of the line
    TilePoint firstTileOnLine;
    Point2D firstTileOnLinePos;
    // true means the first tile of the next line to draw is starting from the left of
    // current first tile. False means the right
    bool incrLeftOrRight = initRenderParams(viewPortPt, &firstTileOnLine, &firstTileOnLinePos);

    listObjectsToDraw(viewPortPt);

    for (int j=0; j<35; j++) {
        for(int i=0; i<10; i++) {
            tileToDraw.tx = firstTileOnLine.tx + i;
            tileToDraw.ty = firstTileOnLine.ty - i;
            tileToDraw.tz = 0;

            // as we draw a fixed number of lines of tiles, some tiles might fall out of the map
            // so check before drawing them.
            if (!pMap_->clip(&tileToDraw)) {
                for (tileToDraw.tz = 0; tileToDraw.tz < pMap_->maxZ(); tileToDraw.tz++) {
                    Point2D tileToDrawScreenPos = {
                                firstTileOnLinePos.x + i * TILE_WIDTH,
                                firstTileOnLinePos.y - tileToDraw.tz * Tile::kThirdTileHeight };

                    if (isTileVisibleOnScreen(tileToDrawScreenPos)) {
                        Tile *pTile = pMap_->getTileAt(tileToDraw.tx, tileToDraw.ty, tileToDraw.tz);

                        if (pTile->notTransparent()) {
                            pTile->drawToScreen(tileToDrawScreenPos.x, tileToDrawScreenPos.y);
                        }

                        drawObjectsOnTile2(tileToDraw, tileToDrawScreenPos);
                    }
                }
            }
        }

        if (incrLeftOrRight) { // left
            firstTileOnLine.ty++;
            firstTileOnLinePos.x -= Tile::kHalfTileWidth;
        } else { // right
            firstTileOnLine.tx++;
            firstTileOnLinePos.x += Tile::kHalfTileWidth;
        }
        firstTileOnLinePos.y += Tile::kThirdTileHeight;
        incrLeftOrRight = !incrLeftOrRight;
    }

    freeUnreleasedResources();

    /*if (debugScreenPos.x != 0) {
        g_Screen.drawLine(debugScreenPos.x, debugScreenPos.y, debugScreenPos.x + TILE_WIDTH, debugScreenPos.y, 11);
        g_Screen.drawLine(debugScreenPos.x + TILE_WIDTH, debugScreenPos.y, debugScreenPos.x + TILE_WIDTH, debugScreenPos.y + TILE_HEIGHT, 11);
        g_Screen.drawLine(debugScreenPos.x, debugScreenPos.y, debugScreenPos.x, debugScreenPos.y + TILE_HEIGHT, 11);
        g_Screen.drawLine(debugScreenPos.x, debugScreenPos.y + TILE_HEIGHT, debugScreenPos.x + TILE_WIDTH, debugScreenPos.y + TILE_HEIGHT, 11);
    }*/

    DEBUG_SPEED_LOG("MapRenderer::render")

/*    TilePoint vpTilePos;
    pMap_->viewportToTileCoordsWithAltitudeZero(viewPortPt, &vpTilePos);
    vpTilePos.ox = 0;
    vpTilePos.oy = 0;
    vpTilePos.oz = 0;

    Point2D screenOffset;
    pMap_->tileToViewportCoords(vpTilePos, &screenOffset);

    // The values used to calculate the borders have been found by testing
    // different values
    TilePoint enclosingStartTile(vpTilePos.tx, vpTilePos.ty - 8, 0, 0, 0, 0);
    pMap_->clip(&enclosingStartTile);

    TilePoint enclosingEndTile(enclosingStartTile.tx + 25, enclosingStartTile.ty + 26, 0, 0, 0, 0);
    pMap_->clip(&enclosingEndTile);

    // this point is used to compute the tile positions for drawing them
    Point2D enclosingViewport;
    pMap_->tileToViewportCoords(enclosingStartTile, &enclosingViewport);

    //DEBUG_SPEED_INIT

    listObjectsToDraw(enclosingStartTile, enclosingEndTile);
    TilePoint currentTile(0, 0, 0, 0, 0, 0);

    for (currentTile.ty = enclosingStartTile.ty; currentTile.ty <= enclosingEndTile.ty; ++currentTile.ty) {
        for (currentTile.tx = enclosingStartTile.tx; currentTile.tx <= enclosingEndTile.tx; ++currentTile.tx) {
            for (currentTile.tz = 0; currentTile.tz < pMap_->maxZ(); currentTile.tz++) {

                Point2D currentTileViewport = {
                        enclosingViewport.x + (currentTile.tx - enclosingStartTile.tx) * (TILE_WIDTH / 2) - (currentTile.ty - enclosingStartTile.ty) * (TILE_WIDTH / 2),
                        enclosingViewport.y + (currentTile.tx - enclosingStartTile.tx) * (TILE_HEIGHT / 3) + (currentTile.ty - enclosingStartTile.ty) * (TILE_HEIGHT / 3) - currentTile.tz * (TILE_HEIGHT / 3)};

                Point2D screenPos = {
                        Screen::kScreenPanelWidth + currentTileViewport.x - viewPortPt.x,
                        currentTileViewport.y - viewPortPt.y
                };

                if (isTileVisibleOnScreen(screenPos)) {
                    Tile *pTile = pMap_->getTileAt(currentTile.tx, currentTile.ty, currentTile.tz);

                    if (currentTile.tx == 100 && currentTile.ty == 10 && currentTile.tz == 2) {
                        if (pTile->notTransparent()) {
                            pTile->drawToScreen(screenPos.x, screenPos.y);
                        }
                        g_Screen.drawLine(screenPos.x, screenPos.y, screenPos.x + TILE_WIDTH, screenPos.y, 11);
                        g_Screen.drawLine(screenPos.x + TILE_WIDTH, screenPos.y, screenPos.x + TILE_WIDTH, screenPos.y + TILE_HEIGHT, 11);
                        g_Screen.drawLine(screenPos.x, screenPos.y, screenPos.x, screenPos.y + TILE_HEIGHT, 11);
                        g_Screen.drawLine(screenPos.x, screenPos.y + TILE_HEIGHT, screenPos.x + TILE_WIDTH, screenPos.y + TILE_HEIGHT, 11);
                    }

                    drawObjectsOnTile2(currentTile, screenPos);
                  if (currentTile.tz - 1 >= 0) {
                            Point2D screenPos = {
                                screen_w - cmx + TILE_WIDTH / 2,
                                coord_h - worldPos.y + TILE_HEIGHT / 3 * 2};

                            nbobject += drawObjectsOnTile(currentTile, screenPos);
                    }
                }
            } // fin for tileZ
        } // Fin for tileX
    } // Fin for tileY

    //DEBUG_SPEED_LOG("MapRenderer::render")*/
}

void MapRenderer::listObjectsToDraw(const Point2D &viewport) {
    // Include peds
    for (size_t i = 0; i < pMission_->numPeds(); i++) {
        PedInstance *pPed = pMission_->ped(i);
        if (pPed->map() != -1) {
            if (isObjectInsideDrawingArea(pPed, viewport)) {
                addObjectToDraw(pPed);
            }
        }
    }
}

bool MapRenderer::isObjectInsideDrawingArea(MapObject *pObject, const Point2D &viewport) {
    Point2D objectViewport;
    pMission_->get_map()->tileToViewportCoords(pObject->position(), &objectViewport);

    return objectViewport.x > viewport.x && objectViewport.y > viewport.y &&
            objectViewport.x <= (viewport.x + Screen::kScreenWidth - Screen::kScreenPanelWidth) &&
            objectViewport.y <= (viewport.y + Screen::kScreenHeight);
}

/**
 * Add the given object to the list of objects that need to be drawn.
 * Objects are put in a list associated with the tile they're on.
 * They are put in order from farthest to closest so they can be drawn in that order.
 * \param pObjectToAdd MapObject*
 * \return void
 *
 */
void MapRenderer::addObjectToDraw(MapObject *pObjectToAdd) {
    int tileKey = fastKey(pObjectToAdd);
    ObjectToDraw *pNewEntry = pool_.getResource();
    pNewEntry->setObject(pObjectToAdd);

    std::map<int, ObjectToDraw *>::iterator element = objectsByTile_.find(tileKey);
    if(element == objectsByTile_.end()) {
        // no element has been set with the tile so add the first element
        objectsByTile_[tileKey] = pNewEntry;
    } else {
        // there is at leastone element already set with the tile
        ObjectToDraw *pObjectInList = element->second;
        if (pObjectToAdd->isBehindObjectOnSameTile(pObjectInList->getObject())) {
            // first case is when the new object should be first in the list
            pNewEntry->setNext(pObjectInList);
            objectsByTile_[tileKey] = pNewEntry;
        } else {
            // second case is when new object is somewhere in the list
            while (pObjectInList != NULL) {
                if (pObjectInList->getNext() != NULL) {
                    if (pObjectToAdd->isBehindObjectOnSameTile(pObjectInList->getNext()->getObject())) {
                        pObjectInList->insertNext(pNewEntry);
                        break;
                    } else {
                        pObjectInList = pObjectInList->getNext();
                    }
                } else {
                    pObjectInList->setNext(pNewEntry);
                    break;
                }
            }
        }
    }
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
int MapRenderer::drawObjectsOnTile2(const TilePoint & tilePos, const Point2D &screenPos) {
    int tileKey = fastKey(tilePos);
    int nbDrawnObjects = 0;

    std::map<int, ObjectToDraw *>::iterator it = objectsByTile_.find(tileKey);
    if(it != objectsByTile_.end()) {
        ObjectToDraw *pObj = it->second;
        objectsByTile_.erase(it);
        while(pObj != NULL) {
            pObj->getObject()->draw(screenPos.x + TILE_WIDTH / 2, screenPos.y + TILE_HEIGHT / 3);
            ObjectToDraw *pNext = pObj->getNext();
            pool_.releaseResource(pObj);
            pObj = pNext;
            nbDrawnObjects++;
        }
    }

    return nbDrawnObjects;
}

void MapRenderer::freeUnreleasedResources() {
    int nbFreed = 0;
    std::map<int, ObjectToDraw *>::iterator itr = objectsByTile_.begin();
    while (itr != objectsByTile_.end()) {
        std::map<int, ObjectToDraw *>::iterator toErase = itr;
        ++itr;
        ObjectToDraw *pObj = toErase->second;
        while(pObj != NULL) {
            ObjectToDraw *pNext = pObj->getNext();
            pool_.releaseResource(pObj);
            nbFreed++;
            pObj = pNext;
        }
        objectsByTile_.erase(toErase);
    }

    //printf("Nb ressources liberees : %d\n", nbFreed);
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
