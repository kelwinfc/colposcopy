#include "utils.hpp"

int num_chars(int n)
{
    std::stringstream ss;
    std::string s;
    
    ss << n;
    ss >> s;
    
    return s.size();
}

std::string spaced_d(int d, int n)
{
    std::stringstream ss;
    std::string ret, shift="";
    
    ss << d;
    ss >> ret;

    n -= ret.size();
    while ( n-- > 0 ){
        shift += " ";
    }

    return shift + ret;
}
