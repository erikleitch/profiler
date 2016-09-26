#include "exceptionutils.h"
#include "String.h"

#include <iostream>
#include <sstream>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <cmath>

using namespace std;
using namespace gcp::util;

const std::string String::emptyString_("");
const std::string String::whiteSpace_(" \t");

/**.......................................................................
 * Constructor.
 */
String::String() 
{
  initialize();
}

String::String(unsigned int iVal, bool convertNonAscii) 
{
  initialize();
  std::ostringstream os;
  os << iVal;
  str_   = os.str();
  ucvec_ = stringToBytes(str_, convertNonAscii);
}

String::String(const std::string& str, bool convertNonAscii) 
{
  initialize();

  // If there are non-ascii characters to be converted, convert to
  // bytes, then back to string.  Though this seems redundant, it
  // allows printable characters to be represented by their ASCII
  // codes.  Those codes will be converted to their printable ASCII
  // equivalents by bytesToString(), allowing predictable comparison
  // to other strings treated in the same way.

  ucvec_    = stringToBytes(str, convertNonAscii);
  str_      = bytesToString(ucvec_, convertNonAscii);
  convertNonAscii_ = convertNonAscii;
}

String::String(const std::vector<unsigned char>& ucvec, bool convertNonAscii)
{
  initialize();
  ucvec_    = ucvec;
  str_      = bytesToString(ucvec, convertNonAscii);
  convertNonAscii_ = convertNonAscii;
}

String::String(const std::vector<char>& cvec, bool convertNonAscii)
{
  initialize();
  ucvec_.resize(cvec.size());

  unsigned nChar = cvec.size();
  std::ostringstream os;

  for(unsigned iChar=0; iChar < nChar; iChar++) {

    ucvec_[iChar] = cvec[iChar];

    if(!isprint(ucvec_[iChar]) && convertNonAscii) {
      os << "(0x" << (unsigned int)ucvec_[iChar] << ")";
    } else {
      os << ucvec_[iChar];
    }

  }

  str_ = os.str();
  convertNonAscii_ = convertNonAscii;
}

void String::initialize()
{
  convertNonAscii_ = false;
  iStart_ = 0;
}

/**.......................................................................
 * Destructor.
 */
String::~String() {}

/**.......................................................................
 * Strip all occurrences of the characters in stripStr from a target
 * string.
 */
void String::strip(std::string& targetStr, const std::string& stripStr)
{
  // For each char in the strip string, remove all occurrences in the
  // target string

  for(unsigned istrip=0; istrip < stripStr.size(); istrip++)
    strip(targetStr, stripStr[istrip]);
}

/**.......................................................................
 * Strip all occurrences of the characters in stripStr from a target
 * string.
 */
void String::strip(const std::string& stripStr)
{
  strip(str_, stripStr);
}

/**.......................................................................
 * Strip all occurrences of a character from a target string.
 */
void String::strip(std::string& targetStr, char stripChar)
{
  bool erased;

  do {
    erased = false;
    
    std::string::size_type idx;
    idx = targetStr.find(stripChar);
    
    if(idx != std::string::npos) {
      targetStr.erase(idx,1);
      erased = true;
    }
    
  } while(erased);

}

/**.......................................................................
 * Strip all occurrences of a character from a target string.
 */
void String::strip(char stripChar)
{
  strip(str_, stripChar);
}

/**.......................................................................
 * Return true if our string contains the target character
 */
bool String::contains(char c)
{
  std::string cmp = convert(c);
  return contains(cmp);
}

/**.......................................................................
 * Return true if our string contains the target string
 */
bool String::contains(string s)
{
  std::string cmp = convert(s);

  std::string::size_type idx;

  idx = str_.find(cmp);

  return idx != std::string::npos;
}

/**.......................................................................                 
 * Return true if the (unsearched) remainder of our string contains                        
 * the target string                                                                       
 */
bool String::remainderContains(string s)
{
  std::string cmp = convert(s);

  std::string::size_type idx;

  idx = str_.find(cmp, iStart_);

  return idx != std::string::npos;
}

std::string String::convert(char& c)
{
  std::ostringstream os;

  if(!isprint(c) && convertNonAscii_) {
    os << "(0x" << (unsigned int)c << ")";
  } else {
    os << c;
  }

  return os.str();
}

