#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
./motor/COMPILAR_MOTOR.sh
g++ -std=c++17 -O2 motor/PROBAR_MOTOR.cpp motor/libgritnova.a -o motor/probar_motor \
  -L./lib -Wl,-rpath,'$ORIGIN/../lib' -l:libSDL2-2.0.so.0 -lpng -pthread
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy motor/probar_motor
