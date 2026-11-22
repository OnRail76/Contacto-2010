# Construcción de AppImage x86_64

El icono y el archivo `.desktop` siguen la especificación AppDir. El ejecutable y los recursos reales se copian a `dist/appimage/Contacto2010.AppDir/`. `AppRun` conserva récord y configuración en `~/.local/share/contacto2010` fuera del montaje temporal de solo lectura.

1. Compilar `./COMPILAR_LINUX.sh` en Linux x86_64.
2. Descargar `appimagetool` oficial de https://github.com/AppImage/appimagetool/releases.
3. Ejecutar `APPIMAGETOOL=/ruta/appimagetool-x86_64.AppImage bash packaging/appimage/CREAR_APPIMAGE.sh`.

Para ejecutar pruebas sin AppImage `python3 packaging/appimage/crear_appdir.py && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy dist/appimage/Contacto2010.AppDir/AppRun --self-test`.

El AppImage oficial V1.2.1 compilado y probado se descarga desde la Release. Esta carpeta contiene las herramientas para reproducir la construcción; la música se carga desde los OGG estéreo de `banda_sonora_original/`.

Compatible con distribuciones Linux x86_64 como Bazzite; en Bazzite puede ser necesario `APPIMAGE_EXTRACT_AND_RUN=1` si el montaje FUSE no está disponible.
