#!/bin/sh

# Simple build system for Emscripten
# Posix-compliant

CC="emcc"
OBJ_PATH="o"
OUT_PATH="bin"
SRC_PATH="src"
FLAGS="-Wall -Wextra -g"
LIBS="-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s USE_SDL_NET=2 -lm"
OUTPUT_FORMAT="program.html" # Can be .html, .js, or .wasm

# Create necessary directories
mkdir -p "$OBJ_PATH" "$OUT_PATH"

# Build all modules
buildAll() {
  echo "... building all modules"
  for src_file in $SRC_PATH/*.c; do
    obj_file="$OBJ_PATH/$(basename "${src_file%.*}").o"
    echo "... compiling $src_file -> $obj_file"
    $CC -c "$src_file" -o "$obj_file" $FLAGS || {
      echo "ERR: Failed to compile $src_file" >&2
      exit 1
    }
  done

  # Link final executable
  echo "... linking executable"
  $CC $OBJ_PATH/*.o -o "$OUT_PATH/$OUTPUT_FORMAT" $FLAGS $LIBS || {
    echo "ERR: Failed to link executable" >&2
    exit 1
  }
  echo ".// build complete: $OUT_PATH/$OUTPUT_FORMAT"
}

# Clean build files
clean() {
  echo "... cleaning build files"
  rm -rf "$OBJ_PATH"/* "$OUT_PATH"/*
}

# Control script
case $1 in
  clean) clean ;;
  *) buildAll ;;
esac
