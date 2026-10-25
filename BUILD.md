# Building ManiaDrive for Android

This document covers the toolchain, the build commands, APK signing, the
Linux test build, how the port works, and the problems met during the port
(with their solutions).

---

## 1. Toolchain

These are the versions the APK was built with.

| Component | Version | Notes |
|---|---|---|
| Android NDK | **r28c** (`28.2.13676358`) | clang 19; any NDK r26+ should work |
| Android SDK build-tools | **34.0.0** | `aapt2`, `d8`, `zipalign`, `apksigner` |
| Android SDK platform | **android-34** | only `android.jar` is used |
| JDK | OpenJDK **21** (17+ works) | `javac`, `keytool`; Java compiled as Java 8 |
| CMake + Ninja | 3.28 / 1.11 | third-party libraries |
| git, zip, python3 | any | |
| **Gradle** | **not used** | |

Third-party libraries (downloaded and pinned by `scripts/fetch_deps.sh`):

| Library | Version | Used for |
|---|---|---|
| gl4es | commit `2037865` | OpenGL 1.x → OpenGL ES 2.0 |
| GLU (ptitSeb) | commit `2fed2bd` | `gluPerspective`, `gluLookAt`, `gluBuild2DMipmaps`, ... |
| ODE | 0.16.6, single precision, OPCODE | car physics |
| OpenAL Soft | 1.24.3, OpenSL ES backend | 3D sound |
| libogg / libvorbis | 1.3.6 / 1.3.7 | music (`.ogg`) |
| libjpeg-turbo | 3.1.2 | menu videos (`.jpgs`, JPEG sequences) |

**Android target settings** (`android/AndroidManifest.xml`, `build_android.sh`):

| Setting | Value |
|---|---|
| Package | `org.maniadrive.android` |
| `minSdkVersion` / `targetSdkVersion` | 26 / 34 |
| ABIs | `arm64-v8a`, `armeabi-v7a`, `x86_64` |
| Activity | `NativeActivity` subclass (`MainActivity.java`), lib `libmaniadrive.so` |
| Orientation | `sensorLandscape` |
| Frame rate | unlocked by default; `-DPORT_FPS=60` locks it to 60 fps |

### Installing the toolchain (Linux / WSL)

```bash
sudo apt install openjdk-21-jdk-headless cmake ninja-build git zip unzip python3
mkdir -p ~/android && cd ~/android
curl -O https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip
unzip commandlinetools-linux-11076708_latest.zip
yes | cmdline-tools/bin/sdkmanager --sdk_root=$HOME/android/sdk \
      "build-tools;34.0.0" "platforms;android-34" "ndk;28.2.13676358"
export ANDROID_HOME=$HOME/android/sdk
export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/28.2.13676358
```

On Windows use WSL (Ubuntu): the scripts are bash scripts.

---

## 2. Build commands

```bash
./build_android.sh                    # -> build/ManiaDrive.apk (3 ABIs)
ABIS=arm64-v8a ./build_android.sh     # faster: one ABI (most phones)
```

The first build downloads and compiles the libraries (≈ 5 minutes per ABI);
later builds take about one minute.

| Variable | Default | Meaning |
|---|---|---|
| `ANDROID_NDK_HOME` (or `NDK`) | – | NDK folder |
| `ANDROID_HOME` (or `SDK`) | – | SDK folder |
| `ABIS` | `arm64-v8a armeabi-v7a x86_64` | ABIs to build |
| `LIBS_ONLY=1` | – | stop after the native libraries |
| `VERSION_CODE`, `VERSION_NAME` | `1`, `1.2-android1` | APK version |
| `KEYSTORE`, `KS_ALIAS`, `KS_PASS` | `android/maniadrive.keystore`, `maniadrive`, `maniadrive` | signing key |

### What the script does

| Step | Command (simplified) | Output |
|---|---|---|
| 1. Libraries | `scripts/fetch_deps.sh`, `scripts/build_deps.sh <abi>` (CMake + NDK toolchain) | `build/deps/<abi>/lib/*.a` |
| 2. Native | `clang --target=aarch64-linux-android26 -O2 -fwrapv -DMANIA_PORT -DNO_PHP_SUPPORT -Dmain=maniadrive_main -c src/mania_drive.c` (the game `#include`s the whole Raydium engine), plus `android_main.c`, `android_native_app_glue.c`, GLU; link with `-Wl,--no-undefined -Wl,-z,max-page-size=16384 -static-libstdc++` | `lib/<abi>/libmaniadrive.so` (3–4 MB) |
| 3. Assets | copy `data/` to `assets/game/`, write `assets/manifest.txt` (file list + content hash) | |
| 4. Java | `javac -source 8 -target 8 MainActivity.java`, `d8 --min-api 26` | `classes.dex` |
| 5. Package | `aapt2 compile/link -A assets`, `zip` dex + libs | `unsigned.apk` |
| 6. Sign | `zipalign -p 4`, `apksigner sign` | `build/ManiaDrive.apk` (≈ 47 MB) |

