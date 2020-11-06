#include <iostream>
#include <string>
#include "jsp.hpp"

int main( void ) {
    std::string src = "{\"key1\":\"value for, \\\"key1\\\"\", \"key2\":100, \"key3\":{ \"key4\":\"value for key 4\", \"key5\":[-1.23, 2, 3000.123, 0.1234, 5], \"key6\":[\"str1\", \"str2\"] }, \"key7\":[ { \"key8\":100 }, { \"key9\":\"value for key 9\"} ], \"key10\": { \"key11\":{ \"key12\":1001 } } }";
    std::cout << "Original string: " << std::endl << src << std::endl << std::endl;    

    Jsp jsp(src);   // Parsing...         
    if( !jsp.ok() ) {   // If error
        std::cout << "Failed to parse the string! Exiting..." << std::endl;
        return 0;
    }
    // Stringifying
    std::string s;
    jsp.to_list_str(s);
    std::cout << "Stringified: " << s << std::endl;

    jsp.stringify(s);
    std::cout << s << std::endl << std::endl;

    int root = jsp.root();  // handle of the root element

    int n = jsp.getPairsNumber(root);   // The number of key-value pairs in root element
    std::cout << "The number of key-value pairs in the root is: " << n << std::endl;

    std::vector<int> *pairs = jsp.getPairs(root);   // The handles of pairs in root element

    std::string *key = jsp.getPairKey( (*pairs)[0] );  // The key of the 0-th key-value Pair in the "root"
    std::cout << "The key of the 0-th element in the root is: " << *key << std::endl;

    int type = jsp.getPairType( (*pairs)[0] );  // The type of the 0-th key-value Pair in the "root" 
    std::string *str = jsp.getStr( (*pairs)[0] );
    std::cout << "The value of the 0-th key-value pair in the root element is: " << *str << std::endl;

    int numArrHandle = jsp.getPairByKey( "key5" );
    std::vector<double> *numArr = jsp.getNumArr( numArrHandle );
    std::cout << "The number array for key5: ";
    for( int i = 0 ; i < (*numArr).size() ; i++ ) {
        std::cout << (*numArr)[i] << " ";
    }
    std::cout << std::endl;

    int strArrHandle = jsp.getPairByKey( "key6" );
    std::vector<std::string> *strArr = jsp.getStrArr( strArrHandle );
    std::cout << "The string array for key6: ";
    for( int i = 0 ; i < (*strArr).size() ; i++ ) {
        std::cout << (*strArr)[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}