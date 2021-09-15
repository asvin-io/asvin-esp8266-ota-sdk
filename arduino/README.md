# Asvin ESP8266 OTA SDK Arduino

## Prerequisites

### Arduino IDE

Install Arduino IDE to edit, compile and upload the code to ESP8266 device. The code has been tested with Arduino 1.8.5.

### ESP8266 Code

Install ESP8266 core. Tested with 2.7.4

## Usage

- clone the repo

```
git clone https://github.com/asvin-io/asvin-esp8266-ota-sdk.git
```

- set sketchbook address to `arduino` folder in the cloned repo.

## API Flow

IMP! : Make sure to use the Correct SHA Finterprint of the URL you are trying to access

1. Connect to WiFi
2. Login To Auth Server
3. Register Device
4. CheckRollout
5. Get CID from BlockChain server
6. Download Firmware from IPFS server and update the ESP
7. Check if Rollout was sucessful
