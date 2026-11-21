#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
./motor/COMPILAR_MOTOR.sh
# Evitar la SDL2 empaquetada en el enlace de CI: requiere otra version de glibc.
# Preferir siempre la biblioteca de la plataforma donde se compila.
if pkg-config --exists sdl2 2>/dev/null; then
  read -r -a SDL_FLAGS <<<"$(pkg-config --libs sdl2)"
else
  SDL_FLAGS=(-l:libSDL2-2.0.so.0)
fi
g++ -std=c++17 -O2 -pipe fuentes/contacto_native.cpp motor/libgritnova.a \
  -o CONTACTO_2010_LINUX "${SDL_FLAGS[@]}" -lpng -pthread
printf 'CONTACTO 2010 V1.1: compilacion SDL2 del sistema correcta.\n'
