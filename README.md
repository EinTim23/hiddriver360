# HidDriver 360
Experimental on-console HID driver focussing on providing third party controller support to the xbox 360 without USB patches or dongles.

## What works
- Supports both 17559 retail and 17489 devkit dashboards
- Reading controller input via USB
- Up to 4 concurrent controllers, in combination with original xbox 360 controllers or without them
- Controllers are registered inside xam and therefore fully recognized in the entire xbox 360 user interface, ring of light will update accordingly, no original controllers are needed etc
- The controller works in all tested games

## Current limitations
- no rumble support

## Supported controllers
- Dualsense (PS5 controller)
- DualShock 4 (PS4 controller)
- more to come, as its quite easy to extend

## How to build
1. Acquire the official Xbox 360 SDK using black magic
2. Install visual studio 2010 ultimate and visual studio 2019
3. Install the sdk using the "FULL" preset
4. Open the solution in visual studio 2019 and build
5. Hopefully enjoy :)

## Showcase
https://github.com/user-attachments/assets/f090e5f4-538d-457f-8189-2c5b98579984


## Credits
- EinTim23 -> Reverse engineering the xbox 360s HID and controller implementation and implementing this driver
- [localcc](https://github.com/localcc/) for providing help about low level USB related questions and beaming the idea of doing this into my head



