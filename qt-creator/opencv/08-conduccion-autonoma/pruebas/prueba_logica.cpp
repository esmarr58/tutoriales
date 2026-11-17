// Prueba de la lógica pura del 08: franja.cpp, color.cpp y fsm.cpp. Sin Qt, sin OpenCV, sin robot.
//
//   g++ -std=c++17 -I.. prueba_logica.cpp ../franja.cpp ../color.cpp ../fsm.cpp -o prueba_logica
//   ./prueba_logica datos
//
// Parte 1: casos escritos a mano (los números del deck de la Clase 26).
// Parte 2: datos reales del robot virtual (carpeta datos/, generada por exportar_datos.py):
//          el C++ debe dar exactamente lo mismo que el prototipo en Python.

#include "color.h"
#include "franja.h"
#include "fsm.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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

// Perfil sintético: asfalto (6) con dos rayas blancas (210).
static std::vector<std::uint8_t> perfil(int ancho, int izq0, int izq1, int der0, int der1,
                                        int fondo = 6, int raya = 210)
{
    std::vector<std::uint8_t> p(ancho, static_cast<std::uint8_t>(fondo));
    for (int x = izq0; x <= izq1 && x < ancho; ++x) if (x >= 0) p[x] = static_cast<std::uint8_t>(raya);
    for (int x = der0; x <= der1 && x < ancho; ++x) if (x >= 0) p[x] = static_cast<std::uint8_t>(raya);
    return p;
}

static void probarFranja()
{
    ParametrosFranja p;
    // Cuadro real de la salida (fila 360): rayas en 153-169 y 470-486.
    Carril c = analizarFranja(perfil(640, 153, 169, 470, 486), p, 301);
    comprobar(c.lineas == Lineas::Dos, "salida: dos rayas");
    comprobar(c.izquierda == 169 && c.derecha == 470, "salida: bordes 169 y 470");
    comprobar(c.ancho == 301 && c.centro == 319 && c.error == -1, "salida: ancho 301, centro 319, error -1");

    // Coche cargado a la izquierda: todo se ve 60 px a la derecha.
    c = analizarFranja(perfil(640, 213, 229, 530, 546), p, 301);
    comprobar(c.lineas == Lineas::Dos && c.centro == 379 && c.error == 59, "desviado: centro 379, error +59");

    // Solo la raya izquierda: el centro se estima con el ancho de referencia.
    c = analizarFranja(perfil(640, 100, 116, 700, 700), p, 301);
    comprobar(c.lineas == Lineas::SoloIzquierda && c.centro == 116 + 150 && c.error == 266 - 320,
              "solo izquierda: centro 266, error -54");

    // Solo la raya derecha.
    c = analizarFranja(perfil(640, -5, -5, 500, 516), p, 301);
    comprobar(c.lineas == Lineas::SoloDerecha && c.centro == 500 - 150 && c.error == 30,
              "solo derecha: centro 350, error +30");

    // Ninguna raya.
    c = analizarFranja(std::vector<std::uint8_t>(640, 6), p, 301);
    comprobar(c.lineas == Lineas::Ninguna && c.error == 0, "sin rayas: ninguna, error 0");

    // La meta a cuadros: blanco a 22 px de cada lado del centro -> ancho 44 < 160: no es carril.
    c = analizarFranja(perfil(640, 280, 298, 342, 360), p, 301);
    comprobar(c.lineas == Lineas::Ninguna && c.ancho == 44, "meta a cuadros: ancho 44, se descarta");

    // La pista se abre (bifurcación): ancho 540 > 512 -> se confía en la raya derecha.
    c = analizarFranja(perfil(640, 30, 50, 590, 606), p, 301);
    comprobar(c.lineas == Lineas::SoloDerecha && c.centro == 590 - 150, "bifurcación: solo derecha");

    // Pista real: tiras negras (30) sobre piso claro (200).
    ParametrosFranja oscuras;
    oscuras.rayasOscuras = true;
    oscuras.umbral = 90;
    c = analizarFranja(perfil(480, 60, 80, 400, 420, 200, 30), oscuras, 320);
    comprobar(c.lineas == Lineas::Dos && c.izquierda == 80 && c.derecha == 400 && c.error == 0,
              "tiras negras: bordes 80 y 400, error 0");

    comprobar(analizarFranja({}, p, 301).lineas == Lineas::Ninguna, "perfil vacío");
}

