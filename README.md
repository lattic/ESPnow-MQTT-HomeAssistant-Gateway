<h1> ESPnow MQTT Home Assistant Gateway </h1>
<h2> SKETCHES MOVED TO PLATFORMIO! </h2>
<h2> Update 2023-02-23:</h2>
<br>
New end device - push buttons - implemented</br>
Example: small box with 6 push buttons to control the devices around the house (i.e. switches, lights, etc.)</br>
Powered with small 500mAh battery, sleeps all the time until button pressed.</br>
Sends button number (that was pushed) to Home Assistant and then Node Red acts accordingly</br></br>
<img width="510" alt="Screenshot 2023-02-24 at 06 29 26" src="https://user-images.githubusercontent.com/46562447/221108210-a48b4102-6a19-4d7d-ab59-d20b0c0a672f.png">

</br>
Still using ESPnow to send data to Home Assistant via gateway</br>
ESP32 is too slow (600ms) - only ESP32-S2 does the job properly (100ms)</br></br>
<img width="979" alt="Screenshot 2023-02-24 at 08 52 40" src="https://user-images.githubusercontent.com/46562447/221134962-d25dbf69-0d07-4138-8d31-2d20ccbfb63c.png">


<br>
<h2> Update 2023-02-17:</h2>
<br>
Force connection to specific gateway - see below commands: 21, 22 and 23</br>
All commands to sensor devices are available in Node Red dashboard - no more hustle 
<br>
<img width="1053" alt="Screenshot 2023-02-17 at 11 48 03" src="https://user-images.githubusercontent.com/46562447/219646236-68ca9795-8753-4aa8-8cf1-a1e3d07dd2b2.png">
<img width="1070" alt="Screenshot 2023-02-17 at 11 47 44" src="https://user-images.githubusercontent.com/46562447/219646257-c54ecdb5-cac6-4c81-b959-ab73cfad1860.png">

<img width="1074" alt="Screenshot 2023-02-17 at 11 47 35" src="https://user-images.githubusercontent.com/46562447/219646275-b14fa555-819c-479a-8b71-997a13372084.png">

<br>
<h2> Update 2023-02-12:</h2>
<br>
LEDs on sensor devices can be turned OFF/ON by MQTT command. If not OFF, then automatically dimmed by measured light.


<br> Implemeneted MQTT commands to receiver (to act on sensor devices):
<ul>
<li> - topic:    
<li>esp32030/cmd/cmd_for_sender - command shall be send to all gateways as nobody knows which gw controls which sensor 
<li> - message:
<li>{"mac":"xx:01:01:01:zz:yy","command":"1"} - OTA   
<li>{"mac":"xx:01:01:01:zz:yy","command":"2"} - restart
<li>{"mac":"xx:01:01:01:zz:yy","command":"3"} - captive portal
<li>{"mac":"xx:01:01:01:zz:yy","command":"4"} - factory reset
<li>{"mac":"xx:01:01:01:zz:yy","command":"10"} - Motion OFF
<li>{"mac":"xx:01:01:01:zz:yy","command":"11"} - Motion ON
<li>{"mac":"xx:01:01:01:zz:yy","command":"21"} - force connection to GW1
<li>{"mac":"xx:01:01:01:zz:yy","command":"22"} - force connection to GW2 (if exists)
<li>{"mac":"xx:01:01:01:zz:yy","command":"23"} - force connection to GW3 (if exists)
<li>{"mac":"xx:01:01:01:zz:yy","command":"200"}  - LEDs completely OFF 
<li>{"mac":"xx:01:01:01:zz:yy","command":"201"}  - LEDs dimm automatically by ESP: lux below 1 ->dc=1, lux between 1 and 10 => dc=10, lux>10 => dc=255 
</ul>
<br><br>
<h2> Update 2023-01-26:</h2>
<br>
OTA is here for sleeping devices. Please check changelog.txt on how to.
<br><br>
<h2> Update 2022-12-31:</h2>
<br>
After 160 days on 1 battery charge, the battery status is as below: 38.66% and 3.8V. But I am recharging it now and putting there on 1st January 2023 to have easy calculation for the entire 2023 - maybe on 1 charge it will work the whole year? Let's see in January 2024... ;-)
<br><br>
<img width="874" alt="Screenshot 2022-12-29 at 08 58 59" src="https://user-images.githubusercontent.com/46562447/209928426-62d2dccf-9bdd-463a-887e-6e0784e1f3b3.png">
<br>
<h2> Idea:</h2>
<ul>
  <li>Build universal sensor device (ESP32 based) equipped with temperature, humidity and light sensors, that can live long on 1 battery charge. <b>EDIT: added "motion only device" that has only motion detector - PIR - temperature, humidity and light sensors are OFF</b></li>
  <li>Clone/deploy it to as many sensors as needed (indoor, outdoor etc.) without changing the software or reconfiguration (beyond what is specific to the device i.e. GPIO connection etc.)
  <li>Build gateway device (that is always powered ON) that transfers data from sensor devices to Home Assistant and automatically creates devices/entities on Home Assistant (no configuration needed). Building multiple gateway devices (and locating them in various places) allows widening the range, without loosing the data when RSSI is very low (extending the range).
