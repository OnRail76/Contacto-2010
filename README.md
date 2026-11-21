# CONTACTO 2010 · V1.1

![Carátula oficial de CONTACTO 2010](documentacion/CONTACTO_2010_CARATULA_OFICIAL.png)

Arcade horizontal de ciencia ficción para **Linux x86_64**, creado por **Tony C. Ware** y desarrollado en **C++17 y SDL2**, sobre **GRIT NOVA ENGINE**. Música original: **FutureDark**, alias musical de Tony C. Ware. Programación y desarrollo: **GPT-6 · OpenAI**.

![Captura real de CONTACTO 2010](capturas/CAPTURA_FASE_1.png)

**Cinco fases, combates arcade, enemigos animados, banda sonora original y la nave Firebird-01.** Esta edición V1.1 incluye su nuevo diseño industrial y el Cementerio de Escombros V2.

## Descargar y jugar

[**Release V1.1 — todas las descargas**](https://github.com/OnRail76/Contacto-2010/releases/tag/v1.1)

- [AppImage x86_64 para Linux](https://github.com/OnRail76/Contacto-2010/releases/download/v1.1/Contacto_2010_V1.1-x86_64.AppImage): un único archivo ejecutable; dar permiso con `chmod +x` y abrir.
- [Windows x64, EXE nativo y DLL en ZIP](https://github.com/OnRail76/Contacto-2010/releases/download/v1.1/CONTACTO_2010_V1.1_WINDOWS_x64.zip): descomprimir entero y abrir `CONTACTO_2010.exe`.
- [Linux x86_64, ZIP clásico](https://github.com/OnRail76/Contacto-2010/releases/download/v1.1/CONTACTO_2010_V1.1_LINUX_COMPLETO.zip): descomprimir y ejecutar `./JUGAR_CONTACTO_2010.sh`.

Las dos distribuciones nuevas se construyen y se prueban de forma nativa en GitHub Actions. El EXE no depende de Wine ni de Python.

Movimiento: **WASD o cursores**. Disparo: **ESPACIO**. Pausa: **ESC**. Pantalla completa: **F11**. Compatible con mando SDL2 con reconexión.

Para compilar desde las fuentes: `bash COMPILAR_LINUX.sh` en Linux, o MSYS2 UCRT64 y `bash packaging/windows/COMPILAR_WINDOWS.sh` en Windows.

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
