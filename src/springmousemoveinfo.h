#ifndef SPRINGMOUSEMOVEINFO_H
#define SPRINGMOUSEMOVEINFO_H

namespace PadderCommon {
    typedef struct springModeInfo
    {
        // Displacement of the X axis
        double displacementX;
        // Displacement of the Y axis
        double displacementY;

        // Width and height of the spring mode box
        unsigned int width;
        unsigned int height;
        // Should the cursor not move around the center
        // of the screen.
        bool relative;
    } springModeInfo;
}

#endif // SPRINGMOUSEMOVEINFO_H
