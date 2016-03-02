#ifndef SPARK_H
#define SPARK_H 
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


class Spark : public Algorithm {
    Spark(const Spark& other) {};
    Spark& operator=(const Spark& other) {};
public:
    Spark(int max_calls=15000, double max_duration=3600) {
        _max_calls = max_calls;
        _max_duration = max_duration;
        _iteration = 0;         // Algorithm iteration for debuging purposes

        _name = "Spark";
        _stop_criteria = "x_dist_Serg";          // Stopping criteria

        // Clean partition log file
        ofstream log_file; 
        log_file.open("log/partition.txt");
        log_file.close();
        log_file.open("log/front.txt");
        log_file.close();
    };

    int _iteration;
    int _D;

    bool is_accurate_enough() {
        if (!_func->is_accurate_enougth()) {
            return false;
        };
        return true;
    };

    void evaluate_at_middle() {
        double X[_D]; 

        for (int i=0; i < _D; i++) {
            X[i] = 0.5;    // Using 1-cube
        };
        Point* point = _func->get(new Point(X, _D)); 
        point->_max_edge_length = 1.;
        point->reset_dims_divided();
        point->update_diameter();

        // Point.divide() - explode this point till best value found
        //   Point.set_edge_lengths(Point.diameter/3., _D)
        //   Point.min_edge 
        //   Point.max_edge

        //// Initialize point diameter:
        // edge_lengths = [1]*D
        // diameter = l2norm(edge_lengths)
        // was_totally_divided = True
        _partition.push_back(point);
    };


    vector<Point*> convex_hull(vector<Point*> points) {
        int m = points.size() - 1;
        if (m <= 1) { return points; };
        int START = 0;
        int v = START;
        int w = m;
        bool flag = false;
        bool leftturn = false;
        int a, b, c;
        double det_val;
        while ((nextv(v, m) != START) or (flag == false)) {
            if (nextv(v, m) == w) {
                flag = true;
            }
            a = v;
            b = nextv(v, m);
            c = nextv(nextv(v, m), m);   // d = x = _diameter;  f = y = _values[0];

            double* matrix[3];
            double line1[3] = {points[a]->_diameter, points[a]->_values[0], 1.};
            double line2[3] = {points[b]->_diameter, points[b]->_values[0], 1.};
            double line3[3] = {points[c]->_diameter, points[c]->_values[0], 1.};
            matrix[0] = line1;
            matrix[1] = line2;
            matrix[2] = line3;
            det_val = Determinant(matrix, 3);

            if (det_val >= 0){
                leftturn = 1;
            } else {
                leftturn = 0;
            };
            if (leftturn) {
                v = nextv(v, m);
            } else {
                points.erase(points.begin() + nextv(v, m));
                m -= 1;
                w -= 1;
                v = predv(v, m);
            };
        };
        return points;
    };

    int nextv(int v, int m) {
        if (v == m) {
            return 0;
        };
        return v + 1;
    };

    int predv(int v, int m) {
        if (v == 0) {
            return m;
        };
        return v - 1;
    };

