# WallpaperSwitcher

## What is this?
This program will create a slideshow on the desktop and lock screen of a
windows computer. It will recursively look for any .jpg or .png files in
`%HOME%\Documents\wallpapers`, and use those images for the wallpaper slideshow.

The initial order of images is undefined, however it is guaranteed to never
show any image twice before every image has been shown once. Essentially, a
random ordering of images is chosen the first time the application is run, then
that order is shown on loop ad infinitum.

## Why is this better?
This application is an improvement over the buit-in slideshow mechanism in
several ways:
1. Every image is guaranteed an equal amount of time on-screen.
2. Both the desktop and the lock screen are set simultaneously.
3. Recursive searching of image files allows for better organization of the
wallpaper source directory.

## How do I use this?
Installation is relatively simple:
1. In your 'Documents' folder create a folder called 'wallpapers'. Place at
least one .png or .jpg file inside that folder, or a subfolder of that folder.
2. Download the latest release.
3. Unzip the relase. There should be two files: `WallpaperSwitcher.exe` and
`sqlite3.dll`.
4. Copy those two files (or preferrably, the directory containing them)
somewhere safe. For example, `Documents\WallpaperSwitcher`.
5. Open the 'Run' dialogue, using the hotkey Win + R. Enter `shell:startup`,
and click 'OK'.
6. In the folder that appears, create a shortcut to wherever you saved
`WallpaperSwitcher.exe`.

## Settings?
The default settings are as follows:
| Setting | Value |
| --- | --- |
| Wallpaper folder | `%HOME%\Documents\wallpapers` |
| Wallpaper duration | 120 seconds |

I designed this project with one specific user in mind: me. That means all
settings are hardcoded directly into the application code. If you want to
change anything, you will have to download the source code, modify the 
desired variables, and recompile.

## Future plans
I created this project with two goals in mind:
1. Fix my problems with the default wallpaper switcher
2. Learn the Win32 C++ api.

Goal 1 has already been met, so this project is likely dead. However,
if I get bored enough, I might improve this application just to learn more
Win32 C++. No promises, though.

Here are some features I'd like to implement:
- User-defined configuration file (no hardcoded settings)
- Run as a windows service instead of a headless application
- GUI for adjusting the order of wallpapers and configuring settings.

