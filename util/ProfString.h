#ifndef PROFILER_STRING_H
#define PROFILER_STRING_H

/**
 * @file String.h
 * 
 * Tagged: Wed May 12 09:30:13 PDT 2004
 * 
 * @author Erik Leitch
 */
#include <string>
#include <vector>

#include "export.h"

namespace profiler {
    
    class String {
    public:
      
        // Constructor.

        PROFILER_API String();
        PROFILER_API String(unsigned int, bool convertNonAscii = false);
        PROFILER_API String(const std::string& str, bool convertNonAscii = false);
        PROFILER_API String(const std::vector<unsigned char>& ucvec, bool convertNonAscii = false);
        PROFILER_API String(const std::vector<char>& ucvec, bool convertNonAscii = false);
      
        // Destructor.

        PROFILER_API virtual ~String();
      
        static PROFILER_API void strip(std::string& targetStr, const std::string& stripStr);
        static PROFILER_API void strip(std::string& targetStr, char stripChar);

        PROFILER_API void strip(const std::string& stripStr);
        PROFILER_API void strip(char stripChar);
      
        PROFILER_API bool contains(char c);
        PROFILER_API bool contains(std::string s);
        PROFILER_API bool remainderContains(std::string s);
        PROFILER_API String remainder();
        PROFILER_API String getNextNchars(unsigned nChar);

        PROFILER_API void replace(char stripChar, char replaceChar);
        PROFILER_API void replace(std::string stripStr, std::string replaceStr);

        PROFILER_API static void replace(std::string& targetStr, char stripChar, 
                                         char replaceChar);

        PROFILER_API void superscriptNumbersForPgplot();

        PROFILER_API void advanceToNextNonWhitespaceChar();
        PROFILER_API void advanceToNextNumericChar();
        PROFILER_API void advanceToNextNonNumericChar();
        PROFILER_API void advance(unsigned nChar);

        PROFILER_API bool isNumeric(char c);
        PROFILER_API bool isNumeric();

        PROFILER_API void operator=(const std::string& str);
        PROFILER_API void operator=(const String str);
      
        PROFILER_API bool operator<(String& str); 
        PROFILER_API bool operator==(String str);
        PROFILER_API bool operator==(std::string str);
        PROFILER_API bool operator!=(std::string str);
        PROFILER_API bool operator!=(String str);

        PROFILER_API char& operator[](unsigned int index);

        PROFILER_API std::string& str();      

        // Allows cout << String

        friend std::ostream& operator<<(std::ostream& os, String str);
      
        PROFILER_API String findFirstInstanceOf(std::string start, bool useStart, 
                                                std::string stop, bool useStop);

        PROFILER_API String findFirstInstanceOf(std::string start, std::string stop);

        PROFILER_API String findFirstInstanceOf(std::string stop);

        PROFILER_API String findNextInstanceOf(std::string start, bool useStart, 
                                  std::string stop, bool useStop, 
                                               bool consumeStop=false);
        
        PROFILER_API String findNextInstanceOf(std::string start, std::string stop);

        PROFILER_API String findNextInstanceOf(std::string stop);

        PROFILER_API String findNextString(bool consumeEnd=true);
        PROFILER_API String findNextNumericString();
        PROFILER_API String findNextNumericString(std::string allowedChars);
        PROFILER_API String findNextNonNumericString();

        PROFILER_API String toLower();
        PROFILER_API String toUpper();

        PROFILER_API static String toLower(std::string);
        PROFILER_API static String toUpper(std::string);

        PROFILER_API static std::string emptyString();
        PROFILER_API static std::string whiteSpace();

        PROFILER_API String findNextStringSeparatedByChars(std::string separators, 
                                                           bool matchEndOfString=true);
        
        PROFILER_API bool atEnd();
        PROFILER_API void resetToBeginning();

        PROFILER_API void initialize();
      
        PROFILER_API static int toInt(std::string s);

        PROFILER_API int toInt();
        PROFILER_API unsigned int toUint();
        PROFILER_API float toFloat();
        PROFILER_API double toDouble();
        PROFILER_API bool toBool();

        PROFILER_API bool isValidPowerExpression();
        PROFILER_API double getPowerExpression();

        PROFILER_API double getInfExpression();
        PROFILER_API bool isValidInfExpression();

        PROFILER_API bool isEmpty();
        PROFILER_API bool matches(unsigned char c, std::string matchSet);

        PROFILER_API unsigned size();

        // Convert from string representation of non-ascii bytes to
        // bytes
      
        PROFILER_API static std::vector<unsigned char> 
            stringToBytes(std::string s, bool convertNonAscii=false);
        
        PROFILER_API static void parseNonAscii(std::string& s, 
                                  unsigned int& iChar, unsigned int nChar,
                                  std::vector<unsigned char>& ucvec);

        PROFILER_API static std::string bytesToString(std::vector<unsigned char> ucvec, bool convertNonAscii = false);
        PROFILER_API static std::string bytesToString(std::vector<char> cvec, bool convertNonAscii = false); 

        PROFILER_API std::vector<double> parseRange();
        PROFILER_API std::vector<double> parseRangeToken(String& tok);

        PROFILER_API std::vector<unsigned char> getData();

        PROFILER_API void expandTilde();
        PROFILER_API bool expandTildeIter();
        PROFILER_API void expandAndReplace(std::ostringstream& os, 
                                           std::string::iterator& i1, 
                                           std::string::iterator& i2);

        PROFILER_API static std::string formatHumanReadableInteger(unsigned val);

        PROFILER_API static std::string numericalSuffix(int i);

    private:
      
        std::string::size_type iStart_;
        std::string str_;
        std::vector<unsigned char> ucvec_;

        // If true, the 'string' may contain non-ascii bytes.  In this
        // case, we represent non-printable characters by their decimal
        // integer equivalents, and any string to be compared to our
        // string is similary converted before the comparison is
        // performed

        bool convertNonAscii_;

        // Convert an input char or string to the appropriate
        // representation for comparison with this String.  For example,
        // if we can contain non-Ascii bytes, then any non-ascii bytes
        // in the input will have been converted to their decimal
        // integer equivalents

        std::string convert(char& c);
        std::string convert(std::string& s);

        static std::vector<char> constructCharVec(std::string& str);

    }; // End class String

    std::ostream& operator<<(std::ostream& os, String str);

} // End namespace profiler

#endif // End #ifndef PROFILER_STRING_H
