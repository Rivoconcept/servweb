#!/bin/bash

# Test rapide des fuites mémoire avec valgrind
# Focus sur CGI

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

cd /home/rivoinfo/Documents/DEV/servwebs/webserv

echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}TESTS DE FUITES MÉMOIRE - WEBSERV (FOCUS CGI)${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"

# Vérifier compilation
if [ ! -f ./webserv ]; then
    echo -e "${YELLOW}[*] Compilation...${NC}"
    make clean && make > /dev/null 2>&1
fi

echo ""
echo -e "${BLUE}── TEST 1: Requêtes simples avec Valgrind (30s) ──${NC}"
timeout 35s valgrind --leak-check=full --show-leak-kinds=all \
    --log-file=valgrind_simple.log ./webserv ./conf.d/webserv.conf &
VAL_PID=$!
sleep 3

# 10 requêtes GET simples
for i in {1..10}; do
    curl -s http://localhost:8080/ > /dev/null 2>&1 &
done
wait

sleep 1
kill $VAL_PID 2>/dev/null || true
wait $VAL_PID 2>/dev/null || true
sleep 2

echo -e "${GREEN}✓ Test 1 terminé${NC}"
echo ""

echo -e "${BLUE}── TEST 2: CGI GET/POST avec Valgrind (30s) ──${NC}"
timeout 35s valgrind --leak-check=full --show-leak-kinds=all \
    --log-file=valgrind_cgi.log ./webserv ./conf.d/webserv.conf &
VAL_PID=$!
sleep 3

# 10 requêtes CGI
for i in {1..10}; do
    curl -s "http://localhost:8080/cgi_bin/test.php?id=$i" > /dev/null 2>&1 &
done
wait

for i in {1..10}; do
    curl -s -X POST -d "name=test$i&email=test@example.com" \
        http://localhost:8080/cgi_bin/Contact.php > /dev/null 2>&1 &
done
wait

sleep 1
kill $VAL_PID 2>/dev/null || true
wait $VAL_PID 2>/dev/null || true
sleep 2

echo -e "${GREEN}✓ Test 2 terminé${NC}"
echo ""

echo -e "${BLUE}── TEST 3: POST volumineux + CGI avec Valgrind (30s) ──${NC}"
timeout 35s valgrind --leak-check=full --show-leak-kinds=all \
    --log-file=valgrind_large.log ./webserv ./conf.d/webserv.conf &
VAL_PID=$!
sleep 3

# 5 POST volumineux
for i in {1..5}; do
    LARGE=$(python3 -c "print('x' * 50000)")
    curl -s -X POST -d "data=$LARGE" http://localhost:8080/ > /dev/null 2>&1 &
done
wait

# 5 CGI POST volumineux
for i in {1..5}; do
    LARGE=$(python3 -c "print('y' * 50000)")
    curl -s -X POST -d "data=$LARGE" \
        http://localhost:8080/cgi_bin/Contact.php > /dev/null 2>&1 &
done
wait

sleep 1
kill $VAL_PID 2>/dev/null || true
wait $VAL_PID 2>/dev/null || true
sleep 2

echo -e "${GREEN}✓ Test 3 terminé${NC}"
echo ""

# ═══════════════════════════════════════════════════════════
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}RÉSULTATS VALGRIND${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"

for logfile in valgrind_simple.log valgrind_cgi.log valgrind_large.log; do
    if [ -f "$logfile" ]; then
        echo ""
        echo -e "${YELLOW}── $logfile ──${NC}"
        if grep -q "SUMMARY: 0 errors" "$logfile"; then
            echo -e "${GREEN}✓ Aucune erreur détectée${NC}"
        else
            grep "SUMMARY" "$logfile" || echo "No summary found"
        fi
        
        echo -e "${YELLOW}Fuites mémoire:${NC}"
        grep "definitely lost\|indirectly lost\|possibly lost" "$logfile" | head -5
        
        echo -e "${YELLOW}Allocations:${NC}"
        grep "allocs, " "$logfile" | head -3
    fi
done

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Tests terminés!${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
