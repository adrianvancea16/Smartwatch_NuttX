# ⌚ Smartwatch Application Development (NuttX RTOS)

This repository contains the core application source code developed during the **Google - IoT Summer School (August 2025)**.  
The project focused on building a **feature-rich, low-power smartwatch application** running on the **ESP32-S3 microcontroller** using the **NuttX Real-Time Operating System (RTOS)**.

---

## 👨‍💻 My Role and Key Contributions
I served as the **team lead**, managing task distribution, system integration, and ensuring seamless operation between hardware components, low-level drivers, and the application layer.

---

## 🔧 Core Technical Contributions
My work was centered on **low-level hardware interaction** and **real-time feature implementation**:

- **Low-Level Driver Programming**: Implemented and configured I2C drivers for key sensors, including:
  - Haptic motor  
  - Accelerometer  
  - Gyroscope (**LSM6DSL**)  

- **Network Protocol Implementation**: Programmed Wi-Fi and **Bluetooth Low Energy (BLE)** using the **NimBLE stack**, enabling smartphone integration and real-time alerts.

- **Real-Time Features**: Developed haptic alerts for turns/steps and reliable BLE battery status notifications.

- **System Integration**: Ensured application logic and drivers were correctly integrated into the NuttX RTOS framework.

---

## 🧩 Application Modules

### 1. Hybrid Watch Face (`hacktor_watch_main.c`)
This C file implements the **main application entry point** and the **user interface** for the smartwatch, using the **LVGL (Light and Versatile Graphics Library).**

**Features:**
- **Analog/Digital Hybrid Display**: Classic analog clock with hour, minute, and sweeping second hands.  
- **Time Synchronization**: Displays digital time (`HH:MM:SS`) and current date (`Day, DD Mon YYYY`).  
- **Custom LVGL Integration**: Creates LVGL objects (clock hands, markers) and real-time display updates via a 1-second timer.  

---

### 2. Real-Time Haptic Feedback (`turn_feedback.c`)
This module demonstrates **real-time sensor processing** to generate physical feedback.

**Functionality:**
- **Sensor Polling**: Continuously reads gyroscope data from the **LSM6DSL** sensor (`/dev/lsm6dsl0`).  
- **Turn/Step Detection**: Monitors Y-axis data against a threshold (`THRESHOLD`) to detect rapid directional changes.  
- **Haptic Motor Control**: Uses NuttX force-feedback driver (`/dev/input_ff0`) to trigger haptic effects of different intensity/duration.  

> ⚡ Designed to work with the BLE stack to send notifications to a paired smartphone.

---

## ⚙️ NuttX RTOS Dependency
A core aspect of this project is its **deep reliance on NuttX RTOS**.  
All hardware interaction, task management, and communication protocols (**I2C, BLE, Wi-Fi**) are built upon the **NuttX kernel and driver layer**.

The applications use configurations and board support defined in the associated NuttX fork:

🔗 **NuttX Repository**: [radupascale/hectorwatch-nuttx](https://github.com/radupascale/hectorwatch-nuttx)

---

## 📦 Build & Run
To successfully build and run this application:  
- Place the source files in the correct NuttX application directory:  

