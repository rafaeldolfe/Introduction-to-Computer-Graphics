/* Release code for program 1 CPE 471 Fall 2016 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <array>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

/*
   Helper function you will want all quarter
   Given a vector of shapes which has already been read from an obj file
   resize all vertices to the range [-1, 1]
 */
void resize_obj(std::vector<tinyobj::shape_t> &shapes){
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001f;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
         if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

         if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
         if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

         if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
         if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
      }
   }

	//From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = float(2.0 / maxExtent);
   shiftX = float(minX + (xExtent/ 2.0));
   scaleY = float(2.0 / maxExtent);
   shiftY = float(minY + (yExtent / 2.0));
   scaleZ = float(2.0/ maxExtent);
   shiftZ = float(minZ + (zExtent)/2.0);

   //Go through all verticies shift and scale them
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
         assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
         assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
         assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
      }
   }
}

struct intPoint
{
	int x, y;
};

bool intpoint_inside_trigon(intPoint s, intPoint a, intPoint b, intPoint c)
{
	int as_x = s.x - a.x;
	int as_y = s.y - a.y;

	bool s_ab = (b.x - a.x) * as_y - (b.y - a.y) * as_x >= 0;

	if ((c.x - a.x) * as_y - (c.y - a.y) * as_x > 0 == s_ab) return false;

	if ((c.x - b.x) * (s.y - b.y) - (c.y - b.y) * (s.x - b.x) >= 0 != s_ab) return false;

	return true;
}

void init_shape(string meshName, vector<array<float, 3>> *vertices, vector<array<array<float, 3>, 3>> *triangles)
{
	vector<unsigned int> triBuf;
	vector<float> posBuf;

	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> objMaterials;
	vector<tinyobj::shape_t> testshapes;

	string errStr;

	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());

	if (!rc) {
		cerr << errStr << endl;

	}
	else {
		resize_obj(shapes);
		posBuf = shapes[0].mesh.positions;
		triBuf = shapes[0].mesh.indices;
		testshapes = shapes;
	}

	for (size_t v = 0; v < posBuf.size(); v = v + 3) {
		array<float, 3> curr;
		curr[0] = posBuf[v];
		curr[1] = posBuf[v + 1];
		curr[2] = posBuf[v + 2];
		(*vertices).push_back(curr);
	}

	for (size_t t = 0; t < triBuf.size(); t = t + 3) {
		array<array<float, 3>, 3> curr;
		curr[0][0] = (*vertices)[triBuf[t]][0];
		curr[0][1] = (*vertices)[triBuf[t]][1];
		curr[0][2] = (*vertices)[triBuf[t]][2];
		curr[1][0] = (*vertices)[triBuf[t + 1]][0];
		curr[1][1] = (*vertices)[triBuf[t + 1]][1];
		curr[1][2] = (*vertices)[triBuf[t + 1]][2];
		curr[2][0] = (*vertices)[triBuf[t + 2]][0];
		curr[2][1] = (*vertices)[triBuf[t + 2]][1];
		curr[2][2] = (*vertices)[triBuf[t + 2]][2];
		(*triangles).push_back(curr);
	}
}

void init_zbuffer(vector<vector<double>>* zbuffer, int g_width, int g_height)
{
	vector<double> row;

	for (int i = 0; i < g_height * 2; i++) {
		row.push_back(0);
	}
	for (int i = 0; i < g_width * 2; i++) {
		(*zbuffer).push_back(row);
	}
}

void init_yinterp(vector<int>* yinterp, float scaleFactor, float img_height, float offsetY)
{
	int offset = int(ceil(offsetY));
	int height = int(ceil(scaleFactor * img_height));
	for (int i = 0; i < height + offset; i++) {
		int curr = int((255 * (i - offset)) / height);
		if (i - offset < 0)
		{
			curr = 0;
		}
		(*yinterp).push_back(curr);
	}
}

void get_img_min_max(const vector<array<float, 3>> vertices, float* minX, float* maxX, float* minY, float* maxY)
{
	(*minX) = 1;
	(*maxX) = -1;
	(*minY) = 1;
	(*maxY) = -1;
	for (size_t v = 0; v < vertices.size(); v++) {
		if (vertices[v][0] < (*minX))
			(*minX) = vertices[v][0];
		if (vertices[v][0] > (*maxX))
			(*maxX) = vertices[v][0];
		if (vertices[v][1] < (*minY))
			(*minY) = vertices[v][1];
		if (vertices[v][1] > (*maxY))
			(*maxY) = vertices[v][1];
	}
}

void get_img_scale(const int g_width, const int g_height, const float img_width, const float img_height, float* scaleFactor)
{
	float screenAspect = (float) g_width / g_height;
	float rectAspect = img_width / img_height;

	if (screenAspect > rectAspect)
		(*scaleFactor) = g_height / img_height;
	else
		(*scaleFactor) = g_width / img_width;
}

