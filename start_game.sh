#!/bin/bash

# Get the port from the first argument, or use a default
PORT=${1:-"/dev/cu.usbmodem*"}

# Find the actual port if a wildcard was used
ACTUAL_PORT=$(ls $PORT 2>/dev/null | head -n 1)

if [ -z "$ACTUAL_PORT" ]; then
    echo "Error: No serial port found matching $PORT"
    echo "Please make sure your Arduino is connected and try again."
    exit 1
fi

echo "=== Starting Audiosurf Game ==="
echo "Step 1: Compiling and uploading game..."
# Compile and upload using PlatformIO
pio run -t upload

if [ $? -ne 0 ]; then
    echo "Error: Failed to compile or upload the game"
    exit 1
fi

echo "Step 2: Waiting for Arduino to reset..."
sleep 2  # Wait for Arduino to reset

echo "Step 3: Starting music..."
afplay "music/DaftPunkOneMoreTime.mp3" &

echo "Step 4: Starting PlatformIO serial monitor..."
echo "Game is starting! You should see the welcome message below."
echo "Press Ctrl+C to exit the serial monitor and stop the game"

# Start PlatformIO serial monitor
pio device monitor

# Stop the music when the serial monitor is closed
pkill afplay 