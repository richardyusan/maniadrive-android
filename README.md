# ManiaDrive for Android

An Android port of **ManiaDrive 1.2** (2006), the free acrobatic racing game
by CQFD Corp. built on the Raydium 3D engine: short, nervous tracks with
loops, jumps and turbo pads, a story mode and LAN multiplayer.

The original C source code and game data run unchanged as far as possible;
a small platform layer replaces the PC parts (window, OpenGL, input, PHP).

| | |
|---|---|
| Android | 8.0 or newer (API 26+) |
| CPU | arm64-v8a, armeabi-v7a, x86_64 |
| Graphics | OpenGL ES 2.0 (the game's OpenGL 1.x calls are translated by [gl4es](https://github.com/ptitSeb/gl4es)) |
| Frame rate | unlocked (optional 60 fps lock at build time) |
| APK size | about 47 MB |

## Install

1. Download `ManiaDrive.apk` from the [Releases](../../releases) page (or the
   *Actions* tab, artifact `ManiaDrive-apk`).
2. Open it on the phone and allow "install unknown apps" for your browser /
   file manager. Play Protect may warn about a self-signed app: choose
   *Install anyway*.
3. The first start takes a few seconds: the game data (about 56 MB) is copied
   to the app's storage once.

## Controls

**Touch** (landscape):

```
 [MENU]                                        [RESET] [CAM]


 [ ◀ ] [ ▶ ]                                [ ▼ ]  [ ▲ ]
  steering (slide between them)           brake   gas
```

- Steering and pedals react to the whole lower-left / lower-right area of the
  screen, so you do not have to hit the squares exactly. Several fingers work
  at the same time.
- **RESET** restarts the track (Space on PC), **CAM** switches the camera (C),
  **MENU** or the Android Back key leaves the track (Esc).
- In the menus a tap is a mouse click. Tapping a text field (player name,
  server address) opens the keyboard.
- The Back key closes popups, goes back to the main menu, and quits from there.

**Gamepad**: left stick / D-pad steering, RT or A = gas, LT or B = brake and
reverse, X = restart, Y = camera, Start/Select = menu. The touch buttons hide
while a gamepad is used. Hardware keyboards use the PC keys.

## What changed compared to the PC version

- Runs full screen at the phone's resolution (capped at 1080p). *Options →
  Max. resolution* lowers it on slow phones (restart the game to apply).
- *Shadow support* is off by default (Options) for a smooth frame rate on every phone.
- The **Internet Tracks** mode and online high scores are gone: the CQFD Corp.
  track and score servers were shut down years ago. Story mode, "Other
  tracks" (your own `.mni` tracks) and LAN multiplayer are kept.
- The embedded PHP interpreter (story progress, track lists, music playlist)
  is replaced by the same logic written in C.
- The track editor (`mania2`) and the dedicated server are not part of the
  Android app; LAN games can join a `mania_server` running on a PC.

Saves (story progress, options) are kept in the app's private storage and
survive updates as long as the APK is signed with the same key.

## Building

Linux or WSL, no Gradle needed:

```bash
export ANDROID_HOME=~/android/sdk
export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/28.2.13676358
./build_android.sh            # -> build/ManiaDrive.apk
```

All details (toolchain, signing, the PC test build, how the port works and
the problems met during porting) are in [BUILD.md](BUILD.md).

## Bahasa Indonesia (singkat)

Port Android dari game balap **ManiaDrive 1.2**. Unduh `ManiaDrive.apk` dari
halaman *Releases*, izinkan "instal aplikasi tidak dikenal", lalu buka.
Kontrol: tombol kiri bawah = setir, kanan bawah = gas/rem, RESET = ulang
lintasan, CAM = ganti kamera, MENU/tombol Back = keluar lintasan. FPS tidak
dibatasi. Cara build ada di [BUILD.md](BUILD.md).

## Credits and license

- **ManiaDrive** and the **Raydium 3D engine**: Xfennec and the CQFD Corp. /
  Raydium team, <http://maniadrive.raydium.org/>. Code and art: GNU GPL v2 or
  later. Music: Creative Commons (see [docs/ORIGINAL_README.txt](docs/ORIGINAL_README.txt)
  for the artists).
- Android port: richardyusan, GPL v2 or later (see [LICENSE](LICENSE)).
- Third-party libraries, downloaded at build time by `scripts/fetch_deps.sh`:
  [gl4es](https://github.com/ptitSeb/gl4es) (MIT),
  [GLU](https://github.com/ptitSeb/GLU) (SGI Free Software License B),
  [ODE 0.16.6](https://github.com/odedevs/ode) (BSD / LGPL),
  [OpenAL Soft 1.24.3](https://github.com/kcat/openal-soft) (LGPL 2),
  [libogg / libvorbis](https://xiph.org) (BSD),
  [libjpeg-turbo](https://libjpeg-turbo.org) (IJG / BSD).
