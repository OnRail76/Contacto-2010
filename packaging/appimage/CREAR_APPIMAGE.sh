#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
python3 "$HERE/crear_appdir.py"
APPIMAGETOOL="${APPIMAGETOOL:-$(command -v appimagetool || true)}"
if [[ -z "$APPIMAGETOOL" ]]; then
  echo 'Descarga appimagetool desde https://github.com/AppImage/appimagetool/releases y define APPIMAGETOOL=/ruta/archivo' >&2
  exit 2
fi
cd "$ROOT/dist/appimage"
export ARCH=x86_64 VERSION=1.3 APPIMAGE_EXTRACT_AND_RUN=1
"$APPIMAGETOOL" Contacto2010.AppDir "Contacto_2010_V1.3-x86_64.AppImage"
chmod +x "Contacto_2010_V1.3-x86_64.AppImage"
echo "Generado: $ROOT/dist/appimage/Contacto_2010_V1.3-x86_64.AppImage"
