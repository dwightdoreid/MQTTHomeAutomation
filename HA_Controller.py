# -*- coding: utf-8 -*-
"""
Created on Sun Nov 13 09:54:29 2016

@author: dwight
"""

import paho.mqtt.client as mqtt
import time

x=True
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    #client.subscribe("$SYS/#")
    client.subscribe("L007")
    client.subscribe("L005")
    client.subscribe("newDevice/#")
    client.subscribe("remDevice")
    client.subscribe("$SYS/broker/log/D")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global x
    print(msg.topic+" "+str(msg.payload))
    if msg.topic=="newDevice":
        print("new device detected")
        time.sleep(1)
        if str(msg.payload) == "lamp/":
            client.publish("newDeviceAdd",payload="L007",qos=0,retain=False)
    if msg.topic=="L007":
        x= not x
        client.publish("L005",payload=int(x),qos=0,retain=False)

client = mqtt.Client(client_id="ha_cntrlr")
client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set("mqttUser", password="mqttuser")
client.connect("192.168.0.106", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()