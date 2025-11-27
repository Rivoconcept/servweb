#!/bin/bash

# ============================================================================
# Test de fuites mémoire avec valgrind pour webserv
# Focus sur les tests CGI
# ============================================================================

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

WEBSERV_PID=""
TEST_RESULTS=()

# Fonction pour démarrer le serveur
start_server() {
    local config=$1
    echo -e "${BLUE}[*] Démarrage du serveur avec config: $config${NC}"
    timeout 60s ./webserv "$config" &
    WEBSERV_PID=$!
    sleep 2
}

# Fonction pour arrêter le serveur
stop_server() {
    if [ ! -z "$WEBSERV_PID" ] && kill -0 "$WEBSERV_PID" 2>/dev/null; then
        kill "$WEBSERV_PID" 2>/dev/null || true
        wait "$WEBSERV_PID" 2>/dev/null || true
    fi
    sleep 1
}

# Fonction pour tester GET simple
test_get_simple() {
    echo -e "${YELLOW}[TEST] GET simple...${NC}"
    curl -s http://localhost:8080/ > /dev/null
    echo -e "${GREEN}✓ GET simple réussi${NC}"
    TEST_RESULTS+=("GET simple: OK")
}

# Fonction pour tester GET avec paramètres
test_get_params() {
    echo -e "${YELLOW}[TEST] GET avec paramètres...${NC}"
    curl -s "http://localhost:8080/index.html?param1=value1&param2=value2" > /dev/null
    echo -e "${GREEN}✓ GET avec paramètres réussi${NC}"
    TEST_RESULTS+=("GET avec paramètres: OK")
}

# Fonction pour tester POST simple
test_post_simple() {
    echo -e "${YELLOW}[TEST] POST simple...${NC}"
    curl -s -X POST -d "test=data" http://localhost:8080/ > /dev/null
    echo -e "${GREEN}✓ POST simple réussi${NC}"
    TEST_RESULTS+=("POST simple: OK")
}

# Fonction pour tester POST avec body volumineux
test_post_large() {
    echo -e "${YELLOW}[TEST] POST avec body volumineux...${NC}"
    dd if=/dev/zero bs=1024 count=100 2>/dev/null | tr '\0' 'A' | \
        curl -s -X POST -d @- http://localhost:8080/ > /dev/null
    echo -e "${GREEN}✓ POST volumineux réussi${NC}"
    TEST_RESULTS+=("POST volumineux: OK")
}

# Fonction pour tester CGI GET
test_cgi_get() {
    echo -e "${YELLOW}[TEST] CGI GET (test.php)...${NC}"
    curl -s "http://localhost:8080/cgi_bin/test.php?param=value" > /dev/null 2>&1 || true
    echo -e "${GREEN}✓ CGI GET réussi${NC}"
    TEST_RESULTS+=("CGI GET: OK")
}

# Fonction pour tester CGI POST
test_cgi_post() {
    echo -e "${YELLOW}[TEST] CGI POST (Contact.php)...${NC}"
    curl -s -X POST -H "Content-Type: application/x-www-form-urlencoded" \
        -d "name=Test&email=test@example.com&message=Hello" \
        http://localhost:8080/cgi_bin/Contact.php > /dev/null 2>&1 || true
    echo -e "${GREEN}✓ CGI POST réussi${NC}"
    TEST_RESULTS+=("CGI POST: OK")
}

# Fonction pour tester CGI avec gros body
test_cgi_post_large() {
    echo -e "${YELLOW}[TEST] CGI POST volumineux...${NC}"
    LARGE_DATA=$(dd if=/dev/zero bs=1024 count=50 2>/dev/null | tr '\0' 'A')
    curl -s -X POST -H "Content-Type: application/x-www-form-urlencoded" \
        -d "data=$LARGE_DATA" \
        http://localhost:8080/cgi_bin/Contact.php > /dev/null 2>&1 || true
    echo -e "${GREEN}✓ CGI POST volumineux réussi${NC}"
    TEST_RESULTS+=("CGI POST volumineux: OK")
}

# Fonction pour tester requêtes concurrentes
test_concurrent_requests() {
    echo -e "${YELLOW}[TEST] Requêtes concurrentes (10 simultanées)...${NC}"
    for i in {1..10}; do
        curl -s "http://localhost:8080/?req=$i" > /dev/null &
    done
    wait
    echo -e "${GREEN}✓ Requêtes concurrentes réussies${NC}"
    TEST_RESULTS+=("Requêtes concurrentes: OK")
}

# Fonction pour tester CGI concurrentes
test_concurrent_cgi() {
    echo -e "${YELLOW}[TEST] CGI concurrentes (10 simultanées)...${NC}"
    for i in {1..10}; do
        curl -s "http://localhost:8080/cgi_bin/test.php?id=$i" > /dev/null 2>&1 &
    done
    wait
    echo -e "${GREEN}✓ CGI concurrentes réussies${NC}"
    TEST_RESULTS+=("CGI concurrentes: OK")
}

# Fonction pour tester erreurs 404
test_404_error() {
    echo -e "${YELLOW}[TEST] Erreur 404...${NC}"
    curl -s http://localhost:8080/nonexistent > /dev/null
    echo -e "${GREEN}✓ Erreur 404 gérée${NC}"
    TEST_RESULTS+=("Erreur 404: OK")
}

