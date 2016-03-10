-export([perf_profile/1, perf_fnstring/1]).

-define(PROFILE, true).
-ifdef(PROFILE).
perf_profile(Tuple) ->
    profiler:profile(Tuple).
-else.
perf_profile(_Tuple) ->
    ok.
-endif.

perf_fnstring({_Label, {Mod, Fn, _Arity}}) ->
    atom_to_list(Mod) ++ ":" ++ atom_to_list(Fn).

-define(FNNAME(), profiler:perf_fnstring(process_info(self(), current_function))).




    
    
