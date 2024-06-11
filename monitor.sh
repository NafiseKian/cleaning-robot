#!/bin/bash

# Paths to your executables
CXX_EXECUTABLE="/home/ciuteam/cleaningrobot/cleaning-robot/runapp"
PYTHON_SCRIPT="/home/ciuteam/cleaningrobot/cleaning-robot/trial.py"

# Function to check if a process is running
is_running() {
    pgrep -f "$1" > /dev/null 2>&1
}

# Function to start the C++ program
start_cxx_program() {
    echo "Starting C++ program..."
    PYTHONHOME=/home/ciuteam/cleaningrobot/tf-env \
    PYTHONPATH=/home/ciuteam/cleaningrobot/tf-env/lib/python3.11/site-packages:/home/ciuteam/cleaningrobot/tf-env/lib/python3.11 \
    "$CXX_EXECUTABLE" &
}

# Function to start the Python script
start_python_script() {
    echo "Starting Python script..."
    python3.11 "$PYTHON_SCRIPT" &
}

# Infinite loop to monitor the processes
while true; do

    if ! is_running "$PYTHON_SCRIPT"; then
        start_python_script
        sleep 45
    fi

    if ! is_running "$CXX_EXECUTABLE"; then
        start_cxx_program
    fi

    # Sleep for a short period before checking again
    sleep 5
done
