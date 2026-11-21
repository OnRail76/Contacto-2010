# CONTACTO 2010 V1.1 para Windows x64

Build nativa con MSYS2 UCRT64. Fuentes C++17 + SDL2 + libpng.
El icono oficial ICO se incorpora al ejecutable mediante windres.

El workflow `.github/workflows/windows.yml` compila en Windows,
realiza pruebas reales en el runner Windows, incluye las DLL UCRT64,
los recursos PNG/WAV y la banda sonora OGG, y publica un ZIP
descargable en la Release v1.1 **solo si las pruebas han pasado**.

Para compilar en MSYS2 UCRT64: `bash packaging/windows/COMPILAR_WINDOWS.sh`
y para empaquetar: `bash packaging/windows/EMPAQUETAR_WINDOWS.sh`.
No es Wine, un emulador, ni un ejecutable Linux renombrado.
