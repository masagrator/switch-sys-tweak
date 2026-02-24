sys-ticon
================

Stripped version of [switch-sys-tweak](https://github.com/p-sam/switch-sys-tweak) to only replace icons, titles, publishers and display versions in qlaunch.

Tested only on 20.5.0 and 21.1.0. Should work with any FW from 19.0.0, maybe with older too.

# How to use

Latest release you can find [HERE](https://github.com/masagrator/sys-ticon/releases), download "sys-ticon.zip", put "atmosphere" folder to sdcard. Restart Switch.

> [!IMPORTANT]
> Any change requires console reboot to see results immediately in home menu because those entries are cached upon boot, only "Options" menu and "All Software" show immediately changes because they are not cached.

### Title, publisher, display version
If you want to replace title + publisher and/or display version, use this template:
```ini
[override_nacp]
name=Some Title
author=Some Publisher
display_version=21.37
```

  System limitations:
  - Config.ini must be saved with encoding UTF-8 without BOM (Byte-Order Mark)
  - `name` cannot take more than 512 bytes
  - `author` cannot take more than 256 bytes
  - `display_version` cannot take more than 16 bytes

If name or author is longer than requested, both are ignored. If display_version is longer than requested, it is ignored.

Save it in "config.ini" and put it to `atmosphere/contents/*titleid*/`.

`name` + `author` must come always in pair, one of the missing will mean that another one will be ignored. This is to avoid work with compressed NACPs.<br>
If you don't want to replace display version, just remove `display_version=` line. If you want to change only `display_version`, remove lines with `name=` and `author=`.

### Icon 
If you want to replace icon:
-  Create 256x256 JPG (must be baseline, aka non-progressive) with max size 131072 B for FWs before 19.0.0, for 19.0.0+ it cannot be bigger than 102400 B. Name it "icon.jpg"
-  For 19.0.0+ create 174x174 JPG (must be baseline, aka non-progressive with max size 65536 B. Name it "icon174.jpg"
-  Put them to `atmosphere/contents/*titleid*/`.

In 19.0.0+ "Options" menu and "All Software" are not handling scaling 256x256 icon to 174x174 internally anymore, that's why we need 174x174 icon separately. If you won't put this icon, original one will show up.

## How to compile

Standard compilation:
```
make FEAT_ALL=1
```
with logger enabled
```
make FEAT_ALL=1 TOGL_LOGGING=1
```
