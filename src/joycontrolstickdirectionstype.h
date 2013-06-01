#ifndef JOYCONTROLSTICKDIRECTIONSTYPE_H
#define JOYCONTROLSTICKDIRECTIONSTYPE_H

class JoyStickDirectionsType {
public:
    enum JoyStickDirections {
        StickCentered = 0, StickUp = 1, StickRight = 3,
        StickDown = 5, StickLeft = 7, StickRightUp = 2,
        StickRightDown = 4, StickLeftUp = 8, StickLeftDown = 6
    };
};

#endif // JOYCONTROLSTICKDIRECTIONSTYPE_H
