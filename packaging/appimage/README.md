# Construcción de AppImage x86_64

El icono y el archivo `.desktop` siguen la especificación AppDir. El ejecutable y los recursos reales se copian a `dist/appimage/Contacto2010.AppDir/`. `AppRun` conserva récord y configuración en `~/.local/share/contacto2010` fuera del montaje temporal de solo lectura.

1. Compilar `./COMPILAR_LINUX.sh` en Linux x86_64.
2. Descargar `appimagetool` oficial de https://github.com/AppImage/appimagetool/releases.
3. Ejecutar `APPIMAGETOOL=/ruta/appimagetool-x86_64.AppImage bash packaging/appimage/CREAR_APPIMAGE.sh`.

Para ejecutar pruebas sin AppImage `python3 packaging/appimage/crear_appdir.py && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy dist/appimage/Contacto2010.AppDir/AppRun --self-test`.

No contiene un AppImage precompilado porque el creador oficial no estuvo disponible en la sesión de empaquetado inicial.
