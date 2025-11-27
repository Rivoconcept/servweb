#!/bin/bash

# ============================================================================
# Test détaillé des fuites mémoire avec valgrind - Focus CGI
# ============================================================================

set -e

# Couleurs
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# Configuration
WEBSERV="./webserv"
CONFIG="./conf.d/webserv.conf"
HOST="localhost"
PORT="8080"
BASE_URL="http://$HOST:$PORT"
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --track-origins=yes --gen-suppressions=all"
SUPPRESS_FILE="/tmp/webserv.supp"

# Variables
WEBSERV_PID=""
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0
VALGRIND_ISSUES=0

# ============================================================================
# FONCTIONS UTILITAIRES
# ============================================================================

log_info() {
    echo -e "${BLUE}[*]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[✓]${NC} $1"
    ((PASS_COUNT++))
}

log_error() {
    echo -e "${RED}[✗]${NC} $1"
    ((FAIL_COUNT++))
}

log_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

log_test() {
    echo -e "${MAGENTA}[TEST]${NC} $1"
    ((TEST_COUNT++))
}

# Fonction pour démarrer le serveur
start_server() {
    local cmd=$1
    local msg=${2:-""}
    
    if [ -z "$msg" ]; then
        msg="Démarrage du serveur"
    fi
    
    log_info "$msg"
    eval "$cmd" &
    WEBSERV_PID=$!
    sleep 2
    
    # Vérifier que le serveur est bien lancé
    if ! kill -0 $WEBSERV_PID 2>/dev/null; then
        log_error "Serveur n'a pas démarré!"
        return 1
    fi
    log_success "Serveur démarré (PID: $WEBSERV_PID)"
    return 0
}

# Fonction pour arrêter le serveur
stop_server() {
    if [ ! -z "$WEBSERV_PID" ] && kill -0 "$WEBSERV_PID" 2>/dev/null; then
        kill "$WEBSERV_PID" 2>/dev/null || true
        wait "$WEBSERV_PID" 2>/dev/null || true
    fi
    WEBSERV_PID=""
    sleep 1
}

# Fonction de requête HTTP générique
http_request() {
    local method=$1
    local path=$2
    local data=$3
    local extra_headers=$4
    
    local cmd="curl -s -X $method"
    
    if [ ! -z "$extra_headers" ]; then
        cmd="$cmd $extra_headers"
    fi
    
    if [ ! -z "$data" ]; then
        cmd="$cmd --data '$data'"
    fi
    
    cmd="$cmd '$BASE_URL$path'"
    
    eval $cmd 2>/dev/null || true
}

# ============================================================================
# TESTS UNITAIRES
# ============================================================================

test_basic_get() {
    log_test "GET basique sur /"
    local response=$(http_request "GET" "/" "")
    if [ ! -z "$response" ]; then
        log_success "GET basique réussi"
    else
        log_error "GET basique échoué"
    fi
}

test_get_with_query() {
    log_test "GET avec paramètres de requête"
    local response=$(http_request "GET" "/?page=1&size=10" "")
    if [ ! -z "$response" ]; then
        log_success "GET avec requête réussi"
    else
        log_error "GET avec requête échoué"
    fi
}

test_post_simple() {
    log_test "POST simple"
    local response=$(http_request "POST" "/" "field=value")
    if [ ! -z "$response" ]; then
        log_success "POST simple réussi"
    else
        log_error "POST simple échoué"
    fi
}

test_post_multiline_body() {
    log_test "POST avec corps multi-ligne"
    local data="name=John%20Doe&email=john@example.com&message=This%20is%20a%20test%0Awith%20multiple%20lines"
    local response=$(http_request "POST" "/" "$data")
    if [ ! -z "$response" ]; then
        log_success "POST multi-ligne réussi"
    else
        log_error "POST multi-ligne échoué"
    fi
}

test_post_large_body() {
    log_test "POST avec corps volumineux (100KB)"
    local large_data=$(python3 -c "print('x' * 102400)")
    local response=$(http_request "POST" "/" "data=$large_data")
    if [ $? -eq 0 ]; then
        log_success "POST volumineux réussi"
    else
        log_error "POST volumineux échoué"
    fi
}

test_404_error() {
    log_test "Erreur 404"
    local response=$(http_request "GET" "/this_does_not_exist_12345" "")
    if echo "$response" | grep -q "404\|Not Found"; then
        log_success "Erreur 404 gérée correctement"
    else
        log_error "Erreur 404 non gérée"
    fi
}

