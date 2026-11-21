# CONTACTO 2010 — V1.2 experimental: FutureDark OGG y cassette

> Implementación en `desarrollo/v1.2-estereo-crt`. Esta rama **no altera la Release V1.1**.

## Música en estéreo auténtico

- Música de las cinco fases, jefes, título, Game Over, victoria y epílogo: **OGG Vorbis estéreo de FutureDark** en `banda_sonora_original/`.
- Mezcla SDL2 PCM de **44.100 Hz / 16 bits / 2 canales**.
- Decodificación con libvorbisfile; **solo se conserva en RAM la pista activa**. Si una fuente OGG viene a otra frecuencia admitida, se remuestrea al cambiar de canción, nunca dentro del callback.
- Los efectos WAV mono de 22.050 Hz se convierten al arrancar a 44.100 Hz y mantienen su panorámica espacial.
- Los ejecutables de prueba incluyen archivos OGG comprimidos y excluyen los antiguos WAV de música del paquete para reducir tamaño. Los WAV originales del repositorio quedan intactos por seguridad.
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
- Los paquetes experimentales se generan como **artifacts** del workflow `v12-preview.yml`; no modifican la Release V1.1 ni se convierten en publicación definitiva.
- Pruebas: fases y jefes, banda sonora, estéreo panorámico, scanlines, jingles, reproducción/pausa/stop y arranque en ambas plataformas.

Se recomienda una comprobación final con auriculares y partida real antes de etiquetar V1.2.

## Arte oficial incorporado a V1.2

Portada oficial Firebird-01 en 