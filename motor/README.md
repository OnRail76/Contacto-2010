# GRIT NOVA ENGINE — núcleo activo en CONTACTO 2010 V1.2.1

Núcleo nativo C++17 / SDL2 / libpng utilizado por CONTACTO 2010. Código bajo licencia MIT.

El motor tiene ahora **fuentes físicas independientes** de las del juego:

- `grit_nova_engine.hpp`: API pública de bajo nivel, ABI SDL2 declarada, estructuras y constantes.
- `grit_nova_engine.cpp`: PNG RGBA → SDL_Texture, teclado WASD/cursores, mandos con hotplug, dibujo 2D, UTF-8 y mezclador PCM estéreo de 32 voces con panorámica espacial.
- `COMPILAR_MOTOR.sh`: produce una biblioteca estática `libgritnova.a`.
- `PROBAR_MOTOR.cpp` / `.sh`: prueba sin depender de las reglas ni de los recursos de CONTACTO 2010.

**Importante:** esta implementación separa un **núcleo reutilizable real**, pero no convierte todavía al motor en una plataforma independiente para juegos sin programación: CONTACTO 2010 todavía implementa por su cuenta la IA, el guion, el HUD específico, el scroll por terrenos, el combate y la lógica de jefes en `fuentes/contacto_native.cpp`. Documentado íntegramente en el PDF.

Se compila desde la raíz del paquete completo: `./motor/COMPILAR_MOTOR.sh` o `./COMPILAR_LINUX.sh`.

Dependencias de compilación: `g++`, `ar`, cabeceras de `libpng` (`png.h`), biblioteca SDL2 compatible. El proyecto conserva una biblioteca SDL2 de compatibilidad, pero la compilación en CI enlaza contra la SDL2 del sistema; el ZIP independiente del motor no contiene binarios ni bibliotecas de terceros.
