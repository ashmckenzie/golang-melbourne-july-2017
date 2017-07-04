#!/bin/bash

apk --update add bash mosquitto-clients
npm install -g chance-cli

while true
do
  # lat:-37.817503 lon:144.9617536
  now=$(date "+%s")
  lat="-$(chance latitude --min 35 --max 38)"
  lng=$(chance longitude --min 143 --max 145)
  mosquitto_pub -h mosquitto -t owntracks -m '{"_type":"location","tid":"s8","acc":20,"batt":91,"conn":"w","lat":'${lat}',"lon":'${lng}',"tst":'${now}'}'
  echo "Sent lat:${lat}, lng:${lng} on $(date)"
  sleep 3
done
