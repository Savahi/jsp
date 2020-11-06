#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

class JspType {
    public:
        static const int UNDEF = 0;
        static const int DICT = 10;
        static const int STR = 20;
        static const int NUM = 21;
        static const int DICTARR = 30;
        static const int STRARR = 31;
        static const int NUMARR = 32;
};

class JspContainer;
class JspArray;

class JspPair {
    public:
        std::string key;
        int parentContainer;

        int keyStart;
        int keyEnd;

        int valueType;

        double num_value;
        std::string str_value;
        int childContainer;

        std::vector<double> num_values;
        std::vector<std::string> str_values;
        std::vector<int> childContainers;

    JspPair( int parent ) {
        parentContainer = parent;
    }

    JspPair( int parent, int start, int end ) {
        parentContainer = parent;
        keyStart = start;
        keyEnd = end;
    }

    JspPair( int parent, std::string& k, int type ) {
        parentContainer = parent;
        key = k;
        valueType = type; 
    }

    // key->string
    JspPair( int parent, std::string& k, std::string& v, int start, int end ) {
        parentContainer = parent;
        keyStart = start;
        keyEnd = end;
        key = k;
        str_value = v;
        valueType = JspType::STR; 
    }

    // key->double
    JspPair( int parent, std::string& k, double v, int start, int end ) {
        parentContainer = parent;
        keyStart = start;
        keyEnd = end;
        key = k;
        num_value = v;
        valueType = JspType::NUM; 
    }

    // key->child container
    JspPair( int parent, int child, std::string& k, int start, int end ) {
        parentContainer = parent;
        childContainer = child;
        keyStart = start;
        keyEnd = end;
        key = k;
        valueType = JspType::DICT; 
    }


    void setValueType( int type ) {
        valueType = type;
    }

    int addContainer( int n ) {
        if( valueType != JspType::DICTARR ) {
            return -1;
        }
        childContainers.push_back(n);
        return 0;
    } 
};

class JspContainer {
    public:
        int type;   // pair, array of 
        JspPair *parentPair;
        int iParentPair;
        std::vector<JspPair*> pairs;
        std::vector<int> childPairs;

        int startPos;
        int endPos;

        JspContainer() {
            return;
        }
        
        JspContainer( int parent, int start, int end ) {
            startPos = start;
            endPos = end;
            iParentPair = parent;
        }

        void addPair( int i) {
            childPairs.push_back(i);
        }

        void addPair( JspPair *p) {
            pairs.push_back(p);
        }
};

class Jsp {
    public:
        std::vector<JspContainer> _containers;
        std::vector<JspPair> _pairs;
        bool _error;

        Jsp() { _error = false; };
        Jsp( std::string fileName );

        int root() {
            return 0;
        }

        // Returns a number of Pairs in a Dict (Container)
        int getPairsNumber( int c ) {
            if( _containers.size() <= c ) {
                return -1;
            }
            return _containers[c].childPairs.size();
        }

        // Returns a vector of Pair handles
        std::vector<int>* getPairs( int c ) {
            if( _containers.size() <= c ) {
                return NULL;
            }
            return &_containers[c].childPairs;
        }

        // Finds key inside a container
        int getPairByKey( int c, std::string key ) {
            if( _containers.size() <= c )
                return -1;
            for( int i = 0 ; i < _containers[c].childPairs.size() ; i++ ) {
                JspPair *pptr = &_pairs[ _containers[c].childPairs[i] ];
                if( pptr->key == key ) {
                     return _containers[c].childPairs[i];
                }
            }
            return -1;
        }

        int getPairByKey( std::string key ) {
            for( int i = 0 ; i < _pairs.size() ; i++ ) {
                JspPair *pptr = &_pairs[ i ];
                if( pptr->key == key ) {
                     return i;
                }
            }
            return -1;
        }

        // Returns the key of the i-th pair of a container 
        std::string* getPairKey( int p ) {
            JspPair *pptr = getPair( p );
            if( pptr == NULL )
                return NULL;
            return &pptr->key;
        }

        // Returns the type of a key-value pair specified by handle (index)
        int getPairType( int p ) {
            JspPair *pptr = getPair( p );
            if( pptr == NULL )
                return JspType::UNDEF;
            return pptr->valueType;
        }

        // Returns the str value of a pair specified by handle (index)
        std::string* getStr( int p ) {
            JspPair *pptr = getPair( p );
            if( pptr == NULL )
                return NULL;
            if( pptr->valueType != JspType::STR )
                return NULL;
            return &pptr->str_value;            
        }

        // Returns the double value of a pair specified by handle (index)
        double getNum( int p, int *error ) {
            if( error != NULL ) 
                *error = 1;
            JspPair *pptr = getPair( p );
            if( pptr == NULL )
                return 0.0;
            if( pptr->valueType != JspType::NUM )
                return 0.0;
            if( *error != NULL )
                error = 0;
            return pptr->num_value;            
        }

        // Returns the dict value a pair specified by handle (index)
        int getDict( int p ) {
            JspPair *pptr = getPair( p );
            if( pptr == NULL )
                return -1;
            if( pptr->valueType != JspType::DICT ) {
                return -1;
            }
            return pptr->childContainer;            
        }

        std::vector<std::string> *getStrArr( int p ) {
            JspPair *pptr = getPair( p );
            if( pptr == NULL )
                return NULL;
            if( pptr->valueType != JspType::STRARR ) {
                return NULL;
            }
            return &pptr->str_values;            
        }

        std::vector<double> *getNumArr( int p ) {
            JspPair *pptr = getPair( p );
            if( pptr == NULL )
                return NULL;
            if( pptr->valueType != JspType::NUMARR ) {
                return NULL;
            }
            return &pptr->num_values;            
        }
        
        // Returns a vector of handles
        std::vector<int> *getDictArr( int p ) {
            JspPair *pptr = getPair( p );
            if( pptr == NULL )
                return NULL;
            if( pptr->valueType != JspType::NUMARR ) {
                return NULL;
            }
            return &pptr->childContainers;            
        }
    
        bool ok() {
            return !_error;
        }

        void to_list_str( std::string& );
        void stringify( std::string& );
        void stringify_helper( std::string&, int );

private:
        JspPair* getPair( int p ) {
            if( _pairs.size() <= p )
                return NULL;
            return &_pairs[ p ];
        }        

        JspPair* getPair( int c, int p ) {
            if( _containers.size() <= c )
                return NULL;
            if( _containers[c].childPairs.size() <= p )
                return NULL;
            return &_pairs[ _containers[c].childPairs[p] ];
        }        

};