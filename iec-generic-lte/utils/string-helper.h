#ifndef STRING_HELPER_H_
#define STRING_HELPER_H_

#include <string>
#include <sstream>

class StringHelper {
public:
     static std::string toString(double const& value);
     static std::string toString(unsigned int const& value);
};

#endif /* STRING_HELPER_H_ */
