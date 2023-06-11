# Project information
| Party | ID |
|-------|----|
| Participants | Me |
| Developer | Me |
| Stakeholders | My family |
| Target release date | 2023-05-01 |

The [ChickenGuard, 2019 version](https://www.youtube.com/watch?v=T8KP0sJ12n4), no longer functions correctly.  The motor only spins for a fraction of a second, and then stops.  The original electronics are covered in epoxy, and are not repairable.  
Instead of buying a new unit for a whopping €200, I will try to replace the electronics with open source, easy-to-repair hardware.

# Objective

Design a device that can pull-up and lower a chicken door, and that can be easily repaired.

# Assumptions, constraints
* The electronics need to fit in the current enclosure
* The geo-location of the device can be taken from the smartphone

# Dependencies

# Background & Strategic Fit
... Risk management

# Scope
The original design allows to control the door manually, based on sunlight and based on preset times.  These features should be present as well.

The original design requires to enter the time by pushing buttons.  This is not very user friendly.  
The device can be setup as webserver and controlled via a web interface.  Time and date can be taken from the smartphone.

The original design has a CdS light sensor, which is not RoHS compliant.  By combining the current time and date with the geolocation of the device.  The device can determine the sunrise and sunset time, and can open and close the door accordingly.  The geolocation can be taken from the smartphone, avoiding the need for a GPS module or the user having to enter the location manually. 

The current chickenguard has no clue how far the door is open.  There are no limit switches etc.  The user has to manually configure the top and bottom positions of the door.  This design will automatically detect door position based on the measured motor current.

The user might still wish to manually open and close the door.  This can be done via the push buttons.

# Product Features

## Enclosure
* PCB fits the current enclosure

### Installation
* easy to install and to remove.

## Electronics
* Two power options: through USB-jack or battery powered.
* minimum number of components

## Functionality
### Setup
1. After power on and when no config is present, the device act as wireless AP.  The user connects to the AP and as such automatically configures time and location of the device.
2. All parameters should be set from the web-interface.

### Button functionality
1. Central button : wakes up the web server
2. Buttons on either side of the central button are only for manual control of the door.

### LED functionality
1. Blink RED when power is low and battery needs to be replaced.
2. Optional : Blink GREEN when door is moving.

### Normal operation
Door opens/closes in three modes:
  * Mode 1 : user pushes up/down
  * Mode 2 : automatical by sunset, sunrise control
  * Mode 3 : automatical by preset time
Door must stop when it hits a chicken before the end of the travel.

# Release Criteria
...performance baseline

# Success Metrics
1. Solution accepted by partner

# Catalog Exclusions
* IoT integration

# Cost
* Less than €10 in BoM