static void probarColor()
{
    // Colores de referencia del mapa Circuito (plan del bloque) y medidos en cuadros reales.
    comprobar(clasificarHsv(1, 237, 217) == Color::Rojo, "rojo de referencia (1,237,217)");
    comprobar(clasificarHsv(178, 230, 210) == Color::Rojo, "rojo del otro lado del cero (178)");
    comprobar(clasificarHsv(65, 235, 166) == Color::Verde, "verde de referencia");
    comprobar(clasificarHsv(112, 226, 230) == Color::Azul, "azul de referencia");
    comprobar(clasificarHsv(25, 241, 242) == Color::Amarillo, "amarillo de referencia");
    comprobar(clasificarHsv(38, 153, 152) == Color::Ninguno, "pasto (38,153,152) no es amarillo ni verde");
    comprobar(clasificarHsv(0, 0, 239) == Color::Ninguno, "cielo blanco: sin saturación");
    comprobar(clasificarHsv(0, 0, 6) == Color::Ninguno, "asfalto negro");
    comprobar(clasificarHsv(90, 200, 200) == Color::Ninguno, "cian: entre verde y azul");
    comprobar(rangosDe(Color::Rojo).size() == 2, "el rojo tiene dos rangos");

    ParametrosDisco d;
    comprobar(esDisco({797, 34, 32}, d), "paleta amarilla real: 797 px en 34 x 32");
    comprobar(!esDisco({233, 29, 9}, d), "piano rojo real: 233 px en 29 x 9 (chico y alargado)");
    comprobar(!esDisco({900, 60, 20}, d), "banda alargada: aspecto 3");
    comprobar(!esDisco({330, 28, 28}, d), "anillo de velocidad: llenado 0.42");
    comprobar(!esDisco({200, 16, 16}, d), "disco lejano: 200 px < 300");
    comprobar(!esDisco({500, 0, 10}, d), "caja sin ancho");

    comprobar(colorDominante(700, 900, 0, 0, 300) == Color::Rojo, "el rojo manda aunque el verde sea mayor");
    comprobar(colorDominante(0, 400, 600, 500, 300) == Color::Azul, "sin rojo: el de mayor área");
    comprobar(colorDominante(299, 299, 299, 299, 300) == Color::Ninguno, "todo por debajo del área mínima");
}

static Medida medida(Lineas l, int error, Color c)
{
    Medida m;
    m.lineas = l;
    m.error = error;
    m.color = c;
    return m;
}

