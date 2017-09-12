#include <string.h>
#include <math.h>

#include <shapefil.h>
#include <proj_api.h>

#include "kvec.h"

typedef struct {
    double x, y;
} vertex_t;

typedef kvec_t(vertex_t) vertex_v;
typedef struct {
    vertex_v vertex;
    vertex_t min, max, center;
} contour_t;

typedef kvec_t(contour_t) contour_v;
typedef struct {
    contour_v contour;
    vertex_t min, max, center;
} shape_t;
typedef kvec_t(shape_t) shape_v;

typedef kvec_t(char*) string_v;

shape_v load_shapes(char* filename) {

    shape_v shapes;
    kv_init(shapes);
    
    double  adfMinBound[4],
            adfMaxBound[4];

    // Read file
    SHPHandle hSHP = SHPOpen( filename, "rb" );
    if(hSHP == NULL) goto end_loading;

    // Print shape bounds
    int country_count, shapes_vype;
    SHPGetInfo( hSHP, &country_count, &shapes_vype, adfMinBound, adfMaxBound );
    
    fprintf(stderr, "Load %d countries\n", country_count);
    // Iterate through countries
    for(int i = 0; i < country_count; i++ ) {
                        
        SHPObject *shp = SHPReadObject(hSHP, i);
        if(shp == NULL) goto end_loading;

        if(shp->nParts == 0) continue;

        // first part starts at point 0
        if(shp->panPartStart[0] != 0) goto end_loading;

        // collect parts of country
        shape_t shape;
        kv_init(shape.contour);
        shape.min = (vertex_t){shp->dfXMin, shp->dfYMin};
        shape.max = (vertex_t){shp->dfXMax, shp->dfYMax};
        shape.center = (vertex_t){
            (shp->dfXMin+shp->dfXMax)/2.0, 
            (shp->dfYMin+shp->dfYMax)/2.0};

        int parts = shp->nParts;
        double cum = 0.0;
        for (int j=0; j<parts; j++) {
            // start index
            int s = shp->panPartStart[j];
            // end index - start of next minus one, or end
            int e = (j+1 < parts) ?
                shp->panPartStart[j+1]:
                shp->nVertices;
                
            contour_t contour;
            kv_init(contour.vertex);
            // collect points of part
            for(int k=s; k<e; k++){
                vertex_t p = {shp->padfX[k], shp->padfY[k]};
                kv_push(vertex_t, contour.vertex, p);
                // cumulative average for center
                if(cum>=1.0) {
                    contour.center.x = (cum-1.0)/cum*contour.center.x + p.x/cum;
                    contour.center.y = (cum-1.0)/cum*contour.center.y + p.y/cum;
                } else {
                    contour.center.x = p.x;
                    contour.center.y = p.y;
                }
                cum+=1.0;
            }
            kv_push(contour_t, shape.contour, contour);
        }
        SHPDestroyObject( shp );

        kv_push(shape_t, shapes, shape);
    }
    SHPClose( hSHP );
    
end_loading:
    return shapes;        
}

// Country name loader DBF
string_v load_names(
        const char* filename,
        const char* colname) {

    DBFHandle hDBF;
    string_v col;
    kv_init(col);

    hDBF = DBFOpen( filename, "rb" );
    if( hDBF == NULL ) {
        fprintf(stderr, "DBFOpen(%s,\"r\") failed.\n", filename );
        return col;
    }
    int fid = DBFGetFieldIndex(hDBF, colname);
    for(int i = 0; i < DBFGetRecordCount(hDBF); i++ ) {
        char* str = (char *) DBFReadStringAttribute(hDBF, i, fid);
        if(str != NULL)
            kv_push(char*, col, strdup(str));
        else            
            kv_push(char*, col, NULL);
    }

    DBFClose( hDBF );
    return col;
}


