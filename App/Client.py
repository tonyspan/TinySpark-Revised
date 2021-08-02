import TinySpark as ts

ts.EstablishConnectionToBackend("0.0.0.0:50051")

my_obj = ts.MFRObject([1, 2, 3, 4, 5])
my_obj.Filter('x => x < 3')
my_obj.Map('x => x ** 3')
my_obj.Reduce('_+_')
print(ts.RPCCall(my_obj).GetRPCResult())

my_obj = ts.MFRObject(['this is', 'an example'])
my_obj.Map('x => x.length')
my_obj.Reduce('_+_')
print(ts.RPCCall(my_obj).GetRPCResult())

my_obj = ts.MFRObject('filename.txt')
my_obj.Map('x => (x,1)')
my_obj.Reduce('x, y => x + y')
print(ts.RPCCall(my_obj).GetRPCResult())

my_obj = ts.MFRObject('filename.txt')
my_obj.Map('x => 1')
my_obj.Reduce('x, y => x + y')
print(ts.RPCCall(my_obj).GetRPCResult())

my_obj = ts.MFRObject('filename.txt')
my_obj.Map('x => x.toNum')
my_obj.Reduce('x, y => x + y')
print(ts.RPCCall(my_obj).GetRPCResult())