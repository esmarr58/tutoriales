// Prueba de la lógica pura de la Práctica 5: color.cpp y fsm_p5.cpp. Sin Qt, sin OpenCV, sin robot.
//
//   g++ -std=c++17 -I.. prueba_logica.cpp ../color.cpp ../fsm_p5.cpp -o prueba_logica
//   ./prueba_logica

#include "color.h"
#include "fsm_p5.h"

#include <cstdio>
#include <cstring>
#include <string>

static int correctas = 0;
static int fallas = 0;

static void comprobar(bool condicion, const std::string &que)
{
    if (condicion) {
        ++correctas;
    } else {
        ++fallas;
        std::printf("FALLA: %s\n", que.c_str());
    }
}

static bool igual(const char *a, const char *b) { return std::strcmp(a, b) == 0; }

int main()
{
    // ---- La tabla color -> orden y su JSON «mover» ----
    comprobar(ordenDe(Color::Verde) == Accion::Avanzar, "verde = avanzar");
    comprobar(ordenDe(Color::Rojo) == Accion::Parar, "rojo = parar");
    comprobar(ordenDe(Color::Azul) == Accion::Izquierda, "azul = izquierda");
    comprobar(ordenDe(Color::Amarillo) == Accion::Derecha, "amarillo = derecha");
    comprobar(ordenDe(Color::Ninguno) == Accion::Parar, "sin hoja = parar");
    comprobar(igual(movimientoDe(Accion::Avanzar).accion, "avanzar") && movimientoDe(Accion::Avanzar).ms == 300,
              "avanzar dura 300 ms");
    comprobar(igual(movimientoDe(Accion::Izquierda).accion, "izquierda") && movimientoDe(Accion::Izquierda).ms == 120,
              "izquierda dura 120 ms");
    comprobar(igual(movimientoDe(Accion::Derecha).accion, "derecha") && movimientoDe(Accion::Derecha).ms == 120,
              "derecha dura 120 ms");
    comprobar(igual(movimientoDe(Accion::Parar).accion, "parar") && movimientoDe(Accion::Parar).ms == 0,
              "parar no lleva ms");

    // ---- La máquina de estados ----
    MaquinaColores m;
    comprobar(m.estado() == Estado::Espera, "arranca en ESPERA");
    for (int i = 0; i < 5; ++i)
        comprobar(m.paso(Color::Verde) == Accion::Parar, "en ESPERA no obedece, aunque vea verde");
    m.iniciar();
    comprobar(m.estado() == Estado::Obedecer, "Iniciar: ESPERA -> OBEDECER");
    comprobar(m.paso(Color::Verde) == Accion::Avanzar, "el verde ya era firme: avanza");

    // Antirrebote: dos ciclos de azul no bastan; tres sí.
    comprobar(m.paso(Color::Azul) == Accion::Avanzar, "azul, ciclo 1: sigue con la orden anterior");
    comprobar(m.paso(Color::Azul) == Accion::Avanzar, "azul, ciclo 2: sigue con la orden anterior");
    comprobar(m.paso(Color::Azul) == Accion::Izquierda, "azul, ciclo 3: izquierda");
    comprobar(m.colorFirme() == Color::Azul, "color firme = azul");
    // Un cuadro con ruido no cambia la orden.
    comprobar(m.paso(Color::Rojo) == Accion::Izquierda, "un ciclo de rojo (ruido) no cambia nada");
    comprobar(m.paso(Color::Azul) == Accion::Izquierda, "sigue azul");
    for (int i = 0; i < 3; ++i) m.paso(Color::Amarillo);
    comprobar(m.paso(Color::Amarillo) == Accion::Derecha, "amarillo firme: derecha");
    for (int i = 0; i < 2; ++i) m.paso(Color::Rojo);
    comprobar(m.paso(Color::Rojo) == Accion::Parar, "rojo firme: parar");
    comprobar(m.estado() == Estado::Obedecer, "con rojo sigue en OBEDECER (parado)");

    // Quitar la hoja: parar a los 3 ciclos.
    for (int i = 0; i < 3; ++i) m.paso(Color::Verde);
    comprobar(m.paso(Color::Ninguno) == Accion::Avanzar, "sin hoja, ciclo 1: todavía avanza");
    comprobar(m.paso(Color::Ninguno) == Accion::Avanzar, "sin hoja, ciclo 2: todavía avanza");
    comprobar(m.paso(Color::Ninguno) == Accion::Parar, "sin hoja, ciclo 3: parar");

    // Detener y paro de seguridad.
    for (int i = 0; i < 3; ++i) m.paso(Color::Verde);
    m.detener();
    comprobar(m.estado() == Estado::Espera && m.paso(Color::Verde) == Accion::Parar, "Detener: ESPERA y parar");
    m.paroDeSeguridad("sin video");
    comprobar(m.estado() == Estado::Espera, "el paro de seguridad no aplica en ESPERA");
    m.iniciar();
    m.paroDeSeguridad("sin video");
    comprobar(m.estado() == Estado::Paro && igual(m.motivo(), "sin video"), "sin video: OBEDECER -> PARO");
    comprobar(m.paso(Color::Verde) == Accion::Parar, "en PARO no obedece");
    m.detener();
    comprobar(m.estado() == Estado::Paro, "de PARO no se sale con Detener");
    m.iniciar();
    comprobar(m.estado() == Estado::Obedecer, "Iniciar: PARO -> OBEDECER");

    // ---- Rangos de partida y «Aprender» ----
    comprobar(clasificarHsv(60, 200, 200) == Color::Verde, "verde de partida");
    comprobar(clasificarHsv(3, 200, 200) == Color::Rojo && clasificarHsv(176, 200, 200) == Color::Rojo,
              "rojo de partida a los dos lados del cero");
    comprobar(clasificarHsv(110, 200, 200) == Color::Azul, "azul de partida");
    comprobar(clasificarHsv(27, 200, 220) == Color::Amarillo, "amarillo de partida");
    comprobar(clasificarHsv(0, 0, 230) == Color::Ninguno, "pared blanca: sin saturación");
    comprobar(clasificarHsv(60, 200, 30) == Color::Ninguno, "verde muy oscuro: sombra");

    std::vector<RangoHsv> r = rangosAprendidos(100, 180, 160);
    comprobar(r.size() == 1 && r[0].hMin == 88 && r[0].hMax == 112 && r[0].sMin == 110 && r[0].vMin == 70,
              "aprender (100,180,160): H 88-112, S min 110, V min 70");
    r = rangosAprendidos(4, 220, 200);
    comprobar(r.size() == 2 && r[0].hMin == 0 && r[0].hMax == 16 && r[1].hMin == 172 && r[1].hMax == 179,
              "aprender un rojo en H 4: 0-16 y 172-179");
    r = rangosAprendidos(175, 220, 200);
    comprobar(r.size() == 2 && r[0].hMin == 163 && r[0].hMax == 179 && r[1].hMin == 0 && r[1].hMax == 7,
              "aprender un rojo en H 175: 163-179 y 0-7");
    r = rangosAprendidos(30, 80, 100);
    comprobar(r[0].sMin == 50 && r[0].vMin == 50, "S y V mínimas nunca bajan de 50");
    // Una hoja «verde» que el punto de partida no reconoce (verde azulado, H 93): tras aprender, sí.
    comprobar(clasificarHsv(93, 150, 150) != Color::Verde, "verde azulado (H 93): no es verde de partida");
    fijarRangos(Color::Verde, rangosAprendidos(93, 150, 150));
    comprobar(clasificarHsv(93, 150, 150) == Color::Verde, "después de Aprender, sí es verde");

    std::printf("%d comprobaciones correctas, %d fallas\n", correctas, fallas);
    return fallas == 0 ? 0 : 1;
}
