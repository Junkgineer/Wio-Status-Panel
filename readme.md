# Wio Terminal Office Status Panel
---
## Description
---
Since the pandemic, I've been working from home and needed a way to alert my family when I was in a meeting or otherwise occupied in my home office. I also have a 24U server 
rack inside my office, so environmental monitoring (such as room temperature) is important. I chose [Seed Studio's Wio Terminal](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)
as the base platform because of it's solid design, feature list, and price. Additionally, I've added temperature and gas monitoring via externally mounted sensors.

The terminal is meant to be mounted on the wall outside of the office, and contains the following features:

|Feature|Description|
|-------|-----------|
|Status | "Available" or "Busy". Can ebe changed either on the device itself, or via a webpage hosted by the terminal.|
|Busy Minutes| Adjustable timer via the Wio Terminal 5 way switch for how long the "Busy" indicator should remain before reverting back to "Available". Default is 60 minutes.|
|Temperature| Uses a DHT-21 Temperature and Humidity sensor mounted inside the room near my server rack.|