# -*- coding: utf-8 -*-
"""
Created on Fri Jan 17 19:59:21 2020

@author: Miha
"""

import paho.mqtt.client as mqtt
#we will need it to decode the API responses
import json 
#we will need it to decode Base64 in the payload
import base64
#to more easily parse date and time.
from datetime import datetime
"""Everything below is slightly adapted from the default paho-mqtt example found
on their website at https://pypi.org/project/paho-mqtt/#usage-and-api. I used the TTN MQTT API
reference at https://www.thethingsnetwork.org/docs/applications/mqtt/api.html, trial and error
and reading the docs to make it work."""

def parse_time(timestr):
    """
    A function which simply parses the time obtained from the API and returns it as a Python3 datetime object so it 
    is more easily manipulated later on. It is likely not the most efficient way to do so, but it is simple and fast.
    """
    step1 = timestr.split('-')
    time = step1[2][3 :]
    timep = time.split(':')
    dt = datetime(int(step1[0]), int(step1[1]), int(time[0 : 2]), int(timep[0]), int(timep[1]), int(timep[2][0:2]), int(timep[2][3:6]) * 1000 )
    #the datetime constructor takes the last time argument in MICROseconds, while what the API gives us is in MILIseconds
    return dt    

#PLEASE INSERT YOUR OWN INFO BELOW
AcKey = "ttn-account-v2.6D1QP1FcPI1-Rj0zfBZw1GUjeO09ZT-gh_TfubPFvr4"
AppID = "miha_test"
DevID = "ard1"

#the string will be constructed on its own, thanks to Python 3's nice formatting syntax
topic = f"{AppID}/devices/{DevID}/up" 

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    rec_dict = json.loads(msg.payload)
    dt = parse_time(rec_dict["metadata"]["time"])
    date = dt.date().isoformat()
    time = dt.time().isoformat()
    dcd = base64.standard_b64decode(rec_dict["payload_raw"])
    print(f"Message received from device {rec_dict['dev_id']} on {date} at {time} saying {dcd}.")

client = mqtt.Client()
client.username_pw_set(AppID, password = AcKey)

client.on_connect = on_connect
client.on_message = on_message

client.connect("eu.thethings.network", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()