</ul>
<br>
<h3>Tasks for the devices:</h3>
<ul>
  <li>Sensor devices: (battery powered) wake up, measure the environment and battery, send to gateway over ESPnow, go to sleep, wake up after specified period and repeat
  <li>Gateway device: (always ON) receive message over ESPnow from sensors, convert to Home Assistant auto discovery code and send to Home Assistant over MQTT/WiFi
  <s><p><b>EDIT:</b>Gateway device, since it is always powered ON, got also motion detector (microwave) so it can be also a part of the home alarm system - REMOVED from gateway due to interferences with WiFi - see "sender-motion-only" folder</s>
  <p><b>EDIT:</b> Motion sensor implemented in sensor device - it works great now
</ul>
<br>
<h3> To satisfy the requirements I've chosen: </h3>
<ul>
  <li>SHT31 temperature and humidity sensor, SDA, around 1.5$ on Aliexpress (breakout)
  <li>TSL2561 light sensor, SDA, around 1.5$ on Aliexpress (breakout)
  <li>MAX17048 LiPo battery sensor, SDA 1-5$ (depending if chip only or breakout)
  <li>ESP32S-WROOM or ESP32-S2 WROOM, around 2$ on Aliexpress (bare metal - <b>do NOT use the development board with additional components! - they cannot go to uA due to the on board elements, i.e. LDO, LED etc.)</b>
  <li>few additional components as per schematics (see below)
</ul>
<br>
<h3>Final schematic (ESP32-S2 based):</h3>
<img width="1088" alt="Screenshot 2022-07-07 at 21 45 12" src="https://user-images.githubusercontent.com/46562447/177868190-8f3dac36-4d2f-49e9-b07d-a71ff26dd909.png">
  <br> With PIR sensor (motion detector)
  <img width="1035" alt="Screenshot 2022-09-20 at 06 46 07" src="https://user-images.githubusercontent.com/46562447/191176937-c55b9155-91a6-47ca-bb4a-603c4fac15da.png">


<br>
<br>
<br>
<h3>Final PCB (ESP32-S2 based) - design:</h3>
<img width="1299" alt="Screenshot 2022-07-07 at 21 27 09" src="https://user-images.githubusercontent.com/46562447/177865693-10016c07-eb08-447c-9519-59ccc6162da4.png">
<br>
<img width="1037" alt="Screenshot 2022-07-07 at 21 36 29" src="https://user-images.githubusercontent.com/46562447/177866919-afb88319-9ee9-4c2e-9bfc-9122b748b7b3.png">

<br>
<h3>Preparing the boards to solder:</h3>
<img width="1221" alt="preparation" src="https://user-images.githubusercontent.com/46562447/180612901-c6468ebf-43ef-4aff-9153-c8fa3c386690.png">



