##`profiler` - A simple Erlang profiler NIF## 

<hr>
###What is profiler?###

```profiler``` is a NIF library that provides a simple interface for
basic profiling of erlang/C++ code.

<hr>
###Usage###

At its most basic, ```profiler``` allows you to measure execution time
between arbitrary points in erlang/C++ code.  Within an erlang
process, an arbitrary number of counters can be accumulated, either
globally or per-thread, and a simple text file with the counter
statistics is output either by command, or by default at process exit.

* <a href=#basic>Basic Usage</a>
* <a href=#perthread>Per-Thread Counters</a>
* <a href=#utilities>Utilities</a>
* <a href=#noop>Turning Profiling Off</a>

<a name="basic">
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
looks something like this (the file name is prefixed with the process
id):

```
unix_prompt:>cat 0x20872600_profile.txt

totalcount 2
label 'tag1'
count 0x0 0
usec 0x0 3855829
```

What does these lines mean?

  * ```totalcount 2```: ```profiler``` keeps track of the number of times a
  ```start/stop``` operation was invoked. This can be useful for
  estimating the total impact of the profiling itself. The first line says
  that the the total count of profiling operations was 2

  * ```label 'tag1'```: This is a whitespace-separated list of all
    counter tags that ```profiler``` has accumulated

  * ```count 0x0 0```: This is a whitespace-separated list of the
    number of times profiling operations were invoked while the named
    counter was accumulating.  This can be helpful for estimating the
    contribution of the profiling to each counter.<br><br>
    **NB** The first (hex) argument after the label ```count``` is a
      thread id (in this case 0x0). If counters are being accumulated
      globally, this will always be 0x0, else a separate line is
      printed for each thread where counters were invoked**

  * ```usec 0x0 3855829```: This is a whitespace-separated list of the
    accumulated time, in micro-seconds, associated with each named counter in the
    ```label``` line.  In this case, 3.86 seconds elapsed while the counter was active.<br><br>
    **NB** The first (hex) argument after the label ```usec``` is a
      thread id (in this case 0x0). If counters are being accumulated
      globally, this will always be 0x0, else a separate line is
      printed for each thread where counters were invoked**

<a name="perthread">
####Per-Thread Counters####

An extra boolean argument to the ```start``` tuple specifies if the
counter should be accumulated on a per-thread basis:

```
Eshell V5.10.3  (abort with ^G)
1> profiler:perf_profile({prefix, './'}).
ok
2> profiler:perf_profile({start, 'tag1', true}).
```

wait a while...

```
3> profiler:perf_profile({stop, 'tag1', true}).
0
4>
```

The file will now record the thread id as well as the elapsed time:
```
unix_prompt:>cat 0x205f2600_profile.txt

totalcount 2
label 'tag1'
count 0xb0ac5000 0
usec 0xb0ac5000 6512157 
```

<a name=utilities>
####Utilities####

* A simple macro ```?FNNAME()``` is provided in ```profiler.hrl``` to
  return the current function name for use as a counter tag

* To print the current counter stats interactively, along with other
  debug information, use ```profiler:perf_profile({debug}).```

* To dump the current counter stats to disk (in this example to the
  current directory) at any point interactively, use
  ```profiler:perf_profile({dump, './'}).```

<a name=noop>
####Turning Profiling Off####

Profiling can be disabled in-situ in a variety of different ways:

* Globally, by commenting out the line ```-define(PROFILE, 1).``` in
  profiler.hrl and recompiling the module.<br>

  In this case, all calls to ```profiler:perf_profile/1``` are no-ops,
  as is the macro ```?FNNAME()```.<br>

  Note however that a separate interface ```profiler:profile/1``` is
  not controlled by the ```PROFILE``` define, so that profiling can be
  turned off globally, but individual counters can still be activated
  if desired.

* Interactively, via a call to ```profiler:perf_profile({noop, true}).```<br><br>

  This disables all profiling until ```profiler:perf_profile({noop,
  false}).``` is issued.<br>

  The status of the no-op flag is shown when
  ```profiler:perf_profile({debug}).``` is issued.
