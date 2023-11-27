
#include "servowrapper.h"


ServoWrapper::ServoWrapper(int port, int upPos, int downPos, int defaultPos) {
            this->port = port;
            this->upPos = upPos;
            this->downPos = downPos;
            servo.setPeriodHertz(50);  
            servo.attach(port, 500, 2400);
            servo.write(defaultPos);
};

ServoWrapper::ServoWrapper() {
    this->port = 0;
    this->upPos = 0;
    this->downPos = 0;
}

void ServoWrapper::setToUp() {
    servo.write(this->upPos);
}

void ServoWrapper::setToDown() {
    servo.write(this->downPos);
}

void ServoWrapper::write(int position) {
    servo.write(position);
}

void ServoWrapper::invert() {
    if (this->position) {
        setToUp();
    } else {
        setToDown();
    }
    position = !position;
}




