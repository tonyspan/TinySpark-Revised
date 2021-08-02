import re

class Parser():
	def __ParseCommand(self, expr):
		if'_+_' in expr.replace(" ", ""):
			# forced
			self.__mFormalArgs = "x,y"
			self.__mReturnStmt = "x+y"
		elif 'x=>(x,1)' in expr.replace(" ", ""):
			# forced
			self.__mFormalArgs = ""
			self.__mReturnStmt = "wc"
		elif 'x=>x.toNum' in expr.replace(" ", ""):
			# forced
			self.__mFormalArgs = ""
			self.__mReturnStmt = "stoi"
		elif 'x=>1' in expr.replace(" ", ""):
			# forced
			self.__mFormalArgs = ""
			self.__mReturnStmt = "count"
		else:
			lhs, rhs = re.split(r"\=\>", expr.replace(" ", ""))

			for elem in lhs:
				if re.match(r"[\+|\-|\*\*|\/|\^]", elem):
					raise TypeError("TinySpark: Syntax Error")

			# lhsTemp, rhsTemp only for syntax check
			lhsTemp = re.split(",", lhs.replace(" ", ""))
			lhsTemp = "".join(lhsTemp)
			rhsTemp = re.sub(r"[\+|\-|\*\*|\/|\^|\<|\<\=|\>|\>\=|\=\=|\!\=|\d*|\b(.length)\b]", "", rhs)
			# print(lhsTemp)
			# print(rhsTemp)
				
			self.__CheckExprSyntax(lhsTemp, rhsTemp)
			self.__CheckExprArgsNumber(lhsTemp, rhsTemp)
			self.__mFormalArgs = lhs
			self.__mReturnStmt = rhs

			# print(self.__mFormalArgs)
			# print(self.__mReturnStmt)
    
	def __CheckExprArgsNumber(self, left, right):
		"""
		Checks if length all elements of lhs, rhs are same, e.g:

		x, y => x ** y : true

		x, y => -x : false
		"""
		if len(left) != len(right):
			raise TypeError("TinySpark: Syntax Error")
		return 1
    
	def __CheckExprSyntax(self, left, right):
		"""
		Checks if parameters used on left and right have same case, e.g:

		x => x : true

		x => X : false (and vice versa)
		"""
		lhs = set(left)
		rhs = set(right)

		mixed1 = any(elem.islower() for elem in lhs) and any(elem.isupper() for elem in lhs)
		mixed2 = any(elem.islower() for elem in rhs) and any(elem.isupper() for elem in rhs)
		if mixed1 != mixed2:
			raise TypeError("TinySpark: Syntax Error")
		return 1
    
	def GetFormalArgs(self):
		return self.__mFormalArgs
	
	def GetReturnStmt(self):
		return self.__mReturnStmt

	def __init__(self, expr: str):
		self.__ParseCommand(expr)