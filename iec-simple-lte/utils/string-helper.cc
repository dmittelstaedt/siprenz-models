#include "string-helper.h"

using namespace std;

// Function to get a value as string
string StringHelper::toString(double const& value) {
     stringstream sstr;
     sstr << value;
     return sstr.str();
}