std::string String::convert(string& s)
{
  std::vector<unsigned char> ucvec = stringToBytes(s, convertNonAscii_);
  return bytesToString(ucvec, convertNonAscii_);
}

bool String::matches(unsigned char c, std::string matchSet)
{
  for(unsigned iEl=0; iEl < matchSet.size(); iEl++)
    if(c == matchSet[iEl])
      return true;

  return false;
}

/**......................................................................
 * Assignment operators
 */
void String::operator=(const std::string& str)
{
  iStart_ = 0;
  str_    = str;
}

void String::operator=(const String str)
{
  iStart_ = str.iStart_;
  str_    = str.str_;
}

bool String::operator==(String str)
{
  return str_ == str.str_;
}

bool String::operator==(string str)
{
  return str_ == str;
}

bool String::operator!=(string str)
{
  return str_ != str;
}

char& String::operator[](unsigned int index)
{
  return str_[index];
}

bool String::operator!=(String str)
{
  return str_ != str.str_;
}

bool String::operator<(String& str)
{
  unsigned thisLen = str_.size();
  unsigned thatLen = str.size();
  unsigned minLen = (thisLen < thatLen) ? thisLen : thatLen;

  if(*this == str)
    return false;

  char c1, c2;
  for(unsigned i=0; i < minLen; i++) {
    c1 = str_[i];
    c2 = str[i];

    if(c1 == c2)
      continue;

    if(islower(c1)) {
      if(islower(c2)) {
	return c1 < c2;
      } else {
	return true;
      }
    } else {
      if(isupper(c2)) {
	return c1 < c2;
      } else {
	return false;
      }
    }
  }

  // Else the strings are equal to the minimum length.  Then the
  // shorted string will be alphabetized first

  return thisLen < thatLen;
}

/**.......................................................................
 * Allows cout << String
 */
std::ostream& gcp::util::operator<<(std::ostream& os, String str)
{
  os << str.str_;
  return os;
}

String String::findFirstInstanceOf(std::string stop)
{
  iStart_ = 0;
  return findNextInstanceOf("", false, stop, true);
}

String String::findFirstInstanceOf(std::string start, std::string stop)
{
  iStart_ = 0;
  return findNextInstanceOf(start, true, stop, true);
}

String String::findNextInstanceOf(std::string stop)
{
  return findNextInstanceOf("", false, stop, true);
}

String String::findNextInstanceOf(std::string start, std::string stop)
{
  return findNextInstanceOf(start, true, stop, true);
}

String String::findNextString(bool consumeEnd)
{
  return findNextInstanceOf(" ", false, " ", false, consumeEnd);
}

/**.......................................................................
 * Search a string for substrings separated by the specified start
 * and stop strings.  
 *
 * If useStart = true, then we will search for the start string first, then
 *                     search for the stop string, and return everything 
 *                     between them.
 * 
 *                     otherwise, we will just search for the end string
 *                     and return everything up to it.
 */
String String::findNextInstanceOf(std::string start, bool useStart, 
				  std::string stop,  bool useStop, bool consumeStop)
{
  String retStr;
  std::string::size_type iStart=iStart_, iStop=0;

  //------------------------------------------------------------
  // If we are searching for the next string separated by the start
  // and stop strings, then start by searching for the start string
  //------------------------------------------------------------

  if(useStart) {

    iStart = str_.find(start, iStart_);

    //------------------------------------------------------------
    // Return empty string if we hit the end of the string without
    // finding a match
    //------------------------------------------------------------

    if(iStart == std::string::npos) {
      return retStr;
    }
    
    //------------------------------------------------------------
    // Else start the search for the end string at the end of the
    // string we just found
    //------------------------------------------------------------

    iStart += start.size();
  }

  //------------------------------------------------------------
  // Now search for the stop string
  //------------------------------------------------------------

  iStop = str_.find(stop, iStart);

  //------------------------------------------------------------
  // If we insist that the stop string is present, return an empty
  // string if it wasn't
  //------------------------------------------------------------

  if(useStop) {
    if(iStop == std::string::npos) {
      return retStr;
    }
  }

  //------------------------------------------------------------
  // Else match anything up to the stop string, if it was found, or
  // the end of the string if it wasn't
  //------------------------------------------------------------

  if(iStop == std::string::npos) {
    retStr = str_.substr(iStart, str_.size()-iStart);
  } else {
    retStr = str_.substr(iStart, iStop-iStart);
  }

  //------------------------------------------------------------
  // We will start the next search at the _beginning_ of the stop
  // string, so just increment iStart_ to point to the first char of
  // the stop string (or end of string, if the stop string wasn't found)
   //------------------------------------------------------------

  iStart_ = (iStop == std::string::npos) ? str_.size() : iStop;

  //------------------------------------------------------------
  // If consumeStop = true, then start the next search just _after_
  // the stop string (if it was found).
  //------------------------------------------------------------

  if(consumeStop && iStop != std::string::npos) {
    iStart_ += stop.size();
  }

  return retStr;
}

