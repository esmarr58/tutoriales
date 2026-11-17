#include "fsm.h"

namespace {
const Orden PARAR{Orden::Tipo::Parar, 0, 0, 100};
const Orden FRENAR{Orden::Tipo::Conducir, 0, 0, 100};
}

bool operator==(const Orden &a, const Orden &b)
{
    return a.tipo == b.tipo && a.acelerador == b.acelerador
           && a.direccion == b.direccion && a.freno == b.freno;
}

bool operator!=(const Orden &a, const Orden &b)
{
    return !(a == b);
}

MaquinaConduccion::MaquinaConduccion(const ConfigFsm &config)
    : m_config(config)
{
}

void MaquinaConduccion::cambiar(Estado nuevo, const char *motivo)
{
    m_estado = nuevo;
    m_motivo = motivo;
    m_tiempo = 0;   // cada estado empieza a contar su tiempo desde cero
}

void MaquinaConduccion::iniciar()
{
    if (m_estado != Estado::Espera && m_estado != Estado::Paro)
        return;
    m_marcha = Marcha::Crucero;
    m_ultimaDireccion = 0;
    cambiar(Estado::Seguir, "boton Iniciar");
}

void MaquinaConduccion::detener()
{
    cambiar(Estado::Espera, "boton Detener");
}

void MaquinaConduccion::paroDeSeguridad(const char *motivo)
{
    if (m_estado != Estado::Espera && m_estado != Estado::Paro)
        cambiar(Estado::Paro, motivo);
}

// Antirrebote: un color solo «cuenta» después de verlo cuadrosColor ciclos seguidos.
// Un cuadro con ruido no debe frenar al coche.
void MaquinaConduccion::filtrarColor(Color visto)
{
    if (visto == m_candidato) {
        ++m_cuenta;
    } else {
        m_candidato = visto;
        m_cuenta = 1;
    }
    if (m_cuenta >= m_config.cuadrosColor)
        m_colorFirme = m_candidato;
    // Al dejar de ver el rojo, la siguiente paleta roja vuelve a contar.
    if (m_colorFirme != Color::Rojo)
        m_rojoAtendido = false;
}

// Control proporcional: entre más lejos del centro, más volante.
Orden MaquinaConduccion::seguirCarril(int error)
{
    int direccion = error * m_config.gananciaPorCien / 100;
    if (direccion > 100) direccion = 100;
    if (direccion < -100) direccion = -100;
    m_ultimaDireccion = direccion;

    int acelerador = m_config.acelCrucero;
    if (m_marcha == Marcha::Lento) acelerador = m_config.acelLento;
    if (m_marcha == Marcha::Rapido) acelerador = m_config.acelRapido;
    return Orden{Orden::Tipo::Conducir, acelerador, direccion, 0};
}

Orden MaquinaConduccion::aCiegas() const
{
    return Orden{Orden::Tipo::Conducir, m_config.acelPerdida, m_ultimaDireccion, 0};
}

Orden MaquinaConduccion::paso(const Medida &m, int dtMs)
{
    filtrarColor(m.color);

    switch (m_estado) {
    case Estado::Espera:
    case Estado::Paro:
        return PARAR;   // solo se sale con el botón Iniciar

    case Estado::Seguir:
        if (m_colorFirme == Color::Rojo && !m_rojoAtendido) {
            cambiar(Estado::Alto, "paleta roja");
            return FRENAR;
        }
        if (m.lineas == Lineas::Ninguna) {
            cambiar(Estado::LineaPerdida, "no se ve ninguna raya");
            return aCiegas();
        }
        // Las otras tres paletas no cambian de estado: cambian la marcha.
        if (m_colorFirme == Color::Amarillo) m_marcha = Marcha::Lento;
        if (m_colorFirme == Color::Verde) m_marcha = Marcha::Crucero;
        if (m_colorFirme == Color::Azul) m_marcha = Marcha::Rapido;
        return seguirCarril(m.error);

    case Estado::Alto:
        m_tiempo += dtMs;
        if (m_tiempo >= m_config.msAlto) {
            m_rojoAtendido = true;          // esta paleta ya se obedeció
            m_marcha = Marcha::Crucero;
            cambiar(Estado::Seguir, "termino el alto");
        }
        return FRENAR;

    case Estado::LineaPerdida:
        if (m.lineas != Lineas::Ninguna) {
            cambiar(Estado::Seguir, "rayas a la vista");
            return seguirCarril(m.error);
        }
        m_tiempo += dtMs;
        if (m_tiempo >= m_config.msPerdida) {
            cambiar(Estado::Paro, "demasiado tiempo sin rayas");
            return PARAR;
        }
        return aCiegas();
    }
    return PARAR;
}

const char *nombre(Estado estado)
{
    switch (estado) {
    case Estado::Espera: return "ESPERA";
    case Estado::Seguir: return "SEGUIR";
    case Estado::Alto: return "ALTO";
    case Estado::LineaPerdida: return "LINEA_PERDIDA";
    case Estado::Paro: return "PARO";
    }
    return "?";
}

const char *nombre(Marcha marcha)
{
    switch (marcha) {
    case Marcha::Lento: return "lento";
    case Marcha::Crucero: return "crucero";
    case Marcha::Rapido: return "rapido";
    }
    return "?";
}

MovimientoReal aRobotReal(const Orden &o, int banda, int msAvance, int msGiro)
{
    if (o.tipo == Orden::Tipo::Parar || o.freno > 0 || o.acelerador <= 0)
        return {"parar", 0};
    if (o.direccion < -banda)
        return {"izquierda", msGiro};
    if (o.direccion > banda)
        return {"derecha", msGiro};
    return {"avanzar", msAvance};
}
