#!/usr/bin/env bash
set -euo pipefail

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/cmake/modules"

MODULE_FILES=(
  CMakeDetermineDUSKCompiler.cmake
  CMakeDUSKCompiler.cmake.in
  CMakeTestDUSKCompiler.cmake
  CMakeDUSKInformation.cmake
)

if ! command -v cmake >/dev/null 2>&1; then
  echo "error: cmake not found on PATH" >&2
  exit 1
fi

CMAKE_ROOT="$(cmake --system-information | awk -F'"' '/^CMAKE_ROOT /{print $2}')"

if [[ -z "$CMAKE_ROOT" ]]; then
  echo "error: could not determine CMAKE_ROOT" >&2
  exit 1
fi

DEST="$CMAKE_ROOT/Modules"

if [[ ! -d "$DEST" ]]; then
  echo "error: $DEST does not exist" >&2
  exit 1
fi

echo "Installing Dusk CMake modules to $DEST"

for file in "${MODULE_FILES[@]}"; do
  src="$SOURCE_DIR/$file"

  if [[ ! -f "$src" ]]; then
    echo "error: missing $src" >&2
    exit 1
  fi

  echo "  $file"
  sudo install -m 644 "$src" "$DEST/$file"
done

echo "Done." 
