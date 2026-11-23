// (SMRPG) Los combates: la lógica a 60 pasos por segundo, el dibujo al «Límite de FPS» que haya puesto quien juega.
//
// La lógica del combate (BattleManager.update_common, main 0x7001cd9360: turnos, golpes, las ventanas del golpe a
// tiempo, los sonidos, los movimientos de cámara) avanza por pasos de 1/60 s: BattleManager.update (main 0x7001cd9250,
// sustituida en tools/game_overrides.c) la llama una vez por imagen (y dos si _frame_rate, +0x1c8, vale 30, el modo
// de depuración). Al empezar un combate el juego llama a set_frame_rate (main 0x7001cd0500, también sustituida) con
// 60. Los animadores de los personajes van por tiempo (BattleCharacterAnimator.internal_set_animator_speed, main
// 0x7001e32420, solo los frena si una imagen tarda más de lo normal), igual que el sonido y la interfaz.
// Antes: mientras el combate tenía su tasa puesta, el vsync (hle/vi.c) late a 60 Hz — 60 imágenes y 60 pasos por
// segundo, como en la consola — porque a 240 imágenes el combate daba 240 pasos, cuatro veces más deprisa que sus
// animaciones y su sonido. Ahora el vsync late al «Límite de FPS» también en combate y los pasos los reparte
// update por tiempo real: un acumulador debe 1/60 s por paso, cada imagen hace 0, 1, 2 o 3 según lo acumulado
// (a 240, uno cada cuatro imágenes), con la deuda cortada a 4 pasos para no hacer espiral si una imagen tarda
// mucho: la lógica sigue dando exactamente 60 pasos por segundo.
// Las pulsaciones no se pierden entre pasos: la instantánea del mando (ControllerObserver._npadState: buttons y
// preButtons; los flancos se calculan al vuelo, GetButtonDown = buttons & ~preButtons) se mira una vez por imagen y
// lo pulsado o soltado desde el paso anterior se retiene y se entrega al paso que toca, como si el botón hubiera
// estado pulsado durante todo ese fotograma de la consola. Los pasos extra de una misma imagen van con la
// instantánea real (el doble paso por imagen ya existía en el modo de 30 y el juego lo lleva bien).
// Con la generación de fotogramas (vk_lsfg.c battle_slow): el combate necesita 60 imágenes reales donde dar sus 60
// pasos; si con el multiplicador y la pantalla no caben, la generación se apaga mientras dure, como antes.
// Pruebas: LARECOMP_BATTLE_FPS=30 (el modo de depuración: el vsync a 60 Hz y dos pasos por imagen, como antes);
// LARECOMP_BATTLE_FIX=0 (como el juego: la tasa que pide y un paso por imagen al «Límite de FPS», el combate
// acelerado, para comparar).
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "../hle.h"
#include "../platform.h"

extern uint64_t renderer_present_count(void);  // (runtime/nvn: la imagen que se está presentando)

static volatile uint64_t bm_statics;  // los campos estáticos de BattleManager (_instance en +0x10)
static volatile int battle_on;        // (lo que lee el hilo del vsync: él no toca la memoria del juego)

// ---- el reparto de pasos y la retención de pulsaciones (tools/game_overrides.c, una vez por imagen) ----
static int want;                       // 60, o 30 con LARECOMP_BATTLE_FPS=30
static uint64_t acc_ns, last_ns;       // lo que debe la lógica, y el momento de la imagen anterior
static uint64_t pend_down, pend_up;    // flancos vistos desde el paso anterior, retenidos hasta el paso que toca
static uint64_t last_buttons;          // buttons de la instantánea en la imagen anterior (flancos propios)
static uint64_t npad_addr;             // ControllerObserver._npadState (estáticos de la clase + 8)
static int pace_started;               // la primera imagen del combate: un paso enseguida, como el original
static uint64_t battle_t0_ns;          // el arranque del combate, para la tasa de llamadas del final
static uint64_t steps_run;             // pasos planeados por el reparto (los cuenta quien los pide)
static uint64_t update_calls;          // llamadas REALES a update_common (las cuenta quien las hace: tiene que
                                       // salir igual que steps_run — si no, alguien da pasos de más)