/**.......................................................................
 * Return up tp the next non-numeric string
 */
String String::findNextNumericString()
{
  String retStr;
  std::string::size_type iStart=0, iStop=0;

  iStart = iStart_;

  for(iStop=iStart_; iStop < str_.size() && isNumeric(str_[iStop]); iStop++);

  iStart_ = iStop;
  
  retStr = str_.substr(iStart, iStop-iStart);

  return retStr;
}

/**.......................................................................
 * Return up tp the next non-numeric string, including any allowed characters
 */
String String::findNextNumericString(std::string allowedChars)
{
  String retStr;
  std::string::size_type iStart=0, iStop=0;

  String allowedCharsStr(allowedChars);

  iStart = iStart_;

  for(iStop=iStart_; iStop < str_.size() && (isNumeric(str_[iStop]) || allowedCharsStr.contains(str_[iStop])); iStop++)
    ;

  iStart_ = iStop;
  
  retStr = str_.substr(iStart, iStop-iStart);

  return retStr;
}

/**.......................................................................
 * Return up tp the next numeric string
 */
String String::findNextNonNumericString()
{
  String retStr;
  std::string::size_type iStart=0, iStop=0;

  iStart = iStart_;

  for(iStop=iStart_; iStop < str_.size() && !isNumeric(str_[iStop]); iStop++);

  iStart_ = iStop;
  
  retStr = str_.substr(iStart, iStop-iStart);

  return retStr;
}

String String::getNextNchars(unsigned nChar)
{
  String retStr;

  if(str_.size() - iStart_ < nChar)
    ThrowRuntimeError("Remaining string is too short (" << (str_.size()-iStart_) << ") to retrieve " << nChar << " chars.");

  retStr = str_.substr(iStart_, nChar);
  iStart_ += nChar;

  return retStr;
}

bool String::atEnd()
{
  return iStart_ == size() || iStart_ == std::string::npos;
}

bool String::toBool()
{
  String str = toLower();

  if(str == "true") {
    return true;
  } else if(str == "false") {
    return false;
  } else {
    ThrowRuntimeError("Unrecognized boolean value: " << str_);
    return false;
  }
}

int String::toInt()
{
  return toInt(str_);
}

unsigned int String::toUint()
{
  return (unsigned)toInt(str_);
}

int String::toInt(std::string str)
{
  std::vector<char> copy = constructCharVec(str);
  char* endPtr = &copy[0];

  errno = 0;
  int iVal = strtol(str.c_str(), &endPtr, 10);

  if(str[0] == '\0') {
    ThrowRuntimeError("Cannot convert '" << str << "' to a valid integer");
  } else if(iVal==0 && endPtr[0] != '\0') {
    ThrowRuntimeError("Cannot convert '" << str << "' to a valid integer");
  }

  if(iVal==0 && errno==EINVAL) {
    ThrowRuntimeError("Cannot convert '" << str << "' to a valid integer");
  }

  return iVal;
}

float String::toFloat()
{
  std::vector<char> copy = constructCharVec(str_);
  char* endPtr = &copy[0];

  errno = 0;
  float fVal = strtof(str_.c_str(), &endPtr);

  if(str_[0] == '\0') {
    ThrowRuntimeError("Cannot convert '" << str_ << "' to a valid float");
  } else if(fVal==0.0 && endPtr[0] != '\0') {
    ThrowRuntimeError("Cannot convert '" << str_ << "' to a valid float");
  }

  if(fVal==0.0 && errno==EINVAL) {
    ThrowRuntimeError("Cannot convert '" << str_ << "' to a valid float");
  }

  return fVal;
}

