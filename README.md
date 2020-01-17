# LoRa_Manual
Code relevant to an User Manual for LoRa/LoRaWAN devices (RAK831, RAK811, Dragino v1.3 LoRa Shield) done as an extracurricular project.

* 'lmic_project_config.h' the personalized config file for the code relevant to the User Manual, done as per the "README.md" found at the [MCCI repo](https://github.com/mcci-catena/arduino-lmic)
* 'basicABPtest_lora.ino' a `"Hello, world!"` test file, adapted from the [Dragino wiki page](https://wiki.dragino.com/index.php?title=Lora_Shield). I adjoined a case in the event handler to avoid printing "Unknown event" and provided extensive comments explaining the code. Tested and working.
* 'loraABPsensor.ino' a basic sensor application, which extends the above example to include a `do_scan` function. Since I did not posses a sensor, it generates random bytes of data and stores them in an array. When the array is full it is transmitted. More info in the comments. Tested and working.
* 'lowpower_lora.ino' an extension of the sensor application which takes inspiration from the [Low Power](https://github.com/rocketscream/Low-Power/blob/master) library as well as [this example](https://github.com/DiederikRhee/ProMini_Lora_Otaa_temperture/blob/master/Lora_temp_hum/Lora_temp_hum.ino) to illustrate what a sensor would look like if one wished to conserve power. Keep in mind the Low Power library needs to be installed/imported so that the code will run. UNTESTED.
* 'downlink.ino' UNTESTED.
* 'basic_SDK.py' simple example of usage of Python 3 SDK for TTN. Tested and working.
* 'Django_SDK.py' an illustration of how the SDK could be integrated into the Django web application framework to make a custom console. Tested and working.
* 'LoRA_UserManual' the actual "User Manual" mentioned above.

