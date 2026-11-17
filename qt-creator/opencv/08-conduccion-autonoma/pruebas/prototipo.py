# -*- coding: utf-8 -*-
"""Prototipo en Python + OpenCV de la lógica del programa 08.

Es el ESPEJO de franja.cpp, color.cpp, fsm.cpp y percepcion.cpp: mismas reglas, mismos números.
Sirve para probar el algoritmo con cuadros reales del robot virtual sin compilar Qt
(validar_cuadros.py, vuelta_simulador.py) y para generar los datos con que se prueba el C++
(exportar_datos.py -> prueba_logica.cpp).
"""
from dataclasses import dataclass, field

import cv2
import numpy as np


def leer_imagen(ruta):
    """cv2.imread falla en Windows si la ruta tiene acentos o ñ: se lee el archivo y se decodifica."""
    return cv2.imdecode(np.fromfile(str(ruta), dtype=np.uint8), cv2.IMREAD_COLOR)


# ---------------------------------------------------------------- franja (franja.cpp)
NINGUNA, SOLO_IZQ, SOLO_DER, DOS = 0, 1, 2, 3
LINEAS = ["ninguna", "solo izquierda", "solo derecha", "dos"]


@dataclass
class ParametrosFranja:
    fila: float = 0.75        # dónde empieza la franja (fracción del alto)
    filas: int = 8            # cuántas filas tiene
    umbral: int = 170
    oscuras: bool = False
    ancho_min: float = 0.25
    ancho_max: float = 0.80
    ancho_inicial: float = 0.47   # ancho del carril antes de verlo (fracción del ancho)


def perfil_de_franja(bgr, p):
    """Gris promedio de cada columna de la franja: una fila de `ancho` números (percepcion.cpp)."""
    y = int(bgr.shape[0] * p.fila)
    gris = cv2.cvtColor(bgr[y:y + p.filas], cv2.COLOR_BGR2GRAY)
    return cv2.reduce(gris, 0, cv2.REDUCE_AVG).ravel()


def es_raya(valor, p):
    return valor <= p.umbral if p.oscuras else valor >= p.umbral


