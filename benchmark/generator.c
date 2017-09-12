#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PI 3.141592654
#define PI2 PI*2.0
void star(FILE* fp, int cx, int cy, int r1, int r2, int cn) {
	fprintf(fp, "%d\n", cn);
	double a = PI2/cn;
	for(int i=0; i< cn; i++) {
		double r = i%2 ? r1 : r2; 
		double x = cx + r*sin(i*a);
		double y = cy + r*cos(i*a);
		fprintf(fp, "%f %f\n", x, y);
	};
}

void random_stars(FILE* fp, int stars_cnt) {
	fprintf(fp, "%d\n", stars_cnt);
	for(int i=0; i<stars_cnt; i++) {
		int cx = rand() % 1000;
		int cy = rand() % 1000;
		int r1 = rand() % 500;
		int r2 = rand() % 100;
		int cn = (3 + rand() % 64) * 2;
		star(fp, cx, cy, r1, r2, cn);
		//fprintf(fp, "\n");
	}
}

long get_nsec() {
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return t.tv_nsec;
}

void main() {
	srand((int)get_nsec());
	random_stars(stdout, 100);
}

