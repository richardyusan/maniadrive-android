#!/usr/bin/env bash
# Build the static third-party libraries for one target.
#
#   scripts/build_deps.sh arm64-v8a      (Android ABI: arm64-v8a, armeabi-v7a, x86_64, x86)
#   scripts/build_deps.sh host           (Linux PC build, used by scripts/test_linux.sh)
#
# Output: build/deps/<target>/{include,lib}
set -euo pipefail
cd "$(dirname "$0")/.."
ROOT=$PWD
TARGET=${1:?usage: build_deps.sh <abi|host>}
TP=$ROOT/third_party
OUT=$ROOT/build/deps/$TARGET
WORK=$ROOT/build/deps-work/$TARGET
JOBS=${JOBS:-$(nproc)}
API=${API:-26}

[ -d "$TP/ode" ] || "$ROOT/scripts/fetch_deps.sh"
mkdir -p "$OUT" "$WORK"

if [ -f "$OUT/.done" ]; then
  echo "* deps for $TARGET already built ($OUT)"; exit 0
fi

COMMON=(-G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$OUT"
        -DCMAKE_PREFIX_PATH="$OUT" -DCMAKE_FIND_ROOT_PATH="$OUT"
        -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5)
if [ "$TARGET" != host ]; then
  NDK=${NDK:-${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}}
  [ -d "$NDK" ] || { echo "Set NDK / ANDROID_NDK_HOME"; exit 1; }
  COMMON+=(-DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake"
           -DANDROID_ABI="$TARGET" -DANDROID_PLATFORM=android-$API
           -DANDROID_STL=c++_static)
fi

build() { # name srcdir [cmake args...]
  local name=$1 src=$2; shift 2
  echo "* building $name for $TARGET"
  rm -rf "$WORK/$name"
  cmake -S "$src" -B "$WORK/$name" "${COMMON[@]}" "$@" > "$WORK/$name.log" 2>&1 \
    || { tail -40 "$WORK/$name.log"; exit 1; }
  cmake --build "$WORK/$name" -j "$JOBS" >> "$WORK/$name.log" 2>&1 \
    || { tail -40 "$WORK/$name.log"; exit 1; }
  cmake --install "$WORK/$name" >> "$WORK/$name.log" 2>&1 \
    || { tail -40 "$WORK/$name.log"; exit 1; }
}

# --- ODE: single precision, OPCODE trimesh collider (as the original Raydium build)
build ode "$TP/ode" -DODE_DOUBLE_PRECISION=OFF -DODE_WITH_DEMOS=OFF -DODE_WITH_TESTS=OFF \
      -DODE_WITH_OPCODE=ON -DODE_WITH_GIMPACT=OFF -DODE_WITH_LIBCCD=OFF -DODE_WITH_OU=OFF \
      -DODE_NO_BUILTIN_THREADING_IMPL=ON -DODE_NO_THREADING_INTF=ON

# --- Ogg / Vorbis (music)
build ogg "$TP/ogg" -DINSTALL_DOCS=OFF
build vorbis "$TP/vorbis"

# --- libjpeg (menu videos are JPEG sequences)
SIMD=ON; [ "$TARGET" = x86_64 ] || [ "$TARGET" = x86 ] || [ "$TARGET" = host ] && SIMD=OFF
build libjpeg-turbo "$TP/libjpeg-turbo" -DENABLE_SHARED=OFF -DENABLE_STATIC=ON \
      -DWITH_TURBOJPEG=OFF -DWITH_TOOLS=OFF -DWITH_TESTS=OFF -DWITH_SIMD=$SIMD \
      -DCMAKE_INSTALL_LIBDIR=lib

# --- OpenAL Soft (sound), static
AL=(-DLIBTYPE=STATIC -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_TESTS=OFF
    -DALSOFT_INSTALL_CONFIG=OFF -DALSOFT_INSTALL_HRTF_DATA=OFF -DALSOFT_INSTALL_AMBDEC_PRESETS=OFF
    -DALSOFT_EMBED_HRTF_DATA=OFF -DALSOFT_UPDATE_BUILD_VERSION=OFF -DALSOFT_NO_CONFIG_UTIL=ON
    -DALSOFT_BACKEND_PIPEWIRE=OFF -DALSOFT_BACKEND_JACK=OFF -DALSOFT_BACKEND_PORTAUDIO=OFF
    -DALSOFT_BACKEND_PULSEAUDIO=OFF -DALSOFT_BACKEND_OBOE=OFF -DALSOFT_BACKEND_SNDIO=OFF
    -DALSOFT_BACKEND_ALSA=OFF -DALSOFT_BACKEND_OSS=OFF -DALSOFT_BACKEND_SOLARIS=OFF
    -DALSOFT_BACKEND_WAVE=ON -DALSOFT_EAX=OFF -DCMAKE_INSTALL_LIBDIR=lib)
if [ "$TARGET" = host ]; then
  AL+=(-DALSOFT_BACKEND_OPENSL=OFF -DALSOFT_REQUIRE_OPENSL=OFF)
else
  AL+=(-DALSOFT_BACKEND_OPENSL=ON -DALSOFT_REQUIRE_OPENSL=ON)
fi
build openal-soft "$TP/openal-soft" "${AL[@]}"

# --- gl4es: OpenGL 1.x -> OpenGL ES 2.0 translation, static, initialised by the port
GL=(-DSTATICLIB=ON -DNOX11=ON -DDEFAULT_ES=2 -DNO_INIT_CONSTRUCTOR=ON -DUSE_CLOCK=ON)
if [ "$TARGET" = host ]; then
  GL+=(-DNOEGL=OFF)
else
  GL+=(-DANDROID=ON -DUSE_ANDROID_LOG=ON)
fi
echo "* building gl4es for $TARGET"
rm -rf "$WORK/gl4es"
cmake -S "$TP/gl4es" -B "$WORK/gl4es" "${COMMON[@]}" "${GL[@]}" > "$WORK/gl4es.log" 2>&1 \
  || { tail -40 "$WORK/gl4es.log"; exit 1; }
cmake --build "$WORK/gl4es" -j "$JOBS" --target GL >> "$WORK/gl4es.log" 2>&1 \
  || { tail -40 "$WORK/gl4es.log"; exit 1; }
cp "$(find "$WORK/gl4es" "$TP/gl4es/lib" -name 'libGL*.a' 2>/dev/null | head -1)" "$OUT/lib/libgl4es.a"
mkdir -p "$OUT/include/gl4es"
cp -r "$TP/gl4es/include/." "$OUT/include/gl4es/"

touch "$OUT/.done"
echo "* deps for $TARGET installed in $OUT"
