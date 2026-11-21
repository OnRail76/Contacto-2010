# CONTACTO 2010 · V1.1

Arcade horizontal de ciencia ficción para **Linux x86_64**, creado por **Tony C. Ware** y desarrollado en **C++17 y SDL2**, sobre **GRIT NOVA ENGINE**. Música original: **FutureDark**, alias musical de Tony C. Ware. Programación y desarrollo: **GPT-6 · OpenAI**.

![Captura real de CONTACTO 2010](capturas/CAPTURA_FASE_1.png)

**Cinco fases, combates arcade, enemigos animados, banda sonora original y la nave Firebird-01.** Esta edición V1.1 incluye su nuevo diseño industrial y el Cementerio de Escombros V2.

## Descargar y jugar

Abre la pestaña [Releases](https://github.com/OnRail76/Contacto-2010/releases) y descarga el paquete Linux V1.1. Descomprímelo y ejecuta `./JUGAR_CONTACTO_2010.sh`. Para compilar desde el repositorio, ejecuta `./COMPILAR_LINUX.sh`.

Movimiento: **WASD o cursores**. Disparo: **ESPACIO**. Pausa: **ESC**. Pantalla completa: **F11**. Compatible con mando SDL2 con reconexión.

## Documentación

[Atlas, memoria y manual V1.1 (PDF de 44 páginas)](documentacion/ATLAS_MEMORIA_MANUAL_CONTACTO_2010_V1.1.pdf) incluye la documentación de GRIT NOVA ENGINE y material visual del juego, sin desvelar su final.

## Código y recursos

- `motor/`: servicios reutilizables de GRIT NOVA ENGINE (compilación separada).
- `fuentes/`: código del juego (fases, físicas, eventos, HUD y combates).
- `assets/`: recursos activos: sprites RGBA, geometría BIN y WAV normalizados.
- `banda_sonora_original/`: diez pistas OGG originales.
- `packaging/`: iconos oficiales y utilidades para Linux/AppImage y Windows.

Prueba automática de arranque: `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./CONTACTO_2010_LINUX --self-test`.

**Licencias:** código MIT (`LICENSE`); ilustraciones y audio conservan sus derechos originales (`LICENCIA_RECURSOS.md`).
