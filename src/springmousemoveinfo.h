#ifndef SPRINGMOUSEMOVEINFO_H
#define SPRINGMOUSEMOVEINFO_H

namespace PadderCommon {
    typedef struct springModeInfo
    {
        double displacementX;
        double displacementY;
        unsigned int width;
        unsigned int height;
        bool relative;
    } springModeInfo;
}

#endif // SPRINGMOUSEMOVEINFO_H
