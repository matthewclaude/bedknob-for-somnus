#!/usr/bin/env python3
"""
Somnus Pad Web Controller
A simple web interface to control your smart bed
"""

from flask import Flask, render_template, jsonify, request
import requests
import csv
import os
from datetime import datetime
from threading import Thread
import time

app = Flask(__name__)

# Bed configuration
BED_IP = "192.168.1.169"
BED_PORT = "8080"
BED_URL = f"http://{BED_IP}:{BED_PORT}"

# Logging configuration
LOG_FILE = "sleep_temps.csv"
LOG_INTERVAL = 300  # 5 minutes in seconds


def get_bed_state():
    """Get current bed state from API"""
    try:
        response = requests.get(f"{BED_URL}/api/state", timeout=5)
        response.raise_for_status()
        return response.json()
    except Exception as e:
        print(f"Error getting state: {e}")
        return None


def set_power(side0_on=None, side1_on=None):
    """Set power for sides"""
    payload = {}
    if side0_on is not None:
        payload["side0"] = {"is_on": side0_on}
    if side1_on is not None:
        payload["side1"] = {"is_on": side1_on}
    
    try:
        response = requests.post(f"{BED_URL}/api/power", json=payload, timeout=5)
        response.raise_for_status()
        return response.json()
    except Exception as e:
        print(f"Error setting power: {e}")
        return None


def set_temperature(side0_temp=None, side1_temp=None):
    """Set temperature for sides"""
    payload = {}
    if side0_temp is not None:
        payload["side0"] = {"target_t": float(side0_temp)}
    if side1_temp is not None:
        payload["side1"] = {"target_t": float(side1_temp)}
    
    try:
        response = requests.post(f"{BED_URL}/api/target_t", json=payload, timeout=5)
        response.raise_for_status()
        return response.json()
    except Exception as e:
        print(f"Error setting temperature: {e}")
        return None


def log_temperature():
    """Log current temperatures to CSV file"""
    state = get_bed_state()
    if not state:
        return
    
    # Check if file exists to determine if we need headers
    file_exists = os.path.isfile(LOG_FILE)
    
    try:
        with open(LOG_FILE, 'a', newline='') as f:
            writer = csv.writer(f)
            
            # Write header if new file
            if not file_exists:
                writer.writerow([
                    'timestamp',
                    'side0_current_temp_c',
                    'side0_target_temp_c',
                    'side0_power',
                    'side0_water_low',
                    'side1_current_temp_c',
                    'side1_target_temp_c',
                    'side1_power',
                    'side1_water_low',
                    'system_error'
                ])
            
            # Write data row
            writer.writerow([
                datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                state['side0'].get('current_t', ''),
                state['side0']['target_t'],
                state['side0']['is_on'],
                state['side0']['is_wl_low'],
                state['side1'].get('current_t', ''),
                state['side1']['target_t'],
                state['side1']['is_on'],
                state['side1']['is_wl_low'],
                state['error']
            ])
    except Exception as e:
        print(f"Error logging temperature: {e}")


def temperature_logger_thread():
    """Background thread to log temperatures periodically"""
    while True:
        log_temperature()
        time.sleep(LOG_INTERVAL)


@app.route('/')
def index():
    """Main page"""
    return render_template('index.html')


@app.route('/api/status')
def api_status():
    """Get bed status"""
    state = get_bed_state()
    if state:
        return jsonify(state)
    else:
        return jsonify({"error": "Could not connect to bed"}), 500


@app.route('/api/power', methods=['POST'])
def api_power():
    """Control power"""
    data = request.json
    side0 = data.get('side0')
    side1 = data.get('side1')
    
    result = set_power(side0_on=side0, side1_on=side1)
    if result:
        return jsonify(result)
    else:
        return jsonify({"error": "Failed to set power"}), 500


@app.route('/api/temperature', methods=['POST'])
def api_temperature():
    """Control temperature"""
    data = request.json
    side0 = data.get('side0')
    side1 = data.get('side1')
    
    result = set_temperature(side0_temp=side0, side1_temp=side1)
    if result:
        return jsonify(result)
    else:
        return jsonify({"error": "Failed to set temperature"}), 500


if __name__ == '__main__':
    print("\n" + "="*60)
    print("🛏️  Sleepy Pad Controller")
    print("="*60)
    print(f"\n✅ Starting web server...")
    print(f"📱 Open in browser: http://localhost:8888")
    print(f"📱 Or from phone/tablet: http://[your-computer-ip]:8888")
    print(f"\n📊 Temperature logging enabled")
    print(f"   Logging every {LOG_INTERVAL//60} minutes to: {LOG_FILE}")
    print(f"\n💡 Press Ctrl+C to stop\n")
    print("="*60 + "\n")
    
    # Start background temperature logger
    logger_thread = Thread(target=temperature_logger_thread, daemon=True)
    logger_thread.start()
    
    # Run the app
    app.run(host='0.0.0.0', port=8888, debug=True)
