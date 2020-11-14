#include <string>
#include <vector>
#include <stdexcept>
#include <stack>
#include "jsp.hpp"

bool isWhiteSpace( char c ) {
    if( c == ' ' || c == '\t' || c == '\n' ) {
        return true;
    }
    return false;
}

bool isFirstCharOfNumber( char c ) {
    if( (c >= '0' && c <= '9') || c == '-' ) {
        return true;
    }
    return false;
}


int parseNum( std::string& json_str, int start, int end, double& dest ) {
    int first = -1;
    int last = -1;
    int i = start;
    for(  ; i <= end ; i++ ) {
        char c = json_str[i];
        if( isWhiteSpace( c ) ) {
            if( first == -1 ) {
                continue;
            } else {
                break;
            }
        }
        if( (c >= '0' && c <= '9') || c == '.' || (first == -1 && c == '-') ) {
            if( first == -1 ) 
                first = i;
            last = i;
            continue;
        }
        break;
    }
    if( first == -1 ) {
        return -1;
    }

    std::string s = json_str.substr( first, last - first + 1 );
    std::string::size_type sz;
    try {
        dest = std::stod (s, &sz);
    } catch(const std::invalid_argument& e) {
        return -1;
    } catch(const std::out_of_range& e) {
        return -1;
    }
    return last;
}

int readNumArray( std::string& json_str, int start, int end, std::vector<double>& dest ) {
    int endOfArrayPos = -1; 
    int start_pos = start;
    bool looking_for_comma = false;
    while(true) { 
        bool error = false;
        int i = start_pos;        
        for( ; i <= end ; i++ ) {
            char c = json_str[i];
            if( isWhiteSpace(c)) {
                continue;
            }
            if( c == ',' ) {  
                if( !looking_for_comma ) {  
                    return -1;
                }
                looking_for_comma = false;
                continue;
            }
            if( c == ']' ) {
                if( !looking_for_comma ) {
                    return -1;
                }
                endOfArrayPos = i;
                break;
            }
            if( isFirstCharOfNumber(c) ) {
                break;
            }
            error = true;
            break;
        }
        if( error || i > end ) {
            return -1;
        }
        if( endOfArrayPos != -1 ) {
            break;
        }
        double num;
        int lastDigitPos = parseNum( json_str, i, end, num );
        if( lastDigitPos == -1 ) {
            return -1;
        }
        dest.push_back(num);
        start_pos = lastDigitPos+1;
        looking_for_comma = true;
    }
    return endOfArrayPos;
}

int readStrArray( std::string& json_str, int start, int end, std::vector<std::string>& dest ) {
    bool looking_for_opening_quote = true;
    bool looking_for_comma = false;
    bool error = false;
    int prev_quote_pos = -1;
    int i = start;
    for( ; i <= end ; i++ ) {
        char c = json_str[i];
        if( c == ']' && looking_for_opening_quote ) {
            break;
        }
        if( looking_for_comma && looking_for_opening_quote ) {   // If looking for a comma
            if( c == ',' ) {
                looking_for_comma = false;
                continue;
            }
            if( !isWhiteSpace(c) ) {
                error = true;
                break;
            }
        }
        if( c != '"' && looking_for_opening_quote && !isWhiteSpace(c) ) {
            error = true;
            break;
        }
        if( c == '"' ) {
            if( i > start && json_str[i-1] == '\\' ) {
                continue;
            }
            if( looking_for_opening_quote ) {
                prev_quote_pos = i;
                looking_for_opening_quote = false;
                continue;
            }  
            looking_for_opening_quote = true;
            int l = i - prev_quote_pos - 1;
            if( l == 0 ) {
                dest.push_back("");    
            } else {
                std::string s = json_str.substr( prev_quote_pos+1, l );
                dest.push_back(s);
            }
            looking_for_comma = true;
        }
    }
    if( i > end || error ) {
        return -1;
    }
    return i;
}


int getTypeOfArray( std::string& json_str, int start, int end, int& right_square_bracket_pos_of_empty_arr ) {
    for( int i = start ; i <= end ; i++ ) {
        char c = json_str[i];
        if( c == '"' ) {
            return JspType::STRARR;
        } 
        if( isFirstCharOfNumber( c ) ) {
            return JspType::NUMARR;
        }
        if( c == '{' ) {
            return JspType::DICTARR;
        }
        if( c == ']' ) {    // An empty array
            right_square_bracket_pos_of_empty_arr = i;
            return JspType::EMPTYARR;
        }
        if( !isWhiteSpace( c ) ) {
            break;
        }
    }
    return JspType::UNDEF;
}