def analizar_franja(perfil, p, ancho_ref):
    ancho = len(perfil)
    mitad = ancho // 2
    c = dict(lineas=NINGUNA, izq=-1, der=-1, centro=mitad, error=0, ancho=0)
    for x in range(mitad - 1, -1, -1):
        if es_raya(perfil[x], p):
            c["izq"] = x
            break
    for x in range(mitad, ancho):
        if es_raya(perfil[x], p):
            c["der"] = x
            break
    if c["izq"] >= 0 and c["der"] >= 0:
        c["ancho"] = c["der"] - c["izq"]
        if c["ancho"] < p.ancho_min * ancho:
            return c
        if c["ancho"] <= p.ancho_max * ancho:
            c["lineas"] = DOS
            c["centro"] = (c["izq"] + c["der"]) // 2
            c["error"] = c["centro"] - mitad
            return c
        c["izq"] = -1
    if c["izq"] >= 0:
        c.update(lineas=SOLO_IZQ, ancho=ancho_ref, centro=c["izq"] + ancho_ref // 2)
        c["error"] = c["centro"] - mitad
    elif c["der"] >= 0:
        c.update(lineas=SOLO_DER, ancho=ancho_ref, centro=c["der"] - ancho_ref // 2)
        c["error"] = c["centro"] - mitad
    return c


# ---------------------------------------------------------------- color (color.cpp)
NINGUNO, ROJO, VERDE, AZUL, AMARILLO = 0, 1, 2, 3, 4
COLORES = ["ninguno", "rojo", "verde", "azul", "amarillo"]
RANGOS = {   # (hMin, hMax, sMin, vMin) en HSV de OpenCV: H 0..179
    ROJO: [(0, 10, 120, 100), (170, 179, 120, 100)],
    VERDE: [(50, 80, 150, 100)],
    AZUL: [(100, 125, 120, 100)],
    AMARILLO: [(20, 34, 180, 180)],
}
AREA_MIN, ASPECTO_MIN, ASPECTO_MAX, LLENADO_MIN = 300, 0.70, 1.40, 0.60
ZONA_COLOR = (0.30, 0.55)    # filas (fracción del alto) donde se buscan las paletas


def clasificar_hsv(h, s, v):
    for color in (ROJO, VERDE, AZUL, AMARILLO):
        for (h0, h1, s0, v0) in RANGOS[color]:
            if h0 <= h <= h1 and s >= s0 and v >= v0:
                return color
    return NINGUNO


def es_disco(area, ancho, alto):
    if area < AREA_MIN or ancho <= 0 or alto <= 0:
        return False
    return ASPECTO_MIN <= ancho / alto <= ASPECTO_MAX and area / (ancho * alto) >= LLENADO_MIN


def mascara(hsv, color):
    m = None
    for (h0, h1, s0, v0) in RANGOS[color]:
        parte = cv2.inRange(hsv, (h0, s0, v0), (h1, 255, 255))
        m = parte if m is None else cv2.bitwise_or(m, parte)
    return m


def manchas(bgr):
    """Por color: disco = la mancha más grande que pasa es_disco; bruta = la más grande sin filtro;
    total = todos los píxeles de ese color en la zona. Cada mancha es (area, x, y, ancho, alto)."""
    alto = bgr.shape[0]
    y0, y1 = int(alto * ZONA_COLOR[0]), int(alto * ZONA_COLOR[1])
    hsv = cv2.cvtColor(bgr[y0:y1], cv2.COLOR_BGR2HSV)
    salida = {}
    for color in RANGOS:
        n, _etiquetas, tabla, _centros = cv2.connectedComponentsWithStats(mascara(hsv, color), connectivity=8)
        disco = bruta = (0, 0, 0, 0, 0)
        total = 0
        for i in range(1, n):                       # la etiqueta 0 es el fondo
            x, y, w, h, area = (int(v) for v in tabla[i])
            total += area
            if area > bruta[0]:
                bruta = (area, x, y + y0, w, h)
            if es_disco(area, w, h) and area > disco[0]:
                disco = (area, x, y + y0, w, h)
        salida[color] = dict(disco=disco, bruta=bruta, total=total)
    return salida


def color_dominante(rojo, verde, azul, amarillo, area_min=AREA_MIN):
    if rojo >= area_min:
        return ROJO
    mejor, area = VERDE, verde
    if azul > area:
        mejor, area = AZUL, azul
    if amarillo > area:
        mejor, area = AMARILLO, amarillo
    return mejor if area >= area_min else NINGUNO


def percibir(bgr, p, ancho_ref):
    """Un cuadro -> (carril, color, manchas). Lo mismo que percibir() de percepcion.cpp."""
    carril = analizar_franja(perfil_de_franja(bgr, p), p, ancho_ref)
    m = manchas(bgr)
    color = color_dominante(*(m[c]["disco"][0] for c in (ROJO, VERDE, AZUL, AMARILLO)))
    return carril, color, m


# ---------------------------------------------------------------- máquina de estados (fsm.cpp)
ESPERA, SEGUIR, ALTO, LINEA_PERDIDA, PARO = range(5)
ESTADOS = ["ESPERA", "SEGUIR", "ALTO", "LINEA_PERDIDA", "PARO"]
LENTO, CRUCERO, RAPIDO = range(3)
MARCHAS = ["lento", "crucero", "rapido"]
PARAR = ("parar", 0, 0, 100)
FRENAR = ("conducir", 0, 0, 100)


@dataclass
class Config:
    ganancia_por_cien: int = 35
    acel: tuple = (22, 28, 34)     # lento, crucero, rápido
    acel_perdida: int = 18
    ms_alto: int = 3000
    ms_perdida: int = 1500
    cuadros_color: int = 3


@dataclass
class Maquina:
    cfg: Config = field(default_factory=Config)
    estado: int = ESPERA
    marcha: int = CRUCERO
    tiempo: int = 0
    ultima_direccion: int = 0
    candidato: int = NINGUNO
    cuenta: int = 0
    color_firme: int = NINGUNO
    rojo_atendido: bool = False

    def cambiar(self, nuevo):
        self.estado, self.tiempo = nuevo, 0

    def iniciar(self):
        if self.estado in (ESPERA, PARO):
            self.marcha, self.ultima_direccion = CRUCERO, 0
            self.cambiar(SEGUIR)

    def detener(self):
        self.cambiar(ESPERA)

    def paro_de_seguridad(self):
        if self.estado not in (ESPERA, PARO):
            self.cambiar(PARO)

    def _seguir(self, error):
        d = int(error * self.cfg.ganancia_por_cien / 100)      # trunca hacia cero, como C++
        d = max(-100, min(100, d))
        self.ultima_direccion = d
        return ("conducir", self.cfg.acel[self.marcha], d, 0)

    def _a_ciegas(self):
        return ("conducir", self.cfg.acel_perdida, self.ultima_direccion, 0)

    def paso(self, lineas, error, color, dt_ms):
        if color == self.candidato:
            self.cuenta += 1
        else:
            self.candidato, self.cuenta = color, 1
        if self.cuenta >= self.cfg.cuadros_color:
            self.color_firme = self.candidato
        if self.color_firme != ROJO:
            self.rojo_atendido = False

        if self.estado in (ESPERA, PARO):
            return PARAR
        if self.estado == SEGUIR:
            if self.color_firme == ROJO and not self.rojo_atendido:
                self.cambiar(ALTO)
                return FRENAR
            if lineas == NINGUNA:
                self.cambiar(LINEA_PERDIDA)
                return self._a_ciegas()
            if self.color_firme == AMARILLO:
                self.marcha = LENTO
            if self.color_firme == VERDE:
                self.marcha = CRUCERO
            if self.color_firme == AZUL:
                self.marcha = RAPIDO
            return self._seguir(error)
        if self.estado == ALTO:
            self.tiempo += dt_ms
            if self.tiempo >= self.cfg.ms_alto:
                self.rojo_atendido = True
                self.marcha = CRUCERO
                self.cambiar(SEGUIR)
            return FRENAR
        # LINEA_PERDIDA
        if lineas != NINGUNA:
            self.cambiar(SEGUIR)
            return self._seguir(error)
        self.tiempo += dt_ms
        if self.tiempo >= self.cfg.ms_perdida:
            self.cambiar(PARO)
            return PARAR
        return self._a_ciegas()


def a_robot_real(orden, banda=25, ms_avance=300, ms_giro=120):
    tipo, acelerador, direccion, freno = orden
    if tipo == "parar" or freno > 0 or acelerador <= 0:
        return ("parar", 0)
    if direccion < -banda:
        return ("izquierda", ms_giro)
    if direccion > banda:
        return ("derecha", ms_giro)
    return ("avanzar", ms_avance)
