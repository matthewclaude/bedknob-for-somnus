import requests
import json
from datetime import datetime

class SomnusPad:
    """Controller for Somnus Pad smart bed via local HTTP API"""
    
    def __init__(self, ip="192.168.1.169", port="8080"):
        self.base_url = f"http://{ip}:{port}"
    
    def get_state(self):
        """Get current device state"""
        try:
            response = requests.get(f"{self.base_url}/api/state")
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            print(f"Error getting state: {e}")
            return None
    
    def set_power(self, side0_on=None, side1_on=None):
        """
        Set power on/off for one or both sides
        
        Args:
            side0_on (bool): Turn side 0 on (True) or off (False), None to skip
            side1_on (bool): Turn side 1 on (True) or off (False), None to skip
        
        Returns:
            dict: Updated state or None on error
        """
        payload = {}
        
        if side0_on is not None:
            payload["side0"] = {"is_on": side0_on}
        
        if side1_on is not None:
            payload["side1"] = {"is_on": side1_on}
        
        if not payload:
            print("Error: Must specify at least one side")
            return None
        
        try:
            response = requests.post(
                f"{self.base_url}/api/power",
                json=payload
            )
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            print(f"Error setting power: {e}")
            return None
    
    def set_temperature(self, side0_temp=None, side1_temp=None):
        """
        Set target temperature for one or both sides
        
        Args:
            side0_temp (float): Temperature in Celsius (12-42.3), None to skip
            side1_temp (float): Temperature in Celsius (12-42.3), None to skip
        
        Returns:
            dict: Updated state or None on error
        """
        payload = {}
        
        if side0_temp is not None:
            # Clamp to valid range
            temp = max(12.0, min(42.3, side0_temp))
            payload["side0"] = {"target_t": temp}
        
        if side1_temp is not None:
            # Clamp to valid range
            temp = max(12.0, min(42.3, side1_temp))
            payload["side1"] = {"target_t": temp}
        
        if not payload:
            print("Error: Must specify at least one side")
            return None
        
        try:
            response = requests.post(
                f"{self.base_url}/api/target_t",
                json=payload
            )
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            print(f"Error setting temperature: {e}")
            return None
    
    def display_state(self):
        """Get and display current state in a nice format"""
        state = self.get_state()
        
        if not state:
            print("Failed to get bed state")
            return
        
        print(f"\n{'='*60}")
        print(f"Somnus Pad Status - {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"{'='*60}")
        
        # Side 0 (Left)
        print(f"\n📍 SIDE 0 (Left):")
        print(f"   Power:       {'🟢 ON' if state['side0']['is_on'] else '🔴 OFF'}")
        
        # Current temperature
        current0 = state['side0'].get('current_t')
        if current0 is not None:
            print(f"   Current:     {current0:.1f}°C ({self._c_to_f(current0):.1f}°F)")
        else:
            print(f"   Current:     (no reading yet)")
        
        # Target temperature
        print(f"   Target:      {state['side0']['target_t']:.1f}°C ({self._c_to_f(state['side0']['target_t']):.1f}°F)")
        print(f"   Water Level: {'⚠️  LOW' if state['side0']['is_wl_low'] else '✅ OK'}")
        
        # Side 1 (Right)
        print(f"\n📍 SIDE 1 (Right):")
        print(f"   Power:       {'🟢 ON' if state['side1']['is_on'] else '🔴 OFF'}")
        
        # Current temperature
        current1 = state['side1'].get('current_t')
        if current1 is not None:
            print(f"   Current:     {current1:.1f}°C ({self._c_to_f(current1):.1f}°F)")
        else:
            print(f"   Current:     (no reading yet)")
        
        # Target temperature
        print(f"   Target:      {state['side1']['target_t']:.1f}°C ({self._c_to_f(state['side1']['target_t']):.1f}°F)")
        print(f"   Water Level: {'⚠️  LOW' if state['side1']['is_wl_low'] else '✅ OK'}")
        
        # System status
        print(f"\n⚙️  System Error: {'⚠️  YES' if state['error'] else '✅ None'}")
        print(f"{'='*60}\n")
    
    @staticmethod
    def _c_to_f(celsius):
        """Convert Celsius to Fahrenheit"""
        return (celsius * 9/5) + 32
    
    @staticmethod
    def _f_to_c(fahrenheit):
        """Convert Fahrenheit to Celsius"""
        return (fahrenheit - 32) * 5/9


# ============================================================================
# EXAMPLE USAGE
# ============================================================================

if __name__ == "__main__":
    # Create bed controller
    bed = SomnusPad(ip="192.168.1.169", port="8080")
    
    # Display current state
    print("Current bed state:")
    bed.display_state()
    
    # Example 1: Turn on side 0
    print("\n--- Turning ON side 0 ---")
    bed.set_power(side0_on=True)
    bed.display_state()
    
    # Example 2: Set temperature to 20°C (68°F) on side 0
    print("\n--- Setting side 0 to 20°C (68°F) ---")
    bed.set_temperature(side0_temp=20.0)
    bed.display_state()
    
    # Example 3: Set temperature using Fahrenheit
    target_f = 72  # 72°F
    target_c = bed._f_to_c(target_f)
    print(f"\n--- Setting side 1 to {target_f}°F ({target_c:.1f}°C) ---")
    bed.set_temperature(side1_temp=target_c)
    bed.display_state()
    
    # Example 4: Turn off both sides
    print("\n--- Turning OFF both sides ---")
    bed.set_power(side0_on=False, side1_on=False)
    bed.display_state()
