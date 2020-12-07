# Usage:

1. The lamp automatically turns on when you move it
2. The lamp starts fading out after being still for 30 seconds, fully turning off after another 30 seconds.

## Charging:

1. Plug it to a 5V USB outlet with a Type-C cable.
2. The battery only charges at 100mA, so even a computer port will be able to charge it.
3. While charging, the lights will fade in and out with a pause between.
4. When fully charged, the lights will turn off.
5. A fully discharged battery reaches 90%+ charge in 2 hours.

## Troubleshooting:

1. After charging, you may need to led the device be still for a few seconds before it can turn on again.
2. If the device still won't turn on, you may need to reset it (this should be exceedingly rare though, and I didn't manage to reproduce it myself after setting the proper BOD). To reset it, open the case by removing the lid (but be careful since the silk filament is a little brittle), and use a fork or knife to briefly short the BAT1 solder joints.


# 3d printing

The 3d print files are located in the Prints director. It's designed for a multi material printer where you want to combine the lid and logo into one mesh, giving the logo priority.

# Schematics

Schematics are located within the Kicad directory.


# Reprogramming

If you want to reprogram your light you'll need the following:

- An arduino (pref arduino UNO)
- A breadboard
- A 10uF capacitor
- A 4.7 kOhm resistor.
- 8-pin ICP clips.

1. Install the MegaTinyCore library in arduino. (If you haven't installed Arduino, do so at https://www.arduino.cc/
2. Then follow the instructions here: https://github.com/SpenceKonde/megaTinyCore/blob/master/MakeUPDIProgrammer.md But **do not connect the power pin to the PPLight**. In the tutorial they use a 470 ohm resistor on the UPDI wire, you can use a 4.7k instead, it doesn't matter that much.
3. Connect pin 6 of your arduino to your breadboard, then to pin 6 of the clips (second from the bottom right, the red wire on the ICP clips is top left).
4. Connect ground to the top right of your test clips.
5. Connect the clips to the attiny on the board. The chamfered edge is left. Align it so the red cable on the clips is top left.
6. Use the following arduino settings:
- Board: ATtiny412/402/212/202
- Chip: ATtiny402
- Clock: 1 MHz internal
- millis()/micros(): Enabled (default timer)
- Support SerialEvent: No
- Port: Port of your arduino
- Programmer: jtag2updi
- The rest don't matter. Don't burn bootloader unless you know what you're doing.
7. You should now be able to upload your code.







