# -*- coding: utf-8 -*-
"""Genera pruebas/datos/ para prueba_logica.cpp a partir de cuadros reales y de una traza real.

    python exportar_datos.py [traza.jsonl]

- datos/perfiles.txt : el perfil de la franja de cada cuadro de pruebas/cuadros/ y lo que el
                       prototipo en Python encontró (rayas, bordes, centro, error, ancho).
- datos/manchas.txt  : cada mancha de color de esos cuadros (área y caja) y si pasa es_disco.
- datos/traza.txt    : (si se da traza.jsonl de vuelta_simulador.py) lo que vio la percepción en cada
                       ciclo y lo que decidió la máquina de estados en Python.
El C++ debe reproducir cada renglón: así se comprueba que franja.cpp, color.cpp y fsm.cpp
hacen lo mismo que el prototipo que sí manejó el coche.
"""
import json
import sys
from pathlib import Path

import cv2

from prototipo import (COLORES, RANGOS, ParametrosFranja, analizar_franja,
                       es_disco, leer_imagen, mascara, perfil_de_franja, ZONA_COLOR)

AQUI = Path(__file__).resolve().parent
CUADROS = AQUI / "cuadros"
DATOS = AQUI / "datos"
DATOS.mkdir(exist_ok=True)

p = ParametrosFranja()
perfiles = ["# cuadro ancho_ref umbral oscuras | lineas izq der centro error ancho | n valores..."]
manchas = ["# cuadro color area ancho alto es_disco"]
for ruta in sorted(CUADROS.glob("*.jpg")):
    img = leer_imagen(ruta)
    ancho_ref = int(p.ancho_inicial * img.shape[1])
    perfil = perfil_de_franja(img, p)
    c = analizar_franja(perfil, p, ancho_ref)
    perfiles.append(" ".join(str(v) for v in [ruta.stem, ancho_ref, p.umbral, int(p.oscuras), c["lineas"], c["izq"],
                                              c["der"], c["centro"], c["error"], c["ancho"], len(perfil)])
                    + " " + " ".join(str(int(v)) for v in perfil))
    alto = img.shape[0]
    y0, y1 = int(alto * ZONA_COLOR[0]), int(alto * ZONA_COLOR[1])
    hsv = cv2.cvtColor(img[y0:y1], cv2.COLOR_BGR2HSV)
    for color in RANGOS:
        n, _e, tabla, _c = cv2.connectedComponentsWithStats(mascara(hsv, color), connectivity=8)
        for i in range(1, n):
            _x, _y, w, h, area = (int(v) for v in tabla[i])
            if area >= 20:            # las motas de menos de 20 px no aportan
                manchas.append(f"{ruta.stem} {COLORES[color]} {area} {w} {h} {int(es_disco(area, w, h))}")
(DATOS / "perfiles.txt").write_text("\n".join(perfiles) + "\n", encoding="utf-8")
(DATOS / "manchas.txt").write_text("\n".join(manchas) + "\n", encoding="utf-8")
print(f"perfiles: {len(perfiles) - 1} · manchas: {len(manchas) - 1}")

if len(sys.argv) > 1:
    filas = ["# lineas error rojo verde azul amarillo dt | color estado marcha firme conducir acel dir freno"]
    for linea in open(sys.argv[1], encoding="utf-8"):
        d = json.loads(linea)
        tipo, acel, direccion, freno = d["orden"]
        filas.append(" ".join(str(v) for v in [d["lineas"], d["error"], *d["areas"], d["dt"], d["color"], d["estado"],
                                               d["marcha"], d["firme"], int(tipo == "conducir"), acel, direccion, freno]))
    (DATOS / "traza.txt").write_text("\n".join(filas) + "\n", encoding="utf-8")
    print(f"traza: {len(filas) - 1} ciclos")
