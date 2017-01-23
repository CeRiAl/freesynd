/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#ifndef MODEL_TRAIN_H_
#define MODEL_TRAIN_H_

#include <string>
#include <list>

#include "vehicle.h"

/*!
 * .
 */
class TrainBody : public Vehicle {
public:
    TrainBody(uint16 id, uint8 aType, VehicleAnimation *pAnimation);
    ~TrainBody();

    bool updatePosition(int elapsed, Mission *m);

    //! Set the destination to reach at given speed
    bool initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed = -1) {
        return false;
    }
};

/*!
 * .
 */
class TrainHead : public TrainBody {
public:
    TrainHead(uint16 id, uint8 aType, VehicleAnimation *pAnimation);
    ~TrainHead();

    //! Animates the train
    bool animate(int elapsed);
private:

};

#endif // MODEL_TRAIN_H_
