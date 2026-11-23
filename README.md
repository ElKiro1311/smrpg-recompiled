# SMRPG Recomp

Port no oficial de **Super Mario RPG** (Nintendo Switch, 2023, versión 1.0.0) a **PC con Windows** y a **Android** por
recompilación estática: el programa del juego se traduce a C y se compila para el procesador de cada equipo, y corre
sobre el runtime de [LASRecomp](https://github.com/peiikai/LASRecomp) (servicios del sistema, kernel y NVN → Vulkan).

*English: an unofficial static recompilation of Super Mario RPG (Switch, 2023) for Windows and Android, built on
LASRecomp. It contains no game files: you need your own copy of the game, already extracted. Docs are in Spanish.*

> **Aviso legal.** Proyecto de aficionados, sin ánimo de lucro, **sin relación con Nintendo ni con Square Enix** y sin
> su respaldo. *Super Mario RPG* y *Mario* son marcas de Nintendo; el juego es © Nintendo / SQUARE ENIX. Ni este
> repositorio ni sus releases contienen archivos, código, gráficos, música o textos del juego. Para jugar hacen falta
> los archivos de **tu propia copia** del juego, ya extraídos (las carpetas `exefs` y `romfs` de la versión 1.0.0); no
> se dan instrucciones para obtenerlos. No compartas tu instalación ni las texturas que vuelque el juego: son del juego.

## Estado

Se juega de principio a fin en lo probado: vídeos, título, menús, partida, combates, guardar y cargar, y zonas de todo
el mapa. Pantalla ancha (21:9 y más), resolución interna hasta 8K, HDR, hasta 240 FPS, AMD FSR 4 / 3.1 / 2.3 con
generación de fotogramas, texturas HD y menús en español.

| Port | Versión | Repositorio y releases |
| --- | --- | --- |
| PC: Windows 10 u 11 de 64 bits, Vulkan 1.3 | 0.51.0-beta | este; cambios: [`VERSIONES.txt`](VERSIONES.txt) |
| Android 13 o más nuevo (arm64), Vulkan 1.3. Alfa | 0.9.1-alpha | [smrpg-recompiled-android](https://github.com/ElKiro1311/smrpg-recompiled-android) |

## Jugar en Windows

Descarga `SMRPG-Recomp-Instalador-<versión>.exe` de la
[última release](https://github.com/ElKiro1311/smrpg-recompiled/releases/latest). En su ventana eliges la carpeta de tu
copia del juego ya extraída (la que trae `exefs` y `romfs`) y la carpeta de instalación. El instalador comprueba que
es la versión 1.0.0 completa, la pone en la instalación (con enlaces duros si está en la misma unidad, sin ocupar el
doble) y construye allí el código del juego: de 10 a 20 minutos. Guía, controles y ajustes recomendados:
[`LEEME.md`](LEEME.md).

La generación de fotogramas de Lossless Scaling (Menú > Vídeo) usa el `Lossless.dll` de **tu** copia de Lossless
Scaling (Steam, de pago): ni el repositorio ni el instalador lo llevan ni lo descargan.

## Compilar

En Windows, con MSYS2 UCRT64: [`LEEME.md`](LEEME.md) y [`docs/BUILD-windows.md`](docs/BUILD-windows.md). El juego
extraído va en `extracted/` (o un enlace a esa carpeta); `make gen` traduce su código a C y `make -j8` crea
`build/smrpg-recomp.exe` y `build/lasgame.dll`. Las rutas de las herramientas de fuera del repositorio (SDK de
Android, JDK…) van en un `local.env` propio, que git no sube: ver [`local.env.ejemplo`](local.env.ejemplo). El estado
técnico y cómo se prueba: [`ESTADO.md`](ESTADO.md).

Solo Windows (Android, en su repositorio). Lo de macOS y Linux, `make dist`/`app`/`dmg`/`appimage`, el
aleatorizador de Link's Awakening (`tools/rando`, `third_party/LAS-Randomizer`) y `tools/shot.sh`, `frames.sh` y
`run.sh` son de LASRecomp y aquí no se usan. El port parte de LASRecomp 579fdad: lo que añade se ve comparando con ese
commit de LASRecomp.

## Android

El port de Android tiene su propio repositorio, con sus releases y su APK:
[smrpg-recompiled-android](https://github.com/ElKiro1311/smrpg-recompiled-android).

## Fallos y sugerencias

En las [issues de este repositorio](https://github.com/ElKiro1311/smrpg-recompiled/issues), con `lasrecomp.log` de la
carpeta del juego. No en las de LASRecomp: este port no es cosa suya.

## Créditos y licencias

- [LASRecomp](https://github.com/peiikai/LASRecomp), de Peiikai y colaboradores (MIT): el recompilador, el runtime y el
  renderizador. Su README original está en [`docs/README-LASRecomp.md`](docs/README-LASRecomp.md).
- [nxnative](https://github.com/seedds/nxnative): referencia de cómo se comporta este juego.
- AMD FidelityFX SDK (FSR y generación de fotogramas; sus DLL van aparte: [`third_party/amd-fsr`](third_party/amd-fsr)),
  Vulkan Memory Allocator, Dear ImGui, SMAA, [lsfg-vk-android](https://github.com/GameNative/lsfg-vk-android) (la
  generación de Lossless Scaling), Snapdragon GSR (Android) y lo que repite el instalador (SDL3, FFmpeg LGPL, Opus,
  shaderc, LLVM/clang/lld, Python, Capstone, Unicorn…): cada uno con su licencia, en `third_party/` y en la carpeta
  `licencias\` de la instalación.
- El código de LASRecomp y el de este port van con la licencia MIT de [`LICENSE`](LICENSE). Esa licencia no cubre
  Super Mario RPG ni nada de Nintendo, ni lo de `third_party/`, que conserva su propia licencia.
