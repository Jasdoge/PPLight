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
