#include <fstream>
#include <iostream>

#include <clipper.hpp>

#include "measure.h"

using namespace std;
using namespace ClipperLib;

//from clipper.hpp ...
//typedef signed long long cInt;
//struct IntPoint {cInt X; cInt Y;};
//typedef std::vector<IntPoint> Path;
//typedef std::vector<Path> Paths;
   
Paths gpf_read(string name);

const double S = 1e6; 

void info(Paths& p);
void data(Paths& p);

int main () {

    Paths subj = gpf_read("../polygons/100_stars_1.gpf");
    Paths clip = gpf_read("../polygons/100_stars_2.gpf" );
    Paths solution;

    info(subj);
    info(clip);
    
    //perform intersection ...
    start();
    Clipper c;
    c.AddPaths(subj, ptSubject, true);
    c.AddPaths(clip, ptClip, true);
    c.Execute(ctIntersection, solution, pftNonZero, pftNonZero);
    stop();

    info(solution);
    data(solution);
}

void data(Paths& p) {
    for(auto&& path : p) {
        for(auto&& point : path) {
            cout << point.X/S << " " << point.Y/S << "\n";
        }
        cout<< path[0].X/S << " " << path[0].Y/S << "\n"; // close path 
        cout << "\n";
    }
}

void info(Paths& p) {
    cerr << p.size() << " contours\n";
    for(auto&& path : p) {
        cerr << "  " << path.size() << " vertices\n";
        // for(auto&& point : path) {
        //  cout << point.X/S << " " << point.Y/S << "\n";
        // }
    }
    cerr << "\n";
}

// gpf format reader
Paths gpf_read(string name) {
    ifstream f(name);
    int path_cnt;
    f >> path_cnt;
    // cout << path_cnt << "\n";
    Paths paths(path_cnt);
    for(int i = 0; i<path_cnt; i++) {
        int point_cnt;
        f >> point_cnt;
        // cout << "  " << point_cnt << "\n";
        for(int j=0; j<point_cnt; j++) {
            double x,y;
            f >> x >> y;
            // cout << "    " << x << " " << y << "\n";
            paths[i]<< IntPoint(x*S,y*S);
        }
    }
    return paths;
}
