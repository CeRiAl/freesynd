#include "train.h"

TrainHead::TrainHead(uint16 anId, uint8 aType) : TrainBody(anId, aType) {
}

TrainHead::~TrainHead() {

}

bool TrainHead::animate(int elapsed) {
    return false;
}

TrainBody::TrainBody(uint16 anId, uint8 aType) : Vehicle(anId, aType, -1) {

}

TrainBody::~TrainBody() {
}
