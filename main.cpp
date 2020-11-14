#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include "jsp.hpp"

int main( int argc, char **argv ) {

    std::string src = "{\"key1\":\"value for, \\\"key1\\\"\", \"key2\":100, \"keyBool\":true, \"keyNull\":null, \"key3\":{ \"key4\":\"value for key 4\", \"key5\":[-1.23, -2, 3000.123, 0.1234, 5], \"key6\":[\"str1\", \"str2\"] }, \"key7\":[ { \"key8\":100 }, { \"key9\":\"value for key 9\"} ], \"key10\": { \"key11\":{ \"key12\":1001 } } }";
    std::cout << "Original string: " << std::endl << src << std::endl << std::endl;    

    Jsp jsp(src);   // Parsing...         
    if( !jsp.ok() ) {   // If error
        std::cout << "Failed to parse the string! Exiting..." << std::endl;
        return 0;
    }
    // Stringifying to a list
    std::cout << "As a list of key-value pairs: " << std::endl << jsp.to_list_str() << std::endl;

    std::cout << "Stringified: " << std::endl << jsp.stringify() << std::endl << std::endl;

    // Testing API
    std::cout << "Testing API: " << std::endl;
    int rootHandle = jsp.root();  // A handle of the root element

    int n = jsp.getPairsNumber(rootHandle);   // The number of key-value Pairs in the root element
    std::cout << "The number of key-value pairs in the root is: " << n << std::endl;

    std::vector<int> *pairs = jsp.getPairs(rootHandle);   // The handles of key=value Pairs in the root element

    std::string *key = jsp.getPairKey( (*pairs)[0] );  // The key of the 0-th key-value Pair in the "root"
    std::cout << "The key of the 0-th element in the root is: " << *key << std::endl;

    int type = jsp.getPairType( (*pairs)[0] );  // The type of the 0-th key-value Pair in the "root" 
	if( type == JspType::STR ) {	// Checking the type is unnecessary here since we definitely know we hardcoded it as a string
    	std::string *str = jsp.getStr( (*pairs)[0] );
    	std::cout << "The value of the 0-th key-value pair in the root element is (str): " << '"' << *str << '"' << std::endl;
	}

    int boolHandle = jsp.getPairByKey( "keyBool" );  // The handle of the "keyBool" Pair
    type = jsp.getPairType( boolHandle );  // The type of the "boolHandle" Pair
	if( type == JspType::BOOL ) {	// Checking the type is unnecessary here since we definitely know we hardcoded it as a bool
    	bool boolValue = jsp.getBool( boolHandle, NULL );    // Reading it's value without querying it's type cause we know it's a bool
    	std::cout << "The value of the \"keyBool\" Pair: ";  // Printing it...  
    	std::cout << boolValue << std::endl;
	}

    int nullHandle = jsp.getPairByKey( "keyNull" );  // The handle of the "keyNull" Pair
    std::cout << "The type of the \"keyNull\" Pair: ";  // Printing it...  
    std::cout << jsp.getPairType(nullHandle) << std::endl;

    int numArrHandle = jsp.getPairByKey( "key5" );  // The handle of the "key5" Pair
    std::vector<double> *numArr = jsp.getNumArr( numArrHandle );    // if "key5":[0,1,2,3] then numArr holds [0,1,2,3]
    std::cout << "The number array for key5: ";  // Printing it...  
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
    std::cout << std::endl << std::endl;

    // Testing with a test-file
    std::string fname="test.json";
    if( argc > 1 ) {
        fname = argv[1];
    }
    std::ifstream f(fname);
    std::cout << "Opening a test file (test.json)..." << std::endl;
    if (f.is_open()) {
        std::ostringstream ss; 
        ss << f.rdbuf();
        f.close();
        std::cout << "Parsing and stringifying back:" << std::endl;

        auto t1 = std::chrono::high_resolution_clock::now();
        Jsp jspf(ss.str());
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        std::cout << "It took: " << fp_ms.count() << " milliseconds." << std::endl;

        if( !jspf.ok() ) {
            std::cout << "Parsing error! Exiting..." << std::endl;
            return -1;
        }
        std::cout << jspf.stringify() << std::endl;
    } else {
        std::cout << "Failed to open test file..." << std::endl;
        return -1;
    }  
    return 0;
}