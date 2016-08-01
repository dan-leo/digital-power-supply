import unittest
from util import *

class MyTests(unittest.TestCase):

    def testIsValidMessage(self):
    	# Test wether the received string meets the following criteria:
    	#   1) contains string "voltage" or "current" followed by a value, separated by a period
   		#   2) value must be convertable to an integer
        self.assertEqual(isValidMessage("voltage.123"), 1)
        self.assertEqual(isValidMessage("current.1235"), 1)
        self.assertEqual(isValidMessage("abc.123"), 0)
        self.assertEqual(isValidMessage("abc.123.45"), 0)
        self.assertEqual(isValidMessage("abc"), 0)
        self.assertEqual(isValidMessage("123"), 0)
        self.assertEqual(isValidMessage("123.45"), 0)
        self.assertEqual(isValidMessage(1234), 0)

    def testCalcError(self): 
        # Return the absolute error of the measured value compared to the desired value as a percentage.
        # Returned value must be rounded to the nearest integer
        self.assertEqual(calcError(0.613,0.5), 23)
        self.assertEqual(calcError(1.0049,1), 0)
        self.assertEqual(calcError(1.0051,1), 1)
        self.assertEqual(calcError("1.3",0.9), -1)
        
if __name__ == '__main__':
    unittest.main()
