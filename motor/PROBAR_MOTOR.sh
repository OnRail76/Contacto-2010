#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
./motor/COMPILAR_MOTOR.sh
if pkg-config --exists sdl2 2>/dev/null; then
  read -r -a SDL_FLAGS <<<"$(pkg-config --libs sdl2)"
else
  SDL_FLAGS=(-l:libSDL2-2.0.so.0)
fi
g++ -std=c++17 -O2 motor/PROBAR_MOTOR.cpp motor/libgritnova.a -o motor/probar_motor \
  "${SDL_FLAGS[@]}" -lpng -pthread
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy motor/probar_motor