int findRightBracket(std::string& json_str, int start, int end) {
    int pos = -1;
    int nLeft = 1;
    for( int i = start ; i <= end ; i++ ) {
        if( json_str[i] == '}' ) {
            if( nLeft == 1 ) {
                pos = i;
                break;
            }
            nLeft--;
            continue;
        }
        if( json_str[i] == '{' ) {
            nLeft++;
        }
    }
    return pos;
}


int findBracketsInArray(std::string& json_str, int start, int end, int& op, int& cl ) {
    int i;
    op = -1;
    cl = -1;
    for( i = start ; i <= end ; i++ ) {
        if( json_str[i] == ']' ) {  // Array end
            return i;
        }
        if( json_str[i] == '{' ) {
            op = i;
            break;                
        }
    }
    if( op == -1 ) {
        return 0;
    }
    int nLeft = 1;
    for(  i=i+1 ; i <= end ; i++ ) {
        if( json_str[i] == '}' ) {
            if( nLeft == 1 ) {
                cl = i;
                break;
            }
            nLeft--;                
        }
        if( json_str[i] == '{' ) {
            nLeft++;
        }
    }
    if( i > end ) {
        return -1;
    }
    return 0;
}


int findKey(std::string& json_str, int start, int end, std::string& dest) {
    int keyStart = -1, keyEnd = -1;
    for( int i = start ; i <= end ; i++ ) {
        if( json_str[i] == '"' ) {
            keyStart = i;
            break;
        }
    }
    if( keyStart == -1 ) {
        return -1;
    }
    for( int i = keyStart+1 ; i <= end ; i++ ) {
        if( json_str[i] == '"' ) {
            keyEnd = i;
            break;
        }
    }
    if( keyEnd == -1 ) {
        return -1;
    }
    int keySize = keyEnd - keyStart - 1;
    if( keySize == 0 ) {
        return -1;
    }

    int semiColorPos = -1;
    for( int i = keyEnd+1 ; i <= end ; i++ ) {
        if( json_str[i] == ':' ) {
            semiColorPos = i;
            break;
        }
        if( !isWhiteSpace( json_str[i] ) ) {
            return -1;
        }
    }
    dest = json_str.substr( keyStart+1, keySize );
    return semiColorPos;
}


int parseStr(std::string& json_str, int start, int end, std::string& dest ) {
    int stringEndPos = -1;
    for( int i = start+1 ; i <= end ; i++ ) {
        if( json_str[i] == '"' ) {
            if( i > start ) {
                if( json_str[i-1] == '\\' ) {
                    continue;
                }
            }
            stringEndPos = i;
            break;
        }
    }
    if( stringEndPos == -1 ) {
        return -1;
    }
    int l = stringEndPos - start -1;
    if( l == 0 ) {
        return -1;
    }
    dest = json_str.substr( start+1, l);
    return stringEndPos;
}


