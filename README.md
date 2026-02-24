sys-ticon
================

Stripped version of [switch-sys-tweak](https://github.com/p-sam/switch-sys-tweak) to only replace icons, titles, publishers and display versions in qlaunch.

Tested only on 21.0.0+, should be compatible with 20.0.0+.

## How to use
If you want to replace title + publisher and/or display version, use this template:
```ini
[override_nacp]
name=Some Title
author=Some Publisher
display_version=21.37
```

Save it in "config.ini" and put it to `atmosphere/contents/*titleid*/`.
name + author must come always in pair, one of the missing will mean that another one will be ignored. This is to avoid work with compressed NACPs.
If you don't want to replace display version, just remove `display_version=` line. If you want to change only `display_version`, remove lines with `name=` and `author=`.

If you want to replace icon, create 256x256 JPG (must be baseline, aka non-progressive) with max size 131072 B for FWs before 19.0.0, for 19.0.0 it cannot be bigger than 102400 B.
On 19.0.0+ icon is not replaced in app's "Options" menu (when you press +) because rescaling is not done anymore by qlaunch and requires manual scaling.

## How to compile

Standard compilation:
```
make FEAT_ALL=1
```
with logger enabled
```
make FEAT_ALL=1 TOGL_LOGGING=1
```
