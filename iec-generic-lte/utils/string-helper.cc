#include "string-helper.h"

using namespace std;

// Function to get a double value as string
string StringHelper::toString(double const& value) {
     stringstream sstr;
     sstr << value;
     return sstr.str();
}

// Function to get a uint32_t value as string
string StringHelper::toString(unsigned int const& value) {
     stringstream sstr;
     sstr << value;
     return sstr.str();
}
