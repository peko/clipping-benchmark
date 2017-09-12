#include <stdio.h>

#include "gpc.h"
#include "measure.h"


void info(gpc_polygon* p);
void data(gpc_polygon* p);

int main() {
    gpc_polygon p1, p2, out;
    
    FILE* fp;

    fp = fopen("../polygons/100_stars_1.gpf", "r");
    gpc_read_polygon(fp, 0, &p1);
    fclose(fp);
    info(&p1);
    
    fp = fopen("../polygons/100_stars_2.gpf", "r");
    gpc_read_polygon(fp, 0, &p2);
    fclose(fp);
    printf("polygon 2 has %d contours\n", p2.num_contours); 
    info(&p2);

    start();
    gpc_polygon_clip(GPC_INT, &p1, &p2, &out);
    stop();
    info(&out);
    data(&out);

    gpc_free_polygon(&out);
    gpc_free_polygon(&p2);
    gpc_free_polygon(&p1);
}

void data(gpc_polygon* p) {
    for(int i=0;i<p->num_contours; i++) {
        gpc_vertex v;
        for(int j=0; j<p->contour[i].num_vertices; j++) {
            v = p->contour[i].vertex[j];
            fprintf(stdout, "%f %f\n", v.x, v.y);
        }
        // close path
        v = p->contour[i].vertex[0];
        fprintf(stdout, "%f %f\n", v.x, v.y);
            
        fprintf(stdout, "\n");
    }
}

void info(gpc_polygon* p) {
    fprintf(stderr, "%d contours\n", p->num_contours);
    for(int i=0;i<p->num_contours; i++) {
        fprintf(stderr, "  %d has %d vertices\n", i, p->contour[i].num_vertices);
    }
    fprintf(stderr, "\n");
}
