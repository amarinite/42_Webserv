#!/bin/bash

# 1. Cabecera obligatoria para que el navegador sepa que es una página web
echo "Content-type: text/html"
echo ""

# 2. Contenido HTML de la página
echo "<!DOCTYPE html>"
echo "<html lang='es'>"
echo "<head>"
echo "    <meta charset='UTF-8'>"
echo "    <title>Mi Página CGI</title>"
echo "</head>"
echo "<body>"
echo "    <h1>¡Hola Mundo desde CGI!</h1>"
echo "    <p>Esta es una página web normal generada por un script en Bash.</p>"
echo "    <p>Fecha y hora del servidor: $(date)</p>"
echo "</body>"
echo "</html>"