<br>
<h3>Soldered on hot plate:</h3>
<img width="1004" alt="soldered" src="https://user-images.githubusercontent.com/46562447/180612912-b02b499b-4335-4df5-aa38-8d840106d9a7.png">

<br>
<h3>SHT31 - temperature and humidity sensor:</h3>
<img width="721" alt="sht" src="https://user-images.githubusercontent.com/46562447/180615851-c810309d-3509-4d79-b8c0-36f1eb9ef930.png">

<br>
<h3>TSL2561 - light sensor:</h3>
<img width="731" alt="tsl" src="https://user-images.githubusercontent.com/46562447/180615863-af1f1b5c-3b35-44e4-bff1-814092fb4a18.png">

<br>
<h3>Testing all before putting into the box:</h3>
<img width="1158" alt="testing" src="https://user-images.githubusercontent.com/46562447/180615874-d182682f-36ac-4bc9-83c6-fdc08481ce72.png">

<br>
<h3>Final product:</h3>
<img  src="https://user-images.githubusercontent.com/46562447/182552359-3cb3451d-04d7-43d8-b472-28fa80eab430.jpg">


<br>

<br>
<h2>Power consumption and management</h2>
<h3> Sensor device is powered with LiPo battery and equipped with TP4056 USB-C charger.</h3>
It can be connected to solar panel or - from time to time if possible/needed - to USB-C charger.
<br>
To minimise the sleep current, the power for all sensors is drawn from one of the ESP32 GPIO, so during the sleep time there is no current leakage - sensors are not powered up.
With the above configuration the sleep current is as following (measured with PPK2):
<ul>
  <li>ESP32-S  WROOM - 4.5uA
  <li>ESP32-S2 WROOM - 22.5uA
</ul>
You would say: "ok, ESP32-S is the winner!" but wait, working time and current are as following:
<ul>
  <li>ESP32-S  WROOM - 600ms, 50mA average
  <li>ESP32-S2 WROOM - 200ms, 38mA average
</ul>

ESP32-S WROOM working time and current:
<br>
<img width="1635" alt="s working time 1-2022-06-17 at 20 33 04" src="https://user-images.githubusercontent.com/46562447/174502504-b3937797-6bf0-40bb-af8e-29bc7e98cdff.png">

ESP32-S2 WROOM working time and current:
<br>
<img width="1667" alt="s2 working time 1-2022-06-18 at 12 51 43" src="https://user-images.githubusercontent.com/46562447/174502510-bb3200b7-647a-4269-9ca5-2978d3c97793.png">


And that is what really matters with the battery life time calculation, because 3 times shorter working time makes the difference.<br>
<h4>Rough and rounded up calculation (1000mAh battery, reserve capacity=20%, working time as above, sleep time=180s) shows as following:</h4>
<ul>
  <li>ESP32-S  WROOM: 4 700 hours with average 170uA = 195 days, 6.5 months
  <li>ESP32-S2 WROOM: 12 370 hours with average 65uA = 515 days, 17 months
</ul>

<br>
<b>ESP32-S WROOM battery life time calculation:</b>
<br>

<img width="454" alt="Screenshot 2022-06-19 at 22 20 17" src="https://user-images.githubusercontent.com/46562447/174500944-ad46fd8c-fa2e-4983-8f97-e45409a844da.png">


<br>
<b>ESP32-S2 WROOM battery life time calculation:</b>
<br>

<img width="435" alt="Screenshot 2022-06-19 at 22 20 59" src="https://user-images.githubusercontent.com/46562447/174500948-ce6a89dc-d225-4094-a5b6-923444074d7f.png">
<br>
So apparently the winner is <b>ESP32-S2 WROOM</b> with almost triple battery life.
<p>
<b>EDIT:</b> Final product as per design above consumes while sleeping 33uA - I have included LDO to make sure I have stable 3.3V - unfortunately it takes 8uA - still, as described above, the clue is in the very short working time rather than few uA here or there during the sleep - we are still talking above 9 200 hours on 1 battery charge - more than a year (383 days).
<p>
<br>
  <img width="470" alt="Screenshot 2022-07-23 at 18 24 08" src="https://user-images.githubusercontent.com/46562447/180616082-d279e82b-7feb-4438-9cb7-816fcb115e12.png">

