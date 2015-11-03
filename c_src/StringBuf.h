// $Id: $

#ifndef NIFUTIL_STRINGBUF_H
#define NIFUTIL_STRINGBUF_H

#define STRING_BUF_MIN_SIZE 1024

/**
 * @file StringBuf.h
 * 
 * Tagged: Tue Oct 20 16:30:05 PDT 2015
 * 
 * @version: $Revision: $, $Date: $
 * 
 * @author /bin/bash: username: command not found
 */
#include <stdlib.h>
#include <stddef.h>

#include <string>

namespace nifutil {

    class StringBuf {
    public:
        
        // Constructors

        StringBuf();
        StringBuf(size_t size);

        // Copy constructors

        StringBuf(const StringBuf& buf);
        StringBuf(StringBuf& buf);

        virtual ~StringBuf();

        // Assignment operators

        void operator=(StringBuf& buf);
        void operator=(const StringBuf& buf);

        // Resize our internal buffer

        void resize(size_t size);

        // Copy a byte array into our buffer

        void copy(char* buf, size_t size);

        // Return the buffer size

        size_t bufSize();

        // Return the data size

        size_t dataSize();

        // Get a pointer to the buffer

        char*  getBuf();

    private:

        void initialize();

        // A fixed-size internal buffer

        char   fixedBuf_[STRING_BUF_MIN_SIZE];

        // If required, a heap-allocated buffer

        char*  heapBuf_;

        // A pointer to whichever buffer is currently relevant

        char*  bufPtr_;

        // The actual size of the buffer
        
        size_t bufSize_;
        
        // The size of the data in the buffer
        
        size_t dataSize_;

    }; // End class StringBuf

} // End namespace nifutil



#endif // End #ifndef NIFUTIL_STRINGBUF_H
