tick = 0

def Reset():
	global tick
	tick = 0

def Advance():
	global tick
	tick += 1

def Get():
	global tick
	return tick
