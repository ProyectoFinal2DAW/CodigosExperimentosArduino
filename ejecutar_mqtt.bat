@echo off
cd /d "C:\Program Files\mosquitto"
start cmd /k "mosquitto_sub -d -h localhost -p 1883 -t sensores/tiempo"
