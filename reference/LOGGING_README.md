# Sleepy Pad Temperature Logging

## What Gets Logged

Every 5 minutes, the app automatically records:
- Timestamp
- Side 0 (Left) current temperature (°C)
- Side 0 target temperature (°C)
- Side 0 power status (on/off)
- Side 0 water level warning
- Side 1 (Right) current temperature (°C)
- Side 1 target temperature (°C)
- Side 1 power status (on/off)
- Side 1 water level warning
- System error status

## Log File Location

The log is saved as `sleep_temps.csv` in the same folder as `bed_web_app.py`

## Opening and Viewing Logs

### In Excel or Numbers:
1. Open Excel/Numbers
2. File → Open
3. Select `sleep_temps.csv`
4. The data will appear in columns

### Creating Graphs:
1. Select the timestamp and temperature columns
2. Insert → Chart → Line Graph
3. You can now see temperature trends over time!

## Example Log Data

```
timestamp,side0_current_temp_c,side0_target_temp_c,side0_power,side0_water_low,side1_current_temp_c,side1_target_temp_c,side1_power,side1_water_low,system_error
2026-05-20 14:35:00,21.5,21.0,True,False,22.1,22.0,True,False,False
2026-05-20 14:40:00,21.3,21.0,True,False,22.0,22.0,True,False,False
2026-05-20 14:45:00,21.1,21.0,True,False,22.1,22.0,True,False,False
```

## Changing Log Interval

To log more or less frequently, edit `bed_web_app.py`:

```python
LOG_INTERVAL = 300  # 5 minutes (in seconds)
```

Change to:
- `60` for every 1 minute
- `180` for every 3 minutes
- `600` for every 10 minutes
- `1800` for every 30 minutes

## Tips

- The log file grows over time - one entry every 5 minutes = ~288 entries per day
- You can delete old log files anytime - a new one will be created automatically
- Great for tracking:
  - How long it takes to reach target temp
  - Temperature patterns during sleep
  - How different presets perform
  - Whether temps hold steady overnight
