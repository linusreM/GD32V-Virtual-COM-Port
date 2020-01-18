<br>

## Test for GD32VF103 USB CDC (virtual COM port).

<br>

This test is based on [CDC_ACM Example](https://github.com/riscv-mcu/GD32VF103_Demo_Suites/tree/master/GD32VF103C_START_Demo_Suites/Projects/04_USBFS/Device/CDC_ACM) from [GD32VF103_Demo_Suites](https://github.com/riscv-mcu/GD32VF103_Demo_Suites) repository which has some issues when compiled under Platformio environment.

The demo is intended to be run on **Longan Nano** board. If testing on another board (without a display), edit the source file `src\app.c` and remove all display related commands.

The changes made to make it compile without issues, fix some bugs and make a better demo of the possible usage:

* The USB drivers are moved into the project source/include directories.
* There was also a missing include in cdc_usb_core.c which was added.
* USB descriptors are slightly changed so that the device does not apear as **modem device** which activates the **Modem Manager** on Linux.
* LCD and RGB LED driver from Sipeed's [Longan_GD32VF_examples](https://github.com/sipeed/Longan_GD32VF_examples) is used to make a LCD demo (some bugs was fixed and Chinese comments translated to English)

### How to test

After the demo is compiled and flashed to the Longan Nano board start some terminal emulator (minicom, putty, ...) connected to `/dev/ttyACM0` (under Linux).

* type some characters (or copy/paste)
* printable characters will be echoed back, non printable characters will be echoed as `.`
* press `Return` key to display the entered characters on Longan Nano display (both `Ctrl+M` and `Ctrl+J` are accepted as line ends)
* up to 100 character can be accepted and shown on the display, if more characters are entered, they will be ignored and echoed as `>` (the number of overflow characters will be displayed when the string is shown on the display)
* some control caharcters are accepted which has the following effects:
  * `Ctrl+R` change the text color and RGB LED color to `RED`
  * `Ctrl+G` change the text color and RGB LED color to `GREEN`
  * `Ctrl+B` change the text color and RGB LED color to `BLUE`
  * `Ctrl+C` change the text color and RGB LED color to `CYAN`
  * `Ctrl+Y` change the text color and RGB LED color to `YELLOW`
  * `Ctrl+W` change the text color and RGB LED color to `WHITE`
  * `Ctrl+D` clear screen
  * `Ctrl+L` display Sipeed logo
  * `Ctrl+S` display status (Sipeed logo, number of received and sent characters, number of received strings)
