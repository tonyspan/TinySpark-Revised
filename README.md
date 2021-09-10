# TinySpark-Revised	[![License](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/tonyspan/TinySpark-Revised/blob/master/LICENSE)

This project is a revised/altered version of my bachelor thesis (Implementation of a highly efficient distributed processing framework in C++).

## Summary

All Apache Spark components, including the Driver, Master, and Executor processes, run in Java virtual machines (JVMs), which are resource-hungry (CPU: serialization/deserialization, Garbage Collector, RAM: Data representation takes multiples of the actual size of data). A small subset of Apache Spark operations was implemented, namely *map, filter* and *reduce* in a native language, and see if it is feasible to overcome those overheads. It was decided to use C++ for the core/Backend system as it is well suitable for higher performance in terms of bandwidth, throughput, and much lower latency, and Python for the Frontend, to give the user a simple high-level API to work with.

The Backend was written in `C++17`, using features like `std::variant, std::filesystem` etc. `boost::tokenizer` for file tokenization, `OpenMP` to speed things up, and tried to be system/platform independent.

# Key Difference
* Old-er version relied on a dispatch table that matched specific *actions* extracted from client's code to the Backend in order to execute an *already* predefined templated function that corresponds to that *action*. Now, it is more dynamic. 
There are two new subsystems in the Backend, the first one, at **runtime**, when it receives a function from Frontend, through gRPC, e.g `x => x < 3`, where `x` is the formal argument and `x<3` is the return statement with type of `x` is assumed to be the same as *MFRObject* List (here int). Final `C++` function looks like: ```T filt(T x) { return x<3; }```, similarly for *map, reduce*, and stores it into a shared library (.so or .dll, subsystem supports both Linux and Windows).
The second new subsystem (supports both Linux and Windows), dynamically invokes *filter, map* and *reduce* functions/methods at runtime on the object.
	* There are some limitations still (e.g number of arguments and type), but it is more versatile now.

# Other Changes
* A better project structure
* Debloated/refactored current codebase, both Backend and Frontend
* New (minimalistic) Frontend API:

```python
import TinySpark as ts

ts.EstablishConnectionToBackend("0.0.0.0:50051")

my_obj = ts.MFRObject([1, 2, 3, 4, 5])
my_obj.Filter('x => x < 3')
my_obj.Map('x => x ** 3')
my_obj.Reduce('_ + _') # or my_obj.Reduce('x, y => x + y')

print(ts.RPCCall(my_obj).GetRPCResult())
```

# Notes
* The system has only one Driver, one or more Workers and Clients.
* Client(s) communicate only with Driver, and there is no link between the Worker(s).
* The system can handle both Lists and Files, in a different way for each. List’s implementation is almost generic (can deal with a variety of types, but with `std::vector` as a container, while file implementation is specific to *WordCount, StringToInt, Count*.
* For the numeric, string List *map, filter* and *reduce* operations were implemented, but with specific order (firstly an optional *filter* operation, then *map* and *reduce*).
* Apache Spark does not evaluate every transformation (e.g map) just as it encounters it but instead waits for an action to be called (e.g collect). In this system, there is no such functionality, as soon as the Backend receives a task, immediately executes it, and sends a response.
* Although *map, filter* and *reduce* were implemented with `C++17`'s Parallel Algorithms (to gain more performance) in mind, in the end they were used without an execution policy.
* The Backend throws runtime exceptions if anything go wrong (it should handle cases like those).
* No proper profiling and optimization(s)

# Benchmarks
For benchmarking a single machine was used, equipped with an Intel(R) i5 6th Gen, 4 cores and 4 threads, 16GB DDR4 RAM, with Ubuntu 20.4, using GCC 9.3.0, whilst running multiple processes.

`WordCount` Each word of a file with its frequency. Comparison to Spark is **not** apples-to-apples, as Spark processes more things than our implementation (i.e ReduceByKey).
```
TinySpark: 46401 ms
Apache Spark: 128380 ms
```
`LineCount` number of lines of a file.
```
TinySpark: 25839 ms
Apache Spark: 36870 ms
```
`StrToInt` Conversion of string into a long int (*map* operation) and summation of those numbers (*reduce* operation).
```
TinySpark: 29876 ms
Apache Spark: 27763 ms
```
# Other examples
* String List

```python
my_obj = ts.MFRObject(['this is', 'an example'])
my_obj.Map('x => x.length')
my_obj.Reduce('_ + _') # or my_obj.Reduce('x, y => x + y')
print(ts.RPCCall(my_obj).GetRPCResult())
```
For string List filter operation is not implemented, and for map operation only `length` is implemented. `x => x.length` bind to `C++` as `T map(S x) { return x.length(); }`, where T = float (handled internally), and S = type of List (here str).

* Files

```python
# word count
my_obj = ts.MFRObject('filename.txt')
my_obj.Map('x => (x,1)')
my_obj.Reduce('_ + _') # or my_obj.Reduce('x, y => x + y')
print(ts.RPCCall(my_obj).GetRPCResult())

# count
my_obj = ts.MFRObject('filename.txt')
my_obj.Map('x => 1')
my_obj.Reduce('_ + _') # or my_obj.Reduce('x, y => x + y')
print(ts.RPCCall(my_obj).GetRPCResult())

# string to number
my_obj = ts.MFRObject('filename.txt')
my_obj.Map('x => x.toNum')
my_obj.Reduce('_ + _') # or my_obj.Reduce('x, y => x + y')
print(ts.RPCCall(my_obj).GetRPCResult())
```

# Getting Started
**Dependencies:**
* gRPC (`C++` and `Python`)
* OpenMP
* Boost Libraries
* (Optional) Intel Threading Blocks (TBB) (for `<execution>` header)

After installing above prerequisites, run `Build.sh` script from project's main folder.

# Usage
Open three terminals:

First:
```
cd build
./Driver
```
Second:
```
cd build
./Worker
```
Third:
```
cd App
python3 Client.py
```
If no address is provided e.g `./Driver 49.123.106.100:44420`, it runs on the address `0.0.0.0:50051`.