static void pace_clear(void) { acc_ns = last_ns = 0, pend_down = pend_up = last_buttons = 0, npad_addr = 0, pace_started = 0; steps_run = update_calls = 0; }

RT_ON_RESET(battle_rate_reset) { bm_statics = 0, battle_on = 0; pace_clear(); }  // (la memoria del juego se rehace al reiniciarlo)

static int fix_on(void) {
    static int on = -1;
    if (on < 0) on = !getenv("LARECOMP_BATTLE_FIX") || atoi(getenv("LARECOMP_BATTLE_FIX")) != 0;
    return on;
}

// la instantánea del mando: la clase de ControllerObserver sale de su hueco de metadatos (el valor apunta a la
// clase), los estáticos de la clase a +0xb8, y _npadState es su campo estático +0x8 (buttons +8, preButtons +0x28)
static uint64_t npad_state_addr(void) {
    if (npad_addr) return npad_addr;
    uint64_t slot = LD64(0x7003a59000ull + 0x4e8);
    uint64_t klass = slot ? LD64(slot) : 0;
    uint64_t statics = klass ? LD64(klass + 0xb8) : 0;
    npad_addr = statics ? statics + 0x8 : 0;
    return npad_addr;
}

// (tools/game_overrides.c, set_frame_rate; el hilo principal de Unity) la tasa del combate que empieza
int rt_battle_frame_rate(uint64_t statics, int requested) {
    if (!fix_on()) {  // (LARECOMP_BATTLE_FIX=0: la tasa que pide el juego; el registro sigue viendo el combate)
        bm_statics = statics, want = 60, pace_clear();
        LOG("[combate] el juego pide %d FPS: %d (LARECOMP_BATTLE_FIX=0: un paso por imagen, como el juego)\n", requested, requested);
        return requested;
    }
    bm_statics = statics;  // (el combate se ve en la próxima imagen, rt_battle_poll: _frame_rate se pone al volver)
    want = getenv("LARECOMP_BATTLE_FPS") && atoi(getenv("LARECOMP_BATTLE_FPS")) == 30 ? 30 : 60;
    pace_clear();  // (la primera imagen que ve el combate arranca el reparto; sin flancos fantasma de antes)
    LOG("[combate] el juego pide %d FPS: %d (%s), con el dibujo al «Límite de FPS» mientras dure (fotograma %llu)\n", requested, want,
        want == 60 ? "60 pasos de lógica por segundo repartidos por tiempo real" : "el modo de depuración: dos pasos por imagen",
        (unsigned long long)renderer_present_count());
    return want;
}

// (nvn/vulkan/device.c backend_present: un hilo del juego, en cada imagen) ¿sigue el combate con su tasa puesta?
// BattleManager._instance con _frame_rate != 0 (_instance es una referencia estática: mientras apunta al objeto, el
// objeto sigue vivo). Lo lee un hilo del juego, que se detiene antes de que se rehaga su memoria.
void rt_battle_poll(void) {
    uint64_t s = bm_statics;
    if (!s) return;
    uint64_t inst = LD64(s + 0x10);
    int on = inst && LD32(inst + 0x1c8) != 0;
    if (on && !battle_on) {
        battle_t0_ns = plat_now_ns();
        LOG("[combate] empieza (fotograma %llu, %lld ms)\n", (unsigned long long)renderer_present_count(),
            (long long)(battle_t0_ns / 1000000));
    }
    if (!on && battle_on)
        LOG("[combate] acaba (fotograma %llu, %lld ms, %llu pasos de lógica, %llu llamadas a update_common, %.1f por segundo)\n",
            (unsigned long long)renderer_present_count(), (long long)(plat_now_ns() / 1000000),
            (unsigned long long)steps_run, (unsigned long long)update_calls,
            update_calls * 1000.0 / (double)((plat_now_ns() - battle_t0_ns) / 1000000 + 1));
    battle_on = on;
}

