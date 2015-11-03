-export([perf_profile/1]).

-define(PROFILE, true).
-ifdef(PROFILE).
perf_profile(Tuple) ->
    profiler:profile(Tuple).
-else.
perf_profile(_Tuple) ->
    ok.
-endif.
