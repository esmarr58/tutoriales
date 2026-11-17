# -*- coding: utf-8 -*-
"""Prueba de escritorio de la Práctica 5, sin teléfono y sin robot.

    python prueba_escritorio.py RUTA/robot_colores[.exe]

Levanta en esta misma PC (127.0.0.1):
  - un video MJPEG de «hojas de color» sintéticas (puerto 8772), con un guion fijo;
  - un robot de mentira por WebSocket (puerto 8771) que anota cada JSON que recibe.
Lanza el programa con --fuente, --robot y --auto, y comprueba que cada hoja produce su «mover»,
que al quitar la hoja manda «parar», que sin video pasa a PARO y que «Aprender» funciona.
Necesita: pip install opencv-python websockets. No usa el robot virtual.
Termina con código 0 si todo pasa.
"""
import asyncio
import json
import subprocess
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import cv2
import numpy as np
import websockets

PUERTO_ROBOT, PUERTO_VIDEO = 8771, 8772
VERDE, AZUL, AMARILLO, ROJO = (60, 170, 40), (200, 90, 30), (40, 215, 235), (45, 40, 205)   # BGR de «papel»
RARO = tuple(int(c) for c in cv2.cvtColor(np.uint8([[[93, 150, 150]]]), cv2.COLOR_HSV2BGR)[0, 0])  # verde azulado

# Guiones: (desde s, hasta s, hoja, BGR o None, orden esperada). «corte» = el video deja de llegar.
GUION_1 = [(0, 7, "ninguna", None, "parar"), (7, 11, "verde", VERDE, "avanzar"), (11, 15, "azul", AZUL, "izquierda"),
           (15, 19, "amarillo", AMARILLO, "derecha"), (19, 23, "rojo", ROJO, "parar"),
           (23, 27, "verde", VERDE, "avanzar"), (27, 31, "ninguna", None, "parar"),
           (31, 35, "verde", VERDE, "avanzar"), (35, 40, "corte", "corte", "parar")]
GUION_2 = [(0, 12, "verde azulado", RARO, "avanzar")]

estado = {"t0": time.time(), "guion": GUION_1, "registro": []}
rng = np.random.default_rng(1)


def escena():
    t = time.time() - estado["t0"]
    actual = estado["guion"][-1]
    for tramo in estado["guion"]:
        if tramo[0] <= t < tramo[1]:
            actual = tramo
            break
    return t, actual


class Video(BaseHTTPRequestHandler):
    def log_message(self, *a):
        pass

    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "multipart/x-mixed-replace; boundary=frame")
        self.end_headers()
        try:
            while True:
                _t, (_a, _b, _nombre, bgr, _orden) = escena()
                if isinstance(bgr, str):                 # corte: no se mandan cuadros
                    time.sleep(0.1)
                    continue
                img = np.full((480, 640, 3), (150, 155, 160), np.uint8)          # pared gris
                if bgr is not None:
                    cv2.rectangle(img, (150, 90), (500, 400), bgr, cv2.FILLED)   # la hoja llena la zona
                img = np.clip(img + rng.normal(0, 6, img.shape), 0, 255).astype(np.uint8)
                jpg = cv2.imencode(".jpg", img, [cv2.IMWRITE_JPEG_QUALITY, 80])[1].tobytes()
                self.wfile.write(b"--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n" % len(jpg)
                                 + jpg + b"\r\n")
                time.sleep(1 / 15)
        except Exception:
            pass


async def robot(ws):
    await ws.send(json.dumps({"tipo": "saludo", "mensaje": "Servidor ESP32-S3 listo", "puerto": 81}))
    try:
        async for texto in ws:
            t, tramo = escena()
            estado["registro"].append((t, tramo[2], json.loads(texto)))
    except websockets.ConnectionClosed:
        pass                     # la prueba cierra el programa sin despedirse: es lo esperado


def servir_robot():
    async def main():
        async with websockets.serve(robot, "127.0.0.1", PUERTO_ROBOT):
            await asyncio.Future()
    asyncio.run(main())