### Install and read the log

```bash
adb install -r build/ManiaDrive.apk
adb logcat -s ManiaDrive LIBGL     # Raydium log, gl4es messages
```

---

## 3. Testing on a PC (no phone needed)

`scripts/test_linux.sh` builds exactly the same port code for Linux, with an
off-screen EGL surface (Mesa), and plays `tests/smoke.script`: main menu →
options (text field + keyboard) → story → first track → drive with the touch
buttons → back to the menu. Screenshots go to `build/host/run/shots/`.

```bash
sudo apt install libegl-dev libgles-dev libgl1-mesa-dri python3-pil
scripts/test_linux.sh
MANIA_SIZE=1920x1080 MANIA_SCRIPT=my.script scripts/test_linux.sh
```

Script commands (`<frame> <command> [args]`, coordinates 0..1 from the top
left): `tap X Y`, `down ID X Y`, `move ID X Y`, `up ID X Y`, `special KEY 0|1`
(`up`, `down`, `left`, `right`, `f1`..), `char C`, `key N` (ASCII code),
`shot NAME.ppm`, `quit`. `MANIA_FRAMES=N` stops after N frames.

---

## 4. Signing

Android only installs signed APKs, and an **update must be signed with the
same key** as the installed version.

- On the first build the script creates `android/maniadrive.keystore`
  (alias `maniadrive`, password `maniadrive`, RSA 2048, 10,000 days).
  **Keep a backup** of it; it is never committed (`.gitignore`).
- Your own key:
  ```bash
  keytool -genkeypair -v -keystore ~/keys/maniadrive-release.jks -alias release \
          -keyalg RSA -keysize 2048 -validity 10000
  KEYSTORE=~/keys/maniadrive-release.jks KS_ALIAS=release KS_PASS='secret' ./build_android.sh
  ```
- GitHub Actions: add the repository secrets `KEYSTORE_BASE64`
  (`base64 -w0 android/maniadrive.keystore`), `KS_PASS` and `KS_ALIAS`, so the
  CI APKs are signed with your key. Without them every CI run uses a new
  temporary key (fine for testing, but users must uninstall before updating).
- Check a signature: `$ANDROID_HOME/build-tools/34.0.0/apksigner verify --print-certs build/ManiaDrive.apk`

---

## 5. How the port works

```
ManiaDrive game (mania_drive.c, unchanged logic)
    │ #include
Raydium engine 0.705 (src/raydium, small #ifdef MANIA_PORT patches)
    │ OpenGL 1.x          │ OpenAL + ALUT        │ myglut (window/input)   │ PHP scripts
    ▼                     ▼                      ▼                         ▼
  gl4es → GLES 2.0     OpenAL Soft →        src/port/myglut-android.c   src/port/php_shim.c
                       OpenSL ES            (EGL, NativeActivity input,  (same logic in C)
                       + port/alut_mini.c    fps limit, pause/resume)  
```

| File | Role |
|---|---|
| `src/port/android_main.c` | entry point: logcat redirection, extracts `assets/game` to internal storage (first start / after an update, checked with the manifest hash), sets `HOME`, calls the game's `main()` |
| `src/port/myglut-android.c` | Raydium's `myglut` API for Android: EGL ES2 context, surface re-creation after pause, touch / keys / gamepad events, soft keyboard (JNI), frame limiter. Also a headless backend for the PC test |
| `src/port/touch_overlay.c` | on-screen buttons while driving; touch = mouse in menus; opens the keyboard when a text field gets the focus |
| `src/port/php_shim.c` | C versions of `mania_story.php`, `mania_localtracks.php`, `mania_playlist.php`, `mania_tracklist.php`, `mania_score*.php` |
| `src/port/alut_mini.c` | `alutInit`/`alutExit` and a WAV loader (freealut does not exist on Android) |
| `src/port/include/GL/glew.h` | GLEW stub (gl4es exports the extension functions) |
| `android/java/.../MainActivity.java` | immersive full screen, soft keyboard, characters typed on it, Wi-Fi multicast lock for LAN discovery |

**Files**: the engine reads its data with `fopen()`, so the data is extracted
once to `/data/data/org.maniadrive.android/files/game` (the working
directory). Settings (`raydium.db`), story progress (`mania_drive.state`) and
generated tracks live in `files/.mania_drive` and are never overwritten.

**Time**: Raydium's clock (`raydium_timecall_clock`) uses a monotonic clock
that stops while the app is in the background, so physics and timers do not
jump after a pause. ODE runs at a fixed 400 Hz independently of the display
frame rate.

