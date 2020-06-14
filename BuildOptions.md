# Build Options for CMake

There are a few application specific options that can be used when running
cmake to build antimicroX. The following file will attempt to list some of those
options and describe their use in the project.


## Universal Options

    -DUPDATE_TRANSLATIONS

Default: OFF. Set updateqm target to call lupdate in order to update 
translation files from source.

    -DTRANS_KEEP_OBSOLETE
    
Default: OFF. Do not specify -noobsolete option when calling lupdate 
command for qm files. -noobsolete is a method for getting rid of obsolete text entries

    -DWITH_TESTS
    
Default: OFF. Allows for the launch of test sources with unit tests



## Linux Options

    -DAPPDATA

Default: ON. Build the project with AppData support.

    -DWITH_UINPUT

Default: ON. Compile the program with uinput support.

    -DWITH_X11

Default: ON. Compile the program with X11 support.

    -DWITH_XTEST

Default: ON. Compile the program with XTest support.

