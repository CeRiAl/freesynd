#include "train.h"

TrainHead::TrainHead(uint16 anId, uint8 aType, VehicleAnimation *pAnimation) : TrainBody(anId, aType, pAnimation) {
}

TrainHead::~TrainHead() {

}

bool TrainHead::animate(int elapsed) {
    return false;
}

TrainBody::TrainBody(uint16 anId, uint8 aType, VehicleAnimation *pAnimation) : Vehicle(anId, aType, -1, pAnimation) {

}

TrainBody::~TrainBody() {
}

/*!
 * Moves a vehicle on the map.
 * \param elapsed Elapsed time sine last frame.
 */
bool TrainBody::move_vehicle(int elapsed)
{
    return false;
}
