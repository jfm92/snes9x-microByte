# Snes9x port for microByte

This is a port of SNES9X emulator for the microByte. It's on a very early stage and can be considered a experiment. 

## What it's working?

- You can execute any game with a size equal or less of 2MB a with no special chip, like DSP.
- All the buttons are functional.

## What it's not working?

- No save or load data.
- No sound. 
- No select game menu.
- Low performance.

## How to use it?

- Download the binary file at: [Byte mix lab app folder](http://bytemixlab.com/resources?preview_id=291&preview_nonce=72ad8b72ae&preview=true)
- Copy to the folder apps of the micro-SD card.
- Create a folder called SNES on the micro-SD card.
- Copy your game to the folder.
- Change the name of the game to ``game.sfc``
- Turn on the microByte, go to external apps, and execute SNES9x-MB.bin.

Credits:

This port was initialia done by Ducalex for the Odroid-GO, you can check the original repo here:
https://github.com/ducalex/snes9x-esp32