int run(string meshName, string imgName, int g_width, int g_height, int cmode)
{
	vector<array<float, 3>> vertices;
	vector<array<array<float, 3>, 3>> triangles;

	float minX;
	float maxX;
	float minY;
	float maxY;
	float scaleFactor;

	init_shape(meshName, &vertices, &triangles);
	get_img_min_max(vertices, &minX, &maxX, &minY, &maxY);
	get_img_scale(g_width, g_height, maxX - minX, maxY - minY, &scaleFactor);

	auto image = make_shared<Image>(g_width, g_height);
	float left = (g_width - (2 * maxX) * scaleFactor) / 2;
	float offsetY = (g_height - (2 * maxY) * scaleFactor) / 2;

	if (cmode == 1)
	{
		vector<vector<double>> zbuffer;
		init_zbuffer(&zbuffer, g_width, g_height);
		for (size_t t = 0; t < triangles.size(); t++) {

			intPoint A = { int((triangles[t][0][0] + abs(minX)) * scaleFactor + left), int((triangles[t][0][1] + abs(minY)) * scaleFactor + offsetY) };
			float za = (triangles[t][0][2] + 1) / 2;
			intPoint B = { int((triangles[t][1][0] + abs(minX)) * scaleFactor + left), int((triangles[t][1][1] + abs(minY)) * scaleFactor + offsetY) };
			float zb = (triangles[t][1][2] + 1) / 2;
			intPoint C = { int((triangles[t][2][0] + abs(minX)) * scaleFactor + left), int((triangles[t][2][1] + abs(minY)) * scaleFactor + offsetY) };
			float zc = (triangles[t][2][2] + 1) / 2;

			int bounding_box_minX = min({ A.x, B.x, C.x });
			int bounding_box_maxX = max({ A.x, B.x, C.x });
			int bounding_box_minY = min({ A.y, B.y, C.y });
			int bounding_box_maxY = max({ A.y, B.y, C.y });

			for (int i = bounding_box_minX; i < bounding_box_maxX; i++) {
				for (int j = bounding_box_minY; j < bounding_box_maxY; j++) {
					intPoint p = { i, j };
					if (intpoint_inside_trigon(p, A, B, C)) {

						intPoint n = { A.x - B.x, A.y - B.y };
						intPoint m = { A.x - C.x, A.y - C.y };
						int crossz = n.x * m.y - n.y * m.x;
						double Area = sqrt(crossz * crossz);

						intPoint nb = { p.x - A.x, p.y - A.y };
						intPoint mb = { p.x - B.x, p.y - B.y };
						crossz = nb.x * mb.y - nb.y * mb.x;
						double AreaB = sqrt(crossz * crossz);

						intPoint nc = { p.x - A.x, p.y - A.y };
						intPoint mc = { p.x - C.x, p.y - C.y };
						crossz = nc.x * mc.y - nc.y * mc.x;
						double AreaC = sqrt(crossz * crossz);


						double beta = AreaB / Area;
						double gamma = AreaC / Area;
						double alpha = 1 - beta - gamma;

						double z = beta * zc + gamma * zb + alpha * za;

						if (zbuffer[i][j] < z || zbuffer[i][j] == 0) {
							zbuffer[i][j] = z;
							unsigned char red = unsigned char(255 * z);
							image->setPixel(i, j, red, 0, 0);
						}
					}
				}
			}
		}
	}
	else if (cmode == 2)
	{
		vector<int> yinterp;
		init_yinterp(&yinterp, scaleFactor, maxY - minY, offsetY);
		for (size_t t = 0; t < triangles.size(); t++) {

			intPoint A = { int((triangles[t][0][0] + abs(minX)) * scaleFactor + left), int((triangles[t][0][1] + abs(minY)) * scaleFactor + offsetY) };
			intPoint B = { int((triangles[t][1][0] + abs(minX)) * scaleFactor + left), int((triangles[t][1][1] + abs(minY)) * scaleFactor + offsetY) };
			intPoint C = { int((triangles[t][2][0] + abs(minX)) * scaleFactor + left), int((triangles[t][2][1] + abs(minY)) * scaleFactor + offsetY) };

			int bounding_box_minX = min({ A.x, B.x, C.x });
			int bounding_box_maxX = max({ A.x, B.x, C.x });
			int bounding_box_minY = min({ A.y, B.y, C.y });
			int bounding_box_maxY = max({ A.y, B.y, C.y });

			for (int i = bounding_box_minX; i < bounding_box_maxX; i++) {
				for (int j = bounding_box_minY; j < bounding_box_maxY; j++) {
					intPoint p = { i, j };
					if (intpoint_inside_trigon(p, A, B, C)) {
						unsigned char blue = unsigned char(yinterp[j]);
						unsigned char red = unsigned char(yinterp[j] * -1 + 255);
						image->setPixel(i, j, red, 255, blue);
					}
				}
			}
		}
	}
	image->writeToFile(imgName);

	return 0;
}

int main()
{
	run("resources/teapot.obj", "Teapot.png", 1200, 500, 2);

	run("resources/bunny.obj", "Bunny.png", 500, 800, 1);
}