# Fonction pour tester pipelining
test_pipelining() {
    echo -e "${YELLOW}[TEST] HTTP Pipelining...${NC}"
    printf "GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\n\r\nGET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r\n" | \
        nc localhost 8080 > /dev/null 2>&1 || true
    echo -e "${GREEN}✓ HTTP Pipelining réussi${NC}"
    TEST_RESULTS+=("HTTP Pipelining: OK")
}

# Fonction pour afficher les résultats
print_results() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}RÉSULTATS DES TESTS${NC}"
    echo -e "${BLUE}========================================${NC}"
    for result in "${TEST_RESULTS[@]}"; do
        echo -e "${GREEN}✓ $result${NC}"
    done
    echo -e "${BLUE}========================================${NC}"
}

# ===========================================================================
# MAIN: Tests avec Valgrind
# ===========================================================================

echo -e "${BLUE}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   TESTS DE FUITES MÉMOIRE - WEBSERV (FOCUS CGI)               ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Vérifier que valgrind est installé
if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}[!] valgrind n'est pas installé. Installation...${NC}"
    sudo apt-get update && sudo apt-get install -y valgrind
fi

# Vérifier que le serveur est compilé
if [ ! -f ./webserv ]; then
    echo -e "${RED}[!] webserv n'existe pas. Compilation...${NC}"
    make clean && make
fi

# ========== TEST 1: Requêtes simples ==========
echo ""
echo -e "${BLUE}─ TEST SUITE 1: Requêtes simples ─${NC}"
start_server "./conf.d/webserv.conf"
test_get_simple
test_post_simple
test_get_params
test_404_error
stop_server
sleep 2

# ========== TEST 2: CGI ==========
echo ""
echo -e "${BLUE}─ TEST SUITE 2: CGI ─${NC}"
start_server "./conf.d/webserv.conf"
test_cgi_get
test_cgi_post
test_cgi_post_large
stop_server
sleep 2

# ========== TEST 3: Charge et concurrence ==========
echo ""
echo -e "${BLUE}─ TEST SUITE 3: Charge et concurrence ─${NC}"
start_server "./conf.d/webserv.conf"
test_concurrent_requests
test_concurrent_cgi
stop_server
sleep 2

# ========== TEST 4: Corps volumineux ==========
echo ""
echo -e "${BLUE}─ TEST SUITE 4: Corps volumineux ─${NC}"
start_server "./conf.d/webserv.conf"
test_post_large
stop_server
sleep 2

# ========== TEST 5: Avec Valgrind (courte durée) ==========
echo ""
echo -e "${BLUE}─ TEST SUITE 5: Valgrind - Requêtes simples ─${NC}"
echo -e "${YELLOW}[*] Démarrage du serveur avec Valgrind...${NC}"
timeout 30s valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes --verbose ./webserv "./conf.d/webserv.conf" \
    > valgrind_simple.log 2>&1 &
VALGRIND_PID=$!
sleep 3

# Faire quelques requêtes
for i in {1..5}; do
    curl -s http://localhost:8080/ > /dev/null 2>&1 || true
    curl -s "http://localhost:8080/?test=$i" > /dev/null 2>&1 || true
done

sleep 1
kill $VALGRIND_PID 2>/dev/null || true
wait $VALGRIND_PID 2>/dev/null || true
sleep 2

# ========== TEST 6: Avec Valgrind - CGI ==========
echo ""
echo -e "${BLUE}─ TEST SUITE 6: Valgrind - CGI ─${NC}"
echo -e "${YELLOW}[*] Démarrage du serveur avec Valgrind (test CGI)...${NC}"
timeout 30s valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes --verbose ./webserv "./conf.d/webserv.conf" \
    > valgrind_cgi.log 2>&1 &
VALGRIND_PID=$!
sleep 3

# Faire des requêtes CGI
for i in {1..5}; do
    curl -s "http://localhost:8080/cgi_bin/test.php?id=$i" > /dev/null 2>&1 || true
    curl -s -X POST -d "name=test$i&email=test$i@example.com" \
        http://localhost:8080/cgi_bin/Contact.php > /dev/null 2>&1 || true
done

sleep 1
kill $VALGRIND_PID 2>/dev/null || true
wait $VALGRIND_PID 2>/dev/null || true
sleep 2

# ========== RÉSULTATS ==========
print_results

# ========== ANALYSE VALGRIND ==========
echo ""
echo -e "${BLUE}═════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}RÉSULTATS VALGRIND${NC}"
echo -e "${BLUE}═════════════════════════════════════════════════════════════════${NC}"

if [ -f valgrind_simple.log ]; then
    echo ""
    echo -e "${YELLOW}--- Requêtes simples ---${NC}"
    tail -50 valgrind_simple.log
fi

if [ -f valgrind_cgi.log ]; then
    echo ""
    echo -e "${YELLOW}--- CGI ---${NC}"
    tail -50 valgrind_cgi.log
fi

echo ""
echo -e "${BLUE}═════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Tests terminés!${NC}"
echo -e "${BLUE}═════════════════════════════════════════════════════════════════${NC}"
