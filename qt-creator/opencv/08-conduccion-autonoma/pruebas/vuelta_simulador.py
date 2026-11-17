# -*- coding: utf-8 -*-
"""Lazo cerrado contra el robot virtual (mapa Circuito) con la lógica de prototipo.py.

    python vuelta_simulador.py [segundos] [ws://127.0.0.1:8081] [carpeta de salida]

Necesita: pip install opencv-python websocket-client, y el juego abierto en el mapa Circuito
(en Linux: robotvirtual -- --mapa=circuito --puerto=8081 --puerto-video=8080).
Video: ws://IP:PUERTO/cam (un JPEG por mensaje; siempre se usa el más nuevo).
Guarda traza.jsonl (un renglón por ciclo de decisión), un cuadro en cada cambio y resumen.json.
Un ciclo cada 125 ms: nunca un mensaje por cuadro.
"""
import json
import sys
import threading
import time
from pathlib import Path

import cv2
import numpy as np
import websocket

from prototipo import (AMARILLO, AZUL, COLORES, DOS, ESTADOS, MARCHAS, PARO, ROJO, VERDE, Maquina,
                       ParametrosFranja, percibir)

SEGUNDOS = float(sys.argv[1]) if len(sys.argv) > 1 else 60
URL = sys.argv[2] if len(sys.argv) > 2 else "ws://127.0.0.1:8081"
SALIDA = Path(sys.argv[3]) if len(sys.argv) > 3 else Path(__file__).resolve().parent / "salida_vuelta"
CICLO = 0.125

ultimo = {}
infracciones = []
cuadro = {"jpg": None, "n": 0}


def al_mensaje(_ws, texto):
    try:
        d = json.loads(texto)
    except ValueError:
        return
    ultimo[d.get("tipo")] = d
    if d.get("tipo") == "infraccion":
        infracciones.append(d)


def al_cuadro(_ws, datos):
    if isinstance(datos, bytes):
        cuadro["jpg"] = datos
        cuadro["n"] += 1


comandos = websocket.WebSocketApp(URL, on_message=al_mensaje)
video = websocket.WebSocketApp(URL.rstrip("/") + "/cam", on_message=al_cuadro)
for w in (comandos, video):
    threading.Thread(target=w.run_forever, daemon=True).start()
time.sleep(1.5)


def enviar(**campos):
    comandos.send(json.dumps(campos))


enviar(tipo="latido")
enviar(tipo="reiniciar")
time.sleep(2.5)
SALIDA.mkdir(parents=True, exist_ok=True)
p = ParametrosFranja()
maquina = Maquina()
ancho_ref = None
traza = open(SALIDA / "traza.jsonl", "w", encoding="utf-8")
maquina.iniciar()
inicio = anterior = time.time()
n = enviados = 0
previo = None
ultimo_n = -1
sin_video = 0
while time.time() - inicio < SEGUNDOS and maquina.estado != PARO:
    ahora = time.time()
    dt = int(round((ahora - anterior) * 1000))
    anterior = ahora
    if cuadro["jpg"] is None or cuadro["n"] == ultimo_n:
        sin_video += dt
        if sin_video > 1000:
            maquina.paro_de_seguridad()
        time.sleep(CICLO)
        continue
    sin_video = 0
    ultimo_n = cuadro["n"]
    img = cv2.imdecode(np.frombuffer(cuadro["jpg"], np.uint8), cv2.IMREAD_COLOR)
    if ancho_ref is None:
        ancho_ref = int(p.ancho_inicial * img.shape[1])
    carril, color, m = percibir(img, p, ancho_ref)
    if carril["lineas"] == DOS:
        ancho_ref = carril["ancho"]
    orden = maquina.paso(carril["lineas"], carril["error"], color, dt)
    if orden[0] == "conducir":
        enviar(tipo="conducir", acelerador=orden[1], direccion=orden[2], freno=orden[3], ms=500)
        enviados += 1
    if n % 8 == 0:
        enviar(tipo="estado")       # una vez por segundo: posición y velocidad para la traza
        enviar(tipo="latido")
    e = ultimo.get("estado", {})
    fila = dict(n=n, t=round(ahora - inicio, 2), dt=dt, lineas=carril["lineas"], izq=carril["izq"],
                der=carril["der"], error=carril["error"], ancho=carril["ancho"],
                areas=[m[c]["disco"][0] for c in (ROJO, VERDE, AZUL, AMARILLO)], color=color,
                firme=maquina.color_firme, estado=maquina.estado, marcha=maquina.marcha,
                orden=list(orden), kmh=e.get("kmh"), x=e.get("x"), y=e.get("y"))
    traza.write(json.dumps(fila) + "\n")
    clave = (maquina.estado, maquina.marcha, carril["lineas"], maquina.color_firme)
    if clave != previo:
        nombre = (f"c{n:05d}_{ESTADOS[maquina.estado]}_{MARCHAS[maquina.marcha]}"
                  f"_{COLORES[maquina.color_firme]}_L{carril['lineas']}.jpg")
        cv2.imwrite(str(SALIDA / nombre), img)
        print(f"{fila['t']:7.2f} s  {ESTADOS[maquina.estado]:13s} {MARCHAS[maquina.marcha]:8s} "
              f"color={COLORES[maquina.color_firme]:8s} lineas={carril['lineas']} "
              f"error={carril['error']:4d} orden={orden} kmh={e.get('kmh')}", flush=True)
    previo = clave
    n += 1
    resto = CICLO - (time.time() - ahora)
    if resto > 0:
        time.sleep(resto)

traza.close()
enviar(tipo="mover", accion="parar")
time.sleep(1.0)
ultimo.pop("estado", None)
enviar(tipo="estado")
time.sleep(0.6)
e = ultimo.get("estado", {})
resumen = dict(ciclos=n, segundos=round(time.time() - inicio, 1), conducir_enviados=enviados,
               estado_final=ESTADOS[maquina.estado], x=e.get("x"), y=e.get("y"),
               infracciones=e.get("infracciones"), eventos=[i.get("clase") for i in infracciones])
print("RESUMEN", json.dumps(resumen, ensure_ascii=False))
(SALIDA / "resumen.json").write_text(json.dumps(resumen, ensure_ascii=False, indent=1), encoding="utf-8")
