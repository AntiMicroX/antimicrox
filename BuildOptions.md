# Build Options for CMake

There are a few application specific options that can be used when running
cmake to build antimicro. The following file will attempt to list some of those
options and describe their use in the project.

## Universal Options

    -DUPDATE_TRANSLATIONS

Default: OFF. Set updateqm target to call lupdate in order to update 
translation files from source.

    -DUSE_SDL_2

Default: ON. Compile the program with SDL 2 instead of SDL 1.2.

## Linux Options

    -DAPPDATA

Default: OFF. Build the project with AppData support.

    -DWITH_UINPUT

Default: OFF. Compile the program with uinput support.

    -DWITH_X11

Default: ON. Compile the program with X11 support.

    -DWITH_XTEST

Default: ON. Compile the program with XTest support.


## Windows Options

    -DPORTABLE_PACKAGE

Default: OFF. Compile the program with extra changes used for containing the
final program to a single directory.

    -DWITH_VMULTI

Default: OFF. Compile the program with support for the vmulti driver.

    -DPERFORM_SIGNING

Default: OFF. This option is only included for testing. It should not be used 
currently.

