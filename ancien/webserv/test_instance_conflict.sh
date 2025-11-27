#!/bin/bash
set -eu
BASE_DIR="$(cd "$(dirname "$0")" && pwd)"
WEBSERV="$BASE_DIR/webserv"
CONF_A="$BASE_DIR/conf.d/instanceA.conf"
CONF_B="$BASE_DIR/conf.d/instanceB.conf"
LOGA="/tmp/webservA.log"
PIDA="/tmp/webservA.pid"
LOGB="/tmp/webservB.log"

rm -f "$LOGA" "$LOGB" "$PIDA"

if [ ! -x "$WEBSERV" ]; then
  echo "ERROR: webserv binary not found; run make" >&2
  exit 2
fi

# Start first instance in background
"$WEBSERV" "$CONF_A" > "$LOGA" 2>&1 &
PID=$!
echo $PID > "$PIDA"

# Wait up to 6 seconds for "Listening on" in log
i=0
while [ $i -lt 6 ]; do
  if grep -q "Listening on" "$LOGA" 2>/dev/null; then
    break
  fi
  sleep 1
  i=$((i+1))
done

if ! grep -q "Listening on" "$LOGA" 2>/dev/null; then
  echo "TEST SETUP FAILED: first instance did not start or listen. See $LOGA" >&2
  kill $PID 2>/dev/null || true
  wait $PID 2>/dev/null || true
  exit 2
fi

# Start second instance (foreground with timeout) and capture output
set +e
timeout 6 "$WEBSERV" "$CONF_B" > "$LOGB" 2>&1
RC=$?
set -e

echo "--- First instance log ($LOGA) ---"
tail -n +1 "$LOGA" || true
echo "--- Second instance log ($LOGB) ---"
tail -n +1 "$LOGB" || true

echo "Second instance exit code: $RC"

# Determine pass/fail: expect second to fail (non-zero) or log contains 'Error'
if [ $RC -ne 0 ] || grep -q "Error" "$LOGB" 2>/dev/null; then
  echo "TEST PASS: second instance failed to bind as expected"
  RESULT=0
else
  echo "TEST FAIL: second instance started successfully (unexpected)"
  RESULT=1
fi

# Cleanup first instance
kill $PID 2>/dev/null || true
wait $PID 2>/dev/null || true

exit $RESULT
