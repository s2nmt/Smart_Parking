# Smart Parking
This project is a part in my Graduation Project. It focus on smart parking.
## Introduction 
The project includes PCB design, firmware development for the MCU, training a YOLOv5 model for license plate detection, and building detection software.
## Project Components

1. Hardware
- MCU STM32F103C8T6
- WiFi Module: RTL8720DN
- RFID/NFC Module: PN532
- IC MAX485 auto control
- Servo
<p align="center">
  <img src="4.%20Image/pcb.jpg" alt="Alt text" width="80%"/>
</p>

2. Firmware
- Utilizes AT commands with RTL8720DN via UART.
- Comminicates with PN532 via SPI.
- Controls servo using PWM.
- Manages data transmission and reception with IC MAX485 via UART.
<p align="center">
  <img src="4.%20Image/case.jpg" alt="Alt text" width="80%"/>
</p>
3. Software

- Trainning YOLO v5 model for license plate detection.

- Buid software interface license plate detection.

**Author** Minh Tuan

**Date** October 12, 2024
