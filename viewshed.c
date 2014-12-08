/* viewshed.c */
#include "viewshed.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

Grid* initGrid(int c, int r, int xll, int yll, int cs, int v) {
	Grid* newGrid = (Grid *)malloc(sizeof(Grid));
	newGrid->rows = r;
	newGrid->cols = c;
	newGrid->xllcorner = xll;
	newGrid->yllcorner = yll;
	newGrid->cellsize = cs;
	newGrid->NODATA_value =  v;
	int i,j,k;
	newGrid->data = (float **)malloc(sizeof(float *) * r);
	for(i=0; i<r; i++) {
		newGrid->data[i] = (float *)malloc(sizeof(float) * c);
	}
	return newGrid;
}

// Reads header informtation from file into grid
Grid* readHeader(Grid* g, char* filename) {

	FILE* f;
	char s[100];
	int cols, rows, yllcorner, xllcorner, cellsize, NODATA_value;

	f = fopen(filename, "r");
	if (f == NULL) {
		printf("cannot open file..");
		exit(1);
	}

	fscanf(f, "%s", s);
	fscanf(f, "%d", &cols);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &rows);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &yllcorner);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &xllcorner);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &cellsize);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &NODATA_value);

	fclose(f);	

	g = initGrid(cols, rows, yllcorner, xllcorner, cellsize, NODATA_value);

	return g;
}

// Reads header and data from file into grid
Grid* readAll(Grid* g, char* filename) {

	FILE* f;
	char s[100];
	int cols, rows, yllcorner, xllcorner, cellsize, NODATA_value;

	f = fopen(filename, "r");
	if (f == NULL) {
		printf("cannot open file..");
		exit(1);
	}

	fscanf(f, "%s", s);
	fscanf(f, "%d", &cols);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &rows);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &yllcorner);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &xllcorner);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &cellsize);

	fscanf(f, "%s", s);
	fscanf(f, "%d", &NODATA_value);

	g = initGrid(cols, rows, yllcorner, xllcorner, cellsize, NODATA_value);

	float entry;
	int r = 0;
	int c = 0;
	fscanf(f, "%f", &entry);
	while(!feof(f)){
		g->data[r][c] = entry;
		c++;
		if (c >= g->cols) {
			r++;
			c=0;
		}
		fscanf(f, "%f", &entry);
	}

	fclose(f);

	return g;
}

//Writes grid to file
void write(Grid* g, char* filename) {

	FILE* f;	
	char s[100];
	int cols, rows, yllcorner, xllcorner, cellsize, NODATA_value;

	f = fopen(filename, "w");	
	if (f == NULL) {
		printf("cannot open file..");
		exit(1);
	}

	fprintf(f, "ncols         %d\n", g->cols);
	fprintf(f, "nrows         %d\n", g->rows);
	fprintf(f, "xllcorner     %d\n", g->xllcorner);
	fprintf(f, "yllcorner     %d\n", g->yllcorner);
	fprintf(f, "cellsize      %d\n", g->cellsize);
	fprintf(f, "NODATA_value  %d\n", g->NODATA_value);

	int i,j;
	for(i=0; i<g->rows; i++) {
		for(j=0; j<g->cols; j++){
			fprintf(f, "%f ", g->data[i][j]);
		}
		fprintf(f,"\n");
	}
}

//Print header of grid
void printHeader(Grid* g) {
	printf("ncols         %d\n", g->cols);
	printf("nrows         %d\n", g->rows);
	printf("xllcorner     %d\n", g->xllcorner);
	printf("yllcorner     %d\n", g->yllcorner);
	printf("cellsize      %d\n", g->cellsize);
	printf("NODATA_value  %d\n", g->NODATA_value);

}

//Print values of grid
void printValues(Grid* g) {
	int i,j;
	for(i=0; i<g->rows; i++) {
		for(j=0; j<g->cols; j++){
			printf("%f ", g->data[i][j]);
		}
		printf("\n");
	}
}

//Print header and values of grid
void printGrid(Grid* g) {
	printHeader(g);
	printValues(g);
}


