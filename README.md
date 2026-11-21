# CONTACTO 2010 · V1.2.1

![Carátula oficial de CONTACTO 2010](documentacion/CONTACTO_2010_CARATULA_OFICIAL.png)

Arcade horizontal de ciencia ficción para **Linux x86_64**, creado por **Tony C. Ware** y desarrollado en **C++17 y SDL2**, sobre **GRIT NOVA ENGINE**. Música original: **FutureDark**, alias musical de Tony C. Ware. Programación y desarrollo: **GPT-6 · OpenAI**.

**AURA-01**, computadora de a bordo de la Firebird-01, es la voz de los sistemas de la nave. Mantiene íntegramente los diálogos, avisos y apariciones establecidos, ahora con un ojo rojo animado de 12 fotogramas. [Ficha de AURA-01](documentacion/AURA-01_COMPUTADORA_FIREBIRD.md).

**NOVEDADES V1.2:** banda sonora FutureDark en OGG Vorbis estéreo real a 44,1 kHz; efectos espaciales, reproductor de cassette animado, medidores L/R y scanlines configurables. Nueva carátula y nueva ilustración final con Firebird-01.

[Detalles de V1.2: audio, cassette, scanlines y arte oficial](documentacion/NOVEDADES_V1.2.md).

![Captura real de CONTACTO 2010](capturas/CAPTURA_FASE_1.png)

**Cinco fases, combates arcade, enemigos animados, banda sonora original y la nave Firebird-01.** Esta edición V1.2.1 incluye su nuevo diseño industrial y el Cementerio de Escombros V2.

## Descargar y jugar

[**Release V1.2.1 — todas las descargas**](https://github.com/OnRail76/Contacto-2010/releases/tag/v1.2.1)

- [AppImage x86_64 para Linux](https://github.com/OnRail76/Contacto-2010/releases/download/v1.2.1/Contacto_2010_V1.2.1-x86_64.AppImage): un único archivo ejecutable; dar permiso con `chmod +x` y abrir.
- [Windows x64, EXE nativo y DLL en ZIP](https://github.com/OnRail76/Contacto-2010/releases/download/v1.2.1/CONTACTO_2010_V1.2.1_WINDOWS_x64.zip): descomprimir entero y abrir `CONTACTO_2010.exe`.

Las dos distribuciones V1.2.1 se construyen y se prueban de forma nativa en GitHub Actions. El EXE no depende de Wine ni de Python.

Movimiento: **WASD o cursores**. Disparo: **ESPACIO**. Pausa: **ESC**. Pantalla completa: **F11**. Compatible con mando SDL2 con reconexión.

Para compilar desde las fuentes: `bash COMPILAR_LINUX.sh` en Linux, o MSYS2 UCRT64 y `bash packaging/windows/COMPILAR_WINDOWS.sh` en Windows.

## Ilustraciones oficiales

La portada Firebird-01 se muestra al principio de este README. La escena final oficial está integrada en `assets/final_escena_firebird01_hd.png` y se presenta dentro del juego. Los PNG originales 4K se encuentran en la [Release V1.2](https://github.com/OnRail76/Contacto-2010/releases/tag/v1.2).

## Documentación

[**Atlas, memoria técnica y manual oficial V1.2.1 (PDF de 52 páginas)**](documentacion/CONTACTO_2010_V1.2.1_ATLAS_MEMORIA_MANUAL_GRIT_NOVA.pdf)


[Atlas, memoria y manual de la edición V1.1 (PDF de 44 páginas)](documentacion/ATLAS_MEMORIA_MANUAL_CONTACTO_2010_V1.1.pdf) se conserva como documentación histórica del diseño anterior. Para las novedades de V1.2, consulta el documento actualizado enlazado arriba.

## Código y recursos

- `motor/`: servicios reutilizables de GRIT NOVA ENGINE (compilación separada).
- `fuentes/`: código del juego (fases, físicas, eventos, HUD y combates).
- `assets/`: recursos activos: sprites RGBA, geometría BIN, efectos WAV y escena final oficial.
- `banda_sonora_original/`: diez pistas OGG originales.
- `packaging/`: iconos oficiales y utilidades para Linux/AppImage y Windows.

Prueba automática de arranque: `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./CONTACTO_2010_LINUX --self-test`.

**Licencias:** código MIT (`LICENSE`); ilustraciones y audio conservan sus derechos originales (`LICENCIA_RECURSOS.md`).
