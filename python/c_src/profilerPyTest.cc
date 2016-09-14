/**.......................................................................
 * Python file for reading from GCP archive data files.
 */
#include <Python.h>
#include <iostream>  
#include <vector>  
#include <map>  

#include "PyParser.h"
#include "Profiler.h"
#include "exceptionutils.h"

#if DIR_HAVE_NUMPY
#include "arrayobject.h"
#endif

using namespace std;
using namespace gcp::python;
using namespace nifutil;

/**.......................................................................
 * Entry point from the Python environment
 */
static PyObject* profile(PyObject* self, PyObject* args)
{

  try {

    int argc = PyParser::getNumberOfDimensions(args);
    
    bool always = false;
    if(argc == 2)
      always = PyParser::getBoolVal(PyParser::getArrayItem(args, 1));

    PyParser cells(PyParser::getArrayItem(args, 0));

    std::string atom  = cells.getString((unsigned)0);
    
    //------------------------------------------------------------
    // Make the profiler a no-op
    //------------------------------------------------------------

    if(atom == "noop") {
      Profiler::noop(cells.getBoolVal((unsigned)1));
      return Py_BuildValue("s", "ok");
    }

    //------------------------------------------------------------
    // Output debug information
    //------------------------------------------------------------

    if(atom == "debug") {
      Profiler::get()->debug();
      return Py_BuildValue("s", "ok");
    }

    //------------------------------------------------------------
    // start/stop a counter
    //------------------------------------------------------------

    if(atom == "start" || atom == "stop") {
      std::string label  = cells.getString((unsigned)1);
                
      bool perThread = false;
      if(cells.getSize() > 2)
        perThread = cells.getBoolVal((unsigned)2);
      
      uint64_t count = Profiler::profile(atom, label, perThread, always);
      return PyLong_FromUnsignedLongLong(count);
    }

    //------------------------------------------------------------
    // dump counters out to disk, or set the prefix dir for output
    //------------------------------------------------------------

    if(atom == "dump" || atom == "prefix") {
      if(cells.getSize() != 2)
        ThrowRuntimeError("You must specify a path with the " << atom << " argument");
      Profiler::profile(atom, cells.getString((unsigned)1), true);
      return Py_BuildValue("s", "ok");
    }

    return Py_BuildValue("s", "Unexpected atom received");

  } catch(std::runtime_error& err) {

    return Py_BuildValue("s", err.what());
        
  } catch(...) {
    return Py_BuildValue("s", "Caught unhandled exception");
  }
}

static PyObject* listFunctions(PyObject* self, PyObject* args)
{
  COUT(std::endl << "Functions available from this module:" << std::endl);
  COUT("     profile");
  COUT("");
  return Py_BuildValue("s", "ok");
}

static PyMethodDef profilerPyTest_methods[] = {
  {"list",    listFunctions, METH_VARARGS, "Call profilerPyTest.list() for a list of functions available from this module."},
  {"profile", profile,       METH_VARARGS, " \n  main profiler method\n"},
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initprofilerPyTest(void)
{
  (void) Py_InitModule("profilerPyTest", profilerPyTest_methods);
}