    vector<Point*> select_regions_value_and_diameter_convex_hull() {   // Selects from best
        vector<Point*> selected_points;

        // Sort points by their diameter
        vector<Point*> sorted_partition = _partition;   // Note: Could sort globally, resorting would take less time

        sort(sorted_partition.begin(), sorted_partition.end(), Point::ascending_diameter);
        double f_min = _func->_f_min;

        // Find point with  minimum metric  and  unique diameters
        Point* min_metric_point = sorted_partition[0];  // Initial value
        vector<double> diameters;
        vector<Point*> best_for_size;

        bool unique_diameter;
        bool found_with_same_size;
        for (int i=0; i < sorted_partition.size(); i++) {
            if (sorted_partition[i]->_values[0] < min_metric_point->_values[0]) {
                min_metric_point= sorted_partition[i];
            };
            // Saves unique diameters
            unique_diameter = true;
            for (int j=0; j < diameters.size(); j++) {
                if (diameters[j] == sorted_partition[i]->_diameter) {
                    unique_diameter = false; break;
                };
            };
            if (unique_diameter) {
                diameters.push_back(sorted_partition[i]->_diameter);
            };

            // If this point is better then previous with same size swap them.
            found_with_same_size = false;
            for (int j=0; j < best_for_size.size(); j++) {
                if (best_for_size[j]->_diameter == sorted_partition[i]->_diameter){
                    found_with_same_size = true;
                    if (best_for_size[j]->_values[0] > sorted_partition[i]->_values[0]) {
                        best_for_size.erase(best_for_size.begin()+j);
                        best_for_size.push_back(sorted_partition[i]);
                    };
                };
            };
            if (!found_with_same_size) {
                best_for_size.push_back(sorted_partition[i]);
            };
        };

        vector<Point*> selected;
        if (min_metric_point == best_for_size[best_for_size.size()-1]) {
            selected.push_back(min_metric_point);
        } else {
            // Is this OK?  Well compared with examples - its ok.
            if ((best_for_size.size() > 2) && (min_metric_point != best_for_size[best_for_size.size()-1])) {
                vector<Point*> points_below_line;
                // double a1 = best_for_size[0]->_diameter;
                // double b1 = best_for_size[0]->_values[0];
                double a1 = min_metric_point->_diameter;  // Should be like this based on Direct Matlab implementation
                double b1 = min_metric_point->_values[0];
                double a2 = best_for_size[best_for_size.size()-1]->_diameter;
                double b2 = best_for_size[best_for_size.size()-1]->_values[0];

                double slope = (b2 - b1)/(a2 - a1);
                double bias = b1 - slope * a1;

                for (int i=0; i < best_for_size.size(); i++) {
                    if (best_for_size[i]->_values[0] < slope*best_for_size[i]->_diameter + bias +1e-12) {
                        points_below_line.push_back(best_for_size[i]);
                    };
                };
                selected = convex_hull(points_below_line);  // Messes up points_below_line
            } else {
                selected = best_for_size;    // TODO: Why we divide all of them? Could divide only min_metrc_point.
                                             // Because practiacally this case does not occur ever.
            };
        };

        for (int i=0; i < selected.size(); i++) {
            selected[i]->_should_be_divided = true;
        };

        //// Should check all criterias, not only first
        // This part is very irational, because tolerance line and f_min point
        // are compared.
        //
        // Should f1, f2 lines be constructed, but in this case lbm should
        // known.
        //
        // Should use min_lbs values instead of tolerance.
        //
        // 




        // Remove points which do not satisfy condition:   f - slope*d > f_min - epsilon*abs(f_min)
        for (int i=0; i < (signed) selected.size() -1; i++) {  // I gess error here - bias is incorrect
            //// Version2: All functions should be improved
            // int improvable = 0;
            // for (int j; j < _funcs.size(); j++) {
            //     if (selected[i]->_min_lbs[j]->_values[0] < _funcs[j]->_f_min - 0.0001 * fabs(_funcs[j]->_f_min)) {
            //         improvable += 1;
            //     };
            // };
            // if (improvable == 0) {
            //     selected[i]->_should_be_divided = false;
            // };

            //// Version3: Too small points should not be divided (but tolerance should ensure this)
            // if (selected[i]->_diameter < 1e-5) {
            //     selected[i]->_should_be_divided = false;
            // };

            //// Version4: tolerance should ensure this.

            //// VersionOriginal: for single criteria
            // double a1 = selected[selected.size() - i -1]->_diameter;
            // double b1 = selected[selected.size() - i -1]->_values[0];
            // double a2 = selected[selected.size() - i -2]->_diameter;
            // double b2 = selected[selected.size() - i -2]->_values[0];
            // double slope = (b2 - double(b1))/(a2 - a1);
            // double bias = b1 - slope * a1;
            // // cout << "slope = (b2 - double(b1))/(a2 - a1);   bias = b1 - slope * a1;" << endl;
            // // cout << "b-remove if    bias > f_min - 0.0001*fabs(f_min)" << endl;
            // // cout << "a1 " << a1 << " b1 " << b1 << " a2 " << a2 << " b2 " << b2 << endl;
            // // cout << "slope " << slope << " bias " << bias << endl;
            // // cout << "bias: " << bias << " fmin " << f_min << endl;
            // // cout << endl;
            //
            // if (bias > f_min - 0.0001*fabs(f_min)) {   // epsilon
            //     selected[selected.size() - i -2]->_should_be_divided = false;
            // };
        };



        // Remove points which should not be divided
        selected.erase(remove_if(selected.begin(), selected.end(), Point::wont_be_divided), selected.end());

        // Select all points which have best _values[0] for its size 
        for (int i=0; i < sorted_partition.size(); i++) {
            for (int j=0; j < selected.size(); j++) {
                if ((sorted_partition[i]->_diameter == selected[j]->_diameter) && 
                    (sorted_partition[i]->_values[0] == selected[j]->_values[0])) {
                    selected_points.push_back(sorted_partition[i]);
                };
            };
        };

        return selected_points;
    };

    vector<Point*> select_regions_to_divide() {
        // When creating point its _diameter should be updated.
            // > Should track euclidian diameter
        // Use convex hull to investigate regions

    
        // How is it possible to determine the diameter?

        // We know:
        //   - the dimension
        //   - initial feasible region size
        //   - and how many dimensions it decreased
        // How to calculate dimension?
        //   - simply calculate the diameter: sqrt(squared sum of dimensions).
        //   - how diameter should be updated? 
        //          How many divisions were made could be saved also.
        //   - How much diameter is reduced, when its divided?
        //      - in 2d case: its trisected (so 3 times)
        //      - in 3d case: its trisected (so 3 times)


        // Using convex hull
        return select_regions_value_and_diameter_convex_hull();
    };


