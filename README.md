# ESP32 Websocket Server

![IMG_20231023_025726](https://github.com/Erik-Eki/ESP32_Websocket_Server/assets/70281449/9504b44c-970b-47c9-b2e8-2e40f6918ee7)

Simple ESP32 project that uses Websocket protocol. 

The ESP functions as a server that receives the duty cycle from the client and updates the brightness of the Built-in LED.

Every new browser instance (client) sends the server a request for the LED brightness so every client is always up-to-date.
