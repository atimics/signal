#!/bin/bash
# Run cgame in logo scene for 3 seconds then exit
./build/cgame &
PID=$!
sleep 3
kill -TERM $PID 2>/dev/null || true
wait $PID 2>/dev/null || true