//Calculates vertical slope from (axInt,ayInt) to (bxInt, byInt)
float calculateSlope(Grid* g, float ax, float ay, float bx, float by, int crossX) {

	float heightA, heightB, difference, distance, roundUp, slope, length;
	heightA = g->data[(int)ax][(int)ay];
	if(crossX == 1) {
		length = by - floor(by);
		if(length == 0) {
			heightB = g->data[(int)bx][(int)by];
		}
		else {
			roundUp = by + 1.0;
			slope = g->data[(int)bx][(int)roundUp] - g->data[(int)bx][(int)by];
			heightB = slope * length + g->data[(int)bx][(int)by];
		}
	} else {
		length = bx - floor(bx);
		if(length == 0) {
			heightB = g->data[(int)bx][(int)by];
		}
		else {
			roundUp = bx + 1.0;
			slope = g->data[(int)roundUp][(int)by] - g->data[(int)bx][(int)by];
			heightB = slope * length + g->data[(int)bx][(int)by];
		}
	}
	difference = heightB - heightA; 
	distance = sqrt((by-ay) * (by-ay) + (bx-ax) * (bx-ax));
	return difference/distance;
}

//Returns true if (bx, by) is visible from (ax, ay)
float isVisible(Grid* g, int axInt, int ayInt, int bxInt, int byInt) {

	float ax, ay, bx, by, m, b, x, y;
	ax = (float)axInt;
	ay = (float)ayInt;
	bx = (float)bxInt;
	by = (float)byInt;
	
	if(axInt == bxInt && ayInt == byInt) {
		return 1.0;
	}
	float mainSlope = calculateSlope(g, axInt, ayInt, bxInt, byInt, 1);

	if(bx-ax == 0) {
		y = ay; //y iterator
		if(y<by) {
			y = y + 1.0;
		}
		if(y>by){
			y = y - 1.0;
		}

		while(y!=by) {
			if(calculateSlope(g, ax, ay, bx, y, 0) > mainSlope) {
				return 0.0;
			}
			if(y<by) {
				y = y + 1.0;
			}else{
				y = y - 1.0;
			}
		}
	} else if(by-ay == 0) {
		x = ax; //y iterator
		if(x<bx) {
			x = x + 1.0;
		}
		if(x>bx){
			x = x - 1.0;
		}

		while(x!=bx) {
			if(calculateSlope(g, ax, ay, x, by, 1) > mainSlope) {
				return 0.0;
			}
			if(x<bx) {
				x = x + 1.0;
			}else{
				x = x - 1.0;
			}
		}
	} else {
		
		m = (by-ay)/(bx-ax); //slope of line
		b = ay - (m*ax); //y-intercept
		x = ax; //x iterator
		
		if(x<bx) {
			x = x + 1.0;
		}else{
			x = x - 1.0;
		}
		while(x != bx) {
			if(calculateSlope(g, ax, ay, x, m*x+b, 1) > mainSlope) {
				return 0.0;
			}

			if(x<bx) {
				x = x + 1.0;
			}else{
				x = x - 1.0;
			}
		}

		y = ay; //y iterator
		if(y<by) {
			y = y + 1.0;
		}else{
			y = y - 1.0;
		}

		while(y != by) {
			if(calculateSlope(g, ax, ay, (y-b)/m, y, 0) > mainSlope) {
				return 0.0;
			}

			if(y<by) {
				y = y + 1.0;
			}else{
				y = y - 1.0;
			}
		}
	}

	return 1.0;
}

//Returns the viewshed grid from point (x,y)
Grid* computeViewshed(Grid* egrid, Grid* vgrid, int x, int y) {
	int i, j;
	for(i=0; i<egrid->rows; i++) {
		for(j=0; j<egrid->cols; j++) {
			vgrid->data[i][j] = (float)isVisible(egrid, x, y, i, j);
		}
	}
	return vgrid;
}

//Main method
int main(int argc, char* args[]) {

	if(argc!=5) {
		printf("Command line arguments must be of form: elevgrid, viewshedgrid, x, y\n");
		exit(1);
	}

	char *elevfname, *viewfname;
	elevfname = args[1];
	viewfname = args[2];

	Grid* elevgrid;
	Grid* viewgrid;

	elevgrid = readAll(elevgrid, elevfname);
	viewgrid = readAll(viewgrid, elevfname);

	viewgrid = computeViewshed(elevgrid, viewgrid, atoi(args[3]), atoi(args[4]));

	write(viewgrid, viewfname);

	return 0;
}
