# fcitx5-fbterm

FbTerm input method frontend for [fcitx5](https://github.com/fcitx/fcitx5)

## Install

### ArchLinux

[fcitx5-fbterm-git(AUR)](https://aur.archlinux.org/packages/fcitx5-fbterm-git/)

### Other

See [Build](#Build)

## Build

### Dependances

- glib2
- fcitx5
- fcitx5-gtk
- cmake
- make

### Comple

```shell
git clone https://github.com/fcitx/fcitx5-fbterm
mkdir fcitx5-fbterm/build
cd fcitx5-fbterm/build
cmake ..
make
make install
```

## Usage
First you need to have a running fcitx5. If you are already using systemd, it is likely that dbus session is already configured.
If you already have a running fcitx5 under graphics display, it is possible that fcitx5-fbterm can use that directly.
```
fbterm -i fcitx5-fbterm
```

FCITX5_FBTERM_BACKGROUND and FCITX5_FBTERM_FOREGROUND environment variables can be used to set the color.
