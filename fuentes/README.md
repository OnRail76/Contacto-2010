# Código fuente de CONTACTO 2010 V1.2.1

`contacto_native.cpp` contiene las reglas del juego y usa el núcleo reutilizable `motor/grit_nova_engine.hpp`. Los módulos del motor están separados de los eventos y la lógica específica de CONTACTO 2010.

Incluye las cinco fases, enemigos, jefes, AURA-01 (computadora de a bordo), HUD, colisiones, disparos, radio, controles, grabación de récord, música OGG estéreo, efectos espaciales, scanlines CRT y reproductor de cassette.

**Recursos activos:** `assets/` contiene las imágenes del juego y los efectos WAV. Las diez pistas de FutureDark en `banda_sonora_original/` son los únicos archivos musicales empleados por el motor.

**Compilar en Linux x86_64 / Bazzite:** `bash COMPILAR_LINUX.sh` desde la raíz del repositorio. Dependencias: C++17, SDL2, libpng y libvorbisfile.

**Compilar en Windows x64:** usar MSYS2 UCRT64, `bash packaging/windows/COMPILAR_WINDOWS.sh`, seguido de `bash packaging/windows/EMPAQUETAR_WINDOWS.sh`.

**Verificar:** `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./CONTACTO_2010_LINUX --self-test-aura`. Los tests completos y el empaquetado están disponibles en GitHub Actions.

**Publicación vigente:** https://github.com/OnRail76/Contacto-2010/releases/tag/v1.2.1

**Licencias:** software MIT, arte y música bajo sus derechos específicos de `LICENCIA_RECURSOS.md`. Música original: **FutureDark**.
