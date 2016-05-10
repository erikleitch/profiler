##`profiler` - A simple Erlang profiler NIF## 

<hr>
###What is profiler?###

```profiler``` is a NIF library that provides a simple interface for
basic profiling of erlang/C++ code.

###Usage###

At its most basic, ```profiler``` allows you to measure execution time
between arbitrary points in erlang/C++ code.  Within an erlang
process, an arbitrary number of counters can be accumulated, either
globally or per-thread, and a simple text file with the counter
statistics is output either by command, or by default at process exit.

###Examples###

The primary erlang interface to ```profiler``` is the
```profiler:perf_profile/1``` function.  This takes a tuple of
arguments, controlling a variety of operations.

```
Eshell V5.10.3  (abort with ^G)
1> profiler:perf_profile({prefix, './'}).
```