static void probarMaquina()
{
    const int dt = 125;
    MaquinaConduccion q;
    Orden o = q.paso(medida(Lineas::Dos, 0, Color::Ninguno), dt);
    comprobar(q.estado() == Estado::Espera && o.tipo == Orden::Tipo::Parar, "sin Iniciar no se mueve");

    q.iniciar();
    o = q.paso(medida(Lineas::Dos, -45, Color::Ninguno), dt);
    comprobar(q.estado() == Estado::Seguir && o.acelerador == 28 && o.direccion == -15,
              "SEGUIR: error -45 -> direccion -15, acelerador 28");
    o = q.paso(medida(Lineas::Dos, 400, Color::Ninguno), dt);
    comprobar(o.direccion == 100, "la dirección se satura en 100");

    // Antirrebote: dos cuadros amarillos no bastan; tres sí.
    q.paso(medida(Lineas::Dos, 0, Color::Amarillo), dt);
    q.paso(medida(Lineas::Dos, 0, Color::Amarillo), dt);
    comprobar(q.marcha() == Marcha::Crucero, "2 cuadros amarillos: sigue en crucero");
    o = q.paso(medida(Lineas::Dos, 0, Color::Amarillo), dt);
    comprobar(q.marcha() == Marcha::Lento && o.acelerador == 22, "3 cuadros amarillos: marcha lenta (22)");
    for (int i = 0; i < 3; ++i) o = q.paso(medida(Lineas::Dos, 0, Color::Azul), dt);
    comprobar(q.marcha() == Marcha::Rapido && o.acelerador == 34, "azul: marcha rápida (34)");
    for (int i = 0; i < 3; ++i) o = q.paso(medida(Lineas::Dos, 0, Color::Verde), dt);
    comprobar(q.marcha() == Marcha::Crucero && o.acelerador == 28, "verde: crucero (28)");

    // Paleta roja: ALTO de 3000 ms = 24 ciclos de 125 ms.
    q.paso(medida(Lineas::Dos, 0, Color::Rojo), dt);
    q.paso(medida(Lineas::Dos, 0, Color::Rojo), dt);
    comprobar(q.estado() == Estado::Seguir, "2 cuadros rojos: todavía SEGUIR");
    o = q.paso(medida(Lineas::Dos, 0, Color::Rojo), dt);
    comprobar(q.estado() == Estado::Alto && o.freno == 100 && o.acelerador == 0, "3 cuadros rojos: ALTO y freno");
    for (int i = 0; i < 23; ++i) q.paso(medida(Lineas::Dos, 0, Color::Rojo), dt);
    comprobar(q.estado() == Estado::Alto, "a los 2875 ms sigue en ALTO");
    q.paso(medida(Lineas::Dos, 0, Color::Rojo), dt);
    comprobar(q.estado() == Estado::Seguir, "a los 3000 ms vuelve a SEGUIR");
    // La paleta roja sigue a la vista: NO debe volver a detenerse.
    for (int i = 0; i < 10; ++i) o = q.paso(medida(Lineas::Dos, 0, Color::Rojo), dt);
    comprobar(q.estado() == Estado::Seguir && o.acelerador == 28, "la misma paleta roja no detiene dos veces");
    // Deja de verla 3 ciclos y aparece OTRA roja: ahora sí.
    for (int i = 0; i < 3; ++i) q.paso(medida(Lineas::Dos, 0, Color::Ninguno), dt);
    for (int i = 0; i < 3; ++i) q.paso(medida(Lineas::Dos, 0, Color::Rojo), dt);
    comprobar(q.estado() == Estado::Alto, "otra paleta roja sí detiene");
    q.detener();
    comprobar(q.estado() == Estado::Espera, "Detener lleva a ESPERA");

    // Línea perdida: 1500 ms = 12 ciclos a ciegas con la última dirección; luego PARO.
    MaquinaConduccion r;
    r.iniciar();
    r.paso(medida(Lineas::Dos, 60, Color::Ninguno), dt);                 // dirección 21
    o = r.paso(medida(Lineas::Ninguna, 0, Color::Ninguno), dt);
    comprobar(r.estado() == Estado::LineaPerdida && o.acelerador == 18 && o.direccion == 21,
              "sin rayas: LINEA_PERDIDA, despacio (18) con la última dirección (21)");
    for (int i = 0; i < 11; ++i) o = r.paso(medida(Lineas::Ninguna, 0, Color::Ninguno), dt);
    comprobar(r.estado() == Estado::LineaPerdida, "a los 1375 ms sigue buscando");
    o = r.paso(medida(Lineas::Ninguna, 0, Color::Ninguno), dt);
    comprobar(r.estado() == Estado::Paro && o.tipo == Orden::Tipo::Parar, "a los 1500 ms: PARO");
    o = r.paso(medida(Lineas::Dos, 0, Color::Ninguno), dt);
    comprobar(r.estado() == Estado::Paro && o.tipo == Orden::Tipo::Parar, "de PARO no se sale solo");
    r.iniciar();
    comprobar(r.estado() == Estado::Seguir, "Iniciar saca de PARO");
    r.paso(medida(Lineas::Ninguna, 0, Color::Ninguno), dt);
    o = r.paso(medida(Lineas::SoloDerecha, 12, Color::Ninguno), dt);
    comprobar(r.estado() == Estado::Seguir && o.direccion == 4, "reaparece una raya: SEGUIR");
    r.paroDeSeguridad("sin video");
    comprobar(r.estado() == Estado::Paro && std::strcmp(r.motivo(), "sin video") == 0, "paro de seguridad");

    // Traducción al robot real: banda muerta de 25.
    comprobar(std::strcmp(aRobotReal({Orden::Tipo::Conducir, 28, 10, 0}).accion, "avanzar") == 0, "real: avanzar");
    comprobar(std::strcmp(aRobotReal({Orden::Tipo::Conducir, 28, -40, 0}).accion, "izquierda") == 0, "real: izquierda");
    comprobar(std::strcmp(aRobotReal({Orden::Tipo::Conducir, 28, 26, 0}).accion, "derecha") == 0, "real: derecha");
    comprobar(std::strcmp(aRobotReal({Orden::Tipo::Conducir, 0, 0, 100}).accion, "parar") == 0, "real: freno = parar");
    comprobar(aRobotReal({Orden::Tipo::Conducir, 28, 0, 0}).ms == 300, "real: avanzar dura 300 ms");
}

