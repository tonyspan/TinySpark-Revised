import time

class Timer(object):

    def __init__(self):
        self.__mBegin = None
        self.__mEnd = None

    def StartClockNow(self):
        self.__mBegin = time.time()

    def StopClockNow(self):
        self.__mEnd = time.time()

    def TimeElapsedMilliSec(self):
        return round((self.__mEnd - self.__mBegin) * 1000)