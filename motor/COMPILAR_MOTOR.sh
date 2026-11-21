#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
g++ -std=c++17 -O2 -Wall -Wextra -c motor/grit_nova_engine.cpp -o motor/grit_nova_engine.o
ar rcs motor/libgritnova.a motor/grit_nova_engine.o
printf 'GRIT NOVA ENGINE: biblioteca estática motor/libgritnova.a construida.\n'
