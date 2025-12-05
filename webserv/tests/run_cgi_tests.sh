#!/bin/bash
# Runner de tests CGI pour l'évaluation
# Exécute quelques requêtes sur /cgi_bin/Contact.php et scripts de test
# ATTENTION: Si php-cgi n'est pas installé le serveur renverra des erreurs d'execve.

BASE_URL="http://localhost:8080/cgi_bin"
OUTDIR="tests/out"
mkdir -p "$OUTDIR"

echo "=== Test: Contact.php (GET) ==="
timeout 6s curl -s -D "$OUTDIR/contact_headers.txt" "$BASE_URL/Contact.php" -o "$OUTDIR/contact_body.txt" || true
echo "--- headers ---"; cat "$OUTDIR/contact_headers.txt" || true
echo "--- body (first 200 chars) ---"; head -c 200 "$OUTDIR/contact_body.txt" || true

echo "\n=== Test: error.php (syntax error) ==="
timeout 6s curl -s -D "$OUTDIR/error_headers.txt" "$BASE_URL/error.php" -o "$OUTDIR/error_body.txt" || true
echo "--- headers ---"; cat "$OUTDIR/error_headers.txt" || true
echo "--- body ---"; sed -n '1,120p' "$OUTDIR/error_body.txt" || true

echo "\n=== Test: exit1.php (exit non-zero) ==="
timeout 6s curl -s -D "$OUTDIR/exit1_headers.txt" "$BASE_URL/exit1.php" -o "$OUTDIR/exit1_body.txt" || true
echo "--- headers ---"; cat "$OUTDIR/exit1_headers.txt" || true
echo "--- body ---"; sed -n '1,120p' "$OUTDIR/exit1_body.txt" || true

echo "\n=== Test: loop.php (controlled loop with timeout) ==="
# use timeout to avoid blocking longer than 6s
timeout 7s curl -s -D "$OUTDIR/loop_headers.txt" "$BASE_URL/loop.php" -o "$OUTDIR/loop_body.txt" || true
echo "--- headers ---"; cat "$OUTDIR/loop_headers.txt" || true
echo "--- body (tail 200 chars) ---"; tail -c 200 "$OUTDIR/loop_body.txt" || true

# show server log tail
echo "\n=== webserv.log tail ==="
tail -n 80 webserv.log || true

echo "\nTests finished. Check $OUTDIR for full outputs. Note: If /usr/bin/php-cgi is not installed, CGI exec will fail (execve error)." 
