#ifndef FSM_P5_H
#define FSM_P5_H

#include "color.h"

// Lógica pura (sin Qt y sin OpenCV): la máquina de estados de la Práctica 5.
//
//   color visto en este ciclo  ->  paso()  ->  Accion (lo que se le manda al robot)
//
// Se llama UNA vez por ciclo de decisión (un QTimer de 125 ms en mainwindow.cpp), no una vez por cuadro.

enum class Estado {
    Espera,     // recién abierto o detenido con el botón: no se mueve
    Obedecer,   // ejecuta la orden del color firme; sin color firme, parar
    Paro        // paro de seguridad: sin video o sin conexión. Solo sale con el botón Iniciar
};

enum class Accion { Parar, Avanzar, Izquierda, Derecha };

// LA TABLA de la práctica: qué orden da cada color. Está en un solo lugar (fsm_p5.cpp).
Accion ordenDe(Color color);

// Lo que entiende el robot real (firmware de las prácticas 3 y 4): {"tipo":"mover","accion":…,"ms":…}
struct Movimiento {
    const char *accion;   // "avanzar", "izquierda", "derecha" o "parar"
    int ms;               // cuánto dura; 0 = sin ms (parar)
};
Movimiento movimientoDe(Accion accion);

class MaquinaColores
{
public:
    void iniciar();                              // botón Iniciar: Espera o Paro -> Obedecer
    void detener();                              // botón Detener: Obedecer -> Espera
    void paroDeSeguridad(const char *motivo);    // sin video o sin conexión: Obedecer -> Paro

    Accion paso(Color visto);                    // un ciclo de decisión

    Estado estado() const { return m_estado; }
    Color colorFirme() const { return m_colorFirme; }
    const char *motivo() const { return m_motivo; }   // por qué fue el último cambio de estado

    static const int CICLOS_COLOR = 3;           // antirrebote: 3 ciclos seguidos (375 ms)

private:
    void cambiar(Estado nuevo, const char *motivo);
    void filtrarColor(Color visto);

    Estado m_estado = Estado::Espera;
    const char *m_motivo = "inicio";
    Color m_candidato = Color::Ninguno;
    int m_cuenta = 0;
    Color m_colorFirme = Color::Ninguno;
};

const char *nombre(Estado estado);
const char *nombre(Accion accion);

#endif // FSM_P5_H
