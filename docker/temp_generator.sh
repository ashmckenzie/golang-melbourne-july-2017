#!/bin/bash

while true
do
  t="$(( ( RANDOM % 25 ) + 5 ))"
  mosquitto_pub -h mosquitto -t temperature -m "${t}"
  echo "Sent temperature ${t} on $(date)"
  sleep 3
done