<br>
<br>
<h3>Measuring the working time</h3>
To measure working time you shall NOT rely only on millis or micros. I.e. ESP32S reports millis as first line in setup() as xx while ESP32-S2 shows 280ms.
And actually both are wrong as you can see below on the screens:
<br>
<br>
<b>ESP32S times measured:</b>
<br>
<img width="1728" alt="esp32s" src="https://user-images.githubusercontent.com/46562447/175294471-01d2a13f-5269-493d-ad02-d072cb563ad3.png">

<br>
<br>
<br>
<b>ESP32S-2 times measured. And on the left side the correct ontime reported by the sketch with error correction applied:</b>
<br>
<br>
<img width="1707" alt="s2" src="https://user-images.githubusercontent.com/46562447/175366621-a6fbe7bb-2338-48c7-8939-bfce6faa7541.png">

<br>
<br>
I used PPK2 and estimated the time the ESP32 works measuring the power consumption.
<br>
<br>
<b>Conclusions:</b>
<ul>
  <li>don't rely on millis() only
  <li>ESP32-S2 starts much faster (35ms) than ESP32S (310ms)
  <li>ESP32-S2 total working time is much shorter so the total power consumption with S2 is much lower accordingly - battery life is extended
</ul>
<br>
<br>
<b>ESP32S and ESP32-S2 ontime after 1.5 days with 180s sleep - almost 2.5-3x more "ontime" for ESP32S:</b>
<br>
<img width="1556" alt="Screenshot 2022-07-01 at 09 51 50" src="https://user-images.githubusercontent.com/46562447/176860918-338ccdcc-a744-4832-8502-c4c54a152008.png">
<br>
<br>
<img width="809" alt="Screenshot 2022-07-01 at 09 54 44" src="https://user-images.githubusercontent.com/46562447/176861422-8171adab-98db-4501-8c49-6f7d3a2e9f59.png">

<br>
<br>


<h3>Charging details</h3>
Sensor device also provides information about charging status:
<ul>
  <li>NC - not connected
  <li>ON - charging
  <li>FULL - charged
</ul>  
To achieve this, you need to connect the pins from TP4056 that control charging/power LEDs - see below - to GPIO of ESP32 - see the schematics above. <br>
<br>
<img width="643" alt="Screenshot 2022-06-19 at 21 39 22" src="https://user-images.githubusercontent.com/46562447/174499779-ea0c5474-8ac7-4ac6-acd1-df2f67e84a80.png">

<img width="626" alt="Screenshot 2022-06-19 at 21 40 27" src="https://user-images.githubusercontent.com/46562447/174499788-fd8f5c83-6684-4a11-8a63-4529930a9508.png">

<h2>Firmware update - OTA</h2>
<h3>Sensor device</h3>
To perform firmware update there are few possibilities:
<ul>
<s>  <li>double reset click - built in functions recognise double reset and if so done, performs firmware update - of course you need to visit the sensor to double click it ;-)</s> - NOT IN USE ANYMORE
<s>  <li>routine check for new firmware availability on the server: every 24h (configurable) sensor device connects to server and if new file found, performs update</s> - NOT IN USE ANYMORE
  <li>3rd: wire TX/RX to the FTDI programmer
  <li><b>EDIT:</b>4th: new schematic and code implements second button for OTA specifically   
  <li>5th: MQTT command to perform OTA after next wake up
</ul>
Sensor device during firmware update:<br>
<br>
<img width="540" alt="Screenshot 2022-06-22 at 18 55 39" src="https://user-images.githubusercontent.com/46562447/175105073-ad607c41-860e-4edd-8235-77bd8709eb10.png">
<br>
<h3>Gateway device</h3>
To perform firmware update you simply click the button "Update" on Home Assistant (in the device section of gateway) - gateway will connect to the server where the binary is stored and if file is found, it will perform firmware update and restart gateway.
I am using Apache minimal add on Home Assistant - since all sensors are in the same network where Home Assistant is, <b>there is no need for internet access for sensors (and gateway).</b>
<br>

