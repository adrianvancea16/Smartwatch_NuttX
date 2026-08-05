# Smartwatch Application (NuttX RTOS)

This repository contains the source code for the smartwatch application we developed during the Google - IoT Summer School (August 2025). The goal of the project was to build a low-power smartwatch app running on an ESP32-S3 microcontroller, using the NuttX RTOS.

## My Role

I was the team lead for this project. This involved managing tasks, integrating the system, and making sure the hardware, drivers, and application layer all worked together smoothly.

## What I Worked On

My main focus was on low-level hardware interaction and implementing real-time features. Specifically:

* Set up I2C drivers for sensors like the haptic motor, accelerometer, and LSM6DSL gyroscope.
* Implemented Wi-Fi and Bluetooth Low Energy (BLE) using the NimBLE stack to connect the watch to a smartphone and handle notifications.
* Developed haptic feedback for turns/steps and set up BLE battery status alerts.
* Integrated the application logic and drivers into the NuttX RTOS environment.

## Application Modules

### Hybrid Watch Face (`hacktor_watch_main.c`)
This is the main entry point and user interface, built with LVGL. It features:
* A hybrid analog/digital display with a sweeping second hand.
* A digital time and date display.
* Custom LVGL integration with a 1-second timer for real-time updates.

### Real-Time Haptic Feedback (`turn_feedback.c`)
This module handles real-time sensor processing for physical feedback:
* Continuously polls data from the LSM6DSL gyroscope.
* Detects turns or steps by monitoring Y-axis changes against a threshold.
* Controls the haptic motor using the NuttX force-feedback driver to create different effects based on the movement.

This was also designed to work alongside the BLE stack to send notifications to a paired phone.

## Dependencies

The project relies heavily on NuttX RTOS. Everything from hardware interaction to I2C, BLE, and Wi-Fi uses the NuttX kernel and driver layer.

You can find the configurations and board support in our NuttX fork:
[radupascale/hectorwatch-nuttx](https://github.com/radupascale/hectorwatch-nuttx)

## Build & Run

To build and run the application, make sure to place the source files in the correct NuttX application directory.
