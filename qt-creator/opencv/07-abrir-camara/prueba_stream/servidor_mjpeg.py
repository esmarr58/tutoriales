#!/usr/bin/env python3
"""Servidor de prueba: publica un video MJPEG por HTTP, como una cámara IP o una ESP32-CAM.

Uso (Ubuntu, necesita ffmpeg: sudo apt install ffmpeg):
    python3 servidor_mjpeg.py            # patrón de prueba en http://0.0.0.0:8080/video
    python3 servidor_mjpeg.py 8080 /dev/video0   # o la webcam del equipo

En la aplicación escribe la fuente:
    escritorio (mismo equipo):  http://localhost:8080/video
    emulador de Android:        http://10.0.2.2:8080/video   (10.0.2.2 = el equipo anfitrión)
    teléfono en la misma red:   http://IP-DEL-EQUIPO:8080/video
"""
import subprocess
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

PUERTO = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
ENTRADA = sys.argv[2] if len(sys.argv) > 2 else None


def ffmpeg_cmd():
    if ENTRADA:
        origen = ['-f', 'v4l2', '-i', ENTRADA]
    else:  # patrón de colores en movimiento con un reloj: se nota si llegan cuadros nuevos
        origen = ['-re', '-f', 'lavfi', '-i', 'testsrc2=size=640x480:rate=15']
    return ['ffmpeg', '-loglevel', 'error', *origen,
            '-vf', 'scale=640:480', '-q:v', '5', '-f', 'mjpeg', 'pipe:1']


class Manejador(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path != '/video':
            self.send_error(404, 'Usa /video')
            return
        self.send_response(200)
        self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=cuadro')
        self.send_header('Cache-Control', 'no-cache')
        self.end_headers()
        proceso = subprocess.Popen(ffmpeg_cmd(), stdout=subprocess.PIPE)
        datos = b''
        try:
            while True:
                bloque = proceso.stdout.read(65536)
                if not bloque:
                    break
                datos += bloque
                # Cada JPEG termina con FF D9: lo enviamos como una parte del multipart.
                while (fin := datos.find(b'\xff\xd9')) != -1:
                    jpeg, datos = datos[:fin + 2], datos[fin + 2:]
                    self.wfile.write(b'--cuadro\r\nContent-Type: image/jpeg\r\n'
                                     b'Content-Length: %d\r\n\r\n' % len(jpeg))
                    self.wfile.write(jpeg + b'\r\n')
        except (BrokenPipeError, ConnectionResetError):
            pass  # el cliente se fue (pulsó Apagar)
        finally:
            proceso.kill()

    def log_message(self, formato, *args):
        print(self.address_string(), formato % args)


print(f'Stream en http://0.0.0.0:{PUERTO}/video  (Ctrl+C para terminar)')
ThreadingHTTPServer(('0.0.0.0', PUERTO), Manejador).serve_forever()
