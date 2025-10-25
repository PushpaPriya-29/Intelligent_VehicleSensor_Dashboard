#!/bin/bash

# Vehicle Dashboard Startup Script
echo "=========================================="
echo " Intelligent Vehicle Sensor Dashboard"
echo "=========================================="

# Clean up any existing processes and shared memory
echo "Cleaning up existing processes and shared memory..."
killall alert collision drivermonitor environmental gpsloc parking cruise dashboard 2>/dev/null
ipcs -m | grep 8108 | awk '{print $2}' | xargs -I {} ipcrm -m {} 2>/dev/null
ipcs -m | grep 9999 | awk '{print $2}' | xargs -I {} ipcrm -m {} 2>/dev/null
rm -f cuise_collision_fifo

# Create FIFO
echo "Creating FIFO for cruise control..."
gcc cruise_fifo.c -o cruise_fifo
./cruise_fifo

# Compile all programs
echo "Compiling all programs..."
make clean 2>/dev/null
make all

if [ $? -ne 0 ]; then
    echo "❌ Compilation failed!"
    exit 1
fi

echo "✅ Compilation successful!"
echo ""

# Start all sensor processes in background
echo "Starting sensor processes..."
./alert > /dev/null 2>&1 &
ALERT_PID=$!
echo "  ✓ Alert process (PID: $ALERT_PID)"

sleep 0.5

./collision > /dev/null 2>&1 &
COLLISION_PID=$!
echo "  ✓ Collision detection (PID: $COLLISION_PID)"

sleep 0.5

./drivermonitor > /dev/null 2>&1 &
DRIVER_PID=$!
echo "  ✓ Driver monitor (PID: $DRIVER_PID)"

sleep 0.5

./environmental > /dev/null 2>&1 &
ENV_PID=$!
echo "  ✓ Environmental sensors (PID: $ENV_PID)"

sleep 0.5

./gpsloc > /dev/null 2>&1 &
GPS_PID=$!
echo "  ✓ GPS navigation (PID: $GPS_PID)"

sleep 0.5

./parking > /dev/null 2>&1 &
PARKING_PID=$!
echo "  ✓ Parking assist (PID: $PARKING_PID)"

# Wait for processes to initialize
echo ""
echo "Initializing shared memory..."
sleep 2

# Start the main dashboard UI
echo "=========================================="
echo "Starting Dashboard UI..."
echo "=========================================="
./dashboard

# Cleanup after dashboard exits
echo ""
echo "Dashboard closed. Cleaning up..."
kill $ALERT_PID $COLLISION_PID $DRIVER_PID $ENV_PID $GPS_PID $PARKING_PID 2>/dev/null

# Remove FIFO
rm -f cuise_collision_fifo

echo "✅ All processes stopped. Goodbye!"
