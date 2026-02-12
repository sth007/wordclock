#!/bin/bash
screen -d -m -S esp_monitor /dev/cu.usbmodem14201 115200
sleep 1
screen -S esp_monitor -p 0 -X stuff "$(echo -ne '\r')"
sleep 1
screen -S esp_monitor -p 0 -X hardcopy /tmp/esp_output.log
cat /tmp/esp_output.log
screen -S esp_monitor -X quit