<h2>Configuration</h2>
All sensors used in the sensor device (SHT31, TSL2561, MAX17048, checking charging status) are optional and can be disabled in configuration file.
On top of that, light sensor (TSL2561) can be replaced with phototransistor (i.e. TEPT4400) - also configurable.
In the simplest (and useless) configuration, sensor sends only... its name and RSSI level to Home Assistant.

Sleep time (configurable) is initially set to 180s (3 minutes).

Other important configurable settings (some mandatory, some optional):
<ul>
  <s><li>WiFi channel - it must be the same for ESPnow communication and gateway to AP (router) communication - gateway and sensor devices</s> - NOT IN USE ANYMORE
  <li>SSID - gateway and sensor devices (NEEDED ONLY TO PERFORM OTA  on sensor device)
  <li>password - gateway and sensor devices (NEEDED ONLY TO PERFORM OTA on sensor device)
  <li>MQTT server IP - gateway device 
  <li>MQTT username - gateway device 
  <li>MQTT password - gateway device 
</ul>
   
  
  <b> UPDATE: </b>
  <br>
  All below configuration parameters are being set up during first integration of gateway devices over Captive Portal. 
  <br>
  All pareameters can be set up also later by invoking Captive Portal (SENSOR DEVICE BELOW):
  </br>
  
<img width="899" alt="Screenshot 2023-04-07 at 10 09 44" src="https://user-images.githubusercontent.com/46562447/230581136-f2b3ec57-c3a1-4095-a11a-8f64f85f72e8.png">





All info on ESP is stored in encrypted form so not readable by "non-super-hacker" ;-)

<br><b>WHEN GATEWAY CANNOT CONNECT TO WIFI OR MQTT, IT WILL START CAPTIVE PORTAL so that you can change the mandatory parameters such us:
  <li>WiFi SSID and password
  <li>MQTT server, port, user, password
   </b></br>
<br><b>When you invoke OTA for sender device, and wifi credentials are NOT OK, device will start Captive Portal so you can update them accordingly</b></br>
   

<h2>Software</h2>
<h3>Sensor device - sender</h3>
Tasks are as described above so the code is in 1 file only: sender.ino <br>
<br>
<s>Next is the file with credentials: passwords.h (ssid, password, webserver where your firmware is stored) - this file is only used for OTA.<br>
<br>
<b>EDIT:</b></s> - SEE BELOW UPDATE
<img width="502" alt="Screenshot 2023-04-07 at 10 25 50" src="https://user-images.githubusercontent.com/46562447/230588907-0c83874e-75f5-4d84-aa21-0d479166bc19.png">

Finally there is a configuration file where you must specify details for each sensor device (such as sensors used, GPIO etc.): devices_config.h<br>
<br>
The sequence is:
<ul>
  <li>configure the device in devices_config.h
  <li>uncomment the DEVICE_ID you want to compile the sketch for in main sketch: sender.ino
  <li>compile/upload to the device
  <li>repeat the above 3 points for the next sensor device
</ul>

<br>
Sender in action (test device):<br>
<br>
<img width="484" alt="Screenshot 2022-06-22 at 18 56 08" src="https://user-images.githubusercontent.com/46562447/175104814-a21c53e7-a631-441d-b5cc-1bffd557b9a9.png">
<br>

<h3>Gateway device - receiver</h3>
Gateway tasks are more complex (as described above) so the code is split into multiple files - per function<br>
Entire configuration is in config.h file<br>
<br>
Next is the file with credentials: passwords.h (ssid, password, mqtt ip and credentials, webserver where your firmware is stored)
<br>

<br>
Receiver in action (real device):<br>
<br>
<img width="529" alt="Screenshot 2022-06-22 at 18 41 34" src="https://user-images.githubusercontent.com/46562447/175102381-c1ade15e-66b6-44f4-95fa-aba3ba5a88b8.png">

