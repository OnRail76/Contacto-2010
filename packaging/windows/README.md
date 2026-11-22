# CONTACTO 2010 V1.2.1 — Windows x64

Compilación nativa con MSYS2 UCRT64, C++17, SDL2, libpng y libvorbisfile.
El icono oficial ICO se incorpora al EXE mediante windres.

El workflow `.github/workflows/verificacion-linux-windows.yml` comprueba el ejecutable Windows en GitHub Actions y permite descargar un paquete de verificación sin alterar la Release oficial.

Para compilar en MSYS2 UCRT64: `bash packaging/windows/COMPILAR_WINDOWS.sh`.
Para empaquetar, con las DLL nativas y las diez pistas OGG estéreo:
`bash packaging/windows/EMPAQUETAR_WINDOWS.sh`.

Los WAV de efectos se conservan. Los antiguos WAV de música ya no se distribuyen.
El ejecutable Windows es nativo; no necesita Wine ni Python.

Versión vigente: https://github.com/OnRail76/Contacto-2010/releases/tag/v1.2.1
