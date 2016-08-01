def isValidMessage(message):
    try:
        if "voltage" in message or "current" in message:
	    	if len(message) > 8 and message[7] == '.':
	    		if message[8:].replace('.','',1).isdigit():
	    		    return 1
        return 0
    except TypeError:
        return 0

def calcError(measured, desired):
    try:
        return int(round(100*abs(measured - desired)/desired))
    except TypeError:
        return -1