// (hle/vi.c, el hilo del vsync) 60 Hz en combate solo con el modo de 30 (sus dos pasos por imagen); con el de 60,
// el «Límite de FPS»: los pasos los reparte BattleManager.update por tiempo real
int rt_battle_vsync_hz(int hz) { return battle_on && want == 30 ? 60 : hz; }

// (vk_lsfg.c battle_slow) true mientras dura un combate: su lógica da 60 pasos por segundo y necesita 60 imágenes
// reales donde darlos; si no caben, la generación se apaga mientras dure
bool rt_battle_vsync_60(void) { return battle_on != 0; }

// (tools/game_overrides.c, BattleManager.update: una vez por imagen) el plan de esta imagen: -1 = el reparto no
// aplica (sin combate, tasa != 60, modo de 30, LARECOMP_BATTLE_FIX=0: el camino original, un paso por imagen);
// 0..3 = pasos de lógica de este reparto según el tiempo acumulado (0: esta imagen no toca ninguno, y NO se llama
// a update_common). La dirección de la instantánea del mando va en *npad (0 solo con -1). Retiene por el camino
// los flancos de la instantánea desde el paso anterior.
int rt_battle_plan(int rate, uint64_t *npad) {
    static const uint64_t PERIOD = 1000000000ull / 60;
    *npad = 0;
    if (!fix_on() || want != 60 || !battle_on || rate != 60) return -1;
    uint64_t n = npad_state_addr();
    if (!n) return -1;
    *npad = n;
    uint64_t b = LD64(n + 8);
    if (!pace_started) {  // (la primera imagen del combate: un paso enseguida, como el original)
        pace_started = 1;
        last_buttons = b, acc_ns = PERIOD, last_ns = plat_now_ns();
    } else {
        pend_down |= b & ~last_buttons;   // (pulsado desde el paso anterior: se lo lleva el paso que toca)
        pend_up |= ~b & last_buttons;      // (soltado: igual)
        last_buttons = b;
        uint64_t now = plat_now_ns();
        acc_ns += now - last_ns, last_ns = now;
    }
    if (acc_ns > 4 * PERIOD) acc_ns = 4 * PERIOD;  // (una imagen muy lenta: lógica perdida, no espiral)
    int steps = (int)(acc_ns / PERIOD);
    if (steps > 3) steps = 3;
    acc_ns -= (uint64_t)steps * PERIOD;
    steps_run += (uint64_t)steps;
    return steps;
}

// (tools/game_overrides.c, antes de cada paso) la instantánea que ve este paso: el primero, la real con los
// flancos retenidos desde el paso anterior; los demás, la real tal cual
void rt_battle_step_view(int k, uint64_t npad, uint64_t *b_set, uint64_t *p_set) {
    uint64_t b = LD64(npad + 8), p = LD64(npad + 0x28);
    if (k == 0 && (pend_down || pend_up)) {
        *b_set = (b & ~pend_up) | pend_down;   // (pulsado aunque ya se haya soltado: como ese fotograma de consola)
        *p_set = (p & ~pend_down) | pend_up;
        pend_down = pend_up = 0;               // (entregados)
    } else { *b_set = b, *p_set = p; }
}

// (tools/game_overrides.c, en cada llamada a update_common, también en el camino original) cuenta las llamadas
// reales: tiene que salir igual que los pasos planeados, y ~60 por segundo de combate
void rt_battle_step_note(void) { update_calls++; }

// (tools/game_overrides.c) los estáticos de BattleManager, guardados al empezar el combate
uint64_t rt_battle_statics(void) { return bm_statics; }
