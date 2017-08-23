#ifndef GCP_UTIL_PYPARSER_H
#define GCP_UTIL_PYPARSER_H

/**
 * @file PyParser.h
 * 
 * Tagged: Wed May  4 23:26:43 PDT 2005
 * 
 * @author Erik Leitch
 */
#define DIR_HAVE_NUMPY 0

#include <Python.h>
#include <string>
#include <vector>

#ifndef Py_ssize_t
typedef ssize_t Py_ssize_t;
#endif

namespace gcp {
  namespace python {
    
    class PyParser {
    public:
      
      enum PythonType {
	PY_NONE    = 0x0,
	PY_UNKNOWN = 0x0,
	PY_BOOL    = 0x1,
	PY_INT     = 0x2,
	PY_FLOAT   = 0x4,
	PY_COMPLEX = 0x8,
	PY_STRING  = 0x10,
	PY_LIST    = 0x20,
	PY_DICT    = 0x40,
	PY_SEQ     = 0x80,
	PY_TUPLE   = 0x100
      };

      /**
       * Constructor.
       */
      PyParser();
      PyParser(const PyObject*);

      void setTo(const PyObject*);
      void setTo(const PyObject**);
      
      /**
       * Destructor.
       */
      virtual ~PyParser();

      bool isArray();
      static bool isArray(PyObject* arr);
      static bool isArray(const PyObject* arr);

      bool isDouble();
      static bool isDouble(PyObject* arr);
      static bool isDouble(const PyObject* arr);

      bool isBool();
      static bool isBool(PyObject* arr);
      static bool isBool(const PyObject* arr);

      bool isUchar();
      static bool isUchar(PyObject* arr);
      static bool isUchar(const PyObject* arr);

      bool isInt();
      static bool isInt(PyObject* arr);
      static bool isInt(const PyObject* arr);
     
      bool isSequence();
      static bool isSequence(PyObject* arr);
      static bool isSequence(const PyObject* arr);

      bool isSet();
      static bool isSet(PyObject* arr);
      static bool isSet(const PyObject* arr);

      bool isDict();
      static bool isDict(PyObject* arr);
      static bool isDict(const PyObject* arr);

      bool isFloat();
      static bool isFloat(PyObject* arr);
      static bool isFloat(const PyObject* arr);

      bool isList();
      static bool isList(PyObject* arr);
      static bool isList(const PyObject* arr);

      bool isTuple();
      static bool isTuple(PyObject* arr);
      static bool isTuple(const PyObject* arr);

      bool isString();
      static bool isString(PyObject* arr);
      static bool isString(const PyObject* arr);

      bool isComplex();
      static bool isComplex(PyObject* arr);
      static bool isComplex(const PyObject* arr);

      static unsigned getUintVal(PyObject* arr);
      static unsigned getUintVal(const PyObject* arr);
      unsigned getUintVal();

      static bool getBoolVal(PyObject* arr, unsigned iEl=0);
      static bool getBoolVal(const PyObject* arr, unsigned iEl=0);
      bool getBoolVal(unsigned iEl=0);

      // Return the number of dimensions in an PyObject

      static int getNumberOfDimensions(PyObject* arr);
      static int getNumberOfDimensions(const PyObject* arr);
      int getNumberOfDimensions();
      
#if DIR_HAVE_NUMPY
      // Numpy object handling

      static double* getNumpyDoublePtr(PyObject* arr, bool directData);
      static std::vector<int> getNumpyDimensions(PyObject* arr);
      static unsigned getNumpyNDimensions(PyObject* arr);
      static int getNumpyLength(PyObject* arr, unsigned index);
#endif

      static unsigned int getNumberOfDictEntries(const PyObject* arr);
      static unsigned int getNumberOfDictEntries(PyObject* arr);

      // Return the vector of dimensions of an PyObject

      static std::vector<int> getDimensions(PyObject* arr);
      static std::vector<int> getDimensions(const PyObject* arr);
      std::vector<int> getDimensions();

      static int getDimension(PyObject* arr, unsigned iDim);
      static int getDimension(const PyObject* arr, unsigned iDim);
      int getDimension(unsigned iDim);

      static PyObject* getArrayItem(PyObject* arr, Py_ssize_t index);

      static unsigned int getNumberOfElements(PyObject* arr);
      static unsigned int getNumberOfElements(const PyObject* arr);
      unsigned int getNumberOfElements();

      int getMaxDimension();

      static unsigned getSize(const PyObject* obj);
      static unsigned getSize(PyObject* obj);
      unsigned getSize();

      void printDimensions();

      // Return the string corresponding to this PyObject

      static std::string getString(PyObject* arr, unsigned iEl=0);
      static std::string getString(const PyObject* arr, unsigned iEl=0);
      std::string getString(unsigned iEl=0);

      static float getFloat(PyObject* arr, unsigned iEl=0);
      static float getFloat(const PyObject* arr, unsigned iEl=0);
      float getFloat(unsigned iEl=0);

      // Return true if the dimensions of two PyObjects match

      static bool dimensionsMatch(PyObject* arr1, PyObject* arr2);
      static bool dimensionsMatch(const PyObject* arr1, const PyObject* arr2);

      bool operator==(PyParser& mp);
      bool operator!=(PyParser& mp);

    private:
      
      const PyObject* array_;

    }; // End class PyParser
    
  } // End namespace python
} // End namespace gcp

#endif // End #ifndef GCP_UTIL_PYPARSER_H