<br>

<h3>Libraries needed:</h3>
<br><b>Some of the libriaries are for specific device - not all are always needed - check the configuration file for details</b></br>
<h4>Gateway:</h4>
<br>
<img width="472" alt="Screenshot 2023-04-07 at 10 25 37" src="https://user-images.githubusercontent.com/46562447/230583764-1db20160-c8ee-4b0f-8bf0-f44068577fd1.png">

<br>
<h4>Sensor device:</h4>
<img width="502" alt="Screenshot 2023-04-07 at 10 25 50" src="https://user-images.githubusercontent.com/46562447/230583802-bb2f1fdf-2315-44ad-af18-47eb8c3d7952.png">

<br>


<h2>Home Assistant</h2>
<h3>Information from sensor device on Home Assistant:</h3>
<ul>
  <li>device name
  <li>temperature [C]
  <li>humidity [%]
  <li>light [lux]
  <li>battery in [%] and [Volt]
</ul>

Additionally few diagnostic information:
<ul>
  <li>wifi signal/rssi [dBm]
  <li>firmware version
  <li>boot number
  <li>[cumulative] ontime (in seconds) for further postprocessing i.e. in Grafana - it resests to 0 when charging starts
  <li>[cumulative] pretty ontime, i.e.: "1d 17h 23m 12s" (human readable) - it resests to 0 when charging starts
</ul>
<br>
<b>Ontime is calculated</b> based on millis (time just before hibernation - start time) including empirically established difference between real startup time and measured with PPK2 (apparently both: ESP32 and ESP32-S2 are "cheating" with millis, however S2 shows hundreds of millis just after start, while ESP32 shows very low value after start - both are innaccurate and mainly depend on the size of the binary file, that has to be loaded into ESP32 memory during startup by bootloader).
Ontime is also reduced by turning off the bootloader logo during wake up from sleep.
<h4>Gateway device (with its entities) as well as all sensor devices (with their entities) are automatically configured in Home Assistant using MQTT discovery</h4>
<br>
<br>
<b>Gateway device as discovered by Home Assistant:<br>
<br>

<img width="695" alt="Screenshot 2023-02-17 at 11 29 37" src="https://user-images.githubusercontent.com/46562447/219640220-d505aeb7-781f-4ccb-8e5c-23c41ab516f1.png">


<br><br>
<b>Gateway device on lovelace dashboard:<br>
<br>

<img width="337" alt="Screenshot 2023-02-17 at 11 28 08" src="https://user-images.githubusercontent.com/46562447/219639838-aef4fced-90cb-4da4-8a0f-981c0b58fe24.png">


<br><br>
<b>Sensor device as discovered by Home Assistant:<br>
<br>

<img width="687" alt="Screenshot 2023-02-17 at 11 25 52" src="https://user-images.githubusercontent.com/46562447/219639310-1aa8ee80-1a1a-4e67-9c69-e6e585ac8109.png">


<br><br><br>
<b>Sensor Device on lovelace dashboard:<br>
<br>

<img width="390" alt="Screenshot 2022-07-04 at 09 11 21" src="https://user-images.githubusercontent.com/46562447/177112009-923aa439-4b00-4d8e-9aac-3a878f2b3087.png">

<br>
<h3>MQTT structure</h3>
All information from sensor device is sent to Home Assistant MQTT broker in one topic: sensor_hostname/sensor/state<br>
<br>
<img width="593" alt="Screenshot 2022-06-20 at 00 17 05" src="https://user-images.githubusercontent.com/46562447/174503988-5944ceb1-49b5-40a9-a96f-f9aeae532dee.png">

<br>And message arrives in JSON format:<br>
<br>
<img width="628" alt="Screenshot 2022-06-22 at 19 26 51" src="https://user-images.githubusercontent.com/46562447/175109966-148e09bf-1cd4-48b1-bd95-0f66ca9b1206.png">


<br>
<h2>TODO list</h2>
Probably nothing at this moment ;)
