import sys
import os
import operator

import grpc

# Find project root directory from THIS file
__ProjectRootDir = os.path.abspath(__file__ + '/../../../')
sys.path.append(__ProjectRootDir + '/GenProtos/py')

# gRPC related
import MFR_pb2
import MFR_pb2_grpc

# Other Utils
import enum
import Parser
import Timer

START_TIMER = lambda x: x.StartClockNow()
TIME_ELAPSED = lambda x : (x.StopClockNow(), print("Time Elapsed " + str(x.TimeElapsedMilliSec()) + " ms"))

class MFRType(enum.Enum):
    NUMERIC = 0
    STRING = 1
    
    def __str__(self):
        return str(self.value)

class MFRObject():
	
	'''obj can be either list or filename (without path)'''
	def __init__(self, obj):
		self.__mHasFilter = "false"
		self.__mFiltFormal = ""
		self.__mFiltRet = ""

		if not isinstance(obj, list) and not isinstance(obj, str):
			raise TypeError("Unsuported Type")
		if isinstance(obj, list):
			if(all(isinstance(x, str) for x in obj)):
				self.__mListType = 1
				self.__mList = [str(elem) for elem in obj] # avoid unicode encoding that gRPC doesn't like
			elif(all(isinstance(x, int) or all(isinstance(x, float))) for x in obj):
				self.__mListType = 0
				self.__mList = obj
				self.__mFile = None
			else:
				raise TypeError("Unsupported input type")
		else:
			self.__mFile = obj
			self.__mList = None

	def Map(self, expr: str):
		"""
		Map for file MUST use following syntax:
		Word Count: x => (x,1)
		Count: x => 1
		toNum: x => x.toNum
		"""
		map = Parser.Parser(expr)
		self.__mMapFormal = map.GetFormalArgs()
		self.__mMapRet = map.GetReturnStmt()
	
	def Reduce(self, expr: str):
		red = Parser.Parser(expr)
		self.__mRedFormal = red.GetFormalArgs()
		self.__mRedRet = red.GetReturnStmt()
	
	def Filter(self, expr: str):
		filt = Parser.Parser(expr)
		self.__mFiltFormal = filt.GetFormalArgs()
		self.__mFiltRet = filt.GetReturnStmt()
		self.__mHasFilter = "true"

	def GetList(self):
		return self.__mList

	def GetFile(self):
		return self.__mFile

	def GetMap(self):
		return self.__mMapFormal, self.__mMapRet
	
	def GetReduce(self):
		return self.__mRedFormal, self.__mRedRet

	def GetFilter(self):
		return self.__mFiltFormal, self.__mFiltRet

	def GetType(self):
		return self.__mListType
	
	def HasFilter(self):
		return self.__mHasFilter

gAddrPort = None

class EstablishConnectionToBackend():

    def __init__(self, addrPort: str):
        self.__mAddrPort = addrPort
        global gAddrPort 
        gAddrPort = addrPort
        self.__mChannel = grpc.insecure_channel(self.__mAddrPort)
        self.__mStub = MFR_pb2_grpc.MFRServiceStub(self.__mChannel)
    
    def GetStub(self):
        return self.__mStub

class RPCCall():
	
	def __init__(self, obj: MFRObject):
		print("Client: Sending Map, Reduce Request to Driver")
		
		t = Timer.Timer()
		START_TIMER(t)

		if obj.GetList() is not None:

			if obj.GetType() == MFRType.NUMERIC.value:
				response = EstablishConnectionToBackend(gAddrPort).GetStub().MFRListCD(MFR_pb2.MFRListRequest(NumList = obj.GetList(),
																											MapForm = obj.GetMap()[0], MapRet = obj.GetMap()[1],
																											FilterForm = obj.GetFilter()[0], FilterRet = obj.GetFilter()[1],
																											ReduceForm = obj.GetReduce()[0], ReduceRet = obj.GetReduce()[1],
																											initial = 0,
																											ArgType = obj.GetType(),
																											hasFilter = obj.HasFilter()))
			else:
				response = EstablishConnectionToBackend(gAddrPort).GetStub().MFRListCD(MFR_pb2.MFRListRequest(StrList = obj.GetList(),
																											MapForm = obj.GetMap()[0], MapRet = obj.GetMap()[1],
																											FilterForm = obj.GetFilter()[0], FilterRet = obj.GetFilter()[1],
																											ReduceForm = obj.GetReduce()[0], ReduceRet = obj.GetReduce()[1],
																											initial = 0,
																											ArgType = obj.GetType(),
																											hasFilter = obj.HasFilter()))
			self.__mListReturn = response.__getattribute__('ReduceResult')
			self.__mFileReturn = None
		else:
			response = EstablishConnectionToBackend(gAddrPort).GetStub().MFRFileCD(MFR_pb2.MFRFileRequest( Filename = obj.GetFile(), Map = obj.GetMap()[1], Reduce = "" ))
			if bool(response.__getattribute__('final')):
				self.__mFileReturn = response.__getattribute__('final')
				self.__mFileReturn = dict(sorted(self.__mFileReturn.items(), key=operator.itemgetter(1), reverse=True)[:10])
			else:
				self.__mFileReturn = response.__getattribute__('ReduceResult')
			self.__mListReturn = None
		
		TIME_ELAPSED(t)
	
	def GetRPCResult(self):
		if self.__mListReturn is not None:
			return self.__mListReturn
		else:
			return self.__mFileReturn