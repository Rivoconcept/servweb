#!/bin/bash

# test_duplicate_ports.sh
# Test that webserv REJECTS configuration with duplicate (host, port) pairs
# Expected: Server should exit with error message about duplicate listen address

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}TEST: Duplicate Ports Configuration (SHOULD FAIL)${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo ""

# Configuration
WEBSERV="./webserv"
CONFIG_DUP="./conf.d/webserv_test_dup_ports.conf"
CONFIG_GOOD="./conf.d/webserv.conf"

# Step 1: Test DUPLICATE ports (SHOULD FAIL)
echo -e "${YELLOW}[TEST 1] Starting webserv with DUPLICATE port configuration...${NC}"
echo "Config: $CONFIG_DUP"
echo "Expected: Should FAIL with error message"
echo ""

OUTPUT=$($WEBSERV $CONFIG_DUP 2>&1)
EXIT_CODE=$?

echo "Output:"
echo "$OUTPUT"
echo ""

if [ $EXIT_CODE -ne 0 ] && echo "$OUTPUT" | grep -qi "duplicate"; then
    echo -e "${GREEN}[✓] PASS: Server correctly REJECTED duplicate ports${NC}"
    echo -e "${GREEN}[✓] Error message contains 'duplicate'${NC}"
else
    echo -e "${RED}[✗] FAIL: Server should have rejected duplicate ports${NC}"
    exit 1
fi

# Step 2: Test VALID configuration (SHOULD SUCCEED)
echo ""
echo -e "${YELLOW}[TEST 2] Starting webserv with VALID configuration...${NC}"
echo "Config: $CONFIG_GOOD"
echo "Expected: Should START successfully"
echo ""

timeout 3s $WEBSERV $CONFIG_GOOD > /tmp/test_good_config.log 2>&1
EXIT_CODE=$?

# timeout exits with 124, but server should have started
if [ $EXIT_CODE -eq 124 ]; then
    # Read the log to verify it started
    if grep -q "Listening on" /tmp/test_good_config.log; then
        echo -e "${GREEN}[✓] PASS: Server started with valid configuration${NC}"
        cat /tmp/test_good_config.log
    else
        echo -e "${RED}[✗] FAIL: Server did not start properly${NC}"
        cat /tmp/test_good_config.log
        exit 1
    fi
elif [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}[✓] PASS: Server started with valid configuration${NC}"
    cat /tmp/test_good_config.log
else
    echo -e "${RED}[✗] FAIL: Server exited with error code $EXIT_CODE${NC}"
    cat /tmp/test_good_config.log
    exit 1
fi

# Step 3: Summary
echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}SUMMARY${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "${GREEN}✓ Duplicate port detection is working correctly${NC}"
echo -e "${GREEN}✓ Server rejects configurations with same (host:port) twice${NC}"
echo -e "${GREEN}✓ Server accepts valid configurations without duplicates${NC}"
echo ""

