#ifndef XBOX360WIRELESS_H
#define XBOX360WIRELESS_H

#include <SDL2/SDL_joystick.h>


typedef struct _SDL_JoystickAxisInfo
{
    Sint16 initial_value;       /* Initial axis state */
    Sint16 value;               /* Current axis state */
    Sint16 zero;                /* Zero point on the axis (-32768 for triggers) */
    SDL_bool has_initial_value; /* Whether we've seen a value on the axis yet */
    SDL_bool sent_initial_value; /* Whether we've sent the initial axis value */
} SDL_JoystickAxisInfo;

struct _SDL_Joystick
{
    SDL_JoystickID instance_id; /* Device instance, monotonically increasing from 0 */
    char *name;                 /* Joystick name - system dependent */
    int player_index;           /* Joystick player index, or -1 if unavailable */
    SDL_JoystickGUID guid;      /* Joystick guid */

    int naxes;                  /* Number of axis controls on the joystick */
    SDL_JoystickAxisInfo *axes;

    int nhats;                  /* Number of hats on the joystick */
    Uint8 *hats;                /* Current hat states */

    int nballs;                 /* Number of trackballs on the joystick */
    struct balldelta {
        int dx;
        int dy;
    } *balls;                   /* Current ball motion deltas */

    int nbuttons;               /* Number of buttons on the joystick */
    Uint8 *buttons;             /* Current button states */

    SDL_bool attached;
    SDL_bool is_game_controller;
    SDL_bool delayed_guide_button; /* SDL_TRUE if this device has the guide button event delayed */
    SDL_bool force_recentering; /* SDL_TRUE if this device needs to have its state reset to 0 */
    SDL_JoystickPowerLevel epowerlevel; /* power level of this joystick, SDL_JOYSTICK_POWER_UNKNOWN if not supported */
    struct _SDL_JoystickDriver *driver;

    struct joystick_hwdata *hwdata;     /* Driver dependent information */

    int ref_count;              /* Reference count for multiple opens */

    struct _SDL_Joystick *next; /* pointer to next joystick we have allocated */
};

class Xbox360Wireless
{
    
public:
    Xbox360Wireless();
    ~Xbox360Wireless();

    char* GUID();
    char* name();
    unsigned int deviceId();
    int controllerType();
    SDL_JoystickType joystickType();
    SDL_JoystickGUID joyGUID();
    SDL_Joystick* getResult();
    
private:
    SDL_Joystick* result;

};


#endif
