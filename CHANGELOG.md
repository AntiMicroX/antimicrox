# Changelog

## [3.5.1](https://github.com/AntiMicroX/antimicrox/tree/3.5.1) (2025-01-27)

**Fixed bugs:**

- Fix appId on Wayland [pull 1100](https://github.com/AntiMicroX/antimicrox/pull/1100)
- Wrong install dependencies on a Debian build with QT6 [\#1104](https://github.com/AntiMicroX/antimicrox/issues/1104)

**Notable changes:**

- New translation: Tamil
- Update translation for: Spanish, French, Japanese, Finnish, 
- Create deb release dor Ubuntu 24.04
- Update Windows and Linux SDL mappings
- Add "game" category to desktop file [pull 1085](https://github.com/AntiMicroX/antimicrox/pull/1085) (by [Fr-Dae](https://github.com/Fr-Dae))
- Cleanup in logs
- Add proper note informing about lack of Wayland support for Auto Profiles
- Cleanup in dependencies
- Fix some typos and update docs (by [zturtleman](https://github.com/zturtleman) )

**Implemented enhancements:**

- Reenabled `--next` option [\#189](https://github.com/AntiMicroX/antimicrox/issues/189)

## [3.5.0](https://github.com/AntiMicroX/antimicrox/tree/3.5.0) (2024-10-31)

**Fixed bugs:**

- Horizontal mouse wheel is too fast when set on stick [\#1024](https://github.com/AntiMicroX/antimicrox/issues/1024) (by [sk2449](https://github.com/sk2449))
- Fix binding labels for Wheel Left/Right buttons [pull 1038](https://github.com/AntiMicroX/antimicrox/pull/1038) (by [sk2449](https://github.com/sk2449))
- Set change sometimes returns to wrong set [\#1050](https://github.com/AntiMicroX/antimicrox/issues/1050)
- Wrong Keyboard Input on Assignment (Wayland) [\#300](https://github.com/AntiMicroX/antimicrox/issues/300)

**Notable changes:**

- Update translation for Finnish, Dutch, Russian
- New translation: Indonesian
- Fixed Builds for QT6 on Windows
- Switched to QT6 on Windows Release Build

## [3.4.1](https://github.com/AntiMicroX/antimicrox/tree/3.4.1) (2024-08-10)

**Fixed bugs:**

- Fix issues with accelerometer stability [pull 1011](https://github.com/AntiMicroX/antimicrox/pull/1011) (by [pepper-jelly](https://github.com/pepper-jelly))
-  Issue with float value for "Auto Reset Cycle After" [\#1002](https://github.com/AntiMicroX/antimicrox/issues/1002) (by [pepper-jelly](https://github.com/pepper-jelly))
-  Accelerometer triggers when switching sets [\#991](https://github.com/AntiMicroX/antimicrox/issues/991) (by [pepper-jelly](https://github.com/pepper-jelly))

**Notable changes:**

- Link AntimicroX repository with issue hunting platform: [Polar.sh](https://polar.sh/)
- Bump SDL version to 2.30.6
- Refactor outdated pieces of code [pull 948](https://github.com/AntiMicroX/antimicrox/pull/948) [pull 950](https://github.com/AntiMicroX/antimicrox/pull/950)
- Update more universal QT libraries detection [pull 957](https://github.com/AntiMicroX/antimicrox/pull/957)
- Allow easy building with QT6 and ensure compatability [pull 919](https://github.com/AntiMicroX/antimicrox/pull/919), [pull 972](https://github.com/AntiMicroX/antimicrox/pull/972)
- Drop Windows XP support [pull 980](https://github.com/AntiMicroX/antimicrox/pull/980)
- Bump minimal QT version requirement to 5.10 [\#996](https://github.com/AntiMicroX/antimicrox/issues/996)


**Implemented enhancements:**

- Updated translations: Chinese, Portugese, German, French and Finnish
- Added Catalan language

And a lot of minor fixes and cleanups linked with QT6 and not only.

## [3.4.0](https://github.com/AntiMicroX/antimicrox/tree/3.4.0) (2024-03-10)

**Fixed bugs:**

- Fix attaching window to a pinned launcher in Linux [pull 886](https://github.com/AntiMicroX/antimicrox/pull/886) (by [mmmaisel](https://github.com/mmmaisel))
- Fixing builds with X11=OFF [pull 927](https://github.com/AntiMicroX/antimicrox/pull/927) (by [sjoblomj](https://github.com/sjoblomj))

**Notable changes:**

- Bump SDL version to 2.30.1
- Updated translations
- New translations: Croatian, Turkish, Persian, Vietnamese
- Fix a lot of deprecation warnings and other code changes required to migrate to QT6 [\#446](https://github.com/AntiMicroX/antimicrox/issues/446)

## [3.3.4](https://github.com/AntiMicroX/antimicrox/tree/3.3.4) (2023-06-03)

**Implemented enhancements:**

- Update translations: Portugese(Brazil), Russian, Norwegian Bokmal
- Add translations: Swedish

**Fixed bugs:**

- Fix and refactor adding default autoprofile  [\#727](https://github.com/AntiMicroX/antimicrox/issues/727)
- Fix GUI does not recognize super + action [\#749](https://github.com/AntiMicroX/antimicrox/issues/749)
- Fix /dev/uinput static device node access [pull 781](https://github.com/AntiMicroX/antimicrox/pull/781) (by [lnussel](https://github.com/lnussel))
- Fix building for SDL version between 2.0.12 and 2.0.16 [\#788](https://github.com/AntiMicroX/antimicrox/issues/788)


**Notable changes:**

- Build AppImage package on Ubuntu 20.04
- Bump SDL version for AppImage and Windows from 2.0.20 to 2.26.5

## [3.3.3](https://github.com/AntiMicroX/antimicrox/tree/3.3.3) (2023-01-30)

**Implemented enhancements:**

- Add checkbox enabling numeric keypad in ButtonEditDialog [\#168](https://github.com/AntiMicroX/antimicrox/issues/168)
- Add MSVC cmake support. [\#667](https://github.com/AntiMicroX/antimicrox/pull/667) (by [nitz](https://github.com/nitz))
- Update translations: Spanish, Korean, Russian
- Add translations: Rusyn, Portugese(Brazil)

**Fixed bugs:**

- Name mismatch in set selector list [\#655](https://github.com/AntiMicroX/antimicrox/issues/655)
- Fix broken loading recently used profile for Windows 11 [\#632](https://github.com/AntiMicroX/antimicrox/issues/632) (by [nitz](https://github.com/nitz))
- Shebang ignored in execute bind [\#699](https://github.com/AntiMicroX/antimicrox/issues/699)
- Mouse spring mode not working in wayland session [\#521](https://github.com/AntiMicroX/antimicrox/issues/521)

**Notable changes:**

- Some code refactors and docs updates
- Logs improvements

## [3.3.2](https://github.com/AntiMicroX/antimicrox/tree/3.3.2) (2022-11-21)

**Implemented enhancements:**

- Use proper theme-based color for tab text [\#583](https://github.com/AntiMicroX/antimicrox/pull/583)
- Show joystick battery level in tab [\#596](https://github.com/AntiMicroX/antimicrox/pull/596)

**Fixed bugs:**

- Fix broken obtaining path of active window by auto profile [\#592](https://github.com/AntiMicroX/antimicrox/pull/592)
- Fix auto profile dialog does not populate title and application fields
- Fix misspeled __BIG_ENDIAN constant [\#609](https://github.com/AntiMicroX/antimicrox/pull/599)
- Fix advanced settings dialog freezes when pressing join on Linux [\#609](https://github.com/AntiMicroX/antimicrox/pull/609)
- Remove disabling Auto-profile title box [\#619](https://github.com/AntiMicroX/antimicrox/pull/619)

**Notable changes:**

- don't minimize to taskbar by default on Linux [\#611](https://github.com/AntiMicroX/antimicrox/pull/611)

**Full Changelog**: https://github.com/AntiMicroX/antimicrox/compare/3.3.1...3.3.2

## [3.3.1](https://github.com/AntiMicroX/antimicrox/tree/3.3.1) (2022-10-24)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.3.0...3.3.1)

Hotfix release reverting PR [\#555](https://github.com/AntiMicroX/antimicrox/pull/555) which can cause segfaults.

## [3.3.0](https://github.com/AntiMicroX/antimicrox/tree/3.3.0) (2022-10-24)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.2.5...3.3.0)


**Implemented enhancements:**

- Add support for DualSense trigger haptic feedback [\#503](https://github.com/AntiMicroX/antimicrox/pull/503) (by [mmmaisel](https://github.com/mmmaisel))
- Add Portable Windows build [\#523](https://github.com/AntiMicroX/antimicrox/pull/523) (by [AntumDeluge](https://github.com/AntumDeluge))

**Fixed bugs:**

- Broken auto profile saving for Windows  [\#492](https://github.com/AntiMicroX/antimicrox/pull/492)
- Selection "None" Preset does not change button labels [\#513](https://github.com/AntiMicroX/antimicrox/pull/513)
- Broken spring mouse Event on Windows [\#518](https://github.com/AntiMicroX/antimicrox/pull/518)
- Multiple problems caused by invalid thread handling [\#573](https://github.com/AntiMicroX/antimicrox/pull/573)
  - Saving PS4 Controller setup stops set toggle from working
  - Saving Breaks Set Switching
  - Saving config breaks turbo
  - Warnings `QObject: Cannot create children for a parent that is in a different thread`

**Notable changes:**

- Fix some code warnings  [\#497](https://github.com/AntiMicroX/antimicrox/pull/497) [\#558](https://github.com/AntiMicroX/antimicrox/pull/558) [\#559](https://github.com/AntiMicroX/antimicrox/pull/559) (by [pktiuk](https://github.com/pktiuk) and [Stefan-TU](https://github.com/Stefan-TU))
- Remove some unused code [\#504](https://github.com/AntiMicroX/antimicrox/pull/504) (by [mmmaisel](https://github.com/mmmaisel))
- Limit number of logged events
- Include Visual Studio Code config files
- Some refactors and docs updates
- Update SDL mappings database
- Update translations: Korean, Ukrainian, Spanish

## [3.2.5](https://github.com/AntiMicroX/antimicrox/tree/3.2.5) (2022-07-18)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.2.4...3.2.5)

Test release containing changes implementing motion sensor support.

**Implemented enhancements:**

- Implement motion sensor support. [\#416](https://github.com/AntiMicroX/antimicrox/pull/416) (by [mmmaisel](https://github.com/mmmaisel))
- Allow storing calibration for multiple controllers in a profile [\#460](https://github.com/AntiMicroX/antimicrox/pull/460) (by [mmmaisel](https://github.com/mmmaisel))

**Fixed bugs:**

- Crash on close after copying set. [\#454](https://github.com/AntiMicroX/antimicrox/issues/454) (by [mmmaisel](https://github.com/mmmaisel))

**Notable changes:**

- Remove unused "Game Controller Mapping" button.

## [3.2.4](https://github.com/AntiMicroX/antimicrox/tree/3.2.4) (2022-06-11)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.2.3...3.2.4)

**Implemented enhancements:**

- Implement modifier button threshold zone [\#310](https://github.com/AntiMicroX/antimicrox/issues/310) (by [mmmaisel](https://github.com/mmmaisel)) - This allows mapping the analog range of a stick to walk/run in game without dedicated spring button.

**Fixed bugs:**

- Fix mouse calculations for multiple mice [\#435](https://github.com/AntiMicroX/antimicrox/pull/435) (by [mmmaisel](https://github.com/mmmaisel))
- Fix flickering JoyControlStickEditDialog layout [\#440](https://github.com/AntiMicroX/antimicrox/pull/440) (by [mmmaisel](https://github.com/mmmaisel))
- Implement proper offset and gain stick calibration [\#441](https://github.com/AntiMicroX/antimicrox/pull/441) (by [mmmaisel](https://github.com/mmmaisel))

**Notable changes:**

- Increase minimum size of ButtonEditDialog [\#439](https://github.com/AntiMicroX/antimicrox/pull/439) (by [mmmaisel](https://github.com/mmmaisel))
- Fixes in translation utulities
  - Rename flag TRANS_KEEP_OBSOLETE to TRANS_REMOVE_OBSOLETE to improve readability
  - Fix wrong argument for utility deleting obsolete translations [\#445](https://github.com/AntiMicroX/antimicrox/pull/445)
- Update translations:
  - Russian
  - Finnish
  - French
  - Chinese
  - Portuguese

## [3.2.3](https://github.com/AntiMicroX/antimicrox/tree/3.2.3) (2022-03-24)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.2.2...3.2.3)

**Fixed bugs:**

- Fixed bug: auto profile not saved when exe path is not defined [\#252](https://github.com/AntiMicroX/antimicrox/issues/252)

**Notable changes:**

- Added missing license for SDL_GameControllerDB
- Many updates in translations:
  - Added Finnish translation by [heidiwenger](https://github.com/heidiwenger)
  - Added Norwegian Bokmål translation bu [comradekingu](https://github.com/comradekingu)
  - Updated German, French, Italian and Polish translations
  - update translation files
- Always show battery level in joystick status window (even when unknown)
- Multiple refactors and cleanups removing deprecated code (kudos to [mmmaisel](https://github.com/mmmaisel)).

## [3.2.2](https://github.com/AntiMicroX/antimicrox/tree/3.2.2) (2022-02-24)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.2.1...3.2.2)

**Implemented enhancements:**

- Add support for special characters in Windows log files [\#363](https://github.com/AntiMicroX/antimicrox/pull/363)
- Change default working directory of launched executables [\#377](https://github.com/AntiMicroX/antimicrox/pull/377)

**Fixed bugs:**

- Fix typo (setting->settings) (by [jose1711](https://github.com/jose1711))
- Random Crashes during longer sessions [\#133](https://github.com/AntiMicroX/antimicrox/issues/133)
- File with gamecontroller mappings not found [\#364](https://github.com/AntiMicroX/antimicrox/pull/364)
- Empty application path in autoprofile [/#371](https://github.com/AntiMicroX/antimicrox/pull/371)

**Notable changes:**

- Enable checking for updates in Debian builds from GitHub
- Log improvements and cleanup [\#336](https://github.com/AntiMicroX/antimicrox/pull/336) [\#342](https://github.com/AntiMicroX/antimicrox/pull/342)
- Daemon mode support removed [\#345](https://github.com/AntiMicroX/antimicrox/pull/345)
- Remove unused "map" and "display" argument options [\#350](https://github.com/AntiMicroX/antimicrox/pull/350)
- Remove unused files With icons, resources and pertially implemented importing utility
- Use colourful antimicrox icon in tray by default [\#361](https://github.com/AntiMicroX/antimicrox/pull/361)
- Update building docs [\#370](https://github.com/AntiMicroX/antimicrox/pull/370) (by [zpangwin](https://github.com/zpangwin))

## [3.2.1](https://github.com/AntiMicroX/antimicrox/tree/3.2.1) (2022-01-03)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.2.0...3.2.1)

**Implemented enhancements:**

- Add note about available update (enabled in Windows) [\#325](https://github.com/AntiMicroX/antimicrox/issues/325)
- Add database of SDL gamepad mappings (more gamepads will be enabled by default) [\#296](https://github.com/AntiMicroX/antimicrox/issues/296)
- Add proper printing information about connected devices to logs [\#282](https://github.com/AntiMicroX/antimicrox/issues/282)
- Apply proper theme to Windows version of app [\#279](https://github.com/AntiMicroX/antimicrox/issues/279)
- Add handling SIGABRT \(with printing stack\) [\#281](https://github.com/AntiMicroX/antimicrox/pull/281)
- Set theme for Windows [\#317](https://github.com/AntiMicroX/antimicrox/pull/317)
- Update QT version used in Windows Release to 5.15 [\#326](https://github.com/AntiMicroX/antimicrox/pull/326)

**Fixed bugs:**

- Crashes linked with deleting child objects [\#320](https://github.com/AntiMicroX/antimicrox/issues/320)
- Broken Auto Profile list on Windows [\#278](https://github.com/AntiMicroX/antimicrox/issues/278)
- Auto Profile Wrongly Shows Languages [\#273](https://github.com/AntiMicroX/antimicrox/issues/273)
- Missing .dll files after Windows installation [\#268](https://github.com/AntiMicroX/antimicrox/issues/268)
- Missing autoprofile settings tab in Windows [\#277](https://github.com/AntiMicroX/antimicrox/pull/277)

**Notable changes:**

- Add note about lack of AutoProfiles for Wayland users [\#314](https://github.com/AntiMicroX/antimicrox/issues/314)
- Add message informaing about solutions in case of problems with uinput [\#248](https://github.com/AntiMicroX/antimicrox/pull/248)
- Log improvements [\#322](https://github.com/AntiMicroX/antimicrox/pull/322) [\#336](https://github.com/AntiMicroX/antimicrox/pull/336)
- Disable calibration for devices without joysticks [\#283](https://github.com/AntiMicroX/antimicrox/pull/283)
- Clarification of names and --profile flag description [\#305](https://github.com/AntiMicroX/antimicrox/pull/305)

**Others:**

- Update copyright date [\#334](https://github.com/AntiMicroX/antimicrox/pull/334)
- Cleanup daemon launching [\#331](https://github.com/AntiMicroX/antimicrox/pull/331)
- Make Changelog links clickable [\#327](https://github.com/AntiMicroX/antimicrox/pull/327)
- Print descriptions of devices in logs [\#319](https://github.com/AntiMicroX/antimicrox/pull/319)
- Add experimental support for clang [\#288](https://github.com/AntiMicroX/antimicrox/pull/288) ([avinal](https://github.com/avinal))
- CI: Add concurrency and remove redundant lines [\#272](https://github.com/AntiMicroX/antimicrox/pull/272) ([avinal](https://github.com/avinal))
- CI: Add CodeQL and infer static code analysis [\#289](https://github.com/AntiMicroX/antimicrox/issues/289) [\#287](https://github.com/AntiMicroX/antimicrox/issues/287)

## [3.2.0](https://github.com/AntiMicroX/antimicrox/tree/3.2.0) (2021-10-29)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.1.7...3.2.0)

First AntiMicroX release supporting Windows

**Implemented enhancements:**

- Prepare installer for Windows [\#237](https://github.com/AntiMicroX/antimicrox/issues/237)
- Handle segfaults and other crashes in code [\#235](https://github.com/AntiMicroX/antimicrox/issues/235)
- Create config for debug builds [\#218](https://github.com/AntiMicroX/antimicrox/issues/218)
- Allow unhiding app window when second instance of AntiMicroX is launched [\#257](https://github.com/AntiMicroX/antimicrox/pull/257)

**Fixed bugs:**

- Typo in udev rule name: 60-antimicrox-uinput.rules instead 60-antimcrox-uinput.rules? [\#204](https://github.com/AntiMicroX/antimicrox/issues/204)

**Notable merged pull requests:**

- Move gui-related files to separate directory [\#253](https://github.com/AntiMicroX/antimicrox/pull/253) ([pktiuk](https://github.com/pktiuk))
- Logging system improvements [\#250](https://github.com/AntiMicroX/antimicrox/pull/250) ([pktiuk](https://github.com/pktiuk))
- Some additional logs and notes [\#233](https://github.com/AntiMicroX/antimicrox/pull/233) [\#249](https://github.com/AntiMicroX/antimicrox/pull/249) ([pktiuk](https://github.com/pktiuk))
- Fix compile warnings [\#246](https://github.com/AntiMicroX/antimicrox/pull/246) [\#242](https://github.com/AntiMicroX/antimicrox/pull/242) ([pktiuk](https://github.com/pktiuk))
- Set homepage url for packages [\#245](https://github.com/AntiMicroX/antimicrox/pull/245) ([pktiuk](https://github.com/pktiuk))
- Create config for debug build [\#243](https://github.com/AntiMicroX/antimicrox/pull/243) ([pktiuk](https://github.com/pktiuk))
- Cmake cleanup [\#219](https://github.com/AntiMicroX/antimicrox/pull/219) ([pktiuk](https://github.com/pktiuk))

**Pull requests restoring Windows support:**

- Add Cpack NSIS config for building packages [\#262](https://github.com/AntiMicroX/antimicrox/pull/262) ([pktiuk](https://github.com/pktiuk))
- Restore full functionality of Windows build [\#225](https://github.com/AntiMicroX/antimicrox/pull/225) ([pktiuk](https://github.com/pktiuk))
- Restore Windows build [\#220](https://github.com/AntiMicroX/antimicrox/pull/220) [\#244](https://github.com/AntiMicroX/antimicrox/pull/244) ([pktiuk](https://github.com/pktiuk))
- Embed icon into windows executable [\#261](https://github.com/AntiMicroX/antimicrox/pull/261) ([pktiuk](https://github.com/pktiuk))
- Saves migration win [\#266](https://github.com/AntiMicroX/antimicrox/pull/266) ([pktiuk](https://github.com/pktiuk))
- Use -rdynamic only in Unix debug builds [\#259](https://github.com/AntiMicroX/antimicrox/pull/259) ([pktiuk](https://github.com/pktiuk))
- Setup Github Actions for Windows build [\#260](https://github.com/AntiMicroX/antimicrox/pull/260) ([avinal](https://github.com/avinal))

## [3.1.7](https://github.com/AntiMicroX/antimicrox/tree/3.1.7) (2021-09-10)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.1.6...3.1.7)

**Implemented enhancements:**

- Wayland support for keyboard emulation [\#32](https://github.com/AntiMicroX/antimicrox/issues/32)

**Fixed bugs:**

- Load save dialogue window won't show saved configs = can't open saves [\#123](https://github.com/AntiMicroX/antimicrox/issues/123)
- Segmentation Fault when change profile from command line [\#76](https://github.com/AntiMicroX/antimicrox/issues/76)

**Notable merged pull requests:**

- Include Appstream information into AppImage Build [\#201](https://github.com/AntiMicroX/antimicrox/pull/201) ([pktiuk](https://github.com/pktiuk))
- fix: Remove warning "QLocalSocket::waitForDisconnected\(\) is not allow… [\#200](https://github.com/AntiMicroX/antimicrox/pull/200) ([pktiuk](https://github.com/pktiuk))
- Crash-related fixes linked with memory management [\#194](https://github.com/AntiMicroX/antimicrox/pull/194) [\#196](https://github.com/AntiMicroX/antimicrox/pull/196) [\#198](https://github.com/AntiMicroX/antimicrox/pull/198) ([pktiuk](https://github.com/pktiuk))
- refactor: Improve log showing gamepad name [\#195](https://github.com/AntiMicroX/antimicrox/pull/195) ([pktiuk](https://github.com/pktiuk))
- refactor: Refactor executing commands [\#192](https://github.com/AntiMicroX/antimicrox/pull/192) ([pktiuk](https://github.com/pktiuk))
- Allow regular users using uinput [\#188](https://github.com/AntiMicroX/antimicrox/pull/188) ([pktiuk](https://github.com/pktiuk))
- feat: switch from xtest to uinput when wayland is detected [\#186](https://github.com/AntiMicroX/antimicrox/pull/186) ([pktiuk](https://github.com/pktiuk))

## [3.1.6](https://github.com/AntiMicroX/antimicrox/tree/3.1.6) (2021-08-14)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.1.5...3.1.6)

**Fixed bugs:**

- Saving logs into file doesn't work properly. [\#78](https://github.com/AntiMicroX/antimicrox/issues/78)
- Doesn't save my configurations/Wont auto load [\#70](https://github.com/AntiMicroX/antimicrox/issues/70)

**Notable closed issues:**

- Refactor logging system [\#177](https://github.com/AntiMicroX/antimicrox/issues/177)

**Other merged pull requests:**

- Add new logs associated with loading profile files [\#182](https://github.com/AntiMicroX/antimicrox/pull/182) ([pktiuk](https://github.com/pktiuk))
- Refactor arguments parsing [\#174](https://github.com/AntiMicroX/antimicrox/pull/174) ([pktiuk](https://github.com/pktiuk))
- docs: Add information about commandline for flatpak packages [\#173](https://github.com/AntiMicroX/antimicrox/pull/173) ([pktiuk](https://github.com/pktiuk))
- Added openSUSE instruction [\#158](https://github.com/AntiMicroX/antimicrox/pull/158) ([Mailaender](https://github.com/Mailaender))

## [3.1.5](https://github.com/AntiMicroX/antimicrox/tree/3.1.5) (2021-03-14)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.1.4...3.1.5)

Big thanks to @mirabilos for help with Debian packages migration and cleaning up project.

**Notable closed issues:**

- Migrate DEB packages to antimicrox [\#87](https://github.com/AntiMicroX/antimicrox/issues/87)
- Integrate builds marking with docs cmake and CI [\#143](https://github.com/AntiMicroX/antimicrox/pull/143)

**Other merged pull requests:**

- Allow cleaner marking package builds [\#143](https://github.com/AntiMicroX/antimicrox/pull/143) ([mirabilos](https://github.com/mirabilos))
- fix appdata formatting [\#142](https://github.com/AntiMicroX/antimicrox/pull/142) ([mirabilos](https://github.com/mirabilos))
- apply lintian-provided spelling fixes [\#141](https://github.com/AntiMicroX/antimicrox/pull/141) ([mirabilos](https://github.com/mirabilos))
- fix the manpage \(hyphen as minus sign\) [\#140](https://github.com/AntiMicroX/antimicrox/pull/140) ([mirabilos](https://github.com/mirabilos))
- fix: change deb arch prefix for building AppImage [\#135](https://github.com/AntiMicroX/antimicrox/pull/135) ([pktiuk](https://github.com/pktiuk))

## [3.1.4](https://github.com/AntiMicroX/antimicrox/tree/3.1.4) (2021-01-11)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.1.3...3.1.4)

Kudos to @Janfel for PR!

**Implemented enhancements:**

- Remove qglobalshortcuts [\#106](https://github.com/AntiMicroX/antimicrox/issues/106)
- File cleanup [\#104](https://github.com/AntiMicroX/antimicrox/issues/104)
- Cmakefile fixes [\#102](https://github.com/AntiMicroX/antimicrox/issues/102)
- Add dark tray icon [\#66](https://github.com/AntiMicroX/antimicrox/issues/66)
- Icons overhaul [\#61](https://github.com/AntiMicroX/antimicrox/issues/61)
- Automate generating changelogs [\#10](https://github.com/AntiMicroX/antimicrox/issues/10)

**Fixed bugs:**

- Clear shortcut not working [\#116](https://github.com/AntiMicroX/antimicrox/issues/116)
- Assigning Text Entry to button doesn't assign proper string. [\#79](https://github.com/AntiMicroX/antimicrox/issues/79)
- The uninstallation leaves several folders and files behind \(cmake\) [\#68](https://github.com/AntiMicroX/antimicrox/issues/68)

**Notable closed issues:**

- Remove antilib shared object because it's not really shared! [\#98](https://github.com/AntiMicroX/antimicrox/issues/98)

**Other merged pull requests:**

- feat: improve mime type definition [\#132](https://github.com/AntiMicroX/antimicrox/pull/132) ([Janfel](https://github.com/Janfel))
- docs: update building and installation instructions [\#129](https://github.com/AntiMicroX/antimicrox/pull/129) ([pktiuk](https://github.com/pktiuk))
- feat: antimicro config migration [\#120](https://github.com/AntiMicroX/antimicrox/pull/120) ([gombosg](https://github.com/gombosg))

## [3.1.3](https://github.com/AntiMicroX/antimicrox/tree/3.1.3) (2020-11-15)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.1.2...3.1.3)

**Fixed bugs:**

- ARM build failure [\#80](https://github.com/AntiMicroX/antimicrox/issues/80)
- AntiMicroX system tray icon not appearing in flatpak package [\#58](https://github.com/AntiMicroX/antimicrox/issues/58)
- Icons in tray menu only showing up as "X" [\#56](https://github.com/AntiMicroX/antimicrox/issues/56)
- Outdated information in the "Credits" tab in the "About" window [\#67](https://github.com/AntiMicroX/antimicrox/issues/67)

**Notable closed issues:**

- Consistent style of code [\#25](https://github.com/AntiMicroX/antimicrox/issues/25)

**Other merged pull requests:**

- Icons overhaul [\#73](https://github.com/AntiMicroX/antimicrox/pull/73) ([pktiuk](https://github.com/pktiuk))
- Fix: Use system icons in common cases to avoid incompatibility issues [\#62](https://github.com/AntiMicroX/antimicrox/pull/62) ([pktiuk](https://github.com/pktiuk))
- General cleanup of repository [\#18](https://github.com/AntiMicroX/antimicrox/pull/18) ([pktiuk](https://github.com/pktiuk))

## [3.1.2](https://github.com/AntiMicroX/antimicrox/tree/3.1.2) (2020-10-02)

[Full Changelog](https://github.com/AntiMicroX/antimicrox/compare/3.1.1...3.1.2)

**Fixed bugs:**

- Right click - Turbo and Clear not working properly [\#7](https://github.com/AntiMicroX/antimicrox/issues/7)
- Wrong order for setting sets [\#6](https://github.com/AntiMicroX/antimicrox/issues/6)

**Notable closed issues:**

- Fedora resubmission [\#29](https://github.com/AntiMicroX/antimicrox/issues/29)
- Flatpak packaging [\#9](https://github.com/AntiMicroX/antimicrox/issues/9)

**Other merged pull requests:**

- refactor: remove Dockerfiles [\#52](https://github.com/AntiMicroX/antimicrox/pull/52) ([gombosg](https://github.com/gombosg))

2020-09-15 Gergely Gombos <gombosg@disroot.org>

Version 3.1.1

- AppStream XML file revision

- New screenshots

- Add 128x128 icon

- Prepare for Flathub submission

2020-09-09 Paweł Kotiuk <kotiuk@zohomail.eu>

Version 3.1.0

- Updated documentation connected with migration to new repository

- Add instruction and config for building Flatpak packages - Gergely "gombosg" Gombos

- Multiple improvements of packaging for Debian/Ubuntu

- Ensure consistent naming across entire project

- Rename used directories and files from antimicroX to antimicrox

- First official AppImage release

2018-16-06 Jagoda Gorska <juliagoda.pl@protonmail.com>

Version 3.0.1

- Works with old antimicro profiles

- Added small changes (in instructions, comments, etc.)

- Added running in tray from right button

Version 3.0

- Added qt directives for compatibility with older versions of qt5

- Arch Linux packages of application of frealgagu and sir_lucjan

  Link1: https://aur.archlinux.org/packages/antimicrox/

  Link2: https://aur.archlinux.org/packages/antimicrox-git/

- Deb package of Paweł "pktiuk" Kotiuk

  Link1: https://github.com/pktiuk

  Link2: https://github.com/juliagoda/antimicroX/pull/120

  Link3: https://github.com/juliagoda/antimicroX/pull/103

- Rpm package of Gergely "gombosg" Gombos meanwhile

  Link1: https://github.com/gombosg

  Link2: https://github.com/juliagoda/antimicroX/issues/118

- Created and added dockerfiles to containers - look README

  Link1: https://hub.docker.com/r/juliagoda/antimicrox

- Changed icon for antimicroX

- Added F13-F24 buttons to "Others" only if they are detected on keyboard

- Added more adjusted debug/info/warning outputs to log settings

- Added new functionality - joining and splitting slots

- Added new functionality - deletion of more slots at once

- Added new functionality - insertion of more slots at once

- Added new functionality - One choice of execution paths and its arguments for multiple slots

- Added new functionality - One choice of profile for multiple slots

- Added new functionality - attaching numeric keypad keys thanks to option in settings

- Added new functionality - global shortcuts for quitting application from settings

- Added support for the same model of gamepads

- Updated german, english and polish translation files

# Fixed issue #92

- Fixed issue #97

- Fixed issue #90

- Fixed issue #98

- Fixed issue #101

- Fixed issue #107

- Fixed issue #108

- Fixed issue #109

- Fixed issue #113

- Fixed issue #112

- Fixed issue #114

- Fixed iisue #116

Version 2.25

- renamed AntiMicro into antimicroX

- added detection of scripts - fixed unfinished JoyExecute slot

- added menu icons and shortcuts

- removed unused files and directives

- removed majority of deprecated code

- updated information about application

- sorted and cleaned headers in header files

- unlocked multiple parameters for command line

- added notifications about battery level

- added guard conditions

Version 2.24.2

- Fixed bug with too many "emit propertyUpdated()"

- Split JoyButtonSlot class into JoyButtonSlotXml and the original

- Created template class for JoyDPad and VDPad xml

- Split GameControllerDPad class into GameControllerDPadXml and the original

- Replaced for loops with iterators and functional mappers

- Replaced for loops with hashiterators

- Added functions about vendor and product information

- Changed guid to uniqueID

Version 2.24.1

- Changed images and screenshots

- Updated Documents (README, _.txt files, _.md, \*.mm files)

- Updated resources

- Created index.theme

- Changed destination directories for icons, images, index.theme and other resources

- Translated new text entries

- Added detection of window partial title

- Added reset settings option

- Created smaller functions in part of classes

- Split part of classes into new xml classes

- Added com.github.juliagoda.antimicro.json for Flatpak (building and installation on Linux)

- Cppcheck Analysis - fixed part of warnings

- Added Comparing to nullptr instead of NULL and smart pointers

- Changed if else to switch statements for int data types

- First preparation for unit tests (app split into library (antilib) and executable (antimicro))

- Updated calibration - reset old values during calibration startup

- Fixed part of memory leaks with Memcheck (application exit)

- Fixed bug in gamecontrollermappingdialog class

- Fixed segmentation fault for activating Turbo option

- Unblocked delay sliders

Version 2.24

- Added calibration

- Ported Qt4 to Qt5

- Removed Qt4 support

2016-11-05 Jeff Backus <jeff@jsbackus.com>

Version 2.23

- Fixed portable version. (issue #37)

- Added ability to specify logging level and file in config dialog. (issue #50)

- Updated build system to find libraries on Windows systems instead of using
  explicit paths. (issue #43)

- Added relevant window information to debug messages related to auto profiles.
  (issue #46)

- Fixed issue with anything in profile name after first period being truncated.
  (issue #70)

- Fixed issues with SDL 2.0.5 on all platforms. (issue #71)

- Added support for additional small-case Latin characters when using Xtest.
  (issue #49)

- Added inclusion of README-SDL.txt when building Windows distributables.

2016-07-30 Jeff Backus <jeff@jsbackus.com>

Version 2.22

- Fixed a segfault on startup issue affecting some Linux distributions

- Fixed issues with SDL 2.0.4 on Windows.

- Updated documentation and URLs in code to note transition in
  management.

- Updated translations and added new ones.

- Added code to revert all virtual buttons to beginning of slot list
  before a set switch.

- Build system updates

- Fixed a bug where invalid joystick was inserted into device list.

- Modified to hide turbo mode combobox for regular buttons

- Changed to allow blank config for auto profile entries again.

- Fixed matching by process filename in AutoProfileWatcher

2016-01-10 Travis Nickles <nickles.travis@gmail.com>

Version 2.21

- Changed unplug routine slightly. The old behavior was slightly wrong anyway
  but the old behavior did not work with SDL 2.0.4.

- Custom Qt builds are being used for the 64 bit Windows build and noSSE
  Windows build.

- Updated Windows build to use Qt 5.5.1.

- Updated Windows build to use SDL 2.0.4. This should clear up a couple
  of problems for people running Windows 10.

- Changes to some acceleration curves. Both the output values and the input
  offsets were changed to make mouse movement looser.

- Removed First Run Wizard from program. In the end, that wizard was causing
  more problems for other users than it was worth.

- Add arguments property for Execute slots.

- Allow a real absolute mouse to be used with uinput support. Previously,
  relative mouse movement was being used to fake an absolute position for the
  mouse pointer. The WiimoteGlue project provided a way to allow uinput to
  send absolute mouse pointer events when using uinput.

- Fixed set changing with analog triggers.

2015-10-19 Travis Nickles <nickles.travis@gmail.com>

Version 2.20.2

- Change Enhanced Precision and Easing Quad/Cubic acceleration
  curves

- Skipped straight to 2.20.2 since version 2.20 actually had 2.20.1 marked
  as its version in the application

2015-10-10 Travis Nickles <nickles.travis@gmail.com>

Version 2.20

- Replace usage of QElapsedTimer with QTime for mouse movement calculations.

- Start using Qt 5 with Ubuntu Vivid package.

- Change XTest pointer acceleration when starting the program.

- Allow release slot to have a 0 ms interval. This is useful for people who
  use a gamepad poll rate less than 10 ms.

- Change minimize to tray code to work better with later versions of Gnome 3
  and Unity.

- Transfer travel distance axis changes when switching sets.

2015-09-29 Travis Nickles <nickles.travis@gmail.com>

Version 2.19.3

- Japanese translation provided by tou omiya.

- Make sure Game Controller Mapping dialog window reads raw joystick events.
  This should fix problems regarding improper mapping causing controller elements
  to no longer be mappable.

- Make Set Change slots activate on button release instead of button press.

- Tweaked the locking being performed in various portions of the program.

- Fixed a problem with blank info window appearing when escaping from window
  capture utility dialog.

- Fixed issues with document-save-as icon staying displayed after resetting
  a profile.

- Fixed "Media Next" key binding for virtual keyboard.

- Fixed various issues regarding binding slots in the Advanced Edit dialog window.

2015-09-15 Travis Nickles <nickles.travis@gmail.com>

Version 2.19.2

- Add --next to allow multiple profiles to be loaded in one command call.

- Add a flag to settings file when wizard is finished. Don't depend on any
  other settings being changed during an application run.

- Fixed issue with inserting Pause slots.

- Fixed issue with blank application filepath being matched for auto profile
  support.

2015-09-03 Travis Nickles <nickles.travis@gmail.com>

Version 2.19.1

- Fixed SDL 1.2 build support.

- Fixed issue with SDL thread not being stopped due to connections not being
  made in some situations.

- Fixed loading of a profile from a second instance.

- Fixed problem with main window not being populated after the App Settings
  Wizard was finished.

- Fixed another portion of the program that assumed that a controller would be
  connected at startup. Fixes excessive CPU load.

2015-08-31 Travis Nickles <nickles.travis@gmail.com>

Version 2.19

- Added curve options for extra acceleration.

- Fixed bug when App Settings Wizard would pop up when not needed.

- Simplified Chinese translation updated.

2015-08-24 Travis Nickles <nickles.travis@gmail.com>

Version 2.18.2

- Fixed overly active timer used to reset acceleration distances when
  no events are returned from SDL in a given gamepad poll.

- Sebian translation updated by Jay Alexander Fleming.

- Changed behavior of 4 Way Cardinal mode for DPads. Diagonals are no longer
  counted as a dead zone. This change is more likely temporary.
  Up+Right - Up, Down+Right - Right, Down+Left - Down, Up+Left - Left

2015-08-18 Travis Nickles <nickles.travis@gmail.com>

Version 2.18.1

- Fixed some threading issues when using X11.

- Fixed excessive CPU load on startup when no controller is connected.

2015-08-15 Travis Nickles <nickles.travis@gmail.com>

Version 2.18

- Perform event simulation in its own thread. Timer used for mouse speed
  calculations has improved as a result.

- Slow down Enhanced Precision mouse acceleration curve slightly. Changes
  were mainly due to overall mouse emulation being tweaked.

- Distributing 64 bit builds for Windows again.

- Using a simulated event to click the minimize button in title bar now works
  on Windows. The move to using a dedicated thread for event simulation allows
  that to work now.

- Simplified Chinese translation added thanks to zzpxyx.

2015-08-02 Travis Nickles <nickles.travis@gmail.com>

Version 2.17

- Changed Enhanced Precision acceleration curve. The curve has been slowed down
  slightly.

- Changed how initial mouse movement is performed when the gamepad poll rate
  and the mouse refresh rate differ.

- Flash interface buttons after restoring the main window from the tray icon.

- Tweaked mouse movement remainder routine. Remainders now only apply when
  the direction along an axis is the same.

- Raised maximum cycle reset time to 60 seconds.

- Fixed improper cycle reset interval.

2015-07-22 Travis Nickles <nickles.travis@gmail.com>

Version 2.16

- Loosen initial mouse movement experienced after going outside the dead zone.

- Added option in the mouse settings section that would allow antimicroX to
  reset the acceleration values being used for the uinput virtual mouse pointer.
  This is useful after playing older Linux games that change the acceleration
  settings for all mouse pointers after a game exits. Postal 2 and Doom 3 are two
  examples of games in my game library that exhibit this behavior.

- Queue all events before performing any actions. The priority of various events
  has been changed as well.

- Added option to change the gamepad poll rate used by the application. By
  default, the old 10 ms poll rate will be used.

- Bundle a few backup icon files. Mainly useful on SteamOS since the themes
  used don't bundle a couple of essential icons that are expected to be present.

- Compiled vmulti support into Windows build. The driver is not currently
  provided. The program needs to be started with the arguments --eventgen vmulti.

- Temporarily stop packaging a 64 bit version of the program for Windows.
  Unfortunately, the version of Qt being used has a problem regarding timer
  accuracy that is not experienced with the 32 bit version of Qt being used.
  At this time, working around the problem is not feasible so there will be no
  64 bit release. Please use the 32 bit version.

- Change shortcuts used for the .msi package.

- Bundle Qt translation files for Windows version.

2015-06-27 Travis Nickles <nickles.travis@gmail.com>

Version 2.15

- Changed routine for extra acceleration for mouse movement. The new version
  is a bit faster and more responsive than the previous version.

- Added release circle setting for spring mouse mode. This can be used to have
  the mouse return to a region around a character based on the last detected
  direction of an analog stick.

- Added a sub-menu to the virtual keyboard. Some multimedia and browser keys
  can now be selected.

- Added Execute slot type. A program can be set to launch when a slot is
  activated.

- Added Text Entry slot type. You can now have a string of text typed
  when a slot is activated. This can be useful for inputting common commands in
  games.

- Added proper interpolation to determine the start of diagonal regions of
  analog sticks. This is used for mouse movement in order to determine the proper
  starting dead zone point.

- Changed Enhanced Precision mouse acceleration curve. The new changes in the
  curve should be much looser than before.

- Fixed --unload command line option. Enforce reloading of config file
  when finished.

- Fixed issue with spring mouse mode where the mouse would temporarily return
  to center when switching stick directions quickly.

- Fixed some bindings in the virtual keyboard when using SendInput for the
  event generator.

- Fixed issue with widgets used to update stick and dpad delay. The spinbox can
  now be used to edit the delay.

- Reduced multiple instances of some objects. Reduces memory usage slightly.

2015-05-01 Travis Nickles <nickles.travis@gmail.com>

Version 2.14

- Refactored extra axis acceleration. This revision uses individual gamepad
  polls again. Other changes in the code have made switching back feasible. With
  minor adjustments, the mouse will be looser but very controllable.

- Added longer duration for extra axis acceleration. This allows extra
  acceleration to be performed over a period of time as opposed to just one
  gamepad poll.

- Changed key repeat routine when using SendInput on Windows. Key releases
  restart the last active key repeat delay.

- Now using Qt64-NG for the 64 bit Windows builds.

- Initial experiments with vmulti support in the Windows version. Using that
  driver would allow antimicroX to use a virtual keyboard and mouse rather than
  using SendInput. That would allow antimicroX to work around UAC problems and
  anti-cheat programs. Support is not currently compiled into the Windows builds
  due to not being able to properly distribute the vmulti driver.

- Do not write empty names tag in profiles if no custom button names have been
  specified.

- Allow mouse history buffer size to go up to 100.

- Allow diagonal range to go up to 90 degrees.

- Remove old mouse speeds from mouse history buffer if stick has been returned
  to the dead zone. History buffer will be filled with zeroes.

- Omit hotplugging dependent code when compiling against SDL 1.2.

- Fixed dpad pointer bug when certain QueuedConnection slots are executed.

2015-03-31 Travis Nickles <nickles.travis@gmail.com>

Version 2.13

- Updated extra axis acceleration routine. Now, extra acceleration is
  calculated after an axis has stopped moving past the assigned threshold
  in one gamepad poll.

- Stick presets now change the diagonal range of a stick. This is mainly
  beneficial for mouse control changes so that 65 degrees is used by default.

- Added a screen selector for Spring mouse mode in settings menu.

- Added a small language selection portion to the settings menu.

- Added a small logger. Please run antimicroX with "--log-level debug" in order
  to get output about button presses.

2015-03-22 Travis Nickles <nickles.travis@gmail.com>

Version 2.12.1

- Implemented a small wizard that will be run when antimicroX is launched
  for the first time. It currently only has a page for editing some of the mouse
  settings and a page for associating antimicroX with .amgp files on Windows.

- Show slots that are active for a stick modifier button on the main interface.
  The text for the main stick direction buttons will be prefixed with the
  currently active slots of the stick modifier button.

- Corrected issue with stick and dpad events not getting activated on a set
  change. Events were being queued but not executed.

2015-03-15 Travis Nickles <nickles.travis@gmail.com>

Version 2.12

- Added option for extra mouse acceleration. Enabling that option will
  increase the speed of the mouse based on the amount an axis has travelled in one
  gamepad poll and the set multiplier. This will greatly affect how the mouse
  behaves and it will make mouse movement looser. Using this feature can be
  problematic if the analog stick on your controller is worn out and loose.

- Corrected issue with extra mouse sync events occurring when not needed (0,0
  events). This change seems to have smoothed out mouse movement on the low end
  of an axis a bit.

- Tweaked controller unplug workaround to still invoke release events for
  axes and buttons. For triggers, the release value is modified from what is
  provided by SDL so an unplug event will cause a release event to occur for the
  triggers.

2015-02-25 Travis Nickles <nickles.travis@gmail.com>

Version 2.11.1

- Added workaround to allow gamepad unplugging on Windows to not
  activate bindings for the triggers. This was caused by the way controller
  unplugging is handled by SDL (primarily with the Windows version).
  SDL sends a release value for all elements of an unplugged joystick a brief
  period before sending the expected SDL_JOYDEVICEREMOVED event. The problem is
  that the released value used for triggers assumes that the dead zone is half the
  trigger's full range. This would cause bindings for triggers to usually activate
  for a brief period before SDL would send the final SDL_JOYDEVICEREMOVED event
  which would then deactivate those bindings.

- Changes for the portable Windows version. Allow relative profile paths for
  use in the portable package on Windows. Associate profiles registry change
  prompt skipped.

- Queue stick and dpad events. Allows for better control of 8-way sticks
  and dpads.

2015-02-07 Travis Nickles <nickles.travis@gmail.com>

Version 2.11

- Adjusted the Enhanced Precision, Easing Quadratic, and Easing Cubic
  mouse acceleration curves. The low end of each curve has been loosened a bit
  and the rest of the curves have been adjusted accordingly. Comparing the two
  versions, the resulting mouse speed for a given axis point is slightly lower
  in the new versions for most of the curve. The extreme low end and the extreme
  high end of the curves will be slightly faster.

- Updated Qt to version 5.4.0 for the Windows builds.

- Compiled a 64 bit version for Windows thanks to the Qt-x64 project.

- Added a set changing slot.

- Added AppData for use when packaging on Linux thanks to Jeff Backus.

- Fixed bug with mouse wheel event methods for axes which resulted in negative
  values being passed to the event timer.

2014-12-29 Travis Nickles <nickles.travis@gmail.com>

Version 2.10.1

- Changed event handler fallback method under Linux.

- Changed interface of Assignments page in Advanced Button Dialog.

- Reset set number upon changing profiles.

- Added "About Development" tab to About Dialog.

- Fixed dynamic text resizing in Button Edit Dialog under Linux.

- Fixed launching a second instance in order to load a profile in the
  first instance.

2014-12-10 Travis Nickles <nickles.travis@gmail.com>

Version 2.10

- Changed allowed values for easing duration. The minimum value has been
  lowered to 0.0 and the maximum value has been increased to 5.0.

- Added a stick modifier button. This button is primarily meant to be used
  to assign walk/run functionality to an analog stick. Instead of having to
  create distance zones and assign keyboard modifier keys to each stick button,
  you can now make the assignment on the stick modifier button and it
  will apply to the stick as a whole. It makes assigning walk/run functionality
  to an analog stick much less cumbersome. The DreadOut demo has already shown a
  use case where only the stick modifier button was needed due to the demo not
  handling running when using the left stick on an Xbox 360 controller.

- Increased idle mouse timer interval to 100 ms.

- Added a load profile slot. You can now tell the program to load a different
  profile upon pressing a button.

- Added gradient functionality for the high end of the Easing Quadratic and
  Easing Cubic mouse acceleration curves.

- Raise process priority on Windows. Now, the antimicroX process will run
  with High priority. This allows internal timers to work better and be less
  susceptible to the activity of other running programs. On Linux, the priority of
  the main thread has been increased.

- Take multiple direction buttons into account when assigning set switching
  to stick buttons.

- If uinput is enabled but not usable at runtime then XTest will be used
  as a fallback option for the Linux version.

- Tweaked Gradient and Pulse turbo modes to make them a bit tighter. A lower
  delay will be needed in profiles to achieve a similar control from previous
  versions. On the plus side, this has been tested to work with FlatOut 2 fairly
  well. It is good enough to use and actually win some races against AI opponents.

- Added analog control for mouse wheel buttons that are mapped to an axis
  button.

- Tweaked mouse movement code to improve mouse accuracy. This is mainly due
  to discovering the QElapsedTimer class that is included in Qt.

- Fixed middle mouse button binding when using the uinput event handler
  on Linux.

- Fixed memory leaks that were discovered by Valgrind.

2014-11-19 Travis Nickles <nickles.travis@gmail.com>

Version 2.9

- Added mouse refresh rate as an option. Please be mindful of how your
  system behaves if you set the refresh rate to a low value like 1 ms.
  In the worst case scenario, you could end up dedicating one CPU core on
  just the antimicroX process. Also, on Windows, you will want to make sure
  to disable the "Enhance Pointer Precision" option if you use a low value
  otherwise Windows will severely slow down the mouse pointer.

- Added an application level mouse smoothing option. The older button level
  smoothing option has been removed. The old option didn't do much since it only
  dealt with the partial distance remainder.

- Button responsiveness has been improved. The old mouse movement code was
  creating a bottleneck for button processing which would result in a slight
  delay.

- Changed mouse movement code. The overall mouse movement should be smoother
  now.

- Allow the Windows setting "Enhance Pointer Precision" to be disabled while
  antimicroX is running. This will make sure Windows does not directly manipulate
  the mouse events sent to the system. This will allow mouse control on an analog
  stick to be more accurate on Windows.

- Changes to Auto Profile to allow more variables for matching. Multiple
  window properties can be specified which will cause antimicroX to count an
  entry as a match only if all specified properties match.

- Changed how windows are grabbed in X on Linux.

- Minor fix for Gradient and Pulse turbo modes. There were times when the
  timer interval controlling those modes could be negative and cause problems.

2014-11-05 Travis Nickles <nickles.travis@gmail.com>

Version 2.8.1

- Fixed some buttons in virtual keyboard when program is using uinput support.

- Fixed Update Joysticks option for SDL 1.2 users.

2014-10-28 Travis Nickles <nickles.travis@gmail.com>

Version 2.8

- Added delay settings for analog sticks and dpads. This is meant to
  keep some games from overreacting when switching directions quickly.
  A delay is especially useful for games that utilize a dash on a key double tap.
  This will also be very useful while playing rougelike games. The new
  delay setting allows for more responsive controls than the old alternative of
  using hold zones on individual direction buttons.

- Added two new mouse acceleration curves: Easing Quadratic and
  Easing Cubic. These new mouse acceleration curves are meant to mimic the
  camera control that is used for gamepad support in some recent first person
  shooters such as Borderlands 2. Once a stick direction has reached a threshold
  (80%) then the mouse speed will gradually accelerate over a period of time
  before the mouse speed reaches its peak. The duration of the easing method is
  set at 0.50 seconds by default but the setting is configurable per
  button.

- Major refactor to mouse event generation. The new routine requires
  fewer system resources and it is more accurate.

- Made uinput support a runtime option for Linux users. The program can
  now be compiled with both XTest and uinput support and the event
  generator can be specified using the --eventgen flag. It defaults
  to XTest if available. The option is only available when the program
  is compiled with both XTest and uinput support.

- Added right click context menus for buttons in main interface.

- Fixed issue with Game Controller Mapping dialog. Controller DPads that
  are exposed as 4 buttons can now be bound to the DPad of an SDL Game
  Controller.

- Fixed an issue with incorrect profile names being displayed in the
  profile combobox.

- Fixed issue introduced in version 2.7 regarding mouse movement calculations
  for the left direction of analog sticks. A major portion of the
  safe zone for that direction was being skipped.

- Changed button groups in the main interface to update immediately
  when a stick or dpad mode has been changed.

- Initial removal of old joystick abstraction support in interface when using
  SDL 2.

2014-10-14 Travis Nickles <nickles.travis@gmail.com>

Version 2.7

- Added a UAC workaround for use in Windows. antimicroX can be restarted
  as Administrator in case a game is running with elevated permissions and
  the events generated by antimicroX are not detected by a game.

- Added more key aliases for uinput support.

- Force higher dead zones for axes in Game Controller Mapping window.

- Fixed virtual keyboard in Button Edit Dialog window for Linux users
  utilizing XTest support.

- Display some minor mouse stats in Mouse Settings dialog.

- Alter Analog Stick dialog window to show some new stats. Also,
  show square stick coordinates as well as adjusted circle
  stick coordinates.

- Added square stick to circle stick conversion.

- Fixed issue with VK_LSHIFT and VK_RSHIFT aliases not being saved properly
  on Windows.

- xinput is used for the uinput virtual pointer in order to
  disable mouse acceleration for the pointer. This allows spring mode to work
  as intended.

- Added some code to guess which axes of a gamepad should be considered
  triggers by default while in old Joystick mode. The initial values of
  axes are taken from SDL and those are used as the point of the axes
  while centered. If the initial value of an axis is below -30,000 then
  an axis is considered to be a trigger.

2014-09-16 Travis Nickles <nickles.travis@gmail.com>

Version 2.6

- Added two new Turbo modes. Gradient mode is used to change the key press
  time depending on the position of an axis (useful for racing games).
  Pulse mode is used to change how many times a key press is invoked depending
  on the position of an axis (scrolling in a web browser using arrow keys).

- Fixed profile resetting in a couple of places.

- A Russian translation has been provided by Dima Koshel.

- Added option to invoke Game Controller mapping window from command line.
  The final mapping string will be printed to stdout. This is useful
  for saving a SDL_GAMECONTROLLERCONFIG for your controller that
  can be used system wide. Any SDL 2 game can then be set up to use
  that mapping and it can be changed if needed.

- Profiles now use a unique .amgp file extension. Older xml profiles will
  continue to be supported.

- Fixed spring mouse mode so that it uses proper axis distance values.

- Set changing has been fixed for analog sticks and virtual dpads.

- EXPERIMENTAL. uinput support has been added to the source code. Binary
  Linux packages will continue to utilize XTest for event generation for the
  time being. If you would like to test uinput integration then you will have
  to compile the program using -DWITH_UINPUT=ON and -DWITH_XTEST=OFF
  when running cmake. Playing Warsow 1.51 in Linux using antimicroX requires
  using uinput integration. Also, keys can now be pressed in a tty.

2014-08-01 Travis Nickles <nickles.travis@gmail.com>

Version 2.5

- Fixed packaging the Windows version so the program will work on
  Windows XP again.

- Delay rendering of flashing buttons. This helps improve controller
  responsiveness when the main window is not hidden.

- Reduced the size of written profiles. Only changed values are
  now stored in profiles.

- Updated German translation provided by phob and Leonard Koenig.

- Allow a profile to be listed as the final argument on the command line.
  Ex: antimicroX ~/antimicroX-profiles/profile.xml.

- Added diagonal distance support for distance slots for Standard
  mode on Sticks and DPads. This was necessary for some modifier assignments
  to work properly. The best example is for assigning walking in Half-Life 1.

- Allow generated events to be sent to a different X display than the
  application is running on. This change was mainly done to better work
  with SteamOS. antimicroX can be run via ssh with X tunneling in order
  to configure profiles on one system but the program will send events to the
  X display that is running Steam and games.

- Auto Profile support has improved for SteamOS. Application grabbing can
  now be done while in the SteamOS BPM Session. Steam BPM can be grabbed as
  well.

- Only show active or soon to be active slots for buttons in the main
  window. The text displayed on the buttons will update when a
  new zone has been reached due to using slots such as distance or hold.

- Allow no profile to be assigned to an application for an Auto Profile entry.
  This means that the program will choose an empty profile when the application
  for that Auto Profile entry has focus. This will help with disabling antimicro
  for applications that already have controller support.

- Controller Mapping dialog now stops processing events until all elements
  have been released on a controller.

2014-05-30 Travis Nickles <nickles.travis@gmail.com>

Version 2.4

- Relative spring mode added.

- Key repeating changes in Windows.

- Updated Windows version to use Qt 5.3.0.

- Set copying added.

- Corrected application checks used for Auto Profile support
  in Windows.

2014-05-23 Travis Nickles <nickles.travis@gmail.com>

Version 2.3.3

- Tweaked Enhanced Precision mouse acceleration curve.

- Tweaked "all" option in Auto Profile. No profile assigned to "all" now
  implies that a blank configuration should be loaded.

- Manpage created by Jeff Backus.

- Migrated to the CMake build system.

2014-05-15 Travis Nickles <nickles.travis@gmail.com>

Version 2.3.2

- Fixed problem with old profiles not being usable in the
  program.

2014-05-13 Travis Nickles <nickles.travis@gmail.com>

Version 2.3.1

- Added new Enhanced Precision mouse curve. It is now the default
  mouse curve. The new mouse curve will make the cursor move slowly
  towards the low end of an axis and the cursor movement will be
  accelerated towards the high end of an axis.

- Added unsaved profile dialogs.

- Added key repeating behavior under Windows.

- Increased maximum turbo interval.

- Added more options to the Edit Settings dialog.

- Added profile name display editing.

- Fixed invalid pointer issue for Hold events.

2014-05-02 Travis Nickles <nickles.travis@gmail.com>

Version 2.3

- Added a daemon mode.

- Added joystick profile compatibility parser for game controllers.
  Old profiles are now usable when using SDL Game Controller support.
  Old profiles have to be mapped against the same controller that
  is connected.

- Added cycle reset support. Sequences with cycles can now get
  returned to the first cycle zone in a sequence after a period of
  time.

- Changed Auto Profile support to work properly in SteamOS
  while running the SteamOS Big Picture Mode session. On many
  tested games (Duke Nukem 3D, SuperTux, World of Goo), the
  game had to be run in windowed mode in order for Auto Profile
  support to be able to detect the application. It is recommended
  that you run games in windowed mode when using Auto Profile support
  in the SteamOS BPM session. This is not an issue when running the Steam
  desktop client in desktop mode.

- Added a delay slot type. A delay slot can be used for
  key presses in a key combination. Unlike other macro slots, slots
  placed before a delay slot will remain active after the specified
  time has passed.

- Added option to allow the program to start on Windows startup.

- Changed dialogs for secondary set buttons to display the set
  that the button is currently in.

- Changed turbo mode to give more control. Key presses are now given
  more time to be active. Key presses and releases now run for a
  duration of t/2 seconds.

- Altered tray menu to display a single list when only one controller
  is connected. An option has been added to allow for a single
  list to also be used when multiple controllers are detected.

- Fixed issue with Windows XP Auto Profile workaround.
  On the plus side, now the program is confirmed to work in
  Windows XP.

- Fixed issues with Auto Profile support saving and precedence.

2014-04-19 Travis Nickles <nickles.travis@gmail.com>

Version 2.2

- Added example controller display to Game Controller Mapping
  dialog window.

- Added Auto Profile support. Allows for profiles to be associated
  with specific applications.

- Added button icons in Windows version.

- Added a press time slot type. That slot type is used
  to have keys active for a specific period of time.

- Allow Pause slots to have a 0 second interval. Allows
  for a forced key release.

- Windows version is now built with SDL 2.0.3.

- Fixed 8-Way D-Pad mode.

- Fixed preset options in various dialogs to account for new aliases.

- Fixed ampersand rendering in set buttons.

- Fixed spring mouse mode dimension support.

- Fixed spring mouse mode accuracy under Windows.

2014-02-28 Travis Nickles <nickles.travis@gmail.com>

Version 2.1

- Added new stick and dpad modes.

- Added set names.

- Minor fixes for Windows.

- Fixed QSettings usage to reduce reads and write to config file.

2014-02-10 Travis Nickles <nickles.travis@gmail.com>

Version 2.0.1

- Active keyboard keys now use a reference count. This will
  be useful for keeping modifier keys held when moving a stick
  from a diagonal direction to a cardinal direction. This will allow a run
  to be maintained properly.

- A release delay has been added to release slot events. This is needed
  for some games where a key press and release might happen too quickly
  for a game, such as The Elder Scrolls IV: Oblivion, to catch it.

- Altered data sent to SendInput under Windows. The change should allow
  games that rely exclusively on scancode data to detect keyboard
  key presses now. Previously, keyboard emulation would not work
  while playing The Elder Scrolls IV: Oblivion on Windows.

- Improved key associations under Windows. VK*OEM*\* keys associations
  are now generated at runtime which will allow associations
  to be more layout independent.

- Changed some portions of the Windows version so that the Numpad
  Enter key can be emulated.

- The recent profile list is now updated when a profile is added
  or removed from the list as opposed to when the program is closed.
  This allows the list to be in sync while utilizing hotplugging.

2014-01-04 Travis Nickles <nickles.travis@gmail.com>

Version 2.0

- Migrated profiles to use Qt key values as opposed to using native key code
  values. Allows for better cross-platform compatibility. Current
  joystick profiles will be migrated when first loaded by the program.

- Program can now be compiled against SDL 2.

- The Game Controller API provided by SDL 2 has been integrated into the
  application. The API is used to abstract various gamepads to a unified
  standard. Profiles made against the Game Controller API can be used with
  any controller that is supported. Unsupported controllers will have to be
  mapped in the program.

- A simple Game Controller mapping window has been made so people that are using
  an unsupported controller can map that controller so that it can be used with
  the Game Controller API. Any saved mapping string will be loaded into
  SDL when antimicro is first launched or when you select the
  "Update Joysticks" option in the main window.

- Any new saved profile will include the device type that it is
  associated with in the filename. Joysticks and game controllers use slightly
  different file specifications.

- Joystick hotplugging support has been added thanks to SDL 2.

- On Windows, XInput support is now available. This allows the Xbox 360
  controller guide button to be usable and both gamepad triggers can be used at
  the same time. Previously, only DirectInput was being used so both triggers were
  being mapped to one axis so they would negate each other if used at the same
  time. This problem would really affect people trying to play Call of Duty with a
  360 controller when the "Hold Aim Down the Sight" option is enabled in the game.

- The list of recent profiles is now tied to a joystick GUID or joystick name
  if the GUID is not available (compiled against SDL 1.2).

- Program options window has been made. For right now, it is only being used to
  allow users to be able to disable Game Controller integration for specific
  controllers.

- Mouse events are queued before a mouse event is sent to the system. This
  allows for smoother diagonal mouse movement. This really helped improve camera
  control for me in Warsow.

- Key checker dialog has been made so you can check the alias values that are
  being used for keyboard keys. This is mainly for debugging purposes. If you
  find that a keyboard key that you use does not have a built-in alias, please
  let me know so an alias can be added. Although, the key can still be used in
  the program and saved to a profile. The major downside is that a profile
  that uses an unsupported key will not be cross-platform compatible.

2013-12-13 Travis Nickles <nickles.travis@gmail.com>

Version 1.2

- Improved while held set changing so that the program should not get stuck on
  the wrong set. The changes made should behave roughly like the old while held
  workaround that used turbo.

- Windows port of antimicro has been made.

- Tweaked code used for button presses and releases in order
  to improve responsiveness.

- Allow time-dependent action slots to have an interval
  as low as 0.01 seconds.

- Tweaked Release action slot. Release slots can now be placed at the
  beginning of the assigned slots. This can be useful for
  Tap and Hold slot configurations.

- Pause slots can now be used with Release slots.

- Profiles can be removed from the recent configs list.

- Spring mouse mode preview has now been enabled.

- Mouse speed modifier action slot has been added. This can be used to modify
  the mouse speed settings used with any controller element while the slot is
  active. The setting will modify the mouse speed by the percentage that is
  specified. The mouse speed modifier can be used to allow for the mouse speed
  to be slowed down while sniping.

- Button and action names have been added. Names can be used to describe
  the action that a slot sequence performs in the game.

- Mouse wheel buttons are now used as a form of mouse movement. Mouse wheel
  scrolling is now possible without using turbo. The speed that the wheel is
  rotated can be specified in the mouse settings dialog window.

- Added support for two extra mouse buttons.

- A new controller properties window has been made. This window shows various
  bits of information provided by SDL about a controller as well as the current
  values of all the controller elements.

- Added quick assign functionality for sticks and virtual dpads.

- Windows version of the program now uses LocalAppData variable to know
  where to place the program's settings file.

- New translations provided by the translation team.

2013-10-03 Travis Nickles <nickles.travis@gmail.com>

Version 1.1

- Added spring mouse mode. This mode is used to move the mouse cursor
  from the center of the screen based on how much an axis has been moved
  from the dead zone. The mouse cursor will be returned to the center
  of the screen when the axis is released.

- Added mouse curve options from QJoyPad.

- Tweaked mouse movement in cursor mode to improve axis responsiveness
  and to allow mouse movement to be less jittery.

- Added optional mouse smoothing for a further reduction
  in jitter in exchange for slightly delayed responsiveness.

- Moved various mouse settings into a new dialog window.
  Several other dialog windows have been changed to point to the
  new dialog window to allow for mouse setting adjustments.

- Added an option to start the program hidden.

- Tray menu has been tweaked to allow configuration profiles to be
  disabled. A blank new profile will be enabled in the program
  for a controller. This is equivalent to selecting <New> in the main window
  combobox.

- Serbian translation provided by Jay Alexander Fleming.

- Brazilian Portuguese translation provided by VaGNaroK.

2013-07-12 Travis Nickles <nickles.travis@gmail.com>

Version 1.0

- 8-way controls have been implemented. This allows keys to be mapped
  to the diagonal directions of controller sticks and dpads. 8-way controls
  allow rougelike games to be playable.

- Virtual Dpad support has been added. Axes and buttons can be mapped
  to a virtual dpad. This is useful for mapping dpads that are detected as a
  pair of axes in SDL.

- A Quick Set option has been added. Using the Quick Set option, you
  can press a button on the controller and the program will bring up the edit
  window for that specific button. The button can then be mapped to an
  assignment from the edit window. The Quick Set option also works for axes,
  controller sticks, and dpads. This is more of a convenience function than
  anything but I have found it really useful since implementing it.

- Main interface button text is now updated whenever the assigned slots
  are changed. This allows the buttons' text to be in sync in many situations
  that was not possible before.

- Toggle and Turbo can be used together to create automated key macros for
  use in MMORPGs.

\* _This Changelog was automatically generated by [github_changelog_generator](https://github.com/github-changelog-generator/github-changelog-generator)_
