#!/usr/bin/env bash
# Builds build/ManiaDrive.apk with the Android SDK/NDK command-line tools
# (no Gradle). See BUILD.md.
#
#   ./build_android.sh                     full APK (arm64-v8a, armeabi-v7a, x86_64)
#   ABIS=arm64-v8a ./build_android.sh      faster build for one ABI
#   LIBS_ONLY=1 ./build_android.sh         native libraries only (CI check)
set -euo pipefail
cd "$(dirname "$0")"
ROOT=$PWD

# ------------------------------------------------------------------ config
NDK=${NDK:-${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}}
SDK=${SDK:-${ANDROID_HOME:-${ANDROID_SDK_ROOT:-}}}
API=${API:-26}
ABIS=${ABIS:-"arm64-v8a armeabi-v7a x86_64"}
PLATFORM=${PLATFORM:-android-34}
VERSION_CODE=${VERSION_CODE:-1}
VERSION_NAME=${VERSION_NAME:-1.2-android1}
KEYSTORE=${KEYSTORE:-$ROOT/android/maniadrive.keystore}
KS_ALIAS=${KS_ALIAS:-maniadrive}
KS_PASS=${KS_PASS:-maniadrive}
OUT=$ROOT/build/android
APK=$ROOT/build/ManiaDrive.apk
export NDK

[ -d "$NDK" ] || { echo "error: set ANDROID_NDK_HOME (or NDK) to your NDK folder"; exit 1; }
HOST_TAG=$(ls "$NDK/toolchains/llvm/prebuilt" | head -1)
TC=$NDK/toolchains/llvm/prebuilt/$HOST_TAG
GLUE=$NDK/sources/android/native_app_glue

echo "== ManiaDrive for Android $VERSION_NAME ($VERSION_CODE), ABIs: $ABIS"
mkdir -p "$OUT"

# ------------------------------------------------------------------ 1. dependencies
[ -d third_party/ode ] || scripts/fetch_deps.sh
for abi in $ABIS; do scripts/build_deps.sh "$abi"; done

# ------------------------------------------------------------------ 2. native code
triple() {
  case $1 in
    arm64-v8a)   echo aarch64-linux-android$API ;;
    armeabi-v7a) echo armv7a-linux-androideabi$API ;;
    x86_64)      echo x86_64-linux-android$API ;;
    x86)         echo i686-linux-android$API ;;
    *) echo "unknown ABI $1" >&2; exit 1 ;;
  esac
}

build_abi() {
  local abi=$1 D=$ROOT/build/deps/$1 O=$OUT/obj/$1 T
  T=$(triple "$abi")
  local CC="$TC/bin/clang --target=$T"
  mkdir -p "$O" "$OUT/lib/$abi"
  local COMMON=(-fPIC -O2 -fwrapv -fno-strict-aliasing -fcommon -fsigned-char -ffunction-sections -fdata-sections
                -DANDROID -DMANIA_PORT -DNO_PHP_SUPPORT -DLIBRAY
                -I"$ROOT/src/port/include" -I"$ROOT/src/port" -I"$GLUE"
                -I"$D/include/gl4es" -I"$D/include")
  [ "$abi" = armeabi-v7a ] && COMMON+=(-mfpu=neon -mfloat-abi=softfp)

  echo "  [$abi] mania_drive.c (game + Raydium engine)"
  $CC "${COMMON[@]}" -w -Dmain=maniadrive_main -c src/mania_drive.c -o "$O/mania_drive.o" &
  local p1=$!
  $CC "${COMMON[@]}" -Wall -Werror=unguarded-availability -c src/port/android_main.c -o "$O/android_main.o" &
  local p2=$!
  $CC "${COMMON[@]}" -w -c "$GLUE/android_native_app_glue.c" -o "$O/glue.o" &
  local p3=$!
  local GLU=$ROOT/third_party/GLU
  for f in error glue mipmap project quad registry; do
    $CC -fPIC -O2 -w -DLIBRARYBUILD -I"$GLU/include" -I"$GLU/src/include" -I"$D/include/gl4es" \
        -c "$GLU/src/libutil/$f.c" -o "$O/glu_$f.o"
  done
  wait $p1; wait $p2; wait $p3          # any failure stops the script (set -e)

  echo "  [$abi] linking libmaniadrive.so"
  $TC/bin/clang++ --target=$T -shared -o "$OUT/lib/$abi/libmaniadrive.so" \
      "$O/mania_drive.o" "$O/android_main.o" "$O/glue.o" "$O"/glu_*.o \
      "$D/lib/libgl4es.a" "$D/lib/libode.a" "$D/lib/libopenal.a" \
      "$D/lib/libvorbisfile.a" "$D/lib/libvorbis.a" "$D/lib/libogg.a" "$D/lib/libjpeg.a" \
      -static-libstdc++ -Wl,--no-undefined -Wl,--gc-sections -Wl,-z,max-page-size=16384 \
      -Wl,--exclude-libs,ALL -u ANativeActivity_onCreate \
      -u Java_org_maniadrive_android_MainActivity_nativeChar \
      -landroid -llog -lEGL -lOpenSLES -lm -ldl
  $TC/bin/llvm-strip --strip-unneeded "$OUT/lib/$abi/libmaniadrive.so"
  echo "  [$abi] $(du -h "$OUT/lib/$abi/libmaniadrive.so" | cut -f1)"
}

