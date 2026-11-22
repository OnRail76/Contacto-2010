from pathlib import Path
import shutil, os, subprocess, zipfile
src=Path(__file__).resolve().parents[2]
icons=src/'packaging/iconos'
root=src/'dist/appimage'
appdir=root/'Contacto2010.AppDir'
game=appdir/'usr/share/contacto2010'
if root.exists():shutil.rmtree(root)
(game/'assets').mkdir(parents=True)
shutil.copy2(src/'CONTACTO_2010_LINUX',game/'CONTACTO_2010_LINUX')
os.chmod(game/'CONTACTO_2010_LINUX',0o755)
shutil.copytree(src/'assets',game/'assets',dirs_exist_ok=True,ignore=shutil.ignore_patterns('musica_*.wav'))
shutil.copytree(src/'banda_sonora_original',game/'banda_sonora_original',dirs_exist_ok=True)
(game/'lib').mkdir(parents=True,exist_ok=True)
# Empaquetar la SDL2 del sistema del runner Ubuntu 22.04, NO la biblioteca
# procedente de otra distribucion (dependia de GLIBC_2.38).
def find_system_library(soname):
    output=subprocess.run(['ldconfig','-p'],capture_output=True,text=True,check=True).stdout
    for line in output.splitlines():
        if soname in line and 'x86-64' in line and '=>' in line:
            candidate=Path(line.split('=>',1)[1].strip())
            if candidate.exists():return candidate
    raise RuntimeError('No se encuentra biblioteca del sistema: '+soname)
for soname in ('libSDL2-2.0.so.0','libpng16.so.16','libz.so.1','libvorbisfile.so.3','libvorbis.so.0','libogg.so.0'):
    shutil.copy2(find_system_library(soname),game/'lib'/soname)
(game/'LEEME.txt').write_text('CONTACTO 2010 V1.3 · Ejecutable y recursos de juego. Música FutureDark OGG estéreo en banda_sonora_original/. Datos persistentes: XDG_DATA_HOME/contacto2010.\n',encoding='utf-8')
(appdir/'AppRun').write_text('''#!/bin/sh
set -eu
APPROOT="${APPDIR:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}"
GAMEROOT="$APPROOT/usr/share/contacto2010"
DATAROOT="${XDG_DATA_HOME:-$HOME/.local/share}/contacto2010"
mkdir -p "$DATAROOT"
# La imagen es de sólo lectura. Recorremos desde la carpeta de datos del usuario,
# con los assets enlazados desde el montaje AppImage (no copiados cada vez).
ln -sfn "$GAMEROOT/assets" "$DATAROOT/assets"
ln -sfn "$GAMEROOT/banda_sonora_original" "$DATAROOT/banda_sonora_original"
export LD_LIBRARY_PATH="$GAMEROOT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$DATAROOT"
exec "$GAMEROOT/CONTACTO_2010_LINUX" "$@"
''',encoding='utf-8')
os.chmod(appdir/'AppRun',0o755)
# Standard AppImage desktop integration
name='contacto2010'
appdesk='''[Desktop Entry]
Type=Application
Name=CONTACTO 2010
GenericName=Matamarcianos horizontal
Comment=Arcade de ciencia ficción de Tony C. Ware
Exec=AppRun
Icon=contacto2010
Terminal=false
Categories=Game;ArcadeGame;
StartupNotify=true
X-AppImage-Version=1.3
'''
(appdir/(name+'.desktop')).write_text(appdesk,encoding='utf-8')
(apppng:=appdir/(name+'.png')).write_bytes((icons/'Linux_PNG/contacto2010_512x512.png').read_bytes())
shutil.copy2(apppng,appdir/'.DirIcon')
iconpath=appdir/'usr/share/icons/hicolor/256x256/apps';iconpath.mkdir(parents=True,exist_ok=True)
shutil.copy2(icons/'Linux_PNG/contacto2010_256x256.png',iconpath/(name+'.png'))
for s in ('48','64','128','512'):
 d=appdir/f'usr/share/icons/hicolor/{s}x{s}/apps';d.mkdir(parents=True,exist_ok=True)
 shutil.copy2(icons/'Linux_PNG'/f'contacto2010_{s}x{s}.png',d/(name+'.png'))
