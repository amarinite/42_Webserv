#!/bin/bash

# Script de regresión HTTP para test.conf (puertos 8080 y 9090)
# Requiere: curl
# Uso: ./test_http.sh   (con el servidor ya corriendo en otra terminal)

HOST="127.0.0.1"
PORT=8080
PORT2=9090

PASS=0
FAIL=0

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# check_status <descripcion> <codigo_esperado> <curl_args...>
check_status() {
    local desc="$1"
    local expected="$2"
    shift 2

    actual=$(curl -s -o /tmp/webserv_test_body -w "%{http_code}" "$@")

    if [ "$actual" == "$expected" ]; then
        echo -e "${GREEN}[PASS]${NC} $desc (esperado $expected, obtenido $actual)"
        PASS=$((PASS+1))
    else
        echo -e "${RED}[FAIL]${NC} $desc (esperado $expected, obtenido $actual)"
        FAIL=$((FAIL+1))
    fi
}

# check_header <descripcion> <header_esperado_contiene> <curl_args...>
check_header_contains() {
    local desc="$1"
    local expected_substr="$2"
    shift 2

    headers=$(curl -s -D - -o /dev/null "$@")

    if echo "$headers" | grep -qi "$expected_substr"; then
        echo -e "${GREEN}[PASS]${NC} $desc (contiene '$expected_substr')"
        PASS=$((PASS+1))
    else
        echo -e "${RED}[FAIL]${NC} $desc (NO contiene '$expected_substr')"
        echo "$headers" | head -5
        FAIL=$((FAIL+1))
    fi
}

echo "=== Server 8080 ==="

echo -e "\n--- /home/ (GET POST DELETE, autoindex on, index index.html) ---"
echo -e "${YELLOW}[NOTA] requiere ./www/html/index.html${NC}"
check_status "GET /home/ -> sirve index.html" 200 "http://$HOST:$PORT/home/"
check_status "PUT /home/ -> metodo no configurado -> 405" 405 -X PUT "http://$HOST:$PORT/home/"

echo -e "\n--- /fih (GET only, index fih.html) ---"
echo -e "${YELLOW}[NOTA] requiere ./www/fih.html o ./www/fih (segun tu resolucion de root)${NC}"
check_status "GET /fih -> 200" 200 "http://$HOST:$PORT/fih"
check_status "POST /fih -> metodo no permitido -> 405" 405 -X POST "http://$HOST:$PORT/fih"

echo -e "\n--- /upload (POST only) ---"
check_status "POST /upload con body -> 200/201" 201 -X POST -d "contenido de prueba" "http://$HOST:$PORT/upload"
check_status "GET /upload -> metodo no permitido -> 405" 405 "http://$HOST:$PORT/upload"

echo -e "\n--- /old (redirect a /new) ---"
check_status "GET /old -> redirect (301 o 302)" 301 "http://$HOST:$PORT/old"
check_header_contains "GET /old -> header Location: /new" "location: /new" "http://$HOST:$PORT/old"

echo -e "\n--- /cgi-bin (.py CGI) ---"
echo -e "${YELLOW}[NOTA] requiere un script .py real en ./www/cgi-bin/ (segun tu root)${NC}"
check_status "GET /cgi-bin/script.py -> 200" 200 "http://$HOST:$PORT/cgi-bin/script.py"

echo -e "\n--- /post_body (POST only, max body 100) ---"
check_status "POST /post_body con body corto (<100) -> 201" 201 -X POST -d "$(head -c 50 < /dev/zero | tr '\0' 'a')" "http://$HOST:$PORT/post_body"
check_status "POST /post_body con body largo (>100) -> 413" 413 -X POST -d "$(head -c 200 < /dev/zero | tr '\0' 'a')" "http://$HOST:$PORT/post_body"

echo -e "\n--- / (autoindex off, sin allowed_methods explicito) ---"
echo -e "${YELLOW}[NOTA] comportamiento depende de como tratas 'sin allowed_methods' en Processor${NC}"
actual_root=$(curl -s -o /dev/null -w "%{http_code}" "http://$HOST:$PORT/")
echo -e "${YELLOW}[INFO]${NC} GET / -> código real: $actual_root (confirma tú si es el esperado)"

echo -e "\n--- Casos genericos de protocolo ---"
check_status "GET a ruta inexistente -> 404" 404 "http://$HOST:$PORT/esto-no-existe-seguro"
check_status "Metodo invalido (PATCH) -> 405" 405 -X PATCH "http://$HOST:$PORT/home/"

echo -e "\n--- Request malformada (sin Host, via printf/nc) ---"
if command -v nc >/dev/null 2>&1; then
    response=$(printf 'GET / HTTP/1.1\r\n\r\n' | nc -w 2 "$HOST" "$PORT" | head -1)
    if echo "$response" | grep -q "400"; then
        echo -e "${GREEN}[PASS]${NC} Request sin Host -> 400 ($response)"
        PASS=$((PASS+1))
    else
        echo -e "${RED}[FAIL]${NC} Request sin Host -> esperado 400, obtenido: $response"
        FAIL=$((FAIL+1))
    fi
else
    echo -e "${YELLOW}[SKIP]${NC} nc no disponible, se omite test de request malformada"
fi

echo -e "\n=== Server 9090 (segundo server, root ./www/admin) ==="
echo -e "${YELLOW}[NOTA] requiere contenido en ./www/admin${NC}"
check_status "GET / en puerto 9090 -> 200" 200 "http://$HOST:$PORT2/"
check_status "POST / en puerto 9090 -> metodo no permitido -> 405" 405 -X POST "http://$HOST:$PORT2/"

echo -e "\n=== Multiples servidores no se mezclan ==="
body_8080=$(curl -s "http://$HOST:$PORT/" )
body_9090=$(curl -s "http://$HOST:$PORT2/")
if [ "$body_8080" != "$body_9090" ]; then
    echo -e "${GREEN}[PASS]${NC} Los dos servers devuelven contenido distinto"
    PASS=$((PASS+1))
else
    echo -e "${RED}[FAIL]${NC} Los dos servers devuelven el MISMO contenido (¿comparten config?)"
    FAIL=$((FAIL+1))
fi

echo -e "\n===================="
echo -e "RESULTADO: ${GREEN}$PASS pass${NC} / ${RED}$FAIL fail${NC}"
echo "===================="

rm -f /tmp/webserv_test_body
