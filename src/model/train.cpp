#include "train.h"

TrainBody::TrainBody(uint16 anId, uint8 aType, VehicleAnimation *pAnimation, int startHp) :
    Vehicle(anId, aType, -1, pAnimation) {

    setHealth(startHp);
    setStartHealth(startHp);

    pNextBody_ = NULL;
}

TrainBody::~TrainBody() {
}

void TrainBody::changeTrainAndPassengersPosition(int distanceX, int distanceY) {
    addOffsetToPosition(distanceX, distanceY);

    if (!passengers_.empty()) {
        for (std::set<PedInstance *>::iterator it = passengers_.begin();
            it != passengers_.end(); it++
        ) {
            (*it)->setPosition(pos_);
        }
    }

    if (pNextBody_ != NULL) {
        pNextBody_->changeTrainAndPassengersPosition(distanceX, distanceY);
    }
}

TrainHead::TrainHead(uint16 anId, uint8 aType, VehicleAnimation *pAnimation, int startHp) :
    TrainBody(anId, aType, pAnimation, startHp) {
        moveOnXaxis_ = true;
}

TrainHead::~TrainHead() {

}

//! Set the destination to reach at given speed
bool TrainHead::initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed) {
    clearDestination();

    dest_path_.push_front(destinationPt);
    speed_ = newSpeed;

    moveOnXaxis_ = (destinationPt.ty == pos_.ty);
    return true;
}

/*!
 * Moves a vehicle on the map.
 * \param elapsed Elapsed time sine last frame.
 */
bool TrainHead::doMove(int elapsed, Mission *m)
{
    bool updated = false;
    int remainingTime = elapsed;

    while ((!dest_path_.empty()) && remainingTime != 0) {

        // Get distance between vehicle and next NodePath
        WorldPoint destination(dest_path_.front());

        double distanceToNextNode = distanceToPosition(destination);
        // This is the time for all the remaining distance to the next node in the path
        double availableTimeToNextNode = (distanceToNextNode / (double)speed_) * 1000.0;
        // We cannot spend more time than the time remaining
        if (availableTimeToNextNode > remainingTime)
            availableTimeToNextNode = remainingTime;

        // computes distance travelled by vehicle in the available time
        WorldPoint currentPos(pos_);
        int  distanceX = 0, distanceY = 0;
        if (isMovementOnXAxis()) {
            int diffx = destination.x - currentPos.x;
            distanceX = (int)((diffx * (speed_ * availableTimeToNextNode) / distanceToNextNode) / 1000);
        } else {
            int diffy = destination.y - currentPos.y;
            distanceY = (int)((diffy * (speed_ * availableTimeToNextNode) / distanceToNextNode) / 1000);
        }

        // Updates the remaining time
        remainingTime -= availableTimeToNextNode;

        // Moves vehicle
        changeTrainAndPassengersPosition(distanceX, distanceY);

        stopIfDestinationReached(destination);

        updated = true;
    }

    return updated;
}

void TrainHead::stopIfDestinationReached(const WorldPoint &destinationPt) {
    WorldPoint currentPos(pos_);
    if(abs(destinationPt.y - currentPos.y) < 4) {
        dest_path_.pop_front();
        speed_ = 0;
    }
}

void TrainHead::appendTrainBody(TrainBody *pTrainBodyToAdd) {
    TrainBody *pBody = this;
    while (pBody->getNext() != NULL) {
        pBody = pBody->getNext();
    }
    pBody->setNext(pTrainBodyToAdd);
}
