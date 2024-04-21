# Wio Terminal Office Status Panel (Alpha)

## Description

Since the pandemic, I've been working from home and needed a way to alert my family when I was in a meeting or otherwise occupied in my home office. I also have a 24U server 
rack inside my office, so environmental monitoring (such as room temperature) is important. I chose [Seed Studio's Wio Terminal](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)
as the base platform because of it's solid design, feature list, and price. Additionally, I've added temperature and gas monitoring via externally mounted sensors.

The project consists of two main components. The terminal itself is meant to be mounted on the wall outside of the office. The sensors are placed inside the office and connected to the Terminal via wire. I used CAT4 telephone wire due to ease of use,
but obviously any wire can be used. Please note that this project contains ONLY the code for the Terminal. It does not give instruction on how to attach the sensors. The Terminal WILL work without sensors, but only the clock and status functionality. Please see the "Issues" section for additional notes.

## Feature List

|Feature|Description|
|-------|-----------|
|Status | "Available" or "Busy". Can be changed either on the device itself, or via a webpage hosted by the terminal.|
|Clock| The Wio Terminal does not have an internal battery. Therefore, the app connects to any NTP server you wish to keep time.|
|Busy Minutes| Adjustable timer via the Wio Terminal 5 way switch for how long the "Busy" indicator should remain before reverting back to "Available". Default is 60 minutes.|
|Temperature & Humidity| Uses a [DHT-22 Temperature and Humidity Sensor](https://www.amazon.com/gp/product/B07XBRMN4P/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1) mounted inside the room near my server rack.|
|Air Quality| Uses an [SGP-30](https://www.amazon.com/gp/product/B0B389LQCQ/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) to monitor VOC's and CO2 levels.|
|Alarms| Configurable, 2-stage alarms for temperature and air quality. Set levels for "Warning" and "Danger". Audio alerts are played from the Wio Terminal when the alarms are tripped.|
|Webpage| Status and Sensor readings are displayed on a webpage hosted by the Wio Terminal. Status can be changed via a toggle on the page.|
|Settings| The application uses a config file located on an SD card for configuring many of the applications settings.|
|Pulse Indicator| A small dot slowly flashes in the upper right of the display. This a quick visual cue that the Terminal has not frozen.|

## Images


## Issues
As this is a hobby project, not all functions are complete and there are known issues that still need to be addressed. These include, but are not limited to:
* Some settings located in the config file do not have any effect. (ex: Daylight Savings settings, Static IP, etc.)
* Gas Sensor readings don't always display on boot up.
* The WiFi connection on bootup has been known to fail. The app handles it, but it's unclear why this happening. 
* Button functionality can be hit or miss due to how it's coded. Some buttons also do things for test purposes.
* Gas sensor readings occasionally wrap around to the left of the display.
* On a few occasions I've had erratic sensor readings display when one of the sensors is disconnected.
* C++ is not my native language, and therefore the code is not well optimized.
* When all else fails, power cycle the terminal.
