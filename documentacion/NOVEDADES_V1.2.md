# CONTACTO 2010 — V1.2 oficial: FutureDark OGG, cassette y arte Firebird-01

> Versión vigente: [Release V1.2.1](https://github.com/OnRail76/Contacto-2010/releases/tag/v1.2.1), con AURA-01 y atlas oficial sin spoilers.

## Música en estéreo auténtico

- Música de las cinco fases, jefes, título, Game Over, victoria y epílogo: **OGG Vorbis estéreo de FutureDark** en `banda_sonora_original/`.
- Mezcla SDL2 PCM de **44.100 Hz / 16 bits / 2 canales**.
- Decodificación con libvorbisfile; **solo se conserva en RAM la pista activa**. Si una fuente OGG viene a otra frecuencia admitida, se remuestrea al cambiar de canción, nunca dentro del callback.
- Los efectos WAV mono de 22.050 Hz se convierten al arrancar a 44.100 Hz y mantienen su panorámica espacial.
- Las diez pistas OGG estéreo son la única fuente musical activa. Se retiraron del árbol de desarrollo los WAV de música duplicados, innecesarios para el juego actual. Las Releases anteriores permanecen como respaldo.
- Las grabaciones OGG deben contener dos canales; el motor informa de cualquier archivo incorrecto. La salida espacial de los efectos sigue siendo opcional desde OPCIONES.

## Reproductor de cassette animado

En el título: **REPRODUCTOR MUSICAL**.

- Deck de estética militar retrofuturista, cassette insertada, carretes de seis brazos giratorios y simulación de cinta recogida conforme avanza el tema.
- Pantalla de pista, LEDs, barra de progreso, contador `MM:SS / MM:SS` y medidores de nivel **L/R** obtenidos de las muestras estéreo actuales.
- Las bobinas giran mientras suena la canción, y quedan inmóviles al pausar o detener la reproducción.
- **Arriba/abajo:** elegir pista. **Izquierda/derecha:** anterior/siguiente y reproducir. **ENTER/A:** reproducir. **P:** pausa/reanudar. **X:** detener. **ESC/B:** regresar al título.
- Mantiene las diez composiciones originales y sus títulos. El reproductor no altera los bucles automáticos de música del juego.

## Opciones CRT

Estéreo espacial SÍ/NO, sonido y volúmenes, y cuatro intensidades de scanlines (OFF, suave, arcade, intenso). Persistencia compatible con el archivo previo `audio_config.ini`.

## Compatibilidad y verificaciones

- Linux x86_64: compilación nativa y AppImage con dependencias libvorbisfile, libvorbis y libogg incluidas.
- Windows x64: EXE compilado con MSYS2 UCRT64 y DLL nativas en el ZIP.
- La Release oficial V1.2.1 incorpora el AppImage Linux y el EXE Windows probados, con música OGG, AURA-01, ilustraciones y checksums. Los workflows de prueba permanecen disponibles para validar nuevas modificaciones.
- Pruebas: fases y jefes, banda sonora, estéreo panorámico, scanlines, jingles, reproducción/pausa/stop y arranque en ambas plataformas.

Las pruebas automáticas de Linux y Windows han finalizado correctamente antes de la publicación. Se recomienda comprobar sonido, rendimiento e imagen en un equipo físico.

## Arte oficial incorporado a V1.2.1

- Carátula Firebird-01: `documentacion/CONTACTO_2010_CARATULA_OFICIAL.png`, visible como primera imagen del README y en la cabecera de la Release.
- Ilustración final Firebird-01: `assets/final_escena_firebird01_hd.png`, integrada en la escena final del juego.
- Los originales 4K de ambas ilustraciones están publicados como archivos adicionales de la [Release V1.2](https://github.com/OnRail76/Contacto-2010/releases/tag/v1.2).