bool String::isValidInfExpression()
{
  String str = toLower();
  str.strip(' ');

  return str == "-inf" || str == "inf";
}

double String::getInfExpression()
{
  String str = toLower();
  str.strip(' ');

  if(str == "-inf")
    return -1.0/0.0;
  else
    return  1.0/0.0;
}

bool String::isValidPowerExpression()
{
  if(contains('^')) {
    String copy(*this);

    String base  = copy.findNextInstanceOf(" ", false, "^", true, false);

    COUT("base = " << base);

    if(base.isEmpty())
      return false;

    errno = 0;
    double baseVal = strtod(base.str().c_str(), NULL);

    if(errno==EINVAL)
      return false;

    String expnt = copy.findNextInstanceOf("^", true, " ", false, true);

    if(expnt.isEmpty())
      return false;

    errno = 0;
    double expntVal = strtod(expnt.str().c_str(), NULL);

    if(errno==EINVAL)
      return false;

    COUT("Found valid power expression with base = " << base << " expnt = " << expnt << " expntVal = " << expntVal << " baseVal = " << baseVal);
    return true;
  }

  return false;
}

double String::getPowerExpression()
{
  if(contains('^')) {
    String copy(*this);
    String base  = copy.findNextInstanceOf(" ", false, "^", true, false);

    if(base.isEmpty())
      ThrowRuntimeError("String does not contain a valid power expression");

    errno = 0;
    double baseVal = strtod(base.str().c_str(), NULL);

    if(errno==EINVAL)
      ThrowRuntimeError("String does not contain a valid power expression");

    String expnt = copy.findNextInstanceOf("^", true, " ", false, true);

    if(expnt.isEmpty())
      ThrowRuntimeError("String does not contain a valid power expression");

    errno = 0;
    double expntVal = strtod(expnt.str().c_str(), NULL);

    if(errno==EINVAL)
      ThrowRuntimeError("String does not contain a valid power expression");

    COUT("Found power expression with: " << baseVal << " " << expntVal);

    return pow(baseVal, expntVal);
  }

  return 0.0;
}

double String::toDouble()
{
  if(isValidPowerExpression()) {
    return getPowerExpression();
  } else if(isValidInfExpression()) {
    return getInfExpression();
  } else {

    std::vector<char> copy = constructCharVec(str_);
    char* endPtr = &copy[0];

    errno = 0;
    double dVal = strtod(str_.c_str(), &endPtr);
    
    if(str_[0] == '\0') {
      ThrowRuntimeError("Cannot convert '" << str_ << "' to a valid double");
    } else if(dVal==0.0 && endPtr[0] != '\0') {
      ThrowRuntimeError("Cannot convert '" << str_ << "' to a valid double");
    }
    
    if(dVal==0.0 && errno==EINVAL) {
      ThrowRuntimeError("Cannot convert '" << str_ << "' to a valid double");
    }
    
    return dVal;
  }
}

bool String::isEmpty()
{
  return str_.size() == 0 || str_[0] == '\0';
}

/**.......................................................................
 * Return the next string separated by any of the chars in separators
 */
String String::findNextStringSeparatedByChars(std::string separators, 
					      bool matchEndOfString)
{
  String retStr("");
  unsigned iEl=0;

  for(iEl=iStart_; iEl < str_.size(); iEl++) {
    if(matches(str_[iEl], separators)) {
      break;
    }
  }

  // If we hit the end of the string without finding a match, return
  // an empty string, and don't advance the start counter.  Unless the
  // separator string was the empty string, in which case we will
  // match end of string too

  if(iEl == str_.size() && !matchEndOfString)
    return retStr;

  // Else extract the portion of the string that matches

  retStr = str_.substr(iStart_, iEl-iStart_);

  // Now consume any trailing characters that also match, including whitespace

  for(unsigned iMatch = iEl; iMatch < str_.size(); iMatch++) {
    if(matches(str_[iMatch], separators) || matches(str_[iMatch], whiteSpace_))
      iEl++;
    else
      break;
  }

  // Advance iStart_ to the first non-matching character

  iStart_ = iEl;

  // And return the matched substring

  return retStr;
}

