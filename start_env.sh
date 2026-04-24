#!/bin/bash

ngrok http 8088 > /dev/null 2>&1 &
sleep 2
NGROK_URL=$(curl -s http://127.0.0.1:4040/api/tunnels | jq -r '.tunnels[0].public_url')

if [ -z "$NGROK_URL" ]; then
	echo "Error"
	exit 1
else
	echo "The URL is ready: $NGROK_URL"
fi