// reproject shapes
shape_t reproject(
    const shape_t* shape, 
    const char* from, 
    const char* to){

    projPJ old_prj = pj_init_plus(from);
    projPJ new_prj = pj_init_plus(to);

    shape_t shape_prj;
    kv_init(shape_prj.contour);
    
    shape_prj.min = (vertex_t){ INFINITY, INFINITY};
    shape_prj.max = (vertex_t){-INFINITY,-INFINITY};
    double k = 0.0;
    for(size_t i=0; i<shape->contour.n; i++) {
        contour_t* contour = &shape->contour.a[i];
        contour_t contour_prj;
        kv_init(contour_prj.vertex);
        for(size_t p=0; p<contour->vertex.n; p++) {
            vertex_t pnt = contour->vertex.a[p];
            pnt.x *= DEG_TO_RAD;
            pnt.y *= DEG_TO_RAD;
            int32_t err = pj_transform(old_prj, new_prj, 1, 0, &pnt.x, &pnt.y, NULL);
            if (err) {
                fprintf(stderr, "ERR%d %s\n", err, pj_strerrno(err));
                continue;
            }

            // cumulitive average for center
            if(k>=1.0) {
                shape_prj.center.x = (k-1.0)/k*shape_prj.center.x + pnt.x/k;
                shape_prj.center.y = (k-1.0)/k*shape_prj.center.y + pnt.y/k;
            }else {
                shape_prj.center.x = pnt.x;
                shape_prj.center.y = pnt.y;
            }
            k+=1.0;

            // new bounds
            if(pnt.x>shape_prj.max.x) shape_prj.max.x = pnt.x;
            if(pnt.y>shape_prj.max.y) shape_prj.max.y = pnt.y;
            if(pnt.x<shape_prj.min.x) shape_prj.min.x = pnt.x;
            if(pnt.y<shape_prj.min.y) shape_prj.min.y = pnt.y;

            kv_push(vertex_t, contour_prj.vertex, pnt);
        }
        kv_push(contour_t, shape_prj.contour, contour_prj);
    }

    pj_free(old_prj);
    pj_free(new_prj);

    return shape_prj;
}

void write_shape(shape_t* shape, char* filename) {
    FILE* fp = fopen(filename, "w");
    // contours number
    fprintf(fp, "%d\n", shape->contour.n);
    for(size_t j=0; j<shape->contour.n; j++) {
        contour_t* contour = &shape->contour.a[j];
        // vertices number
        fprintf(fp, "%d\n", contour->vertex.n);
        // fprintf(fp, "\n");
        for(size_t k=0; k<contour->vertex.n; k++) {
            vertex_t* v = &contour->vertex.a[k];
            // vertex coordinates
            fprintf(fp, "%f %f\n", v->x, v->y);
        }
    }
    fclose(fp);
}

void shape_destroy(shape_t* shape) {
    for(size_t j=0; j<shape->contour.n; j++) kv_destroy(shape->contour.a[j].vertex);
    kv_destroy(shape->contour); // regions
}

void shapes_destroy(shape_v* shapes) {
    for(size_t i=0; i< shapes->n; i++) shape_destroy(&shapes->a[i]);
    kv_destroy(*shapes); // countries
}

void write_gpf(shape_v shapes, string_v names){
    char filename[512];
    char* wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
    char laea[512];
    for(size_t i=0; i< shapes.n; i++) {
        shape_t* shape = &shapes.a[i];
        sprintf(filename, "out/%s.gpf\0", names.a[i]);
        // sprintf(laea, "+proj=laea +lat_0=%f +lon_0=%f +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs", 89.99, 90.0);
        sprintf(laea, "+proj=laea +lat_0=%f +lon_0=%f +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs", shape->center.y, shape->center.x);
	shape_t shape_laea = reproject(shape, wgs84, laea);
        write_shape(&shape_laea, filename);
        shape_destroy(&shape_laea);
    }
}

void names_destroy(string_v* names) {
    for(size_t i=0; i< names->n; i++) {
        free(names->a[i]);
    }
    kv_destroy(*names);
}

int main(int argc, char** argv) {
    if(argc !=2) {
        fprintf(stderr, "Usage: shp2gpf <shapefile>\n"); 
        return 1;
    }
    
    string_v names = load_names(argv[1], "name_long");
    shape_v shapes = load_shapes(argv[1]);

    write_gpf(shapes, names);

    shapes_destroy(&shapes);
    names_destroy(&names);
}
