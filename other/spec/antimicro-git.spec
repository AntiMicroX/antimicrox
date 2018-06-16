Name: antimicro-git
Version: 2.24
Release: 1%{?dist}
Summary: Maps keyboard and mouse actions to gamepad buttons, inspired by qjoypad.

License: GPL3
URL: https://github.com/juliagoda/antimicro
Source: https://github.com/juliagoda/antimicro/archive/%{version}.tar.gz

BuildRequires: git cmake qt5-tools itstool 
Requires: libxtst qt5-base sdl2

%description
Antimicro is a graphical program used to map keyboard keys and mouse controls to a gamepad. This program is useful for playing PC games using a gamepad that do not have any form of built-in gamepad support. However, you can use this program to control any desktop application with a gamepad; on Linux, this means that your system has to be running an X environment in order to run this program.

%prep
%setup -q -n

%build
cmake -DCMAKE_INSTALL_PREFIX=/usr \
      -DUSE_SDL_2=ON \
      -DWITH_XTEST=ON \
      -DWITH_UINPUT=ON \
      -DAPPDATA=ON
      
make

# %check
# make check

%install
make install DESTDIR=%{_prefix} 

# to be continued ...
#%find_lang %{name}

#%files -f %{name}.lang
#%doc README.md Resources.txt ProfileTips.md LICENSE Changelog 
#%{_bindir}/*
#%{_sbindir}/*
#%{_mandir}/man1/*

%changelog
* Sat Jun 16 2018 Jagoda Gorska <juliagoda.pl@protonmail.com>
- Added calibration
- Ported Qt4 to Qt5
- Removed Qt4 support
