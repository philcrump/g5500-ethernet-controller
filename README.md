# G-5500 Ethernet Rotator Controller [![Build Status](https://travis-ci.org/philcrump/g5500-ethernet-controller.svg?branch=master)](https://travis-ci.org/philcrump/g5500-ethernet-controller)

Too many layers of software nowadays are used to simply point antennas. Here's to fixing that.

![GB3HV Web Interface](https://raw.githubusercontent.com/philcrump/g5500-ethernet-controller/master/images/web-screenshot-gb3hv.png) ![GB3HV Azimuth-only Board on STM32F429ZI Nucleo](https://raw.githubusercontent.com/philcrump/g5500-ethernet-controller/master/images/board-photo-gb3hv.jpg)

## Summary

This project exists as firmware and a 'shield' board on an ST NUCLEO-STM32F429ZI Development Board.

The firmware exposes a web server on port 80, through which the rotator can be monitored and commanded.

This project is built on ChibiOS 17.6.x with LwIP 1.4.1

## Wiring

**J1** - "AZ FEEDBACK"
1. G-5500 Azimuth Pin 1 (Feedback Rheostat 360-end)
2. G-5500 Azimuth Pin 2 (Feedback Rheostat Wiper)
3. G-5500 Azimuth Pin 3 (Feedback Rheostat 0-end)

**J6** - "AZIMUTH" -> "MOTOR"
1. (L) G-5500 Azimuth Pin 4 (Motor Winding 1)
2. (R) G-5500 Azimuth Pin 5 (Motor Winding 2)
3. (C) G-5500 Azimuth Pin 6 (Motor Center Tap)

## TODO

- [x] Flash Storage of settings (on settings branch)
- [ ] Interface to edit presets

### Possible future features

- [ ] Merge in satellite tracking computations from old project (using NTP, GPS, & libpredict)
- [ ] Update to ChibiOS 18.2.x (There seems to be a DHCP Client bug - needs investigation)