    vector<Point*> divide(Point* point, int dimension) {
        vector<Point*> result;
        Point* new_p1 = point->dublicate();   // Does not dublicate functions value

        // new_p1->_dims_divided[dimension] = true;  // Todo: merge these two into one method
        //// new_p1->check_if_division_is_complete();

        new_p1->_X[dimension] -= point->_max_edge_length / 3.; 
        new_p1->set_divided_dim(dimension);  // If division is complete - set max_edge to lower and reset divisions

        // cout << "Set as divided: " << dimension << endl;
        // new_p1->print();
        // for (int i=0; i < _D; i++) {
        //     cout << new_p1->_dims_divided[i];
        // };
        // cout << "Diam: " << new_p1->_diameter << endl;


        Point* p1 = _func->get(new_p1);
        // new_p1->print();
        // cout << "p1 " << new_p1 << ", "<< p1 << ", " << p1->_values[0] << endl; 
        if (p1 == new_p1) {
            _partition.push_back(p1);
            result.push_back(p1);
        } else {
            delete new_p1;
        };

        Point* new_p2 = point->dublicate();
        // new_p2->_dims_divided[dimension] = true;  // Todo: merge these two into one method
        new_p2->_X[dimension] += point->_max_edge_length / 3.; 
        new_p2->set_divided_dim(dimension);  // If division is complete - set max_edge to lower and reset divisions

        Point* p2 = _func->get(new_p2);
        if (p2 == new_p2) {
            _partition.push_back(p2);
            result.push_back(p2);
        } else {
            delete new_p2;
        };
        return result;
    };


    void spark(Point* point) {
        Point* best_point = point;
        for (int i=0; i < _D; i++) {
            if (point->_dims_divided[i] == false) {
                vector<Point*> new_points = divide(point, i);  // For each dimension 2 new points

                //> Implement convex_hull method to divide only most perspective regions

                // Do not select if last dimension is divided
                if (i != _D - 1) {
                    for (int j=0; j < new_points.size(); j++) {
                        if (new_points[j]->_values[0] < best_point->_values[0]) {
                            best_point = new_points[j];
                        };
                    };
                };
                // if (new_points[0]->_values[0] < best_point->_values[0]) { best_point = new_points[0]; };
                // if (new_points[1]->_values[0] < best_point->_values[0]) { best_point = new_points[1]; };
            };
        };

        // FixMe: Spark does not stop, when all dimensions are divided.
        point->_max_edge_length = point->_max_edge_length / 3.;
        point->reset_dims_divided();
        point->update_diameter();
        if (best_point != point) {
            // cout << "Before sparking" << endl;
            //     point->print();
            // cout << "   Best point" << endl;
            // best_point->print();

            spark(best_point);

            // cout << "After sparking" << endl;
            //     point->print();
            // cout << "   Best point" << endl;
            // best_point->print();
        };

        // Choose point with best value from sparked points
        // If chosen is not just sparked - spark it
        // Select the best value of the (new division, same point) and repeat the spark if not same point selected.
        //    Investigate only points with same parent_diameter_size.
    };



    void divide_regions(vector<Point*> selected_points) {
        cout << "Dividing selected points: " << selected_points.size() << endl;
        for (int i=0; i < selected_points.size(); i++) {
            // These methods should be Spark part, but point should contain the
            // fields for meta data.

            // Point divide -> sparks and returns new points, self - updates.
            // Divide bigest first, then smaller ones - then loop again

            // Note: need to implement single point spark.
            //      Divide operator divides point space in all directions equally
            //      Spark operator follows minimization path from given point
            // _partition[i]->print();
            spark(selected_points[i]);
        };
    };

    void minimize(Function* func){
        _func = func;
        _D = _func->_D;
        timestamp_t start = get_timestamp();

        evaluate_at_middle();

        while (_func->_calls <= _max_calls && _duration <= _max_duration && !is_accurate_enough()) {
            //// Draw partition in each iteration:

            divide_regions(select_regions_to_divide());

            // Update counters and log the status
            _iteration += 1;
            cout << _iteration << ". Calls: " << _func->_calls << "  f_min:" << _func->_f_min << endl;
            // Point::log_partition(_partition);
            // FILE* testp = popen("python log/show_partition.py log/partition.txt", "r");
            // pclose(testp);



            // if (_iteration == 3) {
            //     Point::log_partition(_partition);
            //     exit(0);
            // };

            timestamp_t end = get_timestamp();
            _duration = (end - start) / 1000000.0L;
        };

        if ((_func->_calls <= _max_calls) && (_duration <= _max_duration)) {
            _status = "D"; 
        } else {
            _status = "S";
        };
    };

    virtual ~Spark(){};
};

#endif
