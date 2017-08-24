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

#include "exceptionutils.h"
#include "profiler_nif.h"

#include "ErlUtil.h"
#include "Profiler.h"

#ifndef ATOMS_H
#include "atoms.h"
#endif

static ErlNifFunc nif_funcs[] =
{
    {"profile",        1, profiler::profile},
    {"profile",        2, profiler::profile},
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

            bool always = false;
            if(argc == 2)
                always = ErlUtil::getBool(env, argv[1]);
            
            std::vector<ERL_NIF_TERM> cells = ErlUtil::getTupleCells(env, argv[0]);
            std::string atom  = ErlUtil::formatTerm(env, cells[0]);

            //------------------------------------------------------------
            // Make the profiler a no-op
            //------------------------------------------------------------

            if(atom == "noop") {
                Profiler::noop(ErlUtil::getBool(env, cells[1]));
                return profiler::ATOM_OK;
            }

            if(atom == "init_atomic_counters") {

                if(ErlUtil::isTuple(env, cells[1])) {

                    std::vector<ERL_NIF_TERM> args = ErlUtil::getTupleCells(env, cells[1]);

                    if(args.size() == 4 && ErlUtil::isList(env, args[0])) {

                        std::vector<ERL_NIF_TERM> list = ErlUtil::getListCells(env, args[0]);
                        unsigned int bufferSize        = ErlUtil::getValAsUint32(env, args[1]);
                        uint64_t minorIntervalMs       = ErlUtil::getValAsUint64(env, args[2]);
                        std::string outputFile         = ErlUtil::getString(env, args[3]);

                        std::map<std::string, std::string> nameMap;
                        
                        for(unsigned i=0; i < list.size(); i++) {
                            std::string name = ErlUtil::getString(env, list[i]);
                            nameMap[name] = name;
                        }

                        Profiler::initializeAtomicCounters(nameMap, bufferSize, minorIntervalMs, outputFile);
                        
                        return profiler::ATOM_OK;
                    }
                }

                ThrowRuntimeError("Use like: profiler:profile({test, {[\"tag1\", \"tag2\"], bufferSize, intervalMs}})");
                return profiler::ATOM_ERROR;
            }

            if(atom == "inc_atomic_counter") {
                uint64_t partPtr = ErlUtil::getValAsUint64(env, cells[1]);
                std::string counterName = ErlUtil::getAsString(env, cells[2]);
                Profiler::incrementAtomicCounter(partPtr, counterName);
                return profiler::ATOM_OK;
            }

            if(atom == "add_ring_partition") {
                uint64_t partPtr = ErlUtil::getValAsUint64(env, cells[1]);
                std::string leveldbFile = ErlUtil::getAsString(env, cells[2]);
                COUT("partPTr = " << partPtr << " file = " << leveldbFile);
                Profiler::addRingPartition(partPtr, leveldbFile);
                return profiler::ATOM_OK;
            }

            //------------------------------------------------------------
            // Output debug information
            //------------------------------------------------------------

            if(atom == "debug") {
                Profiler::profile("debug");
                return profiler::ATOM_OK;
            }

            //------------------------------------------------------------
            // start/stop a counter
            //------------------------------------------------------------

            if(atom == "start" || atom == "stop") {
                std::string label = ErlUtil::getAsString(env, cells[1]);
                
                bool perThread = false;
                if(cells.size() > 2)
                    perThread = ErlUtil::getBool(env, cells[2]);
                
                ErlNifUInt64 count = Profiler::profile(atom, label, perThread, always);
                
                return enif_make_uint64(env, count);
            }

            //------------------------------------------------------------
            // dump counters out to disk, or set the prefix dir for output
            //------------------------------------------------------------

            if(atom == "dump" || atom == "prefix") {
                if(cells.size() != 2)
                    ThrowRuntimeError("You must specify a path with the " << atom << " argument");
                Profiler::profile(atom, ErlUtil::getAsString(env, cells[1]), true);
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

static void on_unload(ErlNifEnv *env, void *priv_data) {}

static int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    try {
        int ret_val = 0;
        
        profiler::ATOM_OK    = enif_make_atom(env, "ok");
        profiler::ATOM_ERROR = enif_make_atom(env, "error");

        try {
            bool noop = ErlUtil::getBool(env, ErlUtil::getOption(env, load_info, "noop"));
            profiler::Profiler::noop(noop);
        } catch(...) {
            profiler::Profiler::noop(false);
        }
        
        return ret_val;

        // Refuse to load the NIF module (I see no way right now to
        // return a more specific exception or log extra information):
        
    } catch(std::exception& e) {
        return -1;
    } catch(...) {
        return -1;
    }
}

extern "C" {
    ERL_NIF_INIT(profiler, nif_funcs, &on_load, NULL, NULL, &on_unload);
}