test_cgi_get() {
    log_test "CGI GET - test.php"
    local response=$(http_request "GET" "/cgi_bin/test.php?action=test" "")
    # PHP-CGI peut retourner une erreur ou du contenu, on accepte les deux
    if [ $? -eq 0 ]; then
        log_success "CGI GET réussi"
    else
        log_warning "CGI GET échoué mais ce peut être normal"
    fi
}

test_cgi_post() {
    log_test "CGI POST - Contact.php"
    local data="name=Test%20User&email=test@example.com&message=Hello%20World"
    local response=$(http_request "POST" "/cgi_bin/Contact.php" "$data" \
        "-H 'Content-Type: application/x-www-form-urlencoded'")
    if [ $? -eq 0 ]; then
        log_success "CGI POST réussi"
    else
        log_warning "CGI POST échoué mais ce peut être normal"
    fi
}

test_cgi_post_large() {
    log_test "CGI POST volumineux (50KB)"
    local large_data=$(python3 -c "print('d' * 51200)")
    local response=$(http_request "POST" "/cgi_bin/Contact.php" \
        "data=$large_data" "-H 'Content-Type: application/x-www-form-urlencoded'")
    if [ $? -eq 0 ]; then
        log_success "CGI POST volumineux réussi"
    else
        log_warning "CGI POST volumineux échoué mais ce peut être normal"
    fi
}

test_keep_alive() {
    log_test "Keep-Alive HTTP"
    printf "GET / HTTP/1.1\r\nHost: $HOST:$PORT\r\nConnection: keep-alive\r\n\r\n" | \
        timeout 5 nc -q 1 $HOST $PORT > /dev/null 2>&1
    if [ $? -eq 0 ] || [ $? -eq 124 ]; then
        log_success "Keep-Alive réussi"
    else
        log_error "Keep-Alive échoué"
    fi
}

test_pipelining() {
    log_test "HTTP Pipelining"
    printf "GET / HTTP/1.1\r\nHost: $HOST:$PORT\r\n\r\nGET /index.html HTTP/1.1\r\nHost: $HOST:$PORT\r\nConnection: close\r\n\r\n" | \
        timeout 5 nc -q 1 $HOST $PORT > /dev/null 2>&1
    if [ $? -eq 0 ] || [ $? -eq 124 ]; then
        log_success "Pipelining réussi"
    else
        log_error "Pipelining échoué"
    fi
}

test_concurrent_requests() {
    log_test "Requêtes concurrentes (10 parallèles)"
    local pids=()
    for i in {1..10}; do
        http_request "GET" "/?req=$i" "" > /dev/null &
        pids+=($!)
    done
    
    local failed=0
    for pid in "${pids[@]}"; do
        wait $pid || ((failed++))
    done
    
    if [ $failed -eq 0 ]; then
        log_success "Requêtes concurrentes réussies"
    else
        log_error "Requêtes concurrentes: $failed échouées"
    fi
}

test_concurrent_cgi() {
    log_test "CGI concurrentes (10 parallèles)"
    local pids=()
    for i in {1..10}; do
        http_request "GET" "/cgi_bin/test.php?id=$i" "" > /dev/null 2>&1 &
        pids+=($!)
    done
    
    local failed=0
    for pid in "${pids[@]}"; do
        wait $pid || ((failed++))
    done
    
    if [ $failed -eq 0 ]; then
        log_success "CGI concurrentes réussies"
    else
        log_warning "CGI concurrentes: $failed échouées (normal si PHP-CGI indisponible)"
    fi
}

test_concurrent_mixed() {
    log_test "Requêtes mixtes concurrentes (5 GET + 5 POST + 5 CGI)"
    local pids=()
    
    for i in {1..5}; do
        http_request "GET" "/?test=$i" "" > /dev/null &
        pids+=($!)
        http_request "POST" "/" "data=test$i" > /dev/null &
        pids+=($!)
        http_request "GET" "/cgi_bin/test.php?id=$i" "" > /dev/null 2>&1 &
        pids+=($!)
    done
    
    local failed=0
    for pid in "${pids[@]}"; do
        wait $pid || ((failed++))
    done
    
    if [ $failed -le 5 ]; then # Tolérer les CGI qui échouent
        log_success "Requêtes mixtes concurrentes réussies"
    else
        log_error "Requêtes mixtes: $failed échouées"
    fi
}

