#include "demo/trapezoidal_map_demo.hpp"
#include "demo/compute_free_space_demo.hpp"
#include "demo/compute_path_demo.hpp"
#include "demo/minkowski_sum_demo.hpp"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
    if (argc == 2) {
        string arg = argv[1];
        if (arg == "trap") {
            trapezoidal_map_demo();
        }
        if (arg == "freespace") {
            compute_free_space_demo();
        }
        if (arg == "path") {
            compute_path_demo();
        }
        if (arg == "mink") {
            minkowski_sum_demo();
        }
    }
     else {
        cout << "Kindly enter some valid arguement";
    }
}
