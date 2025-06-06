#!/bin/bash

echo "=== Stopping Audiosurf Game ==="

# Stop any music playing
echo "Step 1: Stopping music..."
pkill afplay
if [ $? -eq 0 ]; then
    echo "✓ Music stopped"
else
    echo "• No music playing"
fi

# Kill any screen sessions (serial monitor)
echo "Step 2: Closing serial monitors..."
SCREEN_SESSIONS=$(screen -ls | grep Detached | wc -l)
if [ $SCREEN_SESSIONS -gt 0 ]; then
    # Kill all detached screen sessions
    screen -ls | grep Detached | cut -d. -f1 | awk '{print $1}' | xargs -I {} screen -X -S {} quit
    echo "✓ Closed $SCREEN_SESSIONS serial monitor session(s)"
else
    echo "• No detached serial monitors found"
fi

# Kill any active screen sessions that might be running the serial monitor
ACTIVE_SCREENS=$(screen -ls | grep -E "\..*\(" | wc -l)
if [ $ACTIVE_SCREENS -gt 0 ]; then
    echo "Found $ACTIVE_SCREENS active screen session(s) - force closing them..."
    # Force quit all active screen sessions
    screen -ls | grep -E "\..*\(" | cut -d. -f1 | awk '{print $1}' | xargs -I {} screen -X -S {} quit
    echo "✓ Force-closed $ACTIVE_SCREENS active screen session(s)"
fi

# Clean up any remaining dead screen sessions
echo "Cleaning up dead screen sessions..."
screen -wipe > /dev/null 2>&1

# Stop any PlatformIO processes
echo "Step 3: Stopping PlatformIO processes..."
PIO_PIDS=$(pgrep -f "pio run")
if [ ! -z "$PIO_PIDS" ]; then
    echo "$PIO_PIDS" | xargs kill
    echo "✓ Stopped PlatformIO upload processes"
else
    echo "• No PlatformIO processes running"
fi

# Stop any platformio device monitor processes
PIO_MONITOR_PIDS=$(pgrep -f "platformio device monitor")
if [ ! -z "$PIO_MONITOR_PIDS" ]; then
    echo "$PIO_MONITOR_PIDS" | xargs kill
    echo "✓ Stopped PlatformIO monitor processes"
else
    echo "• No PlatformIO monitor processes running"
fi

# Optional: Reset the Arduino
# echo "Step 4: Resetting Arduino..."
# PORT="/dev/cu.usbmodem*"
# ACTUAL_PORT=$(ls $PORT 2>/dev/null | head -n 1)
# if [ ! -z "$ACTUAL_PORT" ]; then
#     # Send reset signal by opening and closing the serial port
#     stty -f $ACTUAL_PORT 1200
#     sleep 1
#     echo "✓ Arduino reset signal sent"
# fi

echo ""
echo "=== Game Stopped Successfully ==="
echo "All processes have been terminated."
echo ""
echo "To restart the game, run: ./start_game.sh"