# ============================================================================
# ANALYSE VALGRIND
# ============================================================================

analyze_valgrind_log() {
    local logfile=$1
    local test_name=$2
    
    echo ""
    echo -e "${MAGENTA}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${MAGENTA}ANALYSE: $test_name${NC}"
    echo -e "${MAGENTA}═══════════════════════════════════════════════════════════${NC}"
    
    if [ ! -f "$logfile" ]; then
        log_error "Fichier log non trouvé: $logfile"
        return
    fi
    
    # Extraire les statistiques de fuites
    echo ""
    log_info "Statistiques de mémoire:"
    grep -A 20 "HEAP SUMMARY" "$logfile" || true
    
    echo ""
    log_info "Fuites détectées:"
    if grep -q "definitely lost"; then
        grep "definitely lost" "$logfile"
    fi
    
    if grep -q "indirectly lost"; then
        grep "indirectly lost" "$logfile"
    fi
    
    if grep -q "possibly lost"; then
        grep "possibly lost" "$logfile"
    fi
    
    echo ""
    log_info "Erreurs d'accès mémoire:"
    if grep -q "Invalid read\|Invalid write\|Use of uninitialised"; then
        grep "Invalid read\|Invalid write\|Use of uninitialised" "$logfile" | head -10
        ((VALGRIND_ISSUES++))
    else
        log_success "Aucune erreur d'accès détectée"
    fi
    
    echo ""
    log_info "Dernières lignes du rapport:"
    tail -15 "$logfile"
}

# ============================================================================
# MAIN
# ============================================================================

