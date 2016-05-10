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

####Basic Usage####
The primary erlang interface to ```profiler``` is the
```profiler:perf_profile/1``` function.  This takes a tuple of
arguments, controlling a variety of operations.

For example:

```
Eshell V5.10.3  (abort with ^G)
1> profiler:perf_profile({prefix, './'}).
ok
2> profiler:perf_profile({start, 'tag1'}).
```

wait a while...

```
3> profiler:perf_profile({stop, 'tag1'}).
0
4>
```

The first line tells ```profiler``` to create its log file in the
current directory, the second starts a counter named ```tag1```, and
the last terminates the named counter.

on exit from the erlang shell, profiler will have created a file that
looks something like this:

```
unix_prompt:>cat 0x20872600_profile.txt

totalcount 2
label 'tag1'
count 0x0 0
usec 0x0 3855829
```




