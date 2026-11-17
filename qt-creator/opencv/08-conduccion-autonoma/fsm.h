#ifndef FSM_H
#define FSM_H

#include "color.h"
#include "franja.h"

// Lógica pura (sin Qt y sin OpenCV): la máquina de estados que conduce.
//
//   Medida (lo que vio la cámara)  ->  paso()  ->  Orden (lo que se le manda al robot)
//
// Se llama UNA vez por ciclo de decisión (un QTimer de 125 ms en mainwindow.cpp),
// no una vez por cuadro.

enum class Estado {
    Espera,         // recién abierto o detenido con el botón: no se mueve
    Seguir,         // sigue el carril; las paletas cambian la marcha
    Alto,           // paleta roja: frena y espera msAlto
    LineaPerdida,   // no ve ninguna raya: avanza despacio con la última dirección
    Paro            // paro de seguridad: sin rayas demasiado tiempo, sin video o sin conexión
};

enum class Marcha { Lento, Crucero, Rapido };

// Lo que la percepción entrega en cada ciclo.
struct Medida {
    Lineas lineas = Lineas::Ninguna;
    int error = 0;                  // px: centro del carril - centro de la imagen
    Color color = Color::Ninguno;   // paleta vista en ESTE cuadro (sin antirrebote)
};

// Lo que la máquina decide. Es abstracto: mainwindow.cpp lo traduce a JSON.
struct Orden {
    enum class Tipo { Parar, Conducir };
    Tipo tipo = Tipo::Parar;
    int acelerador = 0;   // 0 a 100
    int direccion = 0;    // -100 (izquierda) a 100 (derecha)
    int freno = 0;        // 0 a 100
};
bool operator==(const Orden &a, const Orden &b);
bool operator!=(const Orden &a, const Orden &b);

struct ConfigFsm {
    int gananciaPorCien = 35;   // control proporcional: direccion = error * 35 / 100
    int acelLento = 22;         // paleta amarilla
    int acelCrucero = 28;       // paleta verde (y al arrancar)
    int acelRapido = 34;        // paleta azul
    int acelPerdida = 18;       // sin rayas: despacio
    int msAlto = 3000;          // cuánto dura el alto de la paleta roja
    int msPerdida = 1500;       // cuánto se aguanta sin rayas antes del paro
    int cuadrosColor = 3;       // antirrebote: el color cuenta tras verlo 3 ciclos seguidos
};

class MaquinaConduccion
{
public:
    explicit MaquinaConduccion(const ConfigFsm &config = ConfigFsm());

    void iniciar();            // botón Iniciar: Espera o Paro -> Seguir
    void detener();            // botón Detener: cualquier estado -> Espera
    void paroDeSeguridad(const char *motivo);   // sin video o sin conexión -> Paro

    Orden paso(const Medida &m, int dtMs);      // un ciclo de decisión

    Estado estado() const { return m_estado; }
    Marcha marcha() const { return m_marcha; }
    Color colorFirme() const { return m_colorFirme; }
    const char *motivo() const { return m_motivo; }   // por qué fue el último cambio de estado

private:
    void cambiar(Estado nuevo, const char *motivo);
    void filtrarColor(Color visto);
    Orden seguirCarril(int error);
    Orden aCiegas() const;

    ConfigFsm m_config;
    Estado m_estado = Estado::Espera;
    Marcha m_marcha = Marcha::Crucero;
    const char *m_motivo = "inicio";
    int m_tiempo = 0;                // ms que lleva en el estado actual (Alto y LineaPerdida)
    int m_ultimaDireccion = 0;       // para seguir a ciegas
    Color m_candidato = Color::Ninguno;
    int m_cuenta = 0;
    Color m_colorFirme = Color::Ninguno;
    bool m_rojoAtendido = false;     // ya hizo alto por ESTA paleta roja: no repetirlo
};

const char *nombre(Estado estado);
const char *nombre(Marcha marcha);

// Robot real (ESP32-S3 con practica4.ino): no entiende «conducir», solo «mover»
// con accion y ms, y sus motores son todo o nada. La dirección se vuelve tres casos
// con una banda muerta.
struct MovimientoReal {
    const char *accion;   // "avanzar", "izquierda", "derecha" o "parar"
    int ms;
};
MovimientoReal aRobotReal(const Orden &o, int banda = 25, int msAvance = 300,
                          int msGiro = 120);

#endif // FSM_H
