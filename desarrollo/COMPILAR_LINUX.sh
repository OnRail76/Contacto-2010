#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
./motor/COMPILAR_MOTOR.sh
g++ -std=c++17 -O2 -pipe fuentes/contacto_native.cpp motor/libgritnova.a \
  -o CONTACTO_2010_LINUX -L./lib -Wl,-rpath,'$ORIGIN/lib' \
  -l:libSDL2-2.0.so.0 -lpng -pthread
printf 'CONTACTO 2010 V1.1: compilación modular correcta.\n'