# build on user's Linux with official appimagetool (tool unavailable offline here)
(root/'CREAR_APPIMAGE.sh').write_text('''#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
TOOL="${APPIMAGETOOL:-}"
if [[ -z "$TOOL" ]]; then
  TOOL="$(command -v appimagetool || true)"
fi
if [[ -z "$TOOL" ]]; then
  echo 'No se encuentra appimagetool. Descárgalo de https://github.com/AppImage/appimagetool/releases' >&2
  echo 'y ejecuta APPIMAGETOOL=/ruta/al/appimagetool bash CREAR_APPIMAGE.sh' >&2
  exit 2
fi
chmod +x "$TOOL" 2>/dev/null || true
cd "$HERE"
export ARCH=x86_64
export VERSION=1.3
if [[ "$TOOL" == *.AppImage && "${APPIMAGE_EXTRACT_AND_RUN:-}" == "" ]]; then
  export APPIMAGE_EXTRACT_AND_RUN=1
fi
"$TOOL" "$HERE/Contacto2010.AppDir" "$HERE/Contacto_2010_V1.3-x86_64.AppImage"
chmod +x "$HERE/Contacto_2010_V1.3-x86_64.AppImage"
echo "AppImage generado: $HERE/Contacto_2010_V1.3-x86_64.AppImage"
''',encoding='utf-8')
os.chmod(root/'CREAR_APPIMAGE.sh',0o755)
(root/'LEEME_APPIMAGE.txt').write_text('''CONTACTO 2010 V1.3 — PAQUETE AppDir x86_64

La carpeta Contacto2010.AppDir contiene los archivos reales para crear un AppImage con icono oficial.
No se presenta falsamente como AppImage ya compilado: aquí no hay appimagetool instalado y la red de la sesión no permite descargarlo.

1. Descomprime el ZIP.
2. Descarga appimagetool-x86_64.AppImage desde el proyecto oficial https://github.com/AppImage/appimagetool/releases
3. En terminal:
    chmod +x appimagetool-x86_64.AppImage
    APPIMAGETOOL=./appimagetool-x86_64.AppImage ./CREAR_APPIMAGE.sh
4. Ejecuta:
    chmod +x Contacto_2010_V1.3-x86_64.AppImage
    ./Contacto_2010_V1.3-x86_64.AppImage

Para probar antes de empaquetar: ./Contacto2010.AppDir/AppRun --self-test
La imagen guarda record.dat y audio_config.ini en ~/.local/share/contacto2010 (o XDG_DATA_HOME).
El directorio original assets es de solo lectura, enlazado a la carpeta de datos durante cada ejecución.

No se distribuye ningún ejecutable Windows con este paquete. El ICO para Windows va aparte.
''',encoding='utf-8')
# GitHub workflow help for community reproduction
(root/'VERIFICAR_APPDIR.sh').write_text('''#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy
./Contacto2010.AppDir/AppRun --self-test
./Contacto2010.AppDir/AppRun --self-test-controles
./Contacto2010.AppDir/AppRun --self-test-fase5
printf '\\nAppDir comprobado con motor SDL2 (headless).\\n'
''',encoding='utf-8')
os.chmod(root/'VERIFICAR_APPDIR.sh',0o755)
# pack ready AppDir and reproducible script
zfile=root/'CONTACTO_2010_V1.3_APPDIR_PARA_CREAR_APPIMAGE.zip'
with zipfile.ZipFile(zfile,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=5) as z:
 for p in sorted(root.rglob('*')):
  if p.is_file() and p != zfile and not p.is_symlink():
   z.write(p,p.relative_to(root.parent))
print('AppDir prepared:',appdir)
print('ZIP:',zfile,'size:',zfile.stat().st_size)
