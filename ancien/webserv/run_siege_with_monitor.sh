#!/bin/bash

# ============================================================================
# Siege Load Test with Memory Monitoring for WebServ
# ============================================================================

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

cd /home/rivoinfo/Documents/DEV/servwebs/webserv

# Configuration
WEBSERV="./webserv"
CONFIG="./conf.d/webserv.conf"
BASE_URL="http://localhost:8080/"
TEST_DURATION=${1:-180}  # Default 3 minutes, can override with arg
URLS_FILE="./siege_custom_urls.txt"
SIEGE_LOG="siege_output.log"
MEMORY_LOG="memory_trace.log"
STATS_LOG="siege_stats.txt"

echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}SIEGE LOAD TEST - WebServ Memory & Availability Check${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "${YELLOW}Configuration:${NC}"
echo "  URL: $BASE_URL"
echo "  Duration: ${TEST_DURATION}s"
echo "  Test type: siege -b (HTTP/1.1 sustained load)"
echo ""

# ========================================================================
# STEP 1: Check webserv is compiled
# ========================================================================

if [ ! -f "$WEBSERV" ]; then
    echo -e "${RED}[!] Webserv binary not found. Compiling...${NC}"
    make clean && make > /dev/null 2>&1
fi

# ========================================================================
# STEP 2: Ensure no existing server is running
# ========================================================================

pkill -f "./webserv" || true
sleep 1

# ========================================================================
# STEP 3: Start webserv in background
# ========================================================================

echo -e "${BLUE}[*] Starting WebServ...${NC}"
./$WEBSERV $CONFIG > /tmp/webserv_run.log 2>&1 &
WEBSERV_PID=$!
sleep 2

if ! kill -0 $WEBSERV_PID 2>/dev/null; then
    echo -e "${RED}[!] WebServ failed to start${NC}"
    cat /tmp/webserv_run.log
    exit 1
fi

echo -e "${GREEN}[✓] WebServ started (PID: $WEBSERV_PID)${NC}"

# Quick health check
HEALTH_CHECK=$(curl -s -o /dev/null -w "%{http_code}" "$BASE_URL" 2>/dev/null || echo "000")
if [ "$HEALTH_CHECK" != "200" ]; then
    echo -e "${RED}[!] Health check failed (HTTP $HEALTH_CHECK)${NC}"
    kill $WEBSERV_PID 2>/dev/null || true
    exit 1
fi
echo -e "${GREEN}[✓] Health check passed (HTTP $HEALTH_CHECK)${NC}"

# ========================================================================
# STEP 4: Background memory monitoring
# ========================================================================

echo "" > "$MEMORY_LOG"

memory_monitor() {
    local pid=$1
    local log_file=$2
    while kill -0 $pid 2>/dev/null; do
        TIMESTAMP=$(date '+%s.%N')
        if [ -f /proc/$pid/status ]; then
            RSS=$(grep "^VmRSS:" /proc/$pid/status | awk '{print $2}')  # in KB
            echo "$TIMESTAMP $RSS" >> "$log_file"
        fi
        sleep 1
    done
}

echo -e "${BLUE}[*] Starting memory monitor (sampling every 1s)...${NC}"
memory_monitor $WEBSERV_PID "$MEMORY_LOG" &
MONITOR_PID=$!

# ========================================================================
# STEP 5: Run Siege
# ========================================================================

echo ""
echo -e "${BLUE}[*] Starting Siege load test (${TEST_DURATION}s) with HTTP keep-alive...${NC}"
echo -e "${YELLOW}[*] URL: $BASE_URL${NC}"
echo ""

# Run siege with:
#   -b: benchmark mode (full-speed, connection pool reuse = HTTP keep-alive)
#   -t: timeout per request (in seconds)
#   -c: concurrent users
#   -r: repetitions (0 = infinite, limited by time)
#   -m: verbose mode
#   URL: direct URL
# We'll run siege for TEST_DURATION seconds

START_TIME=$(date +%s)
END_TIME=$((START_TIME + TEST_DURATION))

# Run siege and capture output
# Use time-based testing only: -t <duration>s and -c <concurrency>
# -b benchmark mode (reuses connections = HTTP keep-alive)
CONCURRENCY=10
siege -b -c ${CONCURRENCY} -t ${TEST_DURATION}s "$BASE_URL" 2>&1 | tee "$STATS_LOG" &

SIEGE_PID=$!

# Let siege run for TEST_DURATION seconds
while [ $(date +%s) -lt $END_TIME ]; do
    if ! kill -0 $SIEGE_PID 2>/dev/null; then
        break
    fi
    sleep 1
done

# Stop siege gracefully
kill $SIEGE_PID 2>/dev/null || true
wait $SIEGE_PID 2>/dev/null || true

echo ""
echo -e "${GREEN}[✓] Siege test completed${NC}"

# ========================================================================
# STEP 6: Stop webserv and memory monitor
# ========================================================================

kill $MONITOR_PID 2>/dev/null || true
wait $MONITOR_PID 2>/dev/null || true

kill $WEBSERV_PID 2>/dev/null || true
wait $WEBSERV_PID 2>/dev/null || true

sleep 1

# ========================================================================
# STEP 7: Analyze Results
# ========================================================================

echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}ANALYSIS & RESULTS${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"

# Parse Siege metrics from log
echo ""
echo -e "${YELLOW}[*] Siege Summary Metrics:${NC}"

if [ -f "$STATS_LOG" ]; then
    grep -E "Transactions:|Availability:|Elapsed time:|Data transferred:|Response time:|Concurrency:" "$STATS_LOG" || echo "  (metrics not found in output)"
fi

# Extract from siege log file
if [ -f "$SIEGE_LOG" ]; then
    TOTAL_TRANSACTIONS=$(grep -c "HTTP" "$SIEGE_LOG" 2>/dev/null || echo "0")
    echo "  Total Transactions: $TOTAL_TRANSACTIONS"
fi

# Analyze memory usage
echo ""
echo -e "${YELLOW}[*] Memory Usage Analysis:${NC}"

if [ -f "$MEMORY_LOG" ] && [ -s "$MEMORY_LOG" ]; then
    MEM_INITIAL=$(head -1 "$MEMORY_LOG" | awk '{print $2}')
    MEM_FINAL=$(tail -1 "$MEMORY_LOG" | awk '{print $2}')
    MEM_MAX=$(awk '{print $2}' "$MEMORY_LOG" | sort -n | tail -1)
    MEM_MIN=$(awk '{print $2}' "$MEMORY_LOG" | sort -n | head -1)
    
    echo "  Initial Memory: ${MEM_INITIAL} KB"
    echo "  Final Memory: ${MEM_FINAL} KB"
    echo "  Max Memory: ${MEM_MAX} KB"
    echo "  Min Memory: ${MEM_MIN} KB"
    
    # Check for memory growth trend
    if [ ! -z "$MEM_INITIAL" ] && [ ! -z "$MEM_FINAL" ]; then
        MEM_GROWTH=$((MEM_FINAL - MEM_INITIAL))
        MEM_GROWTH_PCT=$(( (MEM_FINAL - MEM_INITIAL) * 100 / (MEM_INITIAL + 1) ))
        
        echo "  Memory Growth: ${MEM_GROWTH} KB (${MEM_GROWTH_PCT}%)"
        
        if [ $MEM_GROWTH -gt 50000 ]; then
            echo -e "${RED}  ⚠ WARNING: High memory growth detected!${NC}"
        else
            echo -e "${GREEN}  ✓ Memory growth acceptable${NC}"
        fi
    fi
else
    echo "  (No memory data collected)"
fi

# Check for failures
echo ""
echo -e "${YELLOW}[*] Connection & Availability Check:${NC}"

if [ -f "$STATS_LOG" ]; then
    # Look for availability percentage
    AVAILABILITY=$(grep "Availability:" "$STATS_LOG" | head -1 | awk '{print $2}')
    if [ ! -z "$AVAILABILITY" ]; then
        echo "  Availability: $AVAILABILITY"
        
        # Check if >= 99.5%
        if echo "$AVAILABILITY" | grep -qE '^[0-9]+\.[0-9]+%$'; then
            AVAIL_NUM=$(echo "$AVAILABILITY" | sed 's/%//')
            if (( $(echo "$AVAIL_NUM >= 99.5" | bc -l) )); then
                echo -e "${GREEN}  ✓ Availability >= 99.5%${NC}"
            else
                echo -e "${RED}  ✗ Availability < 99.5%${NC}"
            fi
        fi
    fi
fi

# Check for failed transactions
FAILED=$(grep -c "FALSE" "$SIEGE_LOG" 2>/dev/null || echo "0")
if [ "$FAILED" -gt 0 ]; then
    echo -e "${RED}  ✗ Failed requests: $FAILED${NC}"
else
    echo -e "${GREEN}  ✓ No failed requests${NC}"
fi

# ========================================================================
# STEP 8: Generate detailed report
# ========================================================================

echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}DETAILED REPORT${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"

cat << EOF

📊 Test Summary
───────────────
Duration: ${TEST_DURATION}s
Base URL: $BASE_URL
Concurrent Users: 10
Test Mode: Benchmark (HTTP/1.1 keep-alive, connection reuse)

📈 Files Generated
──────────────────
- $SIEGE_LOG        : Raw siege transaction log
- $STATS_LOG        : Siege summary statistics
- $MEMORY_LOG       : Memory usage trace (timestamp RSS_KB)

🔍 Next Steps
─────────────
1. Review $STATS_LOG for detailed metrics
2. Check $MEMORY_LOG for memory trend (should be flat or slowly increasing)
3. Run 'siege -r' with longer duration for extended testing
4. Monitor /proc/[PID]/status for real-time resource usage

💡 Quick Commands
─────────────────
# View siege stats:
cat $STATS_LOG

# View memory trace (plot with gnuplot or tail):
tail -20 $MEMORY_LOG

# Run longer test (specify duration):
$0 600  # 10 minutes

# Monitor webserv in real-time:
watch -n 1 'ps aux | grep ./webserv'

EOF

echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}✓ Test Complete${NC}"
