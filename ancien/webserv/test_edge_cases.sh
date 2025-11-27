#!/bin/bash

# ============================================================================
# Tests limites et edge cases avec Valgrind
# ============================================================================

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

cd /home/rivoinfo/Documents/DEV/servwebs/webserv

echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}TESTS LIMITES - EDGE CASES (VALGRIND)${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"

# Fonction pour un test
run_edge_case_test() {
    local test_name=$1
    local test_duration=$2
    local logfile="valgrind_edge_$(echo $test_name | tr ' ' '_').log"
    
    echo ""
    echo -e "${BLUE}── TEST: $test_name ──${NC}"
    
    timeout $((test_duration + 5))s valgrind --leak-check=full --show-leak-kinds=all \
        --log-file="$logfile" ./webserv ./conf.d/webserv.conf &
    VAL_PID=$!
    sleep 3
    
    eval "$3"  # Execute le code test
    
    sleep 1
    kill $VAL_PID 2>/dev/null || true
    wait $VAL_PID 2>/dev/null || true
    sleep 2
    
    # Affiche les résultats
    if [ -f "$logfile" ]; then
        if grep -q "SUMMARY: 0 errors" "$logfile"; then
            echo -e "${GREEN}✓ CLEAN${NC}"
        else
            grep "SUMMARY" "$logfile" | head -1
        fi
        grep "definitely lost\|indirectly lost\|possibly lost" "$logfile" | head -2
    fi
}

# ========================================================================
# TEST 1: Connexions qui se ferment immédiatement
# ========================================================================
run_edge_case_test "Connexions fermées brutalement" 20 \
'
for i in {1..20}; do
    (sleep 0.1; echo -n "GET / HTTP/1.1"; sleep 0.1) | nc -q 1 localhost 8080 2>/dev/null &
done
wait
'

# ========================================================================
# TEST 2: Requêtes mal formées
# ========================================================================
run_edge_case_test "Requêtes mal formées" 20 \
'
# Requête sans HTTP version
(echo "GET /"; sleep 0.5) | nc -q 1 localhost 8080 2>/dev/null &

# Requête avec headers vides
(echo "GET / HTTP/1.1"; echo ""; sleep 0.5) | nc -q 1 localhost 8080 2>/dev/null &

# Requête sans method
(echo "/ HTTP/1.1"; sleep 0.5) | nc -q 1 localhost 8080 2>/dev/null &

# Requête avec headers énormes
(printf "GET / HTTP/1.1\r\nX-Test: "; python3 -c "print(\"Y\" * 50000)"; sleep 0.5) | \
    nc -q 1 localhost 8080 2>/dev/null &

wait
'

# ========================================================================
# TEST 3: Pipelining agressif
# ========================================================================
run_edge_case_test "HTTP Pipelining agressif (100 requêtes)" 25 \
'
{
    for i in {1..100}; do
        echo "GET /?test=$i HTTP/1.1"
        echo "Host: localhost:8080"
        echo "Connection: $([ $i -eq 100 ] && echo close || echo keep-alive)"
        echo ""
    done
} | nc -q 1 localhost 8080 > /dev/null 2>&1 &
wait
'

# ========================================================================
# TEST 4: CGI avec erreurs
# ========================================================================
run_edge_case_test "CGI avec chemins inexistants" 20 \
'
for i in {1..15}; do
    curl -s "http://localhost:8080/cgi_bin/nonexistent_$i.php?id=$i" > /dev/null 2>&1 &
done
wait
'

# ========================================================================
# TEST 5: POST avec chunk encoding (HTTP/1.1)
# ========================================================================
run_edge_case_test "POST avec Transfer-Encoding chunked" 20 \
'
(
    printf "POST / HTTP/1.1\r\nHost: localhost:8080\r\nTransfer-Encoding: chunked\r\n\r\n"
    printf "5\r\nhello\r\n"
    printf "6\r\n world\r\n"
    printf "0\r\n\r\n"
    sleep 0.5
) | nc -q 1 localhost 8080 > /dev/null 2>&1 &
wait
'

# ========================================================================
# TEST 6: CGI POST avec données vides
# ========================================================================
run_edge_case_test "CGI POST avec bodies vides" 20 \
'
for i in {1..20}; do
    curl -s -X POST -d "" http://localhost:8080/cgi_bin/Contact.php > /dev/null 2>&1 &
done
wait
'

# ========================================================================
# TEST 7: CGI avec query string très long
# ========================================================================
run_edge_case_test "CGI avec query string très long (10KB)" 20 \
'
LONG_QUERY=$(python3 -c "print(\"param=\" + \"x\" * 10000)")
for i in {1..10}; do
    curl -s "http://localhost:8080/cgi_bin/test.php?$LONG_QUERY" > /dev/null 2>&1 &
done
wait
'

# ========================================================================
# TEST 8: Requêtes rapides successives sur même connexion
# ========================================================================
run_edge_case_test "Keep-alive intensif (1000 requêtes)" 30 \
'
{
    for i in {1..1000}; do
        printf "GET /?test=$i HTTP/1.1\r\nHost: localhost:8080\r\nConnection: $([ $i -eq 1000 ] && echo close || echo keep-alive)\r\n\r\n"
    done
} | nc -q 1 localhost 8080 > /dev/null 2>&1 &
wait
'

# ========================================================================
# TEST 9: CGI simultanées avec files énormes
# ========================================================================
run_edge_case_test "CGI POST simultanées (100KB x5)" 25 \
'
for i in {1..5}; do
    HUGE=$(python3 -c "print(\"d\" * 100000)")
    curl -s -X POST -d "data=$HUGE" \
        http://localhost:8080/cgi_bin/Contact.php > /dev/null 2>&1 &
done
wait
'

# ========================================================================
# TEST 10: Reconnexions rapides du même client
# ========================================================================
run_edge_case_test "Reconnexions rapides (50 fois)" 20 \
'
for i in {1..50}; do
    curl -s http://localhost:8080/ > /dev/null 2>&1 &
done
wait
'

# ========================================================================
# RÉSUMÉ FINAL
# ========================================================================
echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}RÉSUMÉ FINAL - EDGE CASES${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"

TOTAL_TESTS=$(ls valgrind_edge_*.log 2>/dev/null | wc -l)
CLEAN_TESTS=$(grep -l "SUMMARY: 0 errors" valgrind_edge_*.log 2>/dev/null | wc -l)
LEAKED=$(grep -h "definitely lost:" valgrind_edge_*.log 2>/dev/null | \
    awk '{sum += $(NF-3)} END {print sum}' || echo 0)

echo -e "${YELLOW}Total tests: $TOTAL_TESTS${NC}"
echo -e "${GREEN}Tests CLEAN: $CLEAN_TESTS${NC}"
if [ "$TOTAL_TESTS" -eq "$CLEAN_TESTS" ]; then
    echo -e "${GREEN}✓ TOUS LES EDGE CASES PASSENT SANS FUITES!${NC}"
else
    echo -e "${RED}✗ Certains tests ont des problèmes${NC}"
fi

echo ""
echo -e "${YELLOW}Total mémoire leak détectée: $LEAKED bytes${NC}"
echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