Jsp::Jsp( std::string& json_str ) {
    int i;
    std::stack<int> containers_stack;

    int json_str_size = json_str.size();

    // Searching for root entry... 
    for( i=0 ; i < json_str_size && json_str[i] != '{'; i++ ) {;}
    if( i >= json_str_size )
        return;
    int rootrange_start = i+1;
    for( i = json_str_size-1 ; i > 0 && json_str[i] != '}'; i-- ) {;}
    if( i <= 0 )
        return;
    int rootrange_end = i-1;
    _containers.emplace_back( JspContainer(NULL, rootrange_start, rootrange_end ) );
    containers_stack.push(0);

    _error = false;
    while( !containers_stack.empty() ) {
        int parent_container = containers_stack.top();
        int range_start = _containers[parent_container].startPos;
        int range_end = _containers[parent_container].endPos;
        containers_stack.pop();

        while( true ) {
            // Reading a key
            std::string key;
            int semiColorPos = findKey(json_str, range_start, range_end, key);
            if( semiColorPos < 0 ) {
                break;
            }

            // Reading a value (string, number, container or array)
            for( int i = semiColorPos+1 ; i < range_end ; i++ ) {
                char c = json_str[i];
                if( isWhiteSpace(c) )
                    continue;

                if( c == '"' ) {  // A string value
                    std::string value;
                    int stringEndPos = parseStr(json_str, i, range_end, value);
                    if( stringEndPos == -1 ) {
                        _error = true;
                        break;
                    }
                    _pairs.push_back( JspPair( parent_container, key, value, range_start, stringEndPos ) );
                    _containers[parent_container].addPair( _pairs.size()-1 ); 
                    range_start = stringEndPos+1;
                    break;
                } 
                else if( isFirstCharOfNumber(c) ) {   // A number
                    double value;
                    int numEndPos = parseNum( json_str, i, range_end, value );
                    _pairs.push_back( JspPair( parent_container, key, value, range_start, numEndPos ) ); 
                    _containers[parent_container].addPair( _pairs.size()-1 ); 
                    range_start = numEndPos+1;
                    break;
                } 
                else if( c == 'n' && json_str[i+1] == 'u' && json_str[i+2] == 'l' && json_str[i+3] == 'l' ) {   // A null
                        _pairs.push_back( JspPair( parent_container, key, JspType::NULLVAL, range_start, i+3 ) ); 
                        _containers[parent_container].addPair( _pairs.size()-1 ); 
                        range_start = i+4;
                        break;
                }
                else if( c == 't' && json_str[i+1] == 'r' && json_str[i+2] == 'u' && json_str[i+3] == 'e' ) {   // A true
                        _pairs.push_back( JspPair( parent_container, key, true, range_start, i+3 ) ); 
                        _containers[parent_container].addPair( _pairs.size()-1 ); 
                        range_start = i+4;
                        break;
                }
                else if( c == 'f' && json_str[i+1] == 'a' && json_str[i+2] == 'l' && json_str[i+3] == 's' && json_str[i+4] == 'e' ) {   // A false
                        _pairs.push_back( JspPair( parent_container, key, false, range_start, i+4 ) ); 
                        _containers[parent_container].addPair( _pairs.size()-1 ); 
                        range_start = i+5;
                        break;
                }                 
                else if( c == '{' ) {  // A new container
                    int pos = findRightBracket(json_str, i+1, range_end);
                    if( pos == -1 ) {
                        _error = true;
                        break;
                    }
                    _containers.emplace_back( JspContainer(NULL, i+1, pos-1) );
                    int container_num = _containers.size()-1;
                    containers_stack.push( container_num );
                    _pairs.push_back( 
                        JspPair( parent_container, container_num, key, range_start, pos ) 
                    );
                    int pair_num = _pairs.size() - 1;
                    _containers[container_num].iParentPair = pair_num;
                    _containers[parent_container].addPair( pair_num ); 
                    range_start = pos+1;
                    break;
                }
                else if( c == '[' ) {  // An array found
                    int right_square_bracket_pos_of_empty_arr;
                    int type = getTypeOfArray( json_str, i+1, range_end, right_square_bracket_pos_of_empty_arr );
                    if( type == JspType::EMPTYARR ) {
                        JspPair pair( parent_container, key, JspType::EMPTYARR );
                        _pairs.push_back( pair );
                        _containers[parent_container].addPair( _pairs.size()-1 ); 

                        range_start = right_square_bracket_pos_of_empty_arr + 1;
                        break;
                    }
                    if( type == JspType::UNDEF ) {
                        _error = true;
                        break;
                    }
                    if( type == JspType::NUMARR ) {
                        JspPair pair( parent_container, key, JspType::NUMARR );
                        int endPos = readNumArray( json_str, i+1, range_end, pair.num_values );
                        if( endPos == -1 ) { // If _error...
                            _error = true;
                            break;
                        }
                        _pairs.push_back( pair );
                        _containers[parent_container].addPair( _pairs.size()-1 ); 
                        range_start = endPos+1;
                        break;
                    } else if( type == JspType::STRARR ) {
                        JspPair pair( parent_container, key, JspType::STRARR );
                        int endPos = readStrArray( json_str, i+1, range_end, pair.str_values );
                        if( endPos == -1 ) { // If _error...
                            _error = true;
                            break;
                        }
                        _pairs.push_back( pair );
                        _containers[parent_container].addPair( _pairs.size()-1 ); 
                        range_start = endPos+1;
                        break;
                    } else if( type == JspType::DICTARR ) {
                        _pairs.push_back( JspPair( parent_container, key, JspType::DICTARR ) );
                        int pair_num = _pairs.size() - 1;
                        _containers[parent_container].addPair( pair_num ); 

                        int start_at = i+1;
                        while(true) {
                            bool arrayEndFound;
                            int op, cl;
                            int res = findBracketsInArray(json_str, start_at, range_end, op, cl );
                            if( res == -1 ) {   // _error
                                _error = true;
                                break;
                            }
                            if( op == -1 || cl == -1 ) {
                                range_start = res+1;
                                break;
                            }
                            _containers.emplace_back( JspContainer( NULL, op+1, cl-1) );
                            int container_num = _containers.size()-1;
                            containers_stack.push( container_num );
                            _containers[container_num].iParentPair = pair_num;
                            _pairs[pair_num].addContainer(container_num);
                            start_at = cl+1;
                        } 
                        break;
                    }
                } else {
                    _error = true;
                    break;
                }                                                           
            }
            if( _error )
                break;

            // Looking for a comma...
            while( range_start < range_end ) {
                if( json_str[range_start] == ',' ) {
                    range_start++;
                    break;
                }
                if( isWhiteSpace( json_str[range_start] ) ) {
                    range_start++;
                    continue;
                }
                _error = true;
                break;
            }
            if( _error )
                break;
        }
        if( _error )
            break;
    }
}


