IoT RTOS Project - YoloUNO PlatformIO Extension
============================================================

**Course:** Internet of Things Project  
**Deadline:** 27/11/2025  

**Base Project:**  
https://github.com/nhanksd85/YoloUNO_PlatformIO/tree/RTOS_Project  

**Forked Repository:**  
https://github.com/nguyenhuy0206/YoloUNO_PlatformIO


------------------------------------------------------------
Team Members
------------------------------------------------------------
| Name                | Student ID | Email                     | Role / Responsibility                         |
|---------------------|------------|----------------------------|-----------------------------------------------|
| Nguyen Minh Huy     | 22Cxxxxx   | huy@example.com            | Task 1–3: LED, LCD Display, Semaphore Logic   |
| Bui Quoc Bao        | 22Cxxxxx   | bao@example.com            | Task 4 & 6: Web Server + CoreIoT Integration  |
| Nguyen Tan Huy     | 22Cxxxxx   | dang@example.com           | Task 5: TinyML Model + Evaluation + Report    |

------------------------------------------------------------
1. Project Overview
------------------------------------------------------------
This project extends the YoloUNO RTOS project on the ESP32-S3 platform.
We redesigned and implemented six main tasks to demonstrate real-time
multitasking, sensor integration, web control, cloud connectivity, and
TinyML inference on an embedded microcontroller.

------------------------------------------------------------
2. Implemented Tasks
------------------------------------------------------------
Task 1: LED Blink with Temperature Conditions
- LED behavior changes based on three temperature ranges.
- Semaphore used to synchronize LED and sensor task.

Task 2: NeoPixel LED Control Based on Humidity
- RGB LED color reflects humidity level (Low/Medium/High).
- Synchronized updates using semaphores.

Task 3: LCD Display with Semaphore-Controlled States
- LCD shows three system states: Normal, Warning, Critical.
- No global variables are used (pure RTOS semaphore synchronization).

Task 4: Web Server in Access Point Mode
- Redesigned HTML interface for better user experience.
- Provides two control buttons (LED1, LED2) and live status display.

Task 5: TinyML Deployment and Evaluation
- TinyML model trained for simple sensor-based classification.
- Deployed to ESP32-S3 using TensorFlow Lite Micro.
- Accuracy and inference time evaluated on hardware.

Task 6: CoreIoT Cloud Integration
- Sensor data (temperature, humidity) published to CoreIoT platform.
- Device configured in Station (STA) mode with proper token authentication.
- Dashboard available on https://app.coreiot.io/

------------------------------------------------------------
3. Project Structure
------------------------------------------------------------
```text

YoloUNO_PlatformIO/
├── src/
│   ├── main.cpp
│   ├── task_led.cpp
│   ├── task_neopixel.cpp
│   ├── task_lcd.cpp
│   ├── task_webserver.cpp
│   ├── task_coreiot.cpp
│   ├── task_tinyml.cpp
├── include/
├── data/              # Web UI files (HTML/CSS)
├── model/             # TinyML model (.tflite)
├── doc/
│   └── IoTProject_Report.pdf
└── README.txt
```

------------------------------------------------------------
4. Development Environment
------------------------------------------------------------
- Platform: ESP32-S3
- Framework: Arduino + FreeRTOS
- IDE: Visual Studio Code + PlatformIO
- Dependencies:
  + Adafruit NeoPixel Library
  + DHT Sensor Library
  + LCD 1602 / I2C Library
  + WiFi + WebServer
  + TensorFlow Lite Micro (TinyML)
  + CoreIoT SDK / MQTT Client

------------------------------------------------------------
5. Git Workflow
------------------------------------------------------------
Main Branch: main
Development Branch: RTOS_Project
Each member develops in a separate branch:
- task_led_lcd_member1
- task_web_coreiot_member2
- task_tinyml_member3

Use Pull Requests to merge verified code into RTOS_Project.

------------------------------------------------------------
6. Report and Submission
------------------------------------------------------------
Deliverables:
- PDF Report (IoTProject_Report.pdf)
- GitHub Repository link
- Demo video

Report Sections:
1. Introduction and Objectives
2. Description of All Implemented Tasks
3. Implementation Highlights (Semaphore, Web Server, TinyML)
4. Experimental Results and Evaluation
5. Discussion and Conclusion
6. Team Roles and Contributions

------------------------------------------------------------
7. Acknowledgment
------------------------------------------------------------
Based on the YoloUNO RTOS Project by Nhan KSD.
All code rewritten and extended under group requirements.
Special thanks to instructors and CoreIoT support team.

============================================================
