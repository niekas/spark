#ifndef DISIMPLV_H
#define DISIMPLV_H 
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <math.h> 
#include <string>
#include <sys/time.h>
#include <vector>
#include "utils.h"
#include "functions.h"
#include "algorithm.h"

using namespace std;


class Algorithm {
    Algorithm(const Algorithm& other){};
    Algorithm& operator=(const Algorithm& other){};
public:
    Algorithm(){
        _duration = 0;
    };
    string _name;
    double _duration;   // Duration in seconds
    double _max_duration;   // Maximum allowed duration of minimization in seconds
    string _status;
    int _max_calls;
    string _stop_criteria;
    vector<Point*> _partition;
    Function* _func;
 

    friend ostream& operator<<(ostream& o, const Algorithm& a){
        o << "alg: '" << a._name << "', func: '" << a._func->_name << 
           "', calls: " << a._func->_calls << ", subregions: " << a._partition.size() <<
           ", duration: " << a._duration << ", stop_criteria: '" << a._stop_criteria << 
           "', f_min: " << a._func->_f_min << ", x_min: " << (*a._func->_x_min);
        return o;
    };

    virtual ~Algorithm(){
        // for (int i=0; i < _partition.size(); i++) {
        //     delete _partition[i];
        // };
        _partition.clear();
    };
};

#endif