unsigned String::size()
{
  return str_.size();
}

void String::resetToBeginning()
{
  iStart_ = 0;
}

/**.......................................................................
 * Replace all occurrences of a character with a replacement character
 */
void String::replace(char stripChar, char replaceChar)
{
  replace(str_, stripChar, replaceChar);
}

/**.......................................................................
 * Replace all occurrences of a character with a replacement character
 */
void String::replace(std::string& targetStr, char stripChar, char replaceChar)
{
  bool replaced=false;

  do {
    replaced = false;
    
    std::string::size_type idx;
    idx = targetStr.find(stripChar);
    
    if(idx != std::string::npos) {
      targetStr[idx] = replaceChar;
      replaced = true;
    }
    
  } while(replaced);
}

void String::replace(std::string stripStr, std::string replaceStr)
{
  bool found = false;
  size_t stripSize   = stripStr.size();
  size_t replaceSize = replaceStr.size();
  size_t pos = 0;

  std::ostringstream os;

  do {
    os.str("");
    found = false;

    size_t start = str_.find(stripStr, pos);

    // If we found a match, reconstruct the string around it

    if(start != string::npos) {

      os << str_.substr(0, start);
      os << replaceStr;
      os << str_.substr(start+stripSize, string::npos);

      // Next search should start just past the string we replaced

      pos += start+replaceSize;

      found = true;
      str_ = os.str();
    }

  } while(found);
}

void String::superscriptNumbersForPgplot()
{
  bool found = false;
  std::ostringstream os;

  do {
    os.str("");
    found = false;

    size_t start = str_.find("^", 0);

    //------------------------------------------------------------
    // If we found a match, reconstruct the string around it
    //------------------------------------------------------------

    if(start != string::npos) {
      
      if(start < str_.size()-1 && (isdigit(str_[start+1]) || str_[start+1] == '-' || str_[start+1] == '+')) {

	os.str("");
	os << str_.substr(0, start) << "\\u";

	size_t iStr = start+1;
	for(iStr = start+1; iStr < str_.size() && (isdigit(str_[iStr]) || str_[iStr] == '-' || str_[iStr] == '+'); iStr++) {
	  os << str_[iStr];
	}

	os << "\\d" << str_.substr(iStr, string::npos);
	
	found = true;

	str_ = os.str();
      }
    }

  } while(found);
}

/**.......................................................................
 * Return an lowercase version of this string
 */
String String::toLower()
{
  String retStr = str_;

  for(unsigned i=0; i < size(); i++) {
    retStr[i] = tolower(retStr[i]);
  }

  return retStr;
}

/**.......................................................................
 * Return an uppercase version of this string
 */
String String::toUpper()
{
  String retStr = str_;

  for(unsigned i=0; i < size(); i++) {
    retStr[i] = toupper(retStr[i]);
  }

  return retStr;
}

/**.......................................................................
 * Return an lowercase version of this string
 */
String String::toLower(std::string str) {
  String strStr(str);
  return strStr.toLower();
}

String String::toUpper(std::string str) {
  String strStr(str);
  return strStr.toUpper();
}

/**.......................................................................
 * Convert from string representation to a byte representation.
 *
 * If convertNonAscii == true, convert string representations of
 * non-ascii bytes to bytes
 */
std::string String::bytesToString(std::vector<unsigned char> ucvec, bool convertNonAscii)
{
  unsigned nChar = ucvec.size();
  std::ostringstream os;

  for(unsigned iChar=0; iChar < nChar; iChar++) {
 
    if(!isprint(ucvec[iChar]) && convertNonAscii) {
      os << "(0x" << (unsigned int)ucvec[iChar] << ")";
    } else {
      os << ucvec[iChar];
    }

  }

  return os.str();
}

/**.......................................................................
 * Convert from string representation to a byte representation.
 *
 * If convertNonAscii == true, convert string representations of
 * non-ascii bytes to bytes
 */
