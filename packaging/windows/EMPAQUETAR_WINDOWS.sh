#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
OUT="$ROOT/dist/windows/CONTACTO_2010_V1.2_WINDOWS_x64"
mkdir -p "$OUT"
cp -f CONTACTO_2010.exe "$OUT/"
cp -a assets banda_sonora_original "$OUT/"
rm -f "$OUT/assets/audio/"musica_*.wav
cp LICENSE LICENCIA_RECURSOS.md "$OUT/"
cp packaging/windows/Contacto2010.ico "$OUT/"
for dll in SDL2.dll libpng16-16.dll zlib1.dll libwinpthread-1.dll libvorbisfile-3.dll libvorbis-0.dll libogg-0.dll; do
  if [[ -f "/ucrt64/bin/$dll" ]]; then cp -f "/ucrt64/bin/$dll" "$OUT/"; fi
done
# Copiar DLL UCRT64 transitivas, sin dll de Windows ni del entorno POSIX MSYS.
for pass in 1 2 3 4; do
 for binary in "$OUT/"*.exe "$OUT/"*.dll; do
  [[ -e "$binary" ]] || continue
  while IFS= read -r dep; do
    dep="${dep//$'\r'/}"
    case "$dep" in
      /ucrt64/bin/*.dll)
        if [[ -f "$dep" ]]; then cp -n "$dep" "$OUT/" || true; fi ;;
    esac
  done < <(ldd "$binary" 2>/dev/null | awk '$2=="=>" && $3 ~ /^\/ucrt64\/bin\// {print $3}')
 done
done
cat >"$OUT/JUGAR_CONTACTO_2010.bat" <<'BAT'
@echo off
cd /d "%~dp0"
start "" "CONTACTO_2010.exe"
BAT
cat >"$OUT/LEEME_WINDOWS.txt" <<'TXT'
CONTACTO 2010 V1.2 - Windows x64
Descomprime el ZIP entero y abre CONTACTO_2010.exe.
No se necesita Python, Linux ni instalador.
WASD o cursores: movimiento. ESPACIO: disparo.
ESC: pausa. F11: pantalla completa. Mando compatible SDL2.
Distribucion nativa UCRT64 con DLL, assets y musica incluidos.
Autor: Tony C. Ware. Motor: GRIT NOVA ENGINE C++17 / SDL2.
TXT
echo "Paquete Windows preparado: $OUT"