std::string Jsp::to_list_str() {
    std::string dest = "";
    std::stack<int> containers;
    std::stack<std::string> keys;
    containers.push(0);
    keys.push("root");

    while(!containers.empty()) {
        JspContainer *c = &_containers[containers.top()];
        dest += "CONTAINER KEY: " + keys.top() + '\n';
        containers.pop();
        keys.pop();

        for( int ip = 0 ; ip < c->childPairs.size() ; ip++ ) {
            JspPair *p = &_pairs[ c->childPairs[ip] ];

            if( p->valueType == JspType::DICT) {
                containers.push(p->childContainer);
                keys.push( p->key );
                continue;
            }            
            if( p->valueType == JspType::DICTARR) {
                for( int cc = 0 ; cc < p->childContainers.size() ; cc++ ) {
                    containers.push(p->childContainers[cc]);
                    keys.push( p->key );
                }
                continue;
            }            
            dest += p->key + " = "; 
            if( p->valueType == JspType::STR ) {
                dest += '"' + p->str_value + '"';
            } 
            else if( p->valueType == JspType::NUM ) {
                dest += std::to_string(p->num_value);
            } 
            else if( p->valueType == JspType::BOOL ) {
                dest += (p->bool_value) ? "true" : "false";
            } 
            else if( p->valueType == JspType::NULLVAL ) {
                dest += "null";
            } 
            else if( p->valueType == JspType::STRARR ) {
                for( int k = 0 ; k < p->str_values.size() ; k++ ) {
                    if( k > 0 ) {
                        dest += ", ";
                    }
                    dest += '"' + p->str_values[k] + '"';
                }
            } 
            else if( p->valueType == JspType::NUMARR ) {
                for( int k = 0 ; k < p->num_values.size() ; k++ ) {
                    if( k > 0 ) {
                        dest += ",";
                    }
                    dest += std::to_string(p->num_values[k]);
                }
            }
            dest += '\n';
        }
    }
    return dest;
 }



void Jsp::stringify_helper( std::string& dest, int ci ) {
    JspContainer *c = &_containers[ci];

    dest += '{';

    for( int ip = 0 ; ip < c->childPairs.size() ; ip++ ) {
        if( ip > 0 ) {
            dest += ", ";
        }
        JspPair *p = &_pairs[ c->childPairs[ip] ];
        dest += '"' + p->key + '"' + ": "; 

        if( p->valueType == JspType::DICT) {
            stringify_helper(dest, p->childContainer);
            continue;
        }
        if( p->valueType == JspType::DICTARR) {
            dest += '[';
            for( int cc = 0 ; cc < p->childContainers.size() ; cc++ ) {
                if( cc > 0 ) {
                    dest += ", ";
                }
                stringify_helper(dest, p->childContainers[cc]);
            }
            dest += ']';
            continue;
        }            
        if( p->valueType == JspType::STR ) {
            dest += '"' + p->str_value + '"';
        } 
        else if( p->valueType == JspType::NUM ) {
            dest += std::to_string(p->num_value);
        } 
        else if( p->valueType == JspType::BOOL ) {
            dest += (p->bool_value) ? "true" : "false";
        } 
        else if( p->valueType == JspType::NULLVAL ) {
            dest += "null";
        } 
        else if( p->valueType == JspType::STRARR ) {
            dest += '[';
            for( int k = 0 ; k < p->str_values.size() ; k++ ) {
                if( k > 0 ) {
                    dest += ", ";
                }
                dest += '"' + p->str_values[k] + '"';
            }
            dest += ']';
        } 
        else if( p->valueType == JspType::NUMARR ) {
            dest += '[';
            for( int k = 0 ; k < p->num_values.size() ; k++ ) {
                if( k > 0 ) {
                    dest += ",";
                }
                dest += std::to_string(p->num_values[k]);
            }
            dest += ']';
        }
        else if( p->valueType == JspType::EMPTYARR ) {
            dest += "[]";
        }
    }
    dest += '}';
} 

std::string Jsp::stringify() {
    std::string dest = "";
    stringify_helper(dest, 0);
    return dest;
 }