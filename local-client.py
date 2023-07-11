#! /usr/bin/env nix-shell
#! nix-shell -i python3 -p python3Packages.websocket-client
# -*- coding: iso-8859-15 -*-

from websocket import create_connection
ws = create_connection("ws://p14:9001/socket.io/1/websocket")

ws.send("3:::SETD^i.0.mute^1")
result =  ws.recv()
print("Received '%s'" % result)

ws.send("3:::SETD^i.0.mix^0.5")
result =  ws.recv()
print("Received '%s'" % result)

ws.close()
