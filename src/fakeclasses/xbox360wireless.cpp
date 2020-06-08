#define MAKE_CONTROLLER_ID( nVID, nPID )	(unsigned int)( nVID << 16 | nPID )

#include "xbox360wireless.h"
//#include "SDL-mirror/src/joystick/SDL_sysjoystick.h"




Xbox360Wireless::Xbox360Wireless()
{
    SDL_Joystick Xbox360;

    Xbox360.instance_id = 0;
    Xbox360.name = name();
    Xbox360.player_index = 0;
    Xbox360.guid = joyGUID();
    Xbox360.naxes = 6;
    Xbox360.nhats = 0;
    Xbox360.nbuttons = 15;
   // Xbox360.nballs = 0;

    if (Xbox360.naxes > 0) {
        Xbox360.axes = (SDL_JoystickAxisInfo *) SDL_calloc(Xbox360.naxes, sizeof(SDL_JoystickAxisInfo));
    }
    if (Xbox360.nhats > 0) {
        Xbox360.hats = (Uint8 *) SDL_calloc(Xbox360.nhats, sizeof(Uint8));
    }
    //if (Xbox360.nballs > 0) {
   //     Xbox360.balls = (struct balldelta *) SDL_calloc(Xbox360.balls, sizeof(*Xbox360.balls));
   // }
    if (Xbox360.nbuttons > 0) {
        Xbox360.buttons = (Uint8 *) SDL_calloc(Xbox360.nbuttons, sizeof(Uint8));
    }

    Xbox360.attached = SDL_TRUE;
    Xbox360.is_game_controller = SDL_TRUE;
    Xbox360.epowerlevel = SDL_JOYSTICK_POWER_MEDIUM;
    Xbox360.ref_count = 1;
    Xbox360.next = NULL;

    // _SDL_JoystickDriver - it's faked class so there is no need to use detection from system level

    result = &Xbox360;

}


Xbox360Wireless::~Xbox360Wireless()
{
    SDL_free(result->name);

    /* Free the data associated with this joystick */
    SDL_free(result->axes);
    SDL_free(result->hats);
    SDL_free(result->balls);
    SDL_free(result->buttons);
    SDL_free(result);
}

char *Xbox360Wireless::GUID()
{
    return "030000005e040000e002000000000000";
}

char *Xbox360Wireless::name()
{
    return "Xbox Wireless Controller";
}

unsigned int Xbox360Wireless::deviceId()
{
    return MAKE_CONTROLLER_ID( 0x045e, 0x0291 );
}

int Xbox360Wireless::controllerType()
{
    return 31; // k_eControllerType_XBox360Controller
}

SDL_JoystickType Xbox360Wireless::joystickType()
{
    return SDL_JoystickType::SDL_JOYSTICK_TYPE_GAMECONTROLLER;
}

SDL_JoystickGUID Xbox360Wireless::joyGUID()
{
    return SDL_JoystickGetGUIDFromString(GUID());
}

SDL_Joystick* Xbox360Wireless::getResult()
{
    return result;
}


