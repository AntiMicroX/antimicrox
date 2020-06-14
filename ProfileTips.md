Profile Tips
============

## Sticks

### Dead Zones

The XInput standard of 8,000 will typically be used in profiles for analog
sticks. For an Xbox 360 controller, that value tends to be the best minimum
value to ensure that no unintentional events are produced due to what position
a stick will be centered at.

When using a different controller, you will likely want to play with the value
of the dead zone used for the analog sticks and find what works best for
your controller. As an example, when I make profiles for my Logitech F310
gamepad, I find that using a dead zone value of 2,000 for the analog sticks
works best.

### Left Stick (Keyboard)

The default diagonal range of 45 degrees is recommended when mapping keyboard
keys to an analog stick when using the **standard** stick mode. If only one
action should ever occur at a time and the diagonal direction doesn't matter as
much, you might want to look into **4-way cardinal** stick mode. That can be
useful for navigating menus or mapping weapon hotkeys to an analog stick.

Some games will overreact when mapping WASD or other keyboard keys to an
analog stick. This is mainly due to how quickly directions can be changed
especially if you have the stick positioned right around where a diagonal zone
resides. If you encounter problems in game or you just want to make movement a
little more definitive, you can adjust the **stick delay** value used for that
stick. That will cause direction changes to be slightly delayed. Even setting
the value to 0.01 seconds can greatly increase control in some games.

### Right Stick (Mouse)

To allow for better mouse control with an analog stick, it is recommended
that you use a diagonal range of at least 65 degrees instead of the
normal 45 degree value. If you want more twitchy mouse control, you will
likely want to set the diagonal range of the stick to 89 degrees or 90 degrees.
This will minimize the range used for the absolute cardinal directions.

Another tip that will allow for more twitchy mouse control is related
to the dead zone used for the analog stick. Decreasing the value used for
the stick dead zone can help make mouse control feel more responsive than
when using the standard value of 8,000 even when using an Xbox 360 controller.

Based on my experience, even decreasing the value to 6,000 can make a
huge difference. The big compromise is that there might be some mouse
cursor drift if the analog stick does not center back to the assigned dead
zone. On my Xbox 360 controller, I find that the stick can get stuck slightly
in the southwest region of the stick; the other directions don't have this
issue. However, the mouse cursor drift is very minimal and you can easily
compensate while playing a game. It doesn't hinder gameplay and the benefit
obtained far outweights the minor problem.

One final thing that you can do to make mouse movement more twitchy is to
enable extra acceleration for mouse events. Changing the **extra duration** of
acceleration will have the biggest impact on mouse movement. Increasing the
maximum extra duration of the acceleration will loosen the mouse movement.
Changing the **multiplier** is another option that will change how mouse
movement behaves. One final option that can be changed is to use a different
curve for extra acceleration. **Linear** is still the default setting since it
was the only option available for many versions but I find myself using
**Ease Out Quad** lately. Besides altering the final multiplier used, it also
changes the final duration experienced.

With all the options available for extra acceleration, you will have an easier
time performing a quick 180 degree turn while still having precision on the low
end of a stick for aiming at targets.

## Action Names

It is generally recommended that you specify action names when making profiles
that you will share with other people. This will allow other people to more
easily decipher what buttons are used for in a profile. Specifying action names
can also be a helpful reminder for yourself in order to document a more complex
action.
