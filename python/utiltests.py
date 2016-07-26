import unittest
from util import *

class MyTests(unittest.TestCase):

    def testIsValidMessage(self):
        self.assertEqual(isValidMessage("voltage.123"), 1)
        self.assertEqual(isValidMessage("current.1235"), 1)
        self.assertEqual(isValidMessage("abc.123"), 0)
        self.assertEqual(isValidMessage("abc.123.45"), 0)
        self.assertEqual(isValidMessage("abc"), 0)
        self.assertEqual(isValidMessage("123"), 0)
        self.assertEqual(isValidMessage("123.45"), 0)
        self.assertEqual(isValidMessage(1234), 0)

    #def testCalcError(self): # add your own assertions here
        
if __name__ == '__main__':
    unittest.main()
