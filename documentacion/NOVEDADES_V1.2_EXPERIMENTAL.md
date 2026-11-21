# CONTACTO 2010 — desarrollo experimental V1.2

> Rama de trabajo: no reemplaza a la Release oficial V1.1. Sin cambios en fases, enemigos, jefes, física ni materiales artísticos.

## Sonido estéreo

- Salida nativa SDL2 en **dos canales PCM S16**, 22.050 Hz. Se conserva la frecuencia de los archivos WAV actuales para evitar remuestrear durante las partidas.
- Efectos con panorámica de posición: disparo de la Firebird-01, explosiones enemigas y disparos enemigos. En el centro suenan por los dos canales; hacia los lados se atenúa gradualmente el canal lejano.
- Mezclador de **32 voces** y limitador por canal, sin asignaciones dinámicas dentro del callback.
- Interruptor **ESTÉREO ESPACIAL**: al apagarlo, ambos canales reciben exactamente el mismo resultado. No modifica el mezclado original de las canciones.
- Las pistas WAV actuales se almacenan en mono. Se duplican a L/R para reproducirlas en equipos estéreo; **esto no recupera un máster estéreo original**. Una remasterización musical estéreo real requiere nuevos archivos originales de audio.

## Scanlines de bajo coste

- Filtro opcional aplicado al fotograma ya compuesto: DESACTIVADO / SUAVE / ARCADE / CRT INTENSO.
- Una sola textura de 1 × 540 creada al iniciar y una operación de dibujado SDL2 al finalizar cada fotograma. La textura se escala por GPU a 960 × 540 sin bucles de píxeles en la CPU durante el juego.
- Aplica el efecto a título, menús, HUD, cinemáticas y gameplay por igual; no modifica los sprites ni las colisiones.
- No se incluyen curvatura, aberración cromática ni desenfoque: evitarían mantener la legibilidad de los indicadores y podrían perjudicar los FPS.

## Opciones y persistencia

Desde el título: **OPCIONES**. Cursores arriba/abajo eligen ajuste, izquierda/derecha ajustan valores, ENTER activa, ESC vuelve. El mando usa los controles de menú ya existentes.

Los parámetros se conservan en `audio_config.ini` (compatibilidad con los ajustes anteriores):

```ini
master 100
music 100
effects 100
enabled 1
stereo 1
scanlines 1
```

El modo por defecto usa estéreo espacial y scanlines suaves. El archivo se carga sin exigir las nuevas claves para mantener las configuraciones existentes.

## Verificaciones

- `--self-test-estereo-crt`: diferencia de panorámica a izquierda/derecha, salida idéntica en mono, y textura disponible.
- `--self-test-menu` y pruebas anteriores de CONTACTO 2010.
- CI de preview Linux AppImage y Windows EXE en GitHub Actions **sin subida a la Release V1.1**. Los ficheros de prueba aparecen como artefactos de la ejecución de la PR.
- Pendiente de validar en una partida real: monitor, auriculares, mando, mezcla con oleadas densas, claridad de interfaz y FPS antes de etiquetar V1.2.
