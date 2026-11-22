# CONTACTO 2010 — HISTORIAL CONSOLIDADO

> **Único documento de histórico del proyecto.** Versión oficial vigente: **V1.2.1**.  
> Autor y dirección artística: **Tony C. Ware** · Música original: **FutureDark** · Motor: **GRIT NOVA ENGINE** (C++17/SDL2).  
> Actualizado: **22 de noviembre de 2026**.

Este documento reúne los cambios publicados, las versiones sustituidas y las principales decisiones de mantenimiento para mantener el repositorio orientado a una única edición jugable. **No revela la identidad del último jefe ni el desenlace.** El código sigue disponible en Git y sus etiquetas históricas; las Releases antiguas y sus instaladores ya no son necesarios como descargas públicas.

## Estado vigente — V1.2.1

La [Release oficial V1.2.1](https://github.com/OnRail76/Contacto-2010/releases/tag/v1.2.1) contiene los ejecutables **Linux x86_64 (AppImage)** y **Windows x64 (EXE nativo + DLL)**, el **atlas y manual sin spoilers de 44 páginas**, el atlas animado de **AURA-01**, y las sumas de verificación. Se conservan además en la publicación vigente las ilustraciones oficiales maestras 4K procedentes de V1.2, sin modificar los originales.

Juego y motor actuales: cinco fases, Firebird-01, GRIT NOVA ENGINE, banda sonora de diez pistas FutureDark en OGG Vorbis estéreo, efectos WAV con panorámica, reproductor de cassette animado, scanlines CRT y comunicaciones de AURA-01. El PDF actualizado coloca la lámina industrial en la página 2 y utiliza recursos auténticos del juego.

## Cronología de versiones

| Fecha de publicación | Versión | Cambios principales | Estado |
| --- | --- | --- | --- |
| 21-11-2026, 18:56 UTC | **V1.1** | Primera edición oficial Linux/Windows con campaña de cinco fases, GRIT NOVA ENGINE, Firebird-01 industrial, Cementerio V2, HUD, escudo, mandos, radio y manual original. Código bajo MIT; derechos de arte y audio separados. | Sustituida |
| 21-11-2026, 21:36 UTC | **V1.2** | Música FutureDark OGG Vorbis **estéreo real** a 44,1 kHz, SFX espaciales, cassette animado con medidores L/R y scanlines. Nueva portada Firebird-01 y nueva ilustración del final. | Sustituida |
| 21-11-2026, 22:09 UTC | **V1.2.1** | **AURA-01** sustituye al personaje de comunicaciones anterior, conservando diálogos y tiempos. Nuevo ojo rojo RGBA de 12 fotogramas centrado en radio, briefings e introducciones. Actualización de atlas y documentación y recompilación Linux/Windows. | **Vigente** |
| 22-11-2026 | **Mantenimiento de V1.2.1** | Limpieza de WAV musicales mono duplicados, atlas viejo, binarios de SDL2 no compatibles, documentos y scripts anticuados; actualización de empaquetadores y workflows actuales. No modifica las reglas ni la banda sonora. | Integrado en `main` |

## Evolución técnica

### GRIT NOVA ENGINE

- Núcleo reutilizable C++17 y SDL2, separado de la lógica particular del videojuego.
- Dibujo PNG RGBA, manejo de teclado y mandos y geometría/colisiones del juego.
- Audio actual en **44.100 Hz / 16 bits / 2 canales**, música OGG Vorbis estéreo y efectos WAV mono situados espacialmente en la mezcla.
- Controles de volumen, estéreo espacial y cuatro modos de scanlines, con configuración persistente.
- Verificación mediante pruebas sin pantalla y compilación de AppImage Linux y paquete Windows en GitHub Actions.

### Imagen y presentación

- **Portada oficial:** Firebird-01 revisada; portadas previas documentadas como variantes en el atlas vigente.
- **Lámina industrial:** hoja completa en página 2 del atlas.
- **AURA-01:** computadora de a bordo, atlas de **6 × 2 fotogramas RGBA** (2172 × 724 píxeles), ojo centrado y animado mientras habla. Las frases originales y su sincronización se respetan.
- **Atlas vigente:** 44 páginas con capturas reales, Bazzite/Linux y Windows, **sin imágenes del último jefe ni del final**.
- **Ilustración final:** recurso oficial integrado en el juego; no se publica en las páginas del atlas para evitar destripes.

## Detalles de la modernización audiovisual V1.2

La banda sonora de FutureDark utiliza diez archivos OGG Vorbis con dos canales reales. El motor mezcla sonido PCM de salida a **44.100 Hz, 16 bits, estéreo**, decodifica únicamente la pista activa y adapta la frecuencia cuando corresponde antes de reproducirla; no decodifica OGG en el callback de audio. Los efectos mantienen sus archivos WAV originales: pasan desde 22.050 Hz mono al mezclador estéreo y se posicionan espacialmente, con volúmenes independientes.

El reproductor de cassette integra bobinas animadas, simulación de recogida de cinta, barra de progreso, contador `MM:SS` y medidores L/R tomados de las muestras reproducidas. Los carretes se detienen con la pausa. Controles documentados del reproductor: arriba/abajo para seleccionar, izquierda/derecha para cambiar de pista, INTRO para reproducir, **P** para pausar/reanudar, **X** para detener y **ESC** para volver.

La opción de CRT añade cuatro niveles de scanlines (desactivadas, suaves, arcade, intensas) sin sustituir la imagen original del juego. El estado del audio espacial, los volúmenes y las scanlines se conserva en `audio_config.ini`. El formato comprimido evita distribuir antiguos WAV musicales sin perder la mezcla estéreo original.

## Decisiones de mantenimiento y conservación

1. **Una única Release descargable:** V1.2.1. Las publicaciones V1.1 y V1.2 se retiraron de la sección Releases para evitar instalar versiones anteriores por error.
2. **No eliminar el historial real del código:** las etiquetas de Git y los commits antiguos se conservan para recuperar una fuente histórica cuando haga falta. Eso no genera descargas antiguas en la sección Releases.
3. **Música:** los antiguos WAV musicales, redundantes desde V1.2, se retiraron del árbol activo. Los **OGG estéreo** y **WAV de efectos** permanecen.
4. **Documentación:** un atlas oficial de 44 páginas; este documento reúne la historia de las versiones. No duplicar manuales o notas desactualizadas en la raíz.
5. **Builds:** mantener las pruebas automatizadas de Linux/Windows y no reescribir las etiquetas o los assets actuales como parte de la limpieza.
6. **Licencias:** MIT para el código; música, logotipos e ilustraciones sujetos a `LICENCIA_RECURSOS.md`.

## Registro de publicaciones retiradas (inventario para consulta)

Las entradas siguientes registran el contenido histórico y no son enlaces de descarga activos.

### V1.1 — archivos publicados originalmente

`CONTACTO_2010_CARATULA_OFICIAL.png`, `Contacto_2010_V1.1-x86_64.AppImage`, `Contacto_2010_V1.1_ATLAS_MEMORIA_MANUAL.pdf`, `CONTACTO_2010_V1.1_FUENTES_JUEGO.zip`, `CONTACTO_2010_V1.1_ICONOS_APPIMAGE_WINDOWS_MAC.zip`, `CONTACTO_2010_V1.1_KIT_CONSTRUCCION_APPIMAGE.zip`, `CONTACTO_2010_V1.1_LINUX_COMPLETO.zip`, `CONTACTO_2010_V1.1_WINDOWS_x64.zip`, `GRIT_NOVA_ENGINE_V1.1_FUENTES.zip`, `SHA256_APPIMAGE_V1.1.txt`, `SHA256_RELEASE.txt`, `SHA256_WINDOWS_V1.1.txt`.

### V1.2 — archivos publicados originalmente

`CONTACTO_2010_CARATULA_OFICIAL_4K.png`, `CONTACTO_2010_ESCENA_FINAL_OFICIAL_4K.png`, `Contacto_2010_V1.2-x86_64.AppImage`, `CONTACTO_2010_V1.2_WINDOWS_x64.zip`, `SHA256_CONTACTO_2010_V1.2.txt`.

### V1.2.1 — archivos de la edición vigente

`AURA_01_SHEET_RGBA_12F.png`, `Contacto_2010_V1.2.1-x86_64.AppImage`, `CONTACTO_2010_V1.2.1_ATLAS_MEMORIA_MANUAL_GRIT_NOVA.pdf`, `CONTACTO_2010_V1.2.1_WINDOWS_x64.zip`, `SHA256_ATLAS_V1.2.1.txt`, `SHA256_CONTACTO_2010_V1.2.1.txt`, y las dos imágenes oficiales 4K transferidas desde V1.2.

## Cambios de código registrados en Pull Requests

- **#1** — Audio OGG estéreo, efectos espaciales, cassette y scanlines (fusionada en V1.2).
- **#2** — Detalles mecánicos del cassette.
- **#3** — Eliminación de un icono duplicado.
- **#4** — AURA-01 y sus diálogos/animación centrada (V1.2.1).
- **#5** — Limpieza de WAV, documentación y workflows antiguos, empaquetado actual y conservación del código.

Las Pull Requests fusionadas y los commits siguen disponibles en GitHub como registro técnico detallado. Este archivo es la **referencia editorial única** de la evolución de CONTACTO 2010.