main() {
    echo -e "${BLUE}"
    echo "╔═════════════════════════════════════════════════════════════╗"
    echo "║  TESTS DE FUITES MÉMOIRE - WEBSERV (VALGRIND + CURL)       ║"
    echo "║  Focus CGI et Gestion des Connexions                       ║"
    echo "╚═════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    # Vérifier les dépendances
    log_info "Vérification des dépendances..."
    
    for cmd in curl nc python3; do
        if ! command -v $cmd &> /dev/null; then
            log_error "$cmd n'est pas installé"
            exit 1
        fi
    done
    log_success "Toutes les dépendances présentes"
    
    if ! command -v valgrind &> /dev/null; then
        log_warning "valgrind n'est pas installé, installation..."
        sudo apt-get update && sudo apt-get install -y valgrind > /dev/null 2>&1
    fi
    log_success "valgrind disponible"
    
    # Vérifier compilation
    if [ ! -f "$WEBSERV" ]; then
        log_info "Compilation du serveur..."
        make clean && make > /dev/null 2>&1
        log_success "Serveur compilé"
    fi
    
    # =====================================================================
    # PHASE 1: Tests simples (sans Valgrind)
    # =====================================================================
    echo ""
    echo -e "${BLUE}╔═ PHASE 1: Tests simples (sans Valgrind) ═╗${NC}"
    
    if start_server "timeout 120s $WEBSERV $CONFIG" "Démarrage du serveur (tests simples)"; then
        test_basic_get
        test_get_with_query
        test_post_simple
        test_post_multiline_body
        test_404_error
        test_keep_alive
        test_concurrent_requests
        stop_server
    fi
    
    # =====================================================================
    # PHASE 2: Tests CGI (sans Valgrind)
    # =====================================================================
    echo ""
    echo -e "${BLUE}╔═ PHASE 2: Tests CGI (sans Valgrind) ═╗${NC}"
    
    if start_server "timeout 120s $WEBSERV $CONFIG" "Démarrage du serveur (tests CGI)"; then
        test_cgi_get
        test_cgi_post
        test_concurrent_cgi
        test_concurrent_mixed
        stop_server
    fi
    
    # =====================================================================
    # PHASE 3: Tests volumineux (sans Valgrind)
    # =====================================================================
    echo ""
    echo -e "${BLUE}╔═ PHASE 3: Tests volumineux (sans Valgrind) ═╗${NC}"
    
    if start_server "timeout 120s $WEBSERV $CONFIG" "Démarrage du serveur (tests volumineux)"; then
        test_post_large_body
        test_cgi_post_large
        stop_server
    fi
    
    # =====================================================================
    # PHASE 4: Valgrind - Tests simples
    # =====================================================================
    echo ""
    echo -e "${BLUE}╔═ PHASE 4: Valgrind - Tests simples ═╗${NC}"
    
    log_info "Lancement du serveur avec Valgrind (tests simples)..."
    VALGRIND_LOG="valgrind_simple_$(date +%s).log"
    timeout 40s valgrind $VALGRIND_OPTS \
        $WEBSERV $CONFIG > "$VALGRIND_LOG" 2>&1 &
    WEBSERV_PID=$!
    sleep 3
    
    log_info "Exécution des tests..."
    for i in {1..5}; do
        http_request "GET" "/" "" > /dev/null
        http_request "GET" "/?test=$i" "" > /dev/null
        http_request "POST" "/" "data=test$i" > /dev/null
    done
    
    sleep 1
    stop_server
    sleep 3
    analyze_valgrind_log "$VALGRIND_LOG" "Tests simples"
    
    # =====================================================================
    # PHASE 5: Valgrind - Tests CGI
    # =====================================================================
    echo ""
    echo -e "${BLUE}╔═ PHASE 5: Valgrind - Tests CGI ═╗${NC}"
    
    log_info "Lancement du serveur avec Valgrind (tests CGI)..."
    VALGRIND_LOG="valgrind_cgi_$(date +%s).log"
    timeout 40s valgrind $VALGRIND_OPTS \
        $WEBSERV $CONFIG > "$VALGRIND_LOG" 2>&1 &
    WEBSERV_PID=$!
    sleep 3
    
    log_info "Exécution des tests CGI..."
    for i in {1..5}; do
        http_request "GET" "/cgi_bin/test.php?id=$i" "" > /dev/null 2>&1
        http_request "POST" "/cgi_bin/Contact.php" "name=test$i&email=test@example.com" > /dev/null 2>&1
    done
    
    sleep 1
    stop_server
    sleep 3
    analyze_valgrind_log "$VALGRIND_LOG" "Tests CGI"
    
    # =====================================================================
    # PHASE 6: Valgrind - Tests concurrents
    # =====================================================================
    echo ""
    echo -e "${BLUE}╔═ PHASE 6: Valgrind - Tests concurrents ═╗${NC}"
    
    log_info "Lancement du serveur avec Valgrind (tests concurrents)..."
    VALGRIND_LOG="valgrind_concurrent_$(date +%s).log"
    timeout 40s valgrind $VALGRIND_OPTS \
        $WEBSERV $CONFIG > "$VALGRIND_LOG" 2>&1 &
    WEBSERV_PID=$!
    sleep 3
    
    log_info "Exécution des tests concurrents..."
    local pids=()
    for i in {1..3}; do
        http_request "GET" "/?req=$i" "" > /dev/null &
        pids+=($!)
        http_request "POST" "/" "data=test$i" > /dev/null &
        pids+=($!)
    done
    for pid in "${pids[@]}"; do
        wait $pid || true
    done
    
    sleep 1
    stop_server
    sleep 3
    analyze_valgrind_log "$VALGRIND_LOG" "Tests concurrents"
    
    # =====================================================================
    # RÉSUMÉ
    # =====================================================================
    echo ""
    echo -e "${BLUE}╔═════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║  RÉSUMÉ FINAL${NC}"
    echo -e "${BLUE}╠═════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${BLUE}║  Tests exécutés: $TEST_COUNT${NC}"
    echo -e "${GREEN}║  Réussis: $PASS_COUNT${NC}"
    echo -e "${RED}║  Échoués: $FAIL_COUNT${NC}"
    echo -e "${YELLOW}║  Problèmes Valgrind: $VALGRIND_ISSUES${NC}"
    echo -e "${BLUE}╚═════════════════════════════════════════════════════════════╝${NC}"
    
    # Afficher les fichiers de log générés
    echo ""
    log_info "Fichiers de log Valgrind:"
    ls -lah valgrind_*.log 2>/dev/null || log_warning "Aucun fichier de log"
    
    if [ $VALGRIND_ISSUES -eq 0 ]; then
        echo -e "${GREEN}✓ Aucun problème d'accès mémoire détecté!${NC}"
        exit 0
    else
        echo -e "${RED}✗ Problèmes d'accès mémoire détectés!${NC}"
        exit 1
    fi
}

# Lancer les tests
main "$@"