std::string String::bytesToString(std::vector<char> cvec, bool convertNonAscii)
{
  unsigned nChar = cvec.size();
  std::ostringstream os;

  for(unsigned iChar=0; iChar < nChar; iChar++) {
 
    if(!isprint(cvec[iChar]) && convertNonAscii) {
      os << "(0x" << (unsigned int)cvec[iChar] << ")";
    } else {
      os << cvec[iChar];
    }

  }

  return os.str();
}

/**.......................................................................
 * Convert from string representation to a byte representation.
 *
 * If convertNonAscii == true, convert string representations of
 * non-ascii bytes to bytes
 */
std::vector<unsigned char> String::stringToBytes(std::string s, 
						 bool convertNonAscii)
{
  std::vector<unsigned char> ucvec;

  // string::size() returns the size of the string including the
  // terminating NULL character.  Set the size to size()-1 so that
  // this terminator is no interpreted as an extra byte

  unsigned nChar = s.size();
  unsigned iChar = 0;

  do {

    unsigned char c = s[iChar];

    if(c == '(' && convertNonAscii) {
      parseNonAscii(s, iChar, nChar, ucvec);
    } else {
      ucvec.push_back(c);
      ++iChar;
    }

  } while(iChar < nChar);

  return ucvec;
}

/**.......................................................................
 * Parse a string representation of a non-ascii byte into a byte
 */
void String::parseNonAscii(std::string& s, 
			   unsigned int& iChar, unsigned int nChar,
			   std::vector<unsigned char>& ucvec)
{
  unsigned iCurr = iChar;
  std::ostringstream os;

  if(++iCurr < nChar && s[iCurr] == '0') {

    if(++iCurr < nChar && s[iCurr] == 'x') {

      while(++iCurr < nChar && s[iCurr] != ')') {
	os << s[iCurr];
      }

      // If we found an end-brace, convert the decimal number to the
      // equivalent byte

      if(s[iCurr] == ')') {
	unsigned int iVal = (unsigned int)toInt(os.str());
	ucvec.push_back((unsigned char)iVal);
	iChar = ++iCurr;
	return;
      } else {
	ThrowRuntimeError("String ended before conversion was complete");
      }
    }
    
  } else {
    return;
  }
}

std::vector<unsigned char> String::getData()
{
  return stringToBytes(str_, convertNonAscii_);
}

String String::remainder()
{
  if(iStart_ >= str_.size()) {
    return String("");
  } else {
    return  str_.substr(iStart_, str_.size()-iStart_);
  }
}

/**.......................................................................      
 * Advance the start pointer of this string to the first non-whitespace char    
 */
void String::advance(unsigned nChar)
{
  if(iStart_ + nChar < str_.size()) {
    iStart_ += nChar;
    return;
  }

  ThrowRuntimeError("Advancing by " << nChar << " would exceed the string length");
}

/**.......................................................................      
 * Advance the start pointer of this string to the first non-whitespace char    
 */
void String::advanceToNextNonWhitespaceChar()
{
  for(; iStart_ < str_.size(); iStart_++) {
    if(!matches(str_[iStart_], whiteSpace_))
      break;
  }
}

/**.......................................................................      
 * Advance the start pointer of this string to the first numeric char
 */
void String::advanceToNextNumericChar()
{
  for(; iStart_ < str_.size(); iStart_++) {
    char c = str_[iStart_];
    if(isNumeric(c))
      break;
  }
}

/**.......................................................................      
 * Advance the start pointer of this string to the first non-numeric char
 */
void String::advanceToNextNonNumericChar()
{
  for(; iStart_ < str_.size(); iStart_++) {
    char c = str_[iStart_];
    if(!isNumeric(c))
      break;
  }
}

bool String::isNumeric(char c)
{
  return isdigit(c) || c=='.' || c=='e' || c=='+' || c=='-';
}

void String::expandTilde()
{
  bool found = false;

  do {
    found = expandTildeIter();
  } while(found);
}

bool String::expandTildeIter()
{
  std::string::iterator i1=str_.end();
  std::string::iterator i2=str_.end();

  std::ostringstream os;

  // Search through the entire string for occurences of ~username

  for(std::string::iterator iter=str_.begin(); iter != str_.end(); iter++) {

    if(*iter == '~') {
      i1 = iter;
    } else if(i1 != str_.end()) {

      if(isalnum(*iter)) {
	i2 = iter;
	os << *iter;
      } else {
	expandAndReplace(os, i1, i2);
	return true;
      }

    }
  }

  // If the string ended while we were constructing a username,
  // replace it now

  if(i1 != str_.end()) {
    expandAndReplace(os, i1, i2);
    return true;
  } else {
    return false;
  }
}

