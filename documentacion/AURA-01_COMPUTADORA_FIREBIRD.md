# AURA-01 — Computadora de a bordo de Firebird-01

**Nombre definitivo:** AURA-01  
**Designación:** Asistente Unificado de Reconocimiento y Alerta, unidad 01  
**Nave:** Firebird-01 · Fuerza Espacial Europea  
**Función:** computadora de navegación, sensores, alerta, diagnóstico y comunicaciones de misión

AURA-01 es la interfaz de inteligencia de a bordo, representada por un ojo óptico rojo que se ilumina y palpita al emitir sus mensajes. El nombre aparece en las transmisiones del HUD y en los diálogos anteriores a los jefes.

## Diseño gráfico

- Recurso original suministrado por Tony C. Ware: `COMPUTADORA_FIREBIRD_01_SHEET_RGBA.png`.
- Recurso activo: `assets/AURA_01_SHEET_RGBA.png`.
- **12 fotogramas RGBA**, composición de **6 columnas × 2 filas**, cada celda de 362 × 362 px (2172 × 724 px).
- La animación recorre la variación de intensidad del ojo rojo cuando AURA-01 transmite.
- El renderizado conserva las proporciones del círculo, sin estirar la imagen; en reposo utiliza el primer fotograma.

## Continuidad argumental

**No se altera ninguna frase.** El nombre mostrado en las comunicaciones y en los cinco pre-jefes es **AURA-01**, manteniendo el texto de cada mensaje, los cinco guiones pre-jefe, los tiempos, las fases, el orden de los eventos y los demás interlocutores. Las menciones a instalaciones y módulos científicos se conservan íntegramente porque forman parte del guion original.

El atlas de retratos de otros personajes sigue cargándose para conservar al piloto, operadora, comandante y antagonista. El rol narrativo 3 utiliza exclusivamente el nuevo atlas de AURA-01.

## Pruebas

El comando `./CONTACTO_2010_LINUX --self-test-aura` verifica el atlas 2172 × 724, los prefijos de los cinco diálogos pre-jefe y las comunicaciones en las cinco fases. Las pruebas existentes de combate y audio se mantienen.

Esta sustitución artística no modifica música, efectos estéreo, cassette player, scanlines ni mecánicas.
