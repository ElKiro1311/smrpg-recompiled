# Estado del port (se actualiza en cada versión)

Lo que cambia en cada versión está en [`VERSIONES.txt`](VERSIONES.txt). Este archivo dice dónde está el arranque ahora,
cómo se prueba y qué se sabe del juego.

## Dónde está el arranque

- Pasa nninitStartup (heap con memoria virtual), los 112 constructores del sdk y los 288 del juego, y entra en nnMain.
- Monta el RomFS, inicializa NVN y Vulkan (dispositivo, ventana de 3 texturas, vsync), crea unos 30 hilos de Unity y
  sube las primeras texturas.
- Con la versión 11 (orden de memoria de `LDAR`/`STLR`) el hilo principal sigue adelante, crea `Loading.PreloadManager`
  y empieza a cargar la primera escena, «level0».
- Versiones 13-16: pasa «level0» (despertar perdido en las condvar), crea los shaders (propiedades NVN de la consola),
  presenta fotogramas (sin los ganchos de Link's Awakening) y deja de caerse (el sdk ya no se carga dentro de main).
- 0.18.0-alfa: corre a 60 fps y dibuja la escena 3D (traductor de shaders: EXIT con condición, TEX.B bindless…).
- **Estado actual (versión 19):** se ven los logos, el vídeo de apertura (VP9 de CRIWARE por `movie::Decoder` con
  FFmpeg), la pantalla de título, el menú de dificultad, la cinemática de partida nueva y la escena del castillo de
  Bowser. El audio de los vídeos se decodifica (Opus por el servicio `hwopus` con libopus).
- Ruta de prueba sin mando: `LARECOMP_INPUT="1300:A:8,1600:A:8,1900:A:8,2200:A:8"` (A salta el vídeo, A en el
  título, en el menú y en la dificultad) y `LARECOMP_SHOT_AT="1450,1750,2600,4800"` → `build/frames/frame_N.png`
  (la imagen del juego, no una captura del escritorio). `LARECOMP_AUDOUT_WAV=build/audio.wav` vuelca el audio.
- 0.20.0-alfa: los personajes se ven (Mario, Bowser y Peach en la cinemática y Mario en el puente del castillo,
  fotograma ~5100 de la ruta); todos los shaders se traducen (0 dibujos saltados).
- **Versiones 21-23: se juega.** Tras la cinemática, Mario entra en el castillo con el HUD; el juego se autoguarda
  (`save/save_0/SMR_Save_Ver0011000`), la pantalla «Load» carga la partida (con la hora local desde la 22), Mario se
  mueve, la cámara le sigue y la primera batalla (dos Terrapins) funciona: ataque, daño, efectos y vuelta al mapa.
- Para jugar: `jugar.cmd` (con sonido, mando y teclado; los controles van en el propio archivo). Para probar sin
  cerrar la partida de nadie: `tools/probar.sh <segundos> <registro> VARIABLE=valor…`; con
  `LARECOMP_DATA_ROOT=<carpeta>` (con `extracted` y `build` enlazados) la prueba usa sus propios guardados y ajustes, y
  `LARECOMP_HIDDEN=1` no muestra la ventana.
- **0.24.0-beta:** la aplicación es Super Mario RPG Recomp (`build/smrpg-recomp.exe`, icono propio). Menú (Esc / F1) en
  español con: pantalla ancha de verdad (Unity dibuja al aspecto del monitor: `runtime/hle/unity_screen.c` +
  `tools/game_overrides.c`), resolución interna con nombre (720p-4320p), supersampling x2-x16, nitidez (RCAS),
  anisótropo, límite de FPS (hasta 240; a 120 el movimiento no se acelera) y HDR10.
- Sustituir una función del juego sin regenerar: definirla en `tools/game_overrides.c` (se enlaza la primera con
  `--allow-multiple-definition`, así gana también para las llamadas internas de su archivo generado).
- **Versiones 25-31:** idioma del juego (25), detalle de texturas (26), miniaturas de los guardados (27), menú en
  español (28), interfaz entera en pantalla ancha (29: `CanvasScaler.HandleScaleWithScreenSize` sustituido), barrido
  de shaders (`tools/shader_sweep.c`: 82.006 de los 82.032 del juego se traducen y compilan, 30) y texturas HD con
  volcado (31: `<datos>/texturas` y `<datos>/texturas-volcadas`, `runtime/nvn/vulkan/texpack.c`).
- **Versiones 32-35: AMD FSR.** Antialiasing «AMD FSR» (nativo) y escalado (Calidad x1,5, Equilibrado x1,7,
  Rendimiento x2, Ultra x3), con FSR 4.1.2, 3.1.6 o 2.3.4 a elegir. Los vectores de movimiento son los de URP 12
  (su MotionVectorRenderPass, forzada en `UniversalRenderer.GetRenderPassInputs`, `tools/game_overrides.c`); el
  desplazamiento subpíxel va en el viewport y FSR corre antes del posproceso (`runtime/nvn/vulkan/exec.c`
  fsr_pass_begin, `vk_fsr.cpp`). Hacen falta junto a `build/smrpg-recomp.exe` las DLL del SDK de AMD FSR 2.4
  (`amd_fidelityfx_loader_vk.dll`, `amd_fidelityfx_upscaler_vk.dll`; no están en el repositorio:
  `third_party/amd-fsr/LEEME.txt`).
- **0.38.0-beta: generación de fotogramas de AMD** (FG 4.0.2 por red neuronal en RDNA 4, FG 3.1.6; `vk_fg.cpp`): la
  swapchain de AMD con dos colas más; configuración y preparación tras FSR en el hilo grabador; la cámara sale de la
  vista-proyección de URP (`exec.c` fsr_camera_scan). La Y de los vectores va al revés que para FSR (medido: con la de
  FSR, FG 4 solo funde). Sin HDR, la ventana sin el menú del runtime (HUDLessColor) evita que el contador de FPS se
  deforme. Pruebas: `LARECOMP_FG_PSHOT=N` (lo presentado, a `build/fg_pres_*.png`), `LARECOMP_FG_SET`, `LARECOMP_FG_MV`,
  `LARECOMP_FG_DEBUG=2` (comprobador de AMD), `LARECOMP_FG_CAMDUMP=N`. Necesita `amd_fidelityfx_framegeneration_vk.dll`.
- **0.39.0-beta:** los vídeos (título, películas) llenan su textura escalada a más de 1x (antes, encogidos en la esquina).
  Pruebas: `LARECOMP_SET` cambia ajustes de vídeo en marcha; capturas de ventana con nombre por `LARECOMP_HOST_INPUT`.
- **0.40.0-beta:** modo de pantalla (ventana, sin bordes, pantalla completa exclusiva) y resolución de pantalla
  (`runtime/ui/menu.mm` menu_apply_window_mode_now).
- **0.41.0-beta:** el aspecto del juego (21:9, 16:10) sigue a la ventana también en marcha, con vista ancha real: aviso 31
  de AM al cambiar (`runtime/hle/unity_screen.c` rt_unity_screen_follow), SetResolution de Unity (main 0x700062f1d0)
  sustituida sin su recorte a 1920x1080 ni la marca de resolución propia (`tools/game_overrides.c`), texturas de ventana
  rehechas a la nueva medida (`t->screen`, `runtime/nvn/vulkan/textures.c`). Guía de controles en Menú > Controles y
  «Configurar controles» en español (`runtime/ui/controls_ui.mm`).
- **0.43.0-beta:** el fondo de los menús (la captura de pantalla de Menu.StartScreenCapture en una RawImage de
  1920x1080) ocupa la pantalla ancha: RawImage.set_texture sustituida, que pasa esas imágenes al aspecto de su
  textura (`tools/game_overrides.c`).
- **0.44.0-beta:** Menú > Vídeo: «Rendimiento ahora» (CPU, GPU, RAM, disco, memoria de vídeo medidos:
  `runtime/perf_meter.c`), el impacto de cada ajuste gráfico en los cinco con colores de semáforo y
  los botones Guardar, Deshacer cambios, Restablecer vídeo y Reiniciar el juego (`runtime/ui/menu.mm`).
- **0.45.0-beta:** en Vídeo, el recuadro de la derecha lleva el medidor arriba, el impacto del ajuste elegido
  encima de los botones y los botones abajo, siempre a la vista (`runtime/ui/option_rows.mm` opt_pane_*).
- Zonas comprobadas sin jugar (11-16, `tools/recorrer_zonas.py`: el guardado es JSON en UTF-16 y su
  `_respawn_floor_name` decide la escena al cargar): unas 36 zonas de todo el mapa se dibujan bien y sin dibujos
  saltados (Mushroom Way, casa de Mario, Reino Champiñón, camino de los bandidos, cloacas de Kero, Rose Town y Rose
  Way, bosque laberinto, Moleville, Booster Pass y torre, Marrymore, Star Hill, barco hundido, Seaside Town, Land's End,
  volcán Barrel por fuera y por dentro, mapa del mundo, habichuela y Nimbus Land, castillo de Bowser); en otras Mario
  aparece fuera del mapa (la posición centinela del guardado) y no son concluyentes.
- **0.46.0-beta:** «Negro puro» (la LUT de etalonaje de URP con su negro a 0: `vulkan/exec.c` lut_black_fix), «Vídeos
  en pantalla ancha» (las películas llenan el 21:9: `runtime/hle/movie_fit.c`) y combates siempre a 60 FPS
  (`runtime/hle/battle_rate.c`).
- **0.46.1-beta:** AMD FSR trabaja a la resolución interna: a 2160p en 21:9 (5160x2160) FSR 4 cuesta 3,1 ms por imagen
  en una RX 9070 XT (1,4 ms a 3440x1440; FSR 3.1 2,25 ms, FSR 2.3 1,7 ms; SMAA, nada medible): 240 -> ~175 FPS. El
  recuadro de impacto del menú lo explica; el perfilador de GPU (`LARECOMP_GPUPROF_AT`) da a FSR su propia fila.
- **0.47.0-beta:** instalador (`tools/instalador`, `make instalador` → `build/dist/SMRPG-Recomp-Instalador-<v>.exe`):
  la copia del juego + carpeta → copia el programa (sin nada del juego), pone la copia en extracted/, construye el
  código del juego en ese PC
  (`smrpg-recomp.exe --build-game`: lasgen + zig adelgazado, `runtime/gamebuild.c` con `game_overrides.c` primero y
  `-FORCE:MULTIPLE`) y crea los accesos directos. Sin --play también va directo al juego.
- **0.48.0-beta:** el instalador construye el código del juego DENTRO de su propio proceso, sin procesos hijos:
  `construir\construir.dll` (`tools/instalador/construir.cpp`) lleva el generador en un Python 3.12 embebido (aislado
  con `python312._pth`), clang 23 y lld de MSYS2 como bibliotecas (la misma línea `-cc1` y de enlace que `make`) y lo
  que incluyen y enlazan (`tools/instalador/herramientas.sh`); del PC solo hace falta Windows. C generado idéntico a
  `make gen` y objetos idénticos a los de `make` (salvo la hora COFF). Hilos con 64 MB de pila (con 2 MB,
  `MemorySSAUpdater::recursePhi` la desbordaba), red de fallos propia en los hilos de compilación (el
  `CrashRecoveryContext` de LLVM se apagaba con los fallos que unicorn provoca y recoge), lld con `--threads=1` (su
  grupo de hilos colgaba la salida del proceso). Cualquier CPU x86-64: v3 si hay AVX2, si no v2 o x86-64
  (`lasgame.dll.cpu`, comprobado por `runtime/gamelib.c` al cargar). Prueba suelta: `tools/instalador/probar_construir.c`.
  Con Control Flow Guard impuesto por Windows (#337 de LAS Recomp) unicorn tumba el proceso: el instalador se reabre a
  sí mismo con CFG apagado por la directiva de creación del proceso (probarlo: `tools/instalador/con_cfg.c` arranca un
  programa con CFG forzado). Probar la red: `SMRPG_CONSTRUIR_PRUEBA_FALLO=<trozo>:1` (acceso a 0) o `:2` (pila).
- Más de 60 FPS: en la partida el juego cuenta por tiempo (el movimiento va igual a 240 que a 60). Los combates no: su
  lógica (BattleManager.update_common) da un paso de 1/60 s por imagen y a 240 FPS iba cuatro veces más deprisa que
  sus animaciones y su sonido; desde la 0.46.0 el vsync late a 60 Hz mientras dura un combate. Las entradas
  programadas (`LARECOMP_INPUT`) van por fotogramas: a 240 FPS hay que multiplicar sus números por 4 (en combate, no).
- **0.48.1-beta:** el fundido a negro de las transiciones (GS.Fade: entrar en una zona desde el mapa, en una casa...)
  cubre toda la pantalla ancha: su Image era un recuadro 16:9 y a los lados se veía el fondo caqui de la cámara o la
  zona nueva (`runtime/hle/movie_fit.c` envuelve GS.Fade.Update, como MoviePlayer.Update). Ruta de prueba en 21:9:
  la partida en wld00 (el mapa) y A, con `LARECOMP_WINDOW=1720x720`.
- **0.49.0-beta:** sin siluetas negras al entrar en una zona ya vista. El código de cada sombreador que el juego usa se guarda (cache/shader_code/<huella>.bin,
  `runtime/nvn/shader_seen.c`) y la preparación en segundo plano del arranque construye con él todas las tuberías de la lista propia antes de que el juego
  dibuje (f01out, 2.º arranque: 0 dibujos saltados y 0 tuberías compiladas en la entrada; antes, 9.686 saltados en 2 s). De la rama android (0.8.1-alfa) sin lo de
  la Adreno: en el PC la profundidad es dinámica y las líneas de la lista no llevan « z=». LARECOMP_SHADER_SEEN=0 apaga lista y copias.
- **0.49.1-beta:** la primera fila y la última columna de la imagen ya no salen oscuras a ninguna resolución: la cadena
  de post del propio juego deja sin escribir la última fila/columna de sus búferes intermedios y su blit final las funde
  (medido también a 1920x1080 nativo, 1:1); el «present» replica el borde (`vulkan/shaders.c` present.frag — ventana y
  capturas). LARECOMP_NO_EDGE_FIX=1 lo apaga. Con `LARECOMP_SHOT_RAW=1|2` se vuelca el contenido exacto de una textura
  (el crudo, sin re-renderizar) y con `LARECOMP_DEBUG_PASS_STRIDE/_FROM/_TO` los pases de un fotograma de depuración.
- **0.49.2-beta:** el registro de cada mensaje IPC solo con `LARECOMP_LOG_IPC=1`: antes llenaba ~9,4 MB por minuto
  (53.000 líneas por minuto, el 99 % del registro); una corrida de prueba entera ahora deja 36 KB. De la rama android
  (0.7.2-alfa), sin lo demás de ese cambio.
- **0.49.3-beta:** la memoria local (LDL/STL) del tamaño que pide la SPH de cada sombreador, no 1 KB puesto a cero
  por píxel siempre (`maxwell/translate.c`, de la rama android 0.3.0-alfa, solo la parte de lmem): el sombreador con
  lmem de d01f1b pide 4 palabras y los pases pesados del fotograma quedaron a la mitad de ms. `LARECOMP_LOG_LMEM=1`
  (una línea por sombreador con su tamaño) y `LARECOMP_LMEM_FULL=1` (256 siempre, como antes).
- **0.49.4-beta:** el instalador normaliza las rutas de sus argumentos (/ -> \ y sin barra final, la raíz de la
  unidad se queda): con barras normales no creaba la carpeta y en --auto moría en 3 s (el prefijo \\?\ de sus
  rutas largas no las admite). Probado con --auto y barras normales: crea la carpeta y construye.
- **0.49.5-beta:** del traductor (de la rama android): SHFL con su operando c (las derivadas de textura por
  cuarteto: el nivel de detalle ya no se dispara), MUFU.SIN/COS con el ángulo reducido (rr2pi) y el «bra self»
  con -16 (el LutBuilderLdr fuera del bucle). Las 6 zonas: capturas dentro del ruido entre arranques; sin
  coste por imagen. («Clear targets» no existe en la rama android: el diff da solo estas tres piezas.)
- **0.49.6-beta:** la tabla del RomFS sin el _filelist.json del extractor: 1934 archivos y 6820987323 bytes
  totales; ROMFS_FILES 1934 en launcher.mm y crash_explain.c (los dos seguían en 3836, el número de LAS), el
  caminador y la auditoría del montaje se saltan el json, y game_romfs_bytes pasa a la suma real de los 1934
  (el «tamaño IVFC» anterior hacía saltar el aviso de archivos dañados SIEMPRE). Auditoría limpia al montar.
- **0.49.7-beta:** los fundidos de las transiciones sin GS.Fade (FadeWithEffect/DrawCaptureImage del título ->
  película y EncountEffect del combate del campo) también cubren todo el lienzo en pantalla ancha. Las
  transiciones aún dejan bandas por su CONTENIDO (la captura 16:9 del efecto y la geometría del barrido), no por
  los cuadros: medido a 1720x720 (80 izquierda / 160 centro en el barrido del título; 62/137/50 en el del
  combate); la capa profunda (ensanchar la captura), pendiente de diseñar. El pico blanco llena todo el ancho.
- **0.50.0-beta:** la generación de fotogramas de Lossless Scaling (LSFG 3.1) dentro del juego: Menú > Vídeo >
  «Generación Lossless Scaling» (x2/x3/x4, «Escala del flujo», «Modo rendimiento») con su panel de impacto
  medido. Biblioteca aparte (tools/lsfg/build.sh windows -> lsfg.dll, del lsfg-vk-android de GameNative
  437208a) que el juego carga al vuelo; los sombreadores salen del Lossless.dll de Steam del jugador
  (registro + bibliotecas + junto al exe; NUNCA en el repo ni en el instalador; sin él el juego va igual y el
  menú lo dice: «[lsfg] apagada», 60,0 fps, 0 saltados). Medido (RX 9070 XT, 240 Hz, d01f1b 1280x720): x2
  flujo 75 % calidad, GPU 1,07-1,22 ms por real, presentar 0,23 ms, VRAM 31,5+21,1 MB, 99,5 % presentadas,
  0 fallos; x3, 1,73 ms. Cadena en FIFO con N+1 imágenes de más; presentaciones desde el hilo grabador
  (rec_QueuePresentCall/rec_present_direct); no va con HDR ni con la generación de AMD. Prueba con y sin
  DLL en build/lsfg_sin_run.log / build/lsfg_con_run.log / build/lsfg_x3_run.log; la captura del mismo
  fotograma con y sin generación no cambia (media 1,4, banda de ruido de la zona).
- **0.50.1-beta:** el instalador lleva la lsfg.dll (nuestra, del lsfg-vk-android de GameNative, MIT) junto al exe
  y sus licencias en su carpeta (lsfg-vk-android, el DXBC de DXVK, pe-parse, volk) con su entrada en LICENCIAS.txt;
  el Lossless.dll del jugador NUNCA. Instalación --auto probada: la lsfg.dll y las licencias instaladas, y el
  juego instalado con la generación x2 encuentra el Lossless.dll de Steam («[lsfg] activa»).
- **0.50.3-beta:** la revisión de la 0.50.2 (no publicada) encontró el fallo: el plan respondía 0 tanto para «no
  aplica» como para «imagen sin paso» y el llamador daba un paso igualmente — a 120/240 la lógica corría a la
  velocidad del límite. Corregido (-1 = camino original; 0..3 = pasos, 0 sin llamar a update_common) y medido con
  un contador de llamadas REALES a update_common: 1395=1395 (57,9/s), 1209=1209 (57,9), 1164=1164 (57,1),
  1432=1432 (57,0), 1182=1182 (56,9) a 60/60/120/240/240; el control LARECOMP_BATTLE_FIX=0 a 60 da 1176 llamadas
  (55,2/s: el propio combate salta un 8 % de fases de Update) — el reparto queda igual o por encima del original
  a todos los límites. Duraciones 20,4-25,1 s en todas las corridas (la banda entre corridas iguales, ±3-4 s,
  cubre las diferencias entre límites; la mueve la traba de ~900 ms de la transición de entrada, cosa del motor,
  que desplaza la fase de las pulsaciones). Pulsaciones de 8 ms a 240 no se pierden; la apertura empareja
  (2-4,3 %) y la primera ventana de golpe sale en los tres (2184/2158/2186 píxeles en t=3330). FIX=0 ahora
  también registra su combate y su tasa.
- **0.50.2-beta:** los combates dibujan al «Límite de FPS» con la lógica separada del dibujo: el vsync ya no se
  fuerza a 60 en combate; BattleManager.update (sustituida en game_overrides.c) reparte los pasos de update_common
  por tiempo real (60/s, 0-3 por imagen, deuda cortada a 4 pasos), y las pulsaciones entre pasos se retienen en la
  instantánea del mando (ControllerObserver._npadState) para el paso que toca. Medido a 60/120/240: el juego dibuja
  al límite dentro del combate (60/120/240,0 fps), la lógica a 57-58 pasos/s (el salto de entrada al combate recorta
  deuda), las capturas al mismo tiempo de pared emparejan (2-5 %) y los rótulos de los golpes a tiempo salen en el
  mismo momento en los tres límites; a 240, pulsaciones de 8 ms (menos que un paso) no se pierden. BATTLE_FPS=30 y
  BATTLE_FIX=0 siguen como antes; battle_slow (LSFG) sigue coherente. Prueba: un guion de combate aparte, con su análisis.
  (Sus medidas salieron de la compilación con el fallo que la revisión encontró: las válidas, en la 0.50.3.)
- **0.51.0-beta:** el instalador pide la copia del juego ya extraída (la carpeta con exefs y romfs, o una instalación
  anterior): la comprueba (build ID de main, título de main.npdm y los 1.934 archivos de la RomFS por huella) y la pone
  en `extracted/` con enlaces duros o copiándola. El programa se llama SMRPG Recomp. Probado: instalación completa en
  11 min 42 s con 8 hilos, copia a otra unidad, copias incompletas, de otro juego o de otra versión rechazadas, y el
  juego instalado (oculto): título y cinemática de partida nueva. Primera versión del repositorio público.
- Nombres de IL2CPP (clases, métodos y sus direcciones en main): Il2CppDumper 6.7.46 sobre `extracted/exefs/main` y
  `romfs/Data/Managed/Metadata/global-metadata.dat` (con `DOTNET_ROLL_FORWARD=Major` en .NET 8).
- Pendiente:
  - Cofre «?» sin madera (d02out): tras 6 combates encadenados en el claro y volver al tramo de los montes
    redondos, del cofre solo se dibujan los dos aros negros con tachuelas y el «?», no el cuerpo de madera. Con UN
    combate y la vuelta sale bien. Tres rutas nuevas (golpe1/2/3, 2026-11-20, con el trazador de las 10 funciones
    GS enganchado, LARECOMP_LOG_CHEST=1): CERO llamadas en las tres. Descartado: (1) la parada de la vuelta larga
    (3000 fotogramas a la izquierda tras el segundo combate) deja el cofre a la vista pero NO a alcance de
    martillo — una ráfaga de A ahí no hace nada (dif. 4,6 % = ruido); (2) alargar la caminata para «chocar» con el
    cofre engancha al enemigo del claro: la ráfaga de A abre COMBATE (las capturas de golpe2/3 quedaron en la
    pantalla de batalla esperando entrada, no en un panel del objeto: el registro siguió a 60 fps, 0 dibujos
    saltados, sin crash, 25.400 fotogramas). Nueva hipótesis: el cofre roto es el cofre YA ABIERTO (golpeado antes
    de los combates), y lo que no se dibuja es el cuerpo del cofre abierto al volver. Ruta pendiente de probar:
    la vuelta corta del guion por defecto (150 fotogramas tras el primer combate: Mario llega al cofre), A para
    abrirlo, cerrar el panel, encadenar más combates a la derecha, ida a los montes y vuelta. Pendiente: trazar las
    funciones GS del cofre (un trazador tras dispatch_init en runtime/main.c, aparte).
  - Barrido de TODAS las zonas (2026-11-20, completado a las 416 escenas del catálogo de
    addressables; a 3 instancias a la vez, cada una con su LARECOMP_DATA_ROOT (con extracted enlazado y su propio
    build): SIN fallos reales — 0 dibujos saltados y 0 caidas en las 416, captura doble en 390, y
    las 566 capturas negras + 6 oscuras son contenido del PROPIO juego (volcado de pases con LARECOMP_DEBUG_FRAME:
    sus pases de escena a 0,6 de media y su interfaz, en los mismos pases, a 74): salas en estado de suceso
    oscurecido a las que el juego no te deja reaparecer. Las 19 zonas sin capturas son nombres que no son pisos
    de reaparicion y el propio juego los rechaza («Your game files are incomplete»: e04-e47, d19b4yuka/pp,
    f01fire*, m06kd, m07sky, m3idle...); d04b2bwater y d04b3c/d entre ellas. A 3 instancias: 60 fps, max 20-27
    ms, sin degradacion. Registros: build/sweep_todo.log (y los sweep_pc*.log); capturas en build/zonas/.
  - Mando real y audio por los altavoces, sin probar por mí (las pruebas van con `LARECOMP_NO_AUDIO=1`; el audio
    se comprueba con `LARECOMP_AUDOUT_WAV`).
  - Pantalla ancha: el juego pone su interfaz en un panel fijo de 1920x1080; el fondo del título, el oscurecido de
    los diálogos y el fundido a blanco del título a la apertura no llegan a los lados (los fundidos a negro de las
    transiciones los cubren desde la 0.48.1).
  - 22 shaders sin traducir, ninguno que el juego use (VR, wireframe, LookDev, CopyDepth con MSAA, 7 internos de main).
  - Generación de fotogramas: la interfaz del propio juego (batallas, el panel del campo) se interpola con la escena (sin
    imagen «sin interfaz» del juego: haría falta la imagen tras el posproceso y antes de la interfaz); con HDR, también
    el contador de FPS. Sin probar con la ventana visible ni a pantalla completa (las pruebas van ocultas).
  - Más allá de la primera batalla, sin recorrer: Mushroom Way usa RGBA4 (versión 23, sin comprobar en el juego).

### Cómo dibuja Unity en NVN (lo que hizo falta en el traductor)

- Texturas y samplers separados: el shader carga la palabra de la textura de `c[2][0x168 + 8r]` y la del sampler de
  `c[2][0x568 + 8r]`, las une con OR y muestrea con `TEX.B` / `TLD.B` (bindless). El traductor rastrea esas palabras
  (`tex_b_words`) y el renderizador compone el manejador (`shader_tex_handle`).
- Los shaders de vértices descartan pronto con `VOTE.VTG` + `EXIT CC.FCSM_TR`: ese EXIT no debe ejecutarse.

### Cosas del runtime de Link's Awakening que no valen para SMRPG

- La tabla de direcciones `v1_0_0` (`runtime/game_addresses/*_v1_0_0.h`) es la de Link's Awakening 1.0.0: en SMRPG
  esas direcciones no apuntan a nada suyo. `g_game_las` (false) apaga lo que las lee en cada fotograma; si aparece
  otro fallo que pasa por `GA(...)`, va por el mismo camino.
- El generador ya no aplica los parches ni ganchos de LA a SMRPG (`tools/game_addresses.py GAME = 'smrpg'`). Las
  variantes del aleatorizador (`tools/rando/patch_variants.json`) sí se generan, pero sin semilla ejecutan la
  instrucción original.
- Las bases de los módulos: main en 0x7000000000 (hasta 0x7004dbc000 con su .bss) y sdk en 0x7005000000.

### Leer la memoria del juego colgado con gdb

Las direcciones del invitado son direcciones del proceso, así que gdb las lee tal cual (en Windows `long` es de 32 bits:
usar `long long`):

```sh
PID=$(tasklist //FI "IMAGENAME eq smrpg-recomp.exe" //FO CSV //NH | head -1 | cut -d, -f2 | tr -d '"')
gdb -p $PID -batch -ex 'x/8gx 0x750f804b80+0x390' -ex detach -ex quit
```

## Cómo probar un arranque

Desde MSYS2 (UCRT64), en la carpeta del repositorio, con el volcado del juego enlazado en `extracted/`:

```sh
(export PATH="/c/msys64/ucrt64/bin:/c/msys64/usr/bin:$PATH"; make -j16)
# (si cambian tools/lifter.py, tools/gen.py o runtime/cpu.h: antes «make gen», y se recompila todo el juego)
export PATH="/c/msys64/ucrt64/bin:$PATH"   # ¡sin /usr/bin de MSYS2 delante! (ver abajo)
LARECOMP_HANG_DUMP=20 LARECOMP_NO_AUDIO=1 LARECOMP_NO_MOVIE_AUDIO=1 LARECOMP_FORCE_LAUNCHER=0 \
  timeout 90 ./build/smrpg-recomp.exe --play extracted > build/boot_X.log 2>&1
```

- Trampa: con `/c/msys64/usr/bin` delante en el PATH, el `timeout` de MSYS2 (mezclado con el bash de Git) no pasa las
  variables de entorno al juego: ninguna `LARECOMP_*` llega. Para compilar hace falta ese PATH (en un subshell); para
  lanzar, solo `ucrt64/bin`.
- `LARECOMP_FNPROF=0:999999`: cada volcado de cuelgue escribe `build/fnprof_cuelgueN.txt` (dirección y veces que
  corrió cada función del juego). Con `LARECOMP_FNPROF_BT=<dir>,...` se ve quién llama primero a cada una y con qué x0.
- `LARECOMP_WATCH_ADDR=<lo>,<hi>`: qué funciones del juego tocan esa memoria (desde el arranque).
- `LARECOMP_CRASH_HOLD=<s>`: tras un fallo el proceso espera esos segundos antes de salir (para `gdb -p`).

- `init sdk` / `init main`: los constructores. Un error fatal termina con la pila del juego («guest stack scan») y las
  últimas 32 llamadas al host del hilo que falló.
- `LARECOMP_HANG_DUMP=<s>`: si el hilo principal pasa tanto tiempo en una espera, se vuelca cada hilo con su nombre de
  Unity, lo que espera, su pila y la traza del kernel (los últimos 2.048 eventos de mutex y condvar).
- Las direcciones 0x7000000000+ son de `main` y las 0x7005000000+ del `sdk` (hasta la versión 15, 0x7004000000+).

## Analizar el código del juego

- `nxnative/dm/tools/nso2elf.py` convierte `exefs/main`, `exefs/sdk` y `exefs/subsdk0` en ELF para Ghidra (no se suben:
  son código del juego). En Ghidra hay que poner la base de imagen en 0x7000000000 (main) o 0x7005000000 (sdk).
- Los stubs de la PLT llaman a través de la GOT: para saber a qué función llaman hay que resolver la reubicación
  JUMP_SLOT de su entrada de la GOT (así se renombraron los 8.821 stubs del sdk como `plt_<símbolo>`).
- Los punteros a funciones NVN de Unity están en variables de `main` (0x3da5xxx) que su cargador rellena con
  nvnDeviceGetProcAddress; se reconocen por el uso.
- El `sdk` y el `subsdk0` de la 1.0.0 son idénticos a los de la 1.0.1 (mismo build ID), la que juega nxnative: lo que
  nxnative sabe del SDK de este juego sirve tal cual, y su `docs/reference/` (threads, memory, rendering) recoge muchos
  problemas de Unity en Switch ya resueltos.

## Lo que se sabe del juego

- `nn::init::Start` (sdk 0xfa3c0): nnosInitialize → (llamada temprana) → nninitStartup → (llamada tardía: los
  constructores de sdk y main) → nnMain.
- El NPDM declara 16 MB de «system resource»: la memoria virtual de nn::os (VammManager) está activada. El
  `nninitStartup` del juego (main 0x62de90) reserva (memoria total − usada) ≈ 3,1 GB de espacio virtual para su
  `StandardAllocator` (objeto en main 0x3d822d0) y no usa SetHeapSize. El `AddressRegionManager` trabaja sobre la región
  alias: se queda el primer GB y alinea a 1 GB lo grande. Por eso la región alias es de 48 GB.
- MapPhysicalMemory solo mapea (y pone a cero) las páginas que faltan, y solo en la región alias.
- `svcBreak` con motivo 0 es «Panic» (`nn::diag::detail::AbortImpl`): antes, el juego llama a am SetTerminateResult.
- Unity (`main`) define operator new/delete globales que sustituyen a los débiles del sdk (interposición, como el rtld).
- nn::ro: Unity solo llama a Initialize y a LookupSymbol (no hay NRO); el rtld no existe aquí.
- El GC de IL2CPP para los hilos con SuspendThread/GetThreadContext: pausa cooperativa en puntos seguros.
- NVN 55: la `NVNtextureView` del juego ocupa 16 bytes; la `NVNtexture`, 0x80. Unity pide 277 funciones NVN; faltan
  19 en el runtime (Barrier, SetAlphaRef, SetPointSize, CopyBufferToBuffer, binds «Deferred»…) y varias son vacías
  (BindSeparateSampler(s), ReportValue, DrawTexture…): habrá que hacerlas cuando Unity dibuje.
- Orden de memoria: en AArch64 `STLR` + `LDAR` son seq_cst; en x86 hay que traducir `STLR` con barrera completa.
- Hilos de Unity: MainThread, UnityGfxDeviceWorker, Loading.AsyncRead, Job.Worker 0-1, Background Job.Worker 0-15,
  GC Finalizer, AssetGarbageCollectorHelper, BatchDeleteObjects, NetworkInterfaceWrapper, LibcurlGc, un hilo de vsync
  sin nombre (main 0x6b536c) y NxSystemThread (del SDK).
- `main` importa 31 funciones de `subsdk0` (nn::movie). El runtime no carga `subsdk0`: `runtime/hle/movie.c` las
  implementa con FFmpeg (versión 19: las que usa CRIWARE Sofdec). Sin implementar aún, porque no se han visto
  llamadas: `GetH264NalUnitStartPositionAndNalSize` y las del `Extractor` llamadas por símbolo.
- Los vídeos son USM de CRIWARE con VP9 1920x1080 a 60 imágenes/s y audio Opus (un flujo estéreo, por `hwopus`).
  CRI crea el decodificador con tipo 3 (VP9), formato 3 y modo 2 (textura nativa).
- CRI pide 196 veces el tamaño del búfer de un decodificador Opus multistream de 2 flujos mono (orden 7 de
  `hwopus`) sin abrirlo: es su cálculo de memoria, no un fallo.