void String::expandAndReplace(std::ostringstream& os, std::string::iterator& i1, std::string::iterator& i2) 
{
  struct passwd* pw = getpwnam(os.str().c_str());
  
  if(!pw) 
    ThrowSysError("getpwnam");
  
  std::string expanded(pw->pw_dir);
  str_.replace(i1, i2+1, expanded);
}

std::string String::formatHumanReadableInteger(unsigned val)
{
  ostringstream initVal;
  ostringstream retVal;

  initVal << val;

  std::string initValStr = initVal.str();

  unsigned nDigit = (unsigned)log10((double)val) + 1;

  if(nDigit < 4)
    retVal << val;
  else {

    unsigned nLast = nDigit % 3;
    unsigned nGroup = nDigit/3;

    unsigned ind=0;

    for(; ind < nLast; ind++)
      retVal << initValStr[ind];

    if(nLast > 0)
      retVal << ",";

    for(unsigned iGroup=0; iGroup < nGroup; iGroup++) {
      retVal << initValStr[ind] << initValStr[ind+1] << initValStr[ind+2];

      if(iGroup < nGroup-1)
	retVal << ",";

      ind += 3;
    }
  }

  return retVal.str();
}

std::string String::numericalSuffix(int i)
{
  switch (i) {
  case 1:
    return "st";
    break;
  case 2:
    return "nd";
    break;
  case 3:
    return "rd";
    break;
  default:
    return "th";
  }
}

/**.......................................................................
 * Parse a string that represents a range, list or single value
 */
std::vector<double> String::parseRange()
{
  std::vector<double> vals;

  String nextVal;
    
    do {
      nextVal = findNextStringSeparatedByChars(",", true);
      
      COUT("nextVal = " << nextVal << " empty = " << nextVal.isEmpty());

      if(!nextVal.isEmpty()) {
	std::vector<double> newVals = parseRangeToken(nextVal);

	for(unsigned iNewVal = 0; iNewVal < newVals.size(); iNewVal++) 
	  vals.push_back(newVals[iNewVal]);
      }
      
    } while(!nextVal.isEmpty());
    
  
  return vals;
}

std::vector<double> String::parseRangeToken(String& tok)
{
  std::vector<double> vec;

  COUT("Inside prt with tok = " << tok);

  //------------------------------------------------------------
  // Is this a range, of the form 16-23, or 16-18:2 ?
  //------------------------------------------------------------

  if(tok.contains("-")) {
    String startStr = tok.findNextInstanceOf("", false, "-", true, true);
    String stopStr;
    double deltaVal = 1.0;

    COUT("start = " << startStr);
    COUT("remainder = " << tok.remainder());

    // If the remainder contains a ':', then this is a delta

    if(tok.contains(":")) {
      stopStr  = tok.findNextInstanceOf("-", false, ":", true, true);
      deltaVal = tok.remainder().toDouble();

      // Else just a simple range

    } else {
      stopStr  = tok.remainder();
      COUT("stop = " << stopStr);

    }

    double startVal = startStr.toDouble();
    double stopVal  = stopStr.toDouble();

    if(startVal > stopVal)
      ThrowRuntimeError("Invalid range: " << tok);

    unsigned nVal = (unsigned)((stopVal - startVal)/deltaVal + 1);
    COUT("nVal = " << nVal);
    vec.resize(nVal);

    for(unsigned iVal=0; iVal < nVal; iVal++) {
      vec[iVal] = startVal + deltaVal * iVal;
    }
  } else {
    vec.resize(1);
    vec[0] = tok.toDouble();
  }

  return vec;
}

 std::vector<char> String::constructCharVec(std::string& str)
{
  std::vector<char> charVec(str.size() + 1);

  for(unsigned i=0; i < str.size(); i++)
    charVec[i] = str[i];

  return charVec;
}

bool String::isNumeric()
{
  try {
    toDouble();
    return true;
  } catch(...) {
    return false;
  }
}
