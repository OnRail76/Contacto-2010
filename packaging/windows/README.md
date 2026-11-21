# Icono de Windows

El ICO de Windows contiene las resoluciones 16, 24, 32, 48, 64, 128 y 256 px. Para enlazarlo a un futuro ejecutable nativo de Windows, compilando con MinGW-w64: `windres contacto2010.rc -O coff -o contacto2010_res.o`, y vincular el archivo objeto al proyecto junto a SDL2 y libpng para Windows. **Este repositorio NO contiene ni promete un EXE de Windows**; la build de Linux no funciona como EXE sin una adaptación y verificación.