// ---------------------------------------------------------------- datos reales (Python = C++)

static void probarPerfiles(const std::string &carpeta)
{
    std::ifstream archivo(carpeta + "/perfiles.txt");
    if (!archivo) { std::printf("AVISO: no existe %s/perfiles.txt\n", carpeta.c_str()); return; }
    std::string linea;
    int n = 0;
    while (std::getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        std::istringstream s(linea);
        std::string nombreCuadro;
        int anchoRef, umbral, oscuras, lineas, izq, der, centro, error, anchoCarril, total;
        s >> nombreCuadro >> anchoRef >> umbral >> oscuras >> lineas >> izq >> der >> centro >> error
          >> anchoCarril >> total;
        std::vector<std::uint8_t> p(total);
        for (int i = 0; i < total; ++i) { int v; s >> v; p[i] = static_cast<std::uint8_t>(v); }
        ParametrosFranja par;
        par.umbral = umbral;
        par.rayasOscuras = oscuras != 0;
        const Carril c = analizarFranja(p, par, anchoRef);
        comprobar(static_cast<int>(c.lineas) == lineas && c.izquierda == izq && c.derecha == der
                      && c.centro == centro && c.error == error && c.ancho == anchoCarril,
                  "perfil real " + nombreCuadro);
        ++n;
    }
    std::printf("perfiles reales comparados: %d\n", n);
}

static void probarManchas(const std::string &carpeta)
{
    std::ifstream archivo(carpeta + "/manchas.txt");
    if (!archivo) { std::printf("AVISO: no existe %s/manchas.txt\n", carpeta.c_str()); return; }
    std::string linea;
    int n = 0;
    while (std::getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        std::istringstream s(linea);
        std::string nombreCuadro, colorTexto;
        Mancha m;
        int esperado;
        s >> nombreCuadro >> colorTexto >> m.area >> m.ancho >> m.alto >> esperado;
        comprobar(esDisco(m, ParametrosDisco()) == (esperado != 0),
                  "mancha real " + nombreCuadro + " " + colorTexto);
        ++n;
    }
    std::printf("manchas reales comparadas: %d\n", n);
}

static void probarTraza(const std::string &carpeta)
{
    std::ifstream archivo(carpeta + "/traza.txt");
    if (!archivo) { std::printf("AVISO: no existe %s/traza.txt\n", carpeta.c_str()); return; }
    MaquinaConduccion q;
    q.iniciar();
    std::string linea;
    int n = 0, distintos = 0;
    while (std::getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        std::istringstream s(linea);
        int lineas, error, rojo, verde, azul, amarillo, dt, color, estado, marcha, firme, conducir, acel, dir, freno;
        s >> lineas >> error >> rojo >> verde >> azul >> amarillo >> dt
          >> color >> estado >> marcha >> firme >> conducir >> acel >> dir >> freno;
        const Color visto = colorDominante(rojo, verde, azul, amarillo, ParametrosDisco().areaMin);
        const Orden o = q.paso(medida(static_cast<Lineas>(lineas), error, visto), dt);
        const bool igual = static_cast<int>(visto) == color && static_cast<int>(q.estado()) == estado
                           && static_cast<int>(q.marcha()) == marcha && static_cast<int>(q.colorFirme()) == firme
                           && (o.tipo == Orden::Tipo::Conducir) == (conducir != 0)
                           && o.acelerador == acel && o.direccion == dir && o.freno == freno;
        if (!igual) ++distintos;
        ++n;
    }
    comprobar(n > 0 && distintos == 0, "traza real: " + std::to_string(distintos) + " ciclos distintos de "
                                           + std::to_string(n));
    std::printf("ciclos de la traza real comparados: %d (distintos: %d)\n", n, distintos);
}

int main(int argc, char *argv[])
{
    probarFranja();
    probarColor();
    probarMaquina();
    if (argc > 1) {
        const std::string carpeta = argv[1];
        probarPerfiles(carpeta);
        probarManchas(carpeta);
        probarTraza(carpeta);
    } else {
        std::printf("(sin carpeta de datos: solo los casos a mano)\n");
    }
    std::printf("%d comprobaciones correctas, %d fallas\n", correctas, fallas);
    return fallas == 0 ? 0 : 1;
}
