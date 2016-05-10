%% -------------------------------------------------------------------
%% profiler: Generic profiler for Erlang/C++
%%
%% Copyright (c) 2010-2016 Basho Technologies, Inc. All Rights Reserved.
%%
%% This file is provided to you under the Apache License, Version 2.0
%% (the "License"); you may not use this file except in compliance
%% with the License.  You may obtain a copy of the License at
%%
%%   http://www.apache.org/licenses/LICENSE-2.0
%%
%% Unless required by applicable law or agreed to in writing, software
%% distributed under the License is distributed on an "AS IS" BASIS,
%% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
%% implied.  See the License for the specific language governing
%% permissions and limitations under the License.
%%
%% -------------------------------------------------------------------

-module(profiler).

-export([profile/1, profile/2, perf_profile/1]).

-compile(export_all).

-on_load(init/0).

-ifdef(TEST).
-compile(export_all).
-ifdef(EQC).
-include_lib("eqc/include/eqc.hrl").
-define(QC_OUT(P),
        eqc:on_output(fun(Str, Args) -> io:format(user, Str, Args) end, P)).
-endif.
-include_lib("eunit/include/eunit.hrl").
-endif.

-import(lager,[info/2]).

-include_lib("include/profiler.hrl").

-ifdef(PROFILE).
-define(NOOP, false).
-else.
-define(NOOP, true).
-endif.

%% ------------------------------------------------------------
%% The underlying interface to the profiler.  
%%
%% This interface is NOT controlled by the PROFILE -define in
%% profiler.hrl.  
%%
%% This interface is exported in addition to the recommended interface
%% (perf_profile/1) for convenience, so that profiling can be turned
%% off globally, but isolated profiling can still be done.  I.e., with
%% -define(PROFILE). commented out,
%%
%%     profiler:perf_profile({start, 'mycounter'})
%%
%% is a no-op, but  
%% 
%%     profiler:profile({start, 'mycounter'})
%%
%% is not.
%%
%% Usage:
%%
%% profile/1 is a multi-use function whose single argument is a tuple
%% of
%%
%% {command, val1, val2,...}
%%
%% combinations.
%%
%% Recognized commands are:
%%
%%    {start, 'mylabel', [true | false]} 
%%
%%        Start a counter, and associate the label 'mylabel' with it.
%%        If specified, the last argument determines whether the
%%        counter is a global counter, or a per-thread counter.
%%
%%    {stop, 'mylabel', [true | false]}
%%
%%        Stop the counter with label 'mylabel' associated with it.
%%        If specified, the last argument determines whether the
%%        counter is a global counter, or a per-thread counter.
%%
%%    {noop, true | false} 
%%
%%        If true, make the underlying C++ code a no-op.  This is
%%        provided in addition to the -define(PROFILE) method of
%%        no-op'ing this module, so that profiling can be turned
%%        on/off manually for debugging purposes.
%%            
%%        Note that when the profiler has been no-op'd in this way,
%%        profiling calls can still be manually made by using the
%%        profile/2 interface, with the second argument set to true.
%%
%%    {prefix, 'some/path'} 
%%
%%        Set the directory prefix for profiler output files.  On
%%        process exit, the profiler dumps all profiling stats to a
%%        single file called pid'_profiler.txt', where pid is the
%%        process id.
%%
%%    {dump, 'myfile'}  
%%
%%        Manually dump profiler stats to the file 'myfile'
%%
%%    {debug}               
%%
%%        Print debugging information about the profiler to stdout
%% ------------------------------------------------------------

profile(_Tuple) ->
    erlang:nif_error({error, not_loaded}).

profile(_Tuple, _Always) ->
    erlang:nif_error({error, not_loaded}).

%% ------------------------------------------------------------
%% Recommended user interface to the profiler.  
%%
%% This interface is controlled by the PROFILE -define in
%% profiler.hrl.  Commenting this out will turn all perf_profile/1
%% calls to no-ops
%%
%% The underlying profile/1 interface is also exported above for
%% convenience, so that profiling can be turned off globally, but
%% isolated profiling can still be done.  I.e., with
%% -define(PROFILE). commented out,
%%
%%     profiler:perf_profile({start, 'mycounter'})
%%
%% is a no-op, but  
%% 
%%     profiler:perf_profile({start, 'mycounter'})
%%
%% is not.
%%
%% See profile/1 above for usage
%% ------------------------------------------------------------

-ifdef(PROFILE).
perf_profile({Atom, Index}) when is_integer(Index) ->
    perf_profile({Atom, integer_to_list(Index)});
perf_profile({Atom, Index, _Label}) when is_integer(Index) ->
    perf_profile({Atom, integer_to_list(Index)});
perf_profile(Tuple) ->
    profiler:profile(Tuple).
-else.
perf_profile(_Tuple) ->
    ok.
-endif.

%% ------------------------------------------------------------
%% Module initialization
%%
%% On load, the value of NOOP is passed into the module on_load
%% function, which internally initializes the profiler to do nothing
%% on calls to profiler:profile()
%% ------------------------------------------------------------
-spec init() -> ok | {error, any()}.
init() ->
    SoName = case code:priv_dir(?MODULE) of
                 {error, bad_name} ->
		     case code:which(?MODULE) of
                         Filename when is_list(Filename) ->
			     filename:join([filename:dirname(Filename),"../priv", "profiler"]);
			 _ ->
                             filename:join("../priv", "profiler")
                     end;
                 Dir ->
                     filename:join(Dir, "profiler")
             end,
    erlang:load_nif(SoName, [{noop, ?NOOP}, {opts, application:get_all_env(profiler)}]).

