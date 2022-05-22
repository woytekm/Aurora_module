Use case of Aurora module - bicycle tail light with bluetooth connectivity.

Light is water resistant, has at least 500m visibility in bright daylight and has 2500 mah of battery life. 
It is a bit bulky as for modern rear bike light, but in exchange - in flashing mode it w will last for 83 hours, which should last for 10 day trip (8 hours of daily riding), 
or for more than a month of daily 2h commuting by bike. This version is equipped with tact switch module (three switches) for interaction.

One of interesting side applications of this device, is it's ability to gather road quality data by combinig GPS location with accelerometer data.
Idea is to create database of data points consisting of latitude,longitude registered by GPS and verical shock registered by sensor at this place. 
This data can then be aggregated and visualized as a map of road surface quality.

Aurora onboard flash is accessible through USB, and can be mounted read-only on USB host.
OS X command: 
```
	sudo diskutil mount readOnly /dev/disk4s1
```

GPX data can be then extracted and converted to data points using script located in Aurora_module/applications/BLS2-rear/pothole directory.

Interesting OpenLayers API routine allowing to colorize tracks using custom data contained in GPX: http://viglino.github.io/ol-ext/examples/style/map.style.gpxline.html

Example road quality track prepared in GPS Visualizer ( https://www.gpsvisualizer.com/ )

<p align="center">
<img src="https://github.com/woytekm/Aurora_module/blob/main/applications/BLS2-rear/example_road_quality_track.png" width="800" height="600">
</p>

<p align="center">
<img src="https://github.com/woytekm/Aurora_module/blob/main/applications/BLS2-rear/bls2-rear.png" width="800" height="345">
</p>

