// -------------------------------------------------------------------
//
// eleveldb: Erlang Wrapper for LevelDB (http://code.google.com/p/leveldb/)
//
// Copyright (c) 2011-2015 Basho Technologies, Inc. All Rights Reserved.
//
// This file is provided to you under the Apache License,
// Version 2.0 (the "License"); you may not use this file
// except in compliance with the License.  You may obtain
// a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//
// -------------------------------------------------------------------

#include <syslog.h>

#include <new>
#include <set>
#include <stack>
#include <deque>
#include <sstream>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include "profiler_nif.h"

#include "ErlUtil.h"
#include "Profiler.h"

#ifndef ATOMS_H
    #include "atoms.h"
#endif

static ErlNifFunc nif_funcs[] =
{
    {"profile",        1, profiler::profile},
};


namespace profiler {

    // Atoms (initialized in on_load)

    ERL_NIF_TERM ATOM_OK;
    ERL_NIF_TERM ATOM_ERROR;
}

using std::nothrow;
using namespace nifutil;

namespace profiler {

ERL_NIF_TERM profile(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    try {

        std::vector<ERL_NIF_TERM> cells = ErlUtil::getTupleCells(env, argv[0]);
        std::string atom  = ErlUtil::formatTerm(env, cells[0]);

        //------------------------------------------------------------
        // Resize the internal profiler map
        //------------------------------------------------------------
        
        if(atom == "resize") {
            Profiler::get()->resize(ErlUtil::getValAsUint32(env, cells[1]));
            return profiler::ATOM_OK;
        }

        //------------------------------------------------------------
        // Start a counter
        //------------------------------------------------------------

        if(atom == "start") {
            ErlNifUInt64 count = Profiler::get()->start(ErlUtil::getValAsUint32(env, cells[1]));
            return enif_make_uint64(env, count);
        }

        //------------------------------------------------------------
        // Stop a counter
        //------------------------------------------------------------

        if(atom == "stop") {
            if(cells.size() > 2)
                Profiler::get()->stop(ErlUtil::getValAsUint32(env, cells[1]), ErlUtil::getValAsUint32(env, cells[2]));
            else
                Profiler::get()->stop(ErlUtil::getValAsUint32(env, cells[1]));
            
            return profiler::ATOM_OK;
        }

        //------------------------------------------------------------
        // Dump counters out to disk
        //------------------------------------------------------------

        if(atom == "dump") {
            Profiler::get()->append(ErlUtil::getAsString(env, cells[1]));
            return profiler::ATOM_OK;
        }

        //------------------------------------------------------------
        // Set the prefix for output files
        //------------------------------------------------------------

        if(atom == "prefix") {
            Profiler::get()->setPrefix(ErlUtil::getAsString(env, cells[1]));
            return profiler::ATOM_OK;
        }

	ERL_NIF_TERM msg_str  = enif_make_string(env, "Unexpected atom received", ERL_NIF_LATIN1);
        return enif_make_tuple2(env, profiler::ATOM_ERROR, msg_str);

    } catch(std::runtime_error& err) {
	ERL_NIF_TERM msg_str  = enif_make_string(env, err.what(), ERL_NIF_LATIN1);
        return enif_make_tuple2(env, profiler::ATOM_ERROR, msg_str);
    } catch(...) {
	ERL_NIF_TERM msg_str  = enif_make_string(env, "Unhandled exception caught", ERL_NIF_LATIN1);
        return enif_make_tuple2(env, profiler::ATOM_ERROR, msg_str);
    }
}
}


static void on_unload(ErlNifEnv *env, void *priv_data)
{
}


static int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
try
{
    int ret_val = 0;
// must initialize atoms before processing options
#define ATOM(Id, Value) { Id = enif_make_atom(env, Value); }
    ATOM(profiler::ATOM_OK, "ok");
    ATOM(profiler::ATOM_ERROR, "error");
#undef ATOM
    return ret_val;
}


catch(std::exception& e)
{
    /* Refuse to load the NIF module (I see no way right now to return a more specific exception
    or log extra information): */
    return -1;
}
catch(...)
{
    return -1;
}

extern "C" {
    ERL_NIF_INIT(profiler, nif_funcs, &on_load, NULL, NULL, &on_unload);
}