echo "== native libraries"
for abi in $ABIS; do build_abi "$abi"; done
if [ "${LIBS_ONLY:-0}" = 1 ]; then echo "LIBS_ONLY: done"; exit 0; fi

# ------------------------------------------------------------------ 3. assets
echo "== game data"
[ -d "$SDK" ] || { echo "error: set ANDROID_HOME (or SDK) to your SDK folder"; exit 1; }
BT=${BUILD_TOOLS:-$(ls -d "$SDK"/build-tools/* | sort -V | tail -1)}
JAR=$SDK/platforms/$PLATFORM/android.jar
[ -f "$JAR" ] || { echo "error: $JAR missing (sdkmanager \"platforms;$PLATFORM\")"; exit 1; }
ASSETS=$OUT/assets
rm -rf "$ASSETS"; mkdir -p "$ASSETS/game"
cp -r data/. "$ASSETS/game/"
BUILD_ID="$VERSION_NAME-$(cd data && find . -type f -print0 | sort -z | xargs -0 cat | sha1sum | cut -c1-12)"
{ echo "#build $BUILD_ID"; (cd "$ASSETS/game" && find . -type f | sed 's|^\./||' | sort); } > "$ASSETS/manifest.txt"
echo "  $(($(wc -l < "$ASSETS/manifest.txt") - 1)) files, $(du -sh "$ASSETS/game" | cut -f1)"

# ------------------------------------------------------------------ 4. Java
echo "== Java"
rm -rf "$OUT/classes" "$OUT/dex"; mkdir -p "$OUT/classes" "$OUT/dex"
javac -source 8 -target 8 -nowarn -Xlint:-options -bootclasspath "$JAR" -classpath "$JAR" \
      -d "$OUT/classes" android/java/org/maniadrive/android/MainActivity.java
"$BT/d8" --release --min-api $API --lib "$JAR" --output "$OUT/dex" $(find "$OUT/classes" -name '*.class')

# ------------------------------------------------------------------ 5. package
echo "== packaging"
rm -rf "$OUT/res.zip" "$OUT/unsigned.apk" "$OUT/apk"
"$BT/aapt2" compile --dir android/res -o "$OUT/res.zip"
"$BT/aapt2" link -o "$OUT/unsigned.apk" -I "$JAR" --manifest android/AndroidManifest.xml \
     --version-code "$VERSION_CODE" --version-name "$VERSION_NAME" \
     --min-sdk-version $API --target-sdk-version 34 \
     -A "$ASSETS" "$OUT/res.zip" --auto-add-overlay
mkdir -p "$OUT/apk"
cp "$OUT/dex/classes.dex" "$OUT/apk/"
cp -r "$OUT/lib" "$OUT/apk/"
for abi in $(ls "$OUT/apk/lib"); do
  case " $ABIS " in *" $abi "*) ;; *) rm -rf "$OUT/apk/lib/$abi" ;; esac
done
(cd "$OUT/apk" && zip -q -r "$OUT/unsigned.apk" classes.dex lib)

# ------------------------------------------------------------------ 6. sign
if [ ! -f "$KEYSTORE" ]; then
  echo "== creating signing key $KEYSTORE (keep a backup!)"
  keytool -genkeypair -keystore "$KEYSTORE" -alias "$KS_ALIAS" -storepass "$KS_PASS" -keypass "$KS_PASS" \
          -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=ManiaDrive Android, O=maniadrive-android" >/dev/null 2>&1
fi
"$BT/zipalign" -f -p 4 "$OUT/unsigned.apk" "$OUT/aligned.apk"
"$BT/apksigner" sign --ks "$KEYSTORE" --ks-key-alias "$KS_ALIAS" --ks-pass "pass:$KS_PASS" \
     --key-pass "pass:$KS_PASS" --out "$APK" "$OUT/aligned.apk"
"$BT/apksigner" verify "$APK"
echo "== done: $APK ($(du -h "$APK" | cut -f1))"