def correr(programa, guion, segundos, extra):
    estado.update(t0=time.time(), guion=guion, registro=[])
    proceso = subprocess.Popen([programa, "--fuente", f"http://127.0.0.1:{PUERTO_VIDEO}/stream.mjpg",
                                "--robot", f"ws://127.0.0.1:{PUERTO_ROBOT}", "--auto"] + extra,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(segundos)
    proceso.terminate()          # se cierra SOLO el proceso que lanzó esta prueba
    proceso.wait(timeout=10)
    return [(t, hoja, m) for t, hoja, m in estado["registro"] if m.get("tipo") == "mover"]


fallas = 0


def comprobar(condicion, que):
    global fallas
    print(("  ok     " if condicion else "  FALLA  ") + que)
    fallas += 0 if condicion else 1


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    programa = sys.argv[1]
    threading.Thread(target=ThreadingHTTPServer(("127.0.0.1", PUERTO_VIDEO), Video).serve_forever, daemon=True).start()
    threading.Thread(target=servir_robot, daemon=True).start()
    time.sleep(1.0)

    print("Corrida 1: las cuatro hojas, quitar la hoja y corte de video (41 s)")
    movs = correr(programa, GUION_1, 41, [])
    comprobar(len(movs) > 50, f"llegaron {len(movs)} mensajes «mover»")
    for desde, hasta, hoja, _bgr, orden in GUION_1:
        # Se deja 1.2 s para el antirrebote (3 ciclos) y el retraso del video.
        tramo = [m for t, _h, m in movs if desde + 1.2 <= t < hasta]
        vistos = sorted({m["accion"] for m in tramo})
        if orden == "parar":
            comprobar(vistos in ([], ["parar"]), f"{desde:2d}-{hasta:2d} s  {hoja:9s} -> parar (se recibió {vistos or 'nada: ya estaba parado'})")
        else:
            comprobar(vistos == [orden] and len(tramo) >= 15,
                      f"{desde:2d}-{hasta:2d} s  {hoja:9s} -> {orden} ({len(tramo)} mensajes; se recibió {vistos or 'nada'})")
    mueven = [m for _t, _h, m in movs if m["accion"] != "parar"]
    comprobar(all(m.get("vel") == 30 and m.get("ms") in (120, 300) for m in mueven), "los que mueven llevan ms (300 o 120) y vel 30")
    comprobar(all("ms" not in m for _t, _h, m in movs if m["accion"] == "parar"), "«parar» no lleva ms")
    seguidos = sum(1 for (a, b) in zip(movs, movs[1:]) if a[2]["accion"] == "parar" and b[2]["accion"] == "parar")
    comprobar(seguidos == 0, "«parar» nunca se repite dos veces seguidas")
    avance = [t for t, h, m in movs if 8.5 <= t < 10.5]
    comprobar(14 <= len(avance) <= 18, f"ritmo: {len(avance) / 2:.1f} mensajes por segundo (se esperan 8)")
    primero = next((t for t, _h, m in movs if t >= 27 and m["accion"] == "parar"), None)
    comprobar(primero is not None and primero - 27 < 1.0, f"al quitar la hoja, «parar» a los {0 if primero is None else primero - 27:.2f} s")
    corte = next((t for t, _h, m in movs if t >= 35 and m["accion"] == "parar"), None)
    comprobar(corte is not None and corte - 35 < 2.0, f"al cortarse el video, «parar» a los {0 if corte is None else corte - 35:.2f} s")

    print("Corrida 2: una hoja que el punto de partida no reconoce, SIN Aprender (9 s)")
    movs = correr(programa, GUION_2, 9, [])
    comprobar(all(m["accion"] == "parar" for _t, _h, m in movs), "verde azulado sin aprender: el robot no se mueve")
    print("Corrida 3: la misma hoja, con el botón Aprender «verde» (10 s)")
    movs = correr(programa, GUION_2, 10, ["--aprender", "verde"])
    tramo = [m for t, _h, m in movs if t >= 5]
    comprobar(len(tramo) >= 15 and all(m["accion"] == "avanzar" for m in tramo), f"verde azulado aprendido: avanzar ({len(tramo)} mensajes)")

    print("RESULTADO:", "TODO PASA" if fallas == 0 else f"{fallas} FALLAS")
    sys.exit(0 if fallas == 0 else 1)


if __name__ == "__main__":
    main()
