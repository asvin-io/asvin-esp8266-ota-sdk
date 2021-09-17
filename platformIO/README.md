# Asvin ESP8266 OTA SDK Arduino

## Prerequisites

### PlatformIO plugin

Install PlatformIO plugin to edit, compile and upload the code to ESP8266 device. The code has been tested with v2.3.3.


## Usage

- clone the repo

```
git clone https://github.com/asvin-io/asvin-esp8266-ota-sdk.git
```

- Use the PlatformIO plugin to compile, upload and monitor.

## API Flow

IMP! : Make sure to use the Correct SHA Finterprint of the URL you are trying to access

1. Connect to WiFi
2. Login To Auth Server
3. Register Device
4. CheckRollout
5. Get CID from BlockChain server
6. Download Firmware from IPFS server and update the ESP
7. Check if Rollout was sucessful

    

