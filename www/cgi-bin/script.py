#!/usr/bin/env python3
import sys
import os

# 1. Encabezados HTTP obligatorios (debe ir antes de cualquier contenido)
print("Content-Type: text/html; charset=utf-8")
print()  # Línea en blanco obligatoria para separar headers del body

print("<!DOCTYPE html>")
print("<html>")
print("<head><title>CGI Test Success</title></head>")
print("<head><meta charset='UTF-8'></head>")
print("<body style='font-family: sans-serif; padding: 2rem;'>")
print("  <h1 style='color: #2e7d32;'>¡Éxito! El CGI funciona correctamente 🚀</h1>")
print(f"  <p><b>Versión de Python en el servidor:</b> {sys.version}</p>")
print(f"  <p><b>Método de petición:</b> {os.environ.get('REQUEST_METHOD', 'N/A')}</p>")
print(f"  <p><b>Query String:</b> {os.environ.get('QUERY_STRING', 'Ninguno')}</p>")
print("</body>")
print("</html>")
