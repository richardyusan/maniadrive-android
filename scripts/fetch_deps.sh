#!/usr/bin/env bash
# Download the third-party libraries used by the Android port.
# Versions are pinned so every build uses exactly the same code.
#
#   scripts/fetch_deps.sh            -> clones into third_party/
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p third_party
cd third_party

# name  url  ref (tag or commit)
DEPS=(
  "gl4es         https://github.com/ptitSeb/gl4es                 2037865590d410bcd7b8d1f22b841472ed6d39ac"
  "GLU           https://github.com/ptitSeb/GLU                   2fed2bda2b725d2b9e32c435b48d5141cc95827f"
  "ode           https://github.com/odedevs/ode                   0.16.6"
  "openal-soft   https://github.com/kcat/openal-soft              1.24.3"
  "ogg           https://github.com/xiph/ogg                      v1.3.6"
  "vorbis        https://github.com/xiph/vorbis                   v1.3.7"
  "libjpeg-turbo https://github.com/libjpeg-turbo/libjpeg-turbo   3.1.2"
)

for line in "${DEPS[@]}"; do
  read -r name url ref <<<"$line"
  if [ -f "$name/.fetched-$ref" ]; then
    echo "* $name ($ref) already present"
    continue
  fi
  echo "* fetching $name ($ref)"
  rm -rf "$name"
  git init -q "$name"
  git -C "$name" remote add origin "$url"
  git -C "$name" fetch -q --depth 1 origin "$ref"
  git -C "$name" checkout -q FETCH_HEAD
  touch "$name/.fetched-$ref"
done
echo "done."
