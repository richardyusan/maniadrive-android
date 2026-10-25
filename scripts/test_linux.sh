#!/usr/bin/env bash
# Build the Android port's code for a Linux PC (off-screen EGL, Mesa) and run
# a scripted smoke test that takes screenshots. No phone needed.
#
#   scripts/test_linux.sh                 -> build + run tests/smoke.script
#   MANIA_SCRIPT=my.script scripts/test_linux.sh
#
# Needs: libegl-dev libgles-dev (Mesa), cmake, ninja.
set -euo pipefail
cd "$(dirname "$0")/.."
ROOT=$PWD
D=$ROOT/build/deps/host
OUT=$ROOT/build/host
mkdir -p "$OUT"

[ -f "$D/.done" ] || scripts/build_deps.sh host

CFLAGS=(-O1 -g ${EXTRA_CFLAGS:-} -fwrapv -fno-strict-aliasing -fcommon -fsigned-char -w
        -DMANIA_PORT -DMANIA_HEADLESS -DNO_PHP_SUPPORT -DLIBRAY
        -I"$ROOT/src/port/include" -I"$ROOT/src/port"
        -I"$D/include/gl4es" -I"$D/include")

echo "* compiling GLU"
GLU=$ROOT/third_party/GLU
GLU_OBJS=()
for f in error glue mipmap project quad registry; do
  cc -O2 -w -c "$GLU/src/libutil/$f.c" -o "$OUT/glu_$f.o" \
     -I"$GLU/include" -I"$GLU/src/include" -I"$D/include/gl4es" -DLIBRARYBUILD
  GLU_OBJS+=("$OUT/glu_$f.o")
done

echo "* compiling mania_drive (host, headless)"
cc "${CFLAGS[@]}" -c src/mania_drive.c -o "$OUT/mania_drive.o"
c++ -o "$OUT/mania_drive" "$OUT/mania_drive.o" "${GLU_OBJS[@]}" \
    "$D/lib/libgl4es.a" "$D/lib/libode.a" "$D/lib/libopenal.a" \
    "$D/lib/libvorbisfile.a" "$D/lib/libvorbis.a" "$D/lib/libogg.a" "$D/lib/libjpeg.a" \
    -lEGL -lm -ldl -lpthread
echo "* built $OUT/mania_drive"

[ "${BUILD_ONLY:-0}" = 1 ] && exit 0

# run from a copy of the game data, with a private HOME
RUN=$OUT/run
rm -rf "$RUN"; mkdir -p "$RUN/home" "$RUN/shots"
cp -r data "$RUN/game"
cp "$OUT/mania_drive" "$RUN/game/"
export HOME=$RUN/home
export EGL_PLATFORM=surfaceless
export LIBGL_ES=2 LIBGL_GL=21
export MANIA_SCRIPT=${MANIA_SCRIPT:-$ROOT/tests/smoke.script}
export MANIA_FRAMES=${MANIA_FRAMES:-900}
export MANIA_SHOTS=$RUN/shots
cd "$RUN/shots"
set +e
timeout ${TIMEOUT:-900} "$RUN/game/mania_drive" --repository-disable "$@" > "$OUT/run.log" 2>&1
status=$?
set -e
grep -v '^LIBGL' "$OUT/run.log" | grep -E 'headless|ERROR|Signal|signal' | grep -v 'sound: ERROR' || true
# convert screenshots to PNG when Pillow is available
python3 - "$RUN/shots" <<'PY' 2>/dev/null || true
import sys, glob, os
from PIL import Image
for f in glob.glob(os.path.join(sys.argv[1], "*.ppm")):
    Image.open(f).save(f[:-4] + ".png"); os.remove(f)
PY
echo "* exit status $status, log: $OUT/run.log, screenshots: $RUN/shots"
exit $status
