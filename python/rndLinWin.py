#!/usr/bin/python3.6

import json
import random
import os
import platform

todoList = json.load( open("todoList.json") )
listSize = len(todoList["objects"])-2
idx = random.randint(0, listSize)

outStr = {
	'Windows' 	: (todoList["objects"][idx]).encode(encoding='ANSI').decode(),
	'Linux'		: (todoList["objects"][idx])
}

print( "\nToday " + outStr[platform.system()] + "\n" )	
