#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
# Compilacion Windows x64 nativa con MSYS2 UCRT64.
windres -I packaging/windows -i packaging/windows/contacto2010.rc \
  -o packaging/windows/contacto2010_res.o -O coff
"${CXX:-g++}" -std=c++17 -O2 -pipe \
  fuentes/contacto_native.cpp motor/grit_nova_engine.cpp \
  packaging/windows/contacto2010_res.o \
  -o CONTACTO_2010.exe -lSDL2 -lpng -lz -mwindows
echo 'CONTACTO 2010 Windows x64: EXE compilado.'
