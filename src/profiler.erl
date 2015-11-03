%% -------------------------------------------------------------------
%%
%%  eleveldb: Erlang Wrapper for LevelDB (http://code.google.com/p/leveldb/)
%%
%% Copyright (c) 2010-2015 Basho Technologies, Inc. All Rights Reserved.
%%
%% This file is provided to you under the Apache License,
%% Version 2.0 (the "License"); you may not use this file
%% except in compliance with the License.  You may obtain
%% a copy of the License at
%%
%%   http://www.apache.org/licenses/LICENSE-2.0
%%
%% Unless required by applicable law or agreed to in writing,
%% software distributed under the License is distributed on an
%% "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
%% KIND, either express or implied.  See the License for the
%% specific language governing permissions and limitations
%% under the License.
%%
%% -------------------------------------------------------------------
-module(profiler).

-export([profile/1]).

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

-spec init() -> ok | {error, any()}.
init() ->
    
    SoName = case code:priv_dir(?MODULE) of
                 {error, bad_name} ->
		     
                     case code:which(?MODULE) of
                         Filename when is_list(Filename) ->
			     
                             filename:join([filename:dirname(Filename),"../priv", "profiler"]);
			                          _ ->                             filename:join("../priv", "profiler")
                     end;
                 Dir ->
                     filename:join(Dir, "profiler")
             end,
    erlang:load_nif(SoName, application:get_all_env(profiler)).

profile(_Tuple) ->
    erlang:nif_error({error, not_loaded}).
