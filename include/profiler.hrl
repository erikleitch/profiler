-export([perf_fnstring/1]).

-define(PROFILE, 1).

-ifdef(PROFILE).
-define(FNNAME(), profiler:perf_fnstring(process_info(self(), current_function))).
-else.
-define(FNNAME(), ok).
-endif.

perf_fnstring({_Label, {Mod, Fn, _Arity}}) ->
    atom_to_list(Mod) ++ ":" ++ atom_to_list(Fn).





    
    