**Frame rate**: unlocked by default (`PORT_FPS 0` in `src/port/port.h`):
`eglSwapInterval(0)` and no limiter, so the game runs as fast as the phone
allows (physics stay correct because ODE uses its own fixed 400 Hz clock).
To lock it to 60 fps again, add `-DPORT_FPS=60` to `COMMON` in
`build_android.sh`: this enables vsync, `ANativeWindow_setFrameRate(60)` on
Android 11+ and a limiter after each swap (1 ms margin so a 60 Hz vsync is
never missed; 90/120 Hz screens also run at 60 fps).

**Resolution**: the EGL surface is at most 1080 pixels high (the display
scaler does the rest); *Options → Max. resolution* (the PC "window mode"
option) lowers this limit.

### Engine patches (all inside `#ifdef MANIA_PORT`)

| File | Change |
|---|---|
| `raydium/myglut.c`, `index.c` | use the Android backend; include `src/port/port_engine.c` |
| `raydium/common.h` | no V4L webcam support, include `port.h` |
| `raydium/timecall.c` | monotonic, pause-aware clock |
| `raydium/render.c` | FPS counter used `clock()` (CPU time): wrong once the limiter sleeps |
| `raydium/joy.c` | no `/dev/js0`; gamepad sticks come from Android |
| `raydium/ode.c`, `ode.h` | `dInitODE2()` (needed by ODE ≥ 0.10), system ODE headers |
| `raydium/shader.c` | GLSL support off (not used by ManiaDrive, see below) |
| `raydium/osd.c` | no mouse arrow on touch screens |
| `raydium/network.c` | default player name "Player" instead of `u0_a123` |
| `raydium/live.c/h` | V4L code disabled |
| `mania_drive.c` | touch-mode hint for the overlay, Back key navigation, no Internet Tracks button, touch help text, resolution option, shadows off by default, `exit()` → `port_exit()` (finishes the activity) |

---

## 6. Problems and pitfalls, with solutions

Rows marked **●** happened during this port.

| Symptom | Cause | Solution |
|---|---|---|
| **●** crash in gl4es `realize_glenv()` on the first draw | gl4es probes the GPU with its own EGL context and then calls `eglTerminate()`, which destroyed the game's context; every shader compile then failed | initialise gl4es (`initialize_gl4es()`) **before** creating the game's EGL display/context |
| **●** crash in `glUseProgramObjectARB(0)` | Raydium detected GLSL support and switched programs every texture change | `raydium_shader_support=0` (ManiaDrive uses no shaders) |
| **●** `call to undeclared function` errors | clang ≥ 16 treats implicit declarations as errors (2006 C code) | prototypes in `port.h`, `<sys/stat.h>`/`<ctype.h>` added |
| **●** `d8`: `NullPointerException ... MainActivity$1.class` | d8 of build-tools 34 fails on anonymous classes compiled by JDK 21 | named nested class `KeyboardTask` |
| **●** black screen and "not responding" after choosing a track | ARM `char` is unsigned: `mni_generate()` stores `fgetc()` in a `char`, never sees `EOF` and writes the track file forever | build with `-fsigned-char` (same behaviour as the PC version) |
| physics objects fall through / crash in collisions | ODE ≥ 0.10 must be initialised | `dInitODE2(0)` + `dAllocateODEDataForThread()` |
| `dReal` mismatch with OpenGL floats | Raydium expects single-precision ODE | ODE built with `-DODE_DOUBLE_PRECISION=OFF` |
| no PHP on Android | story, track lists and playlist were PHP scripts | rewritten in C (`php_shim.c`), same variables through the Raydium register API |
| "Internet Tracks" never loads | CQFD Corp. servers are offline | menu entry removed, cached list kept |
| FPS display wrong with the limiter | `clock()` is CPU time | monotonic clock |
| game time jumps after the app was in background | wall clock kept running | pause-aware clock, audio paused (`alcDevicePauseSOFT`) |
| ANR on first start | extraction blocked the event loop | events are pumped between files |
| Android user name `u0_a…` in the HUD | `getpwuid()` | default name "Player" (editable in Options) |
| Back key quit the game from any menu | Esc = quit in the PC menus | Back closes popups / returns to the main menu first |

### Installing

| Message | Fix |
|---|---|
| "App not installed" / `INSTALL_FAILED_UPDATE_INCOMPATIBLE` | the installed version was signed with another key: uninstall it first (story progress is lost) |
| `INSTALL_FAILED_NO_MATCHING_ABIS` | build the right ABI (`armeabi-v7a` for old 32-bit phones) |
| "Blocked by Play Protect" | self-signed APK: *Install anyway* |
| Black screen | `adb logcat -s ManiaDrive LIBGL` |
