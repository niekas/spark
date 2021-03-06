#include <iostream>
#include <getopt.h>
// getopt example: http://stackoverflow.com/questions/8793020/using-getopt-long-c-how-do-i-code-up-a-long-short-option-to-both-require-a
#include "Spark.h"
#include <stdio.h>
#include <fstream>
#include "Eigen/Dense"

#define no_argument 0
#define required_argument 1
#define optional_argument 2


using namespace std;

int main(int argc, char* argv[]) {
    // Parse parameters
    const struct option longopts[] = {
        {"func_cls", required_argument, 0, 'c'},
        {"func_id", required_argument, 0, 'f'},
        {"task_id", required_argument, 0, 't'},
        {"callback", required_argument, 0, 'b'},
        {"max_duration", optional_argument, 0, 'd'},
        {"max_calls", optional_argument, 0, 'i'},
        {"glob_L", optional_argument, 0, 'g'},
    };
    int cls;
    int fid;
    int task_id;
    char* callback = {'\0'};
    int max_calls = 40000;
    int max_duration = 3600;
    double glob_L = numeric_limits<double>::max();

    int opt_id;
    int iarg = 0;
    while(iarg != -1) {
        iarg = getopt_long(argc, argv, "cftbdig", longopts, &opt_id);
        switch (iarg) {
            case 'c':
                cls = strtoul(optarg, 0, 0);
                break;
            case 'f':
                fid = strtoul(optarg, 0, 0);
                break;
            case 't':
                task_id = strtoul(optarg, 0, 0);
                break;
            case 'b':
                callback = strdup(optarg);
                break;
            case 'd':
                max_duration = strtoul(optarg, 0, 0);
                break;
            case 'i':
                max_calls = strtoul(optarg, 0, 0);
                break;
            case 'g':
                glob_L = strtod(optarg, '\0');
                break;
        };
    };

    // Put function vector in order to be able to use more than 2 functions in the future
    Function* func = new GKLSFunction(cls, fid);

    Spark* alg;
    alg = new Spark(max_calls, max_duration);

    // if (glob_L != numeric_limits<double>::max()) {
    //     Simplex::glob_Ls.push_back(glob_L);
    // };

    alg->minimize(func);

    // Print results
    cout << "Cls: " << cls << "  Fid: " << fid << endl;
    if (alg->_status == "S") { cout << "  -->> Suspended <<--" << endl; }
    cout << "Calls: " << func->_calls 
         << ", status: " << alg->_status  
         << ", duration: " << alg->_duration << endl;

    cout.precision(10);
    cout << "Solution:" << endl;
    func->_x_nearest_to_glob_x->print();
    cout << "   Global minima:" << endl;
    func->_glob_x->print();

    // Save results
    if (callback != '\0') {
        string cmd;
        stringstream cmd_ss; 
        cmd_ss.precision(10);
        cmd_ss << callback
               << " --calls=" << func->_calls
               << " --duration=" << alg->_duration
               << " --task_id=" << task_id
               << " --status=" << alg->_status
               << " --x_min=" << *func->_x_min
               << " --f_min=" << func->_f_min
               << " --min_diam=" << alg->_partition[0]->_diameter
               << " --max_diam=" << alg->_partition[alg->_partition.size() - 1]->_diameter
               << " -exe=" << argv[0] << endl;
        cmd = cmd_ss.str();
        popen(cmd.c_str(), "r");
    };

    // Free memory
    delete alg;
    delete func;
    return 0;
};
