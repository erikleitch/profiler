#ifndef GCP_UTIL_STRING_H
#define GCP_UTIL_STRING_H

/**
 * @file String.h
 * 
 * Tagged: Wed May 12 09:30:13 PDT 2004
 * 
 * @author Erik Leitch
 */
#include <string>
#include <vector>

namespace gcp {
  namespace util {
    
    class String {
    public:
      
      // Constructor.

      String();
      String(unsigned int, bool convertNonAscii = false);
      String(const std::string& str, bool convertNonAscii = false);
      String(const std::vector<unsigned char>& ucvec, bool convertNonAscii = false);
      String(const std::vector<char>& ucvec, bool convertNonAscii = false);
      
      // Destructor.

      virtual ~String();
      
      static void strip(std::string& targetStr, const std::string& stripStr);
      static void strip(std::string& targetStr, char stripChar);

      void strip(const std::string& stripStr);
      void strip(char stripChar);
      
      bool contains(char c);
      bool contains(std::string s);
      bool remainderContains(std::string s);
      String remainder();
      String getNextNchars(unsigned nChar);

      void replace(char stripChar, char replaceChar);
      void replace(std::string stripStr, std::string replaceStr);

      static void replace(std::string& targetStr, char stripChar, 
			  char replaceChar);

      void superscriptNumbersForPgplot();

      void advanceToNextNonWhitespaceChar();
      void advanceToNextNumericChar();
      void advanceToNextNonNumericChar();
      void advance(unsigned nChar);

      bool isNumeric(char c);
      bool isNumeric();

      void operator=(const std::string& str);
      void operator=(const String str);
      
      bool operator<(String& str); 
      bool operator==(String str);
      bool operator==(std::string str);
      bool operator!=(std::string str);
      bool operator!=(String str);

      char& operator[](unsigned int index);

      inline std::string& str() {
	return str_;
      }
      
      // Allows cout << String

      friend std::ostream& operator<<(std::ostream& os, String str);
      
      String findFirstInstanceOf(std::string start, bool useStart, 
				 std::string stop, bool useStop);

      String findFirstInstanceOf(std::string start, std::string stop);

      String findFirstInstanceOf(std::string stop);

      String findNextInstanceOf(std::string start, bool useStart, 
				std::string stop, bool useStop, 
				bool consumeStop=false);

      String findNextInstanceOf(std::string start, std::string stop);

      String findNextInstanceOf(std::string stop);

      String findNextString(bool consumeEnd=true);
      String findNextNumericString();
      String findNextNumericString(std::string allowedChars);
      String findNextNonNumericString();

      String toLower();
      String toUpper();

      static String toLower(std::string);
      static String toUpper(std::string);

      static const std::string emptyString_;
      static const std::string whiteSpace_;

      String findNextStringSeparatedByChars(std::string separators, 
					    bool matchEndOfString=true);

      bool atEnd();
      void resetToBeginning();

      void initialize();
      
      static int toInt(std::string s);

      int toInt();
      unsigned int toUint();
      float toFloat();
      double toDouble();
      bool toBool();

      bool isValidPowerExpression();
      double getPowerExpression();

      double getInfExpression();
      bool isValidInfExpression();

      bool isEmpty();
      bool matches(unsigned char c, std::string matchSet);

      unsigned size();

      // Convert from string representation of non-ascii bytes to
      // bytes
      
      static std::vector<unsigned char> 
	stringToBytes(std::string s, bool convertNonAscii=false);

      static void parseNonAscii(std::string& s, 
				unsigned int& iChar, unsigned int nChar,
				std::vector<unsigned char>& ucvec);

      static std::string bytesToString(std::vector<unsigned char> ucvec, bool convertNonAscii = false);
      static std::string bytesToString(std::vector<char> cvec, bool convertNonAscii = false); 

      std::vector<double> parseRange();
      std::vector<double> parseRangeToken(String& tok);

      std::vector<unsigned char> getData();

      void expandTilde();
      bool expandTildeIter();
      void expandAndReplace(std::ostringstream& os, 
			    std::string::iterator& i1, 
			    std::string::iterator& i2);

      static std::string formatHumanReadableInteger(unsigned val);

      static std::string numericalSuffix(int i);

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

  } // End namespace util
} // End namespace gcp




#endif // End #ifndef GCP_UTIL_STRING_H
