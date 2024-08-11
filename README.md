# ESP32 IMU Data Logger Project

This project uses an ESP32 microcontroller to collect data from multiple IMU sensors and store it on an SD card. Data collection is managed by two FreeRTOS tasks: one for reading sensor data and another for writing this data to a file on the SD card. Additionally, the project includes a web server for control and data visualization.

## Project Structure

- **main.cpp**: Initial setup, task creation, and web server.
- **task1.cpp**: Task responsible for writing data to the SD card.
- **task2.cpp**: Task responsible for reading data from the IMU sensors.
- **tasks.h**: Headers and shared definitions between tasks.

## Features

- **IMU Sensor Reading**: Collects data from up to 5 IMU sensors using I2C.
- **Data Logging**: Stores collected data in a text file on the SD card.
- **Web Server**:
  - **/toggle**: Toggles the data collection state.
  - **/getData**: Provides the recorded data file from the SD card.

## Requirements

- ESP32
- Compatible IMU sensors (connected via I2C)
- SD card
- SD library
- ESPAsyncWebServer library
- Wire library

## Environment Setup

1. **Install the required libraries**:
   - Install the SD and ESPAsyncWebServer libraries through the Arduino IDE Library Manager or manually.

2. **Connect the Hardware**:
   - Connect the IMU sensors to the ESP32 I2C pins.
   - Connect the SD card to the ESP32.

3. **Network Configuration**:
   - Adjust the `ssid` and `password` in `main.cpp` for the desired access point.

## Compilation and Upload

1. **Open Arduino IDE** and load the project.
2. **Select the ESP32 board** and the appropriate serial port.
3. **Compile and upload** the code to the ESP32.

## Usage

1. **Connect to the ESP32 access point** created by the project.
2. **Access the web server** at the IP provided by the ESP32 (e.g., `192.168.4.1`).
3. **Control data collection** by accessing `http://<IP>/toggle`. The data collection state will toggle between on and off.
4. **Access recorded data** by navigating to `http://<IP>/getData` when data collection is off. The data file will be returned.

## Code

### `main.cpp`

Sets up the ESP32, creates tasks, and starts the web server.

### `task1.cpp`

Writes received data to a file on the SD card.

### `task2.cpp`

Reads data from IMU sensors and sends it to the message queue.

### `tasks.h`

Defines functions and global variables used by the tasks.

## Notes

- Ensure the SD card is correctly formatted and inserted.
- Adjust the queue size as needed based on available memory and project requirements.
- Check the IMU sensor connections and adjust the pins as necessary.

## Known Issues

- Performance may be impacted if the number of sensors or the queue size is too large.
- Task synchronization may need adjustments depending on workload and data collection frequency.
