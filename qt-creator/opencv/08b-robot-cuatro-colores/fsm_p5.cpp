#include "fsm_p5.h"

// LA TABLA color -> orden. Si el profesor cambia la tabla, se cambia aquí y en ningún otro lado.
Accion ordenDe(Color color)
{
    switch (color) {
    case Color::Verde: return Accion::Avanzar;
    case Color::Azul: return Accion::Izquierda;
    case Color::Amarillo: return Accion::Derecha;
    case Color::Rojo: return Accion::Parar;
    case Color::Ninguno: return Accion::Parar;     // sin hoja no se mueve
    }
    return Accion::Parar;
}

// Los motores del robot son todo o nada: avanzar en tramos de 300 ms y girar en tramos de 120 ms.
// La orden se repite en cada ciclo (125 ms) mientras el color siga firme; si deja de llegar,
// el robot se detiene solo al cumplirse ms.
Movimiento movimientoDe(Accion accion)
{
    switch (accion) {
    case Accion::Avanzar: return {"avanzar", 300};
    case Accion::Izquierda: return {"izquierda", 120};
    case Accion::Derecha: return {"derecha", 120};
    case Accion::Parar: break;
    }
    return {"parar", 0};
}

void MaquinaColores::cambiar(Estado nuevo, const char *motivo)
{
    m_estado = nuevo;
    m_motivo = motivo;
}

void MaquinaColores::iniciar()
{
    if (m_estado == Estado::Espera || m_estado == Estado::Paro)
        cambiar(Estado::Obedecer, "boton Iniciar");
}

void MaquinaColores::detener()
{
    if (m_estado == Estado::Obedecer)
        cambiar(Estado::Espera, "boton Detener");
}

void MaquinaColores::paroDeSeguridad(const char *motivo)
{
    if (m_estado == Estado::Obedecer)
        cambiar(Estado::Paro, motivo);
}

// Antirrebote: un color solo «cuenta» después de verlo CICLOS_COLOR ciclos seguidos.
// Lo mismo vale para «ninguno»: al quitar la hoja, el robot para 3 ciclos después.
void MaquinaColores::filtrarColor(Color visto)
{
    if (visto == m_candidato) {
        ++m_cuenta;
    } else {
        m_candidato = visto;
        m_cuenta = 1;
    }
    if (m_cuenta >= CICLOS_COLOR)
        m_colorFirme = m_candidato;
}

Accion MaquinaColores::paso(Color visto)
{
    filtrarColor(visto);

    switch (m_estado) {
    case Estado::Espera:
    case Estado::Paro:
        return Accion::Parar;              // solo se sale con el botón Iniciar

    case Estado::Obedecer:
        return ordenDe(m_colorFirme);      // la orden de la tabla; sin color firme, parar
    }
    return Accion::Parar;
}

const char *nombre(Estado estado)
{
    switch (estado) {
    case Estado::Espera: return "ESPERA";
    case Estado::Obedecer: return "OBEDECER";
    case Estado::Paro: return "PARO";
    }
    return "?";
}

const char *nombre(Accion accion)
{
    return movimientoDe(accion).accion;
}
