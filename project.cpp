/*
    Sample code by Wallace Lira <http://www.sfu.ca/~wpintoli/> based on
    the four Nanogui examples and also on the sample code provided in
          https://github.com/darrenmothersele/nanogui-test

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/glcanvas.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <limits>
#include <random>
#include <set>
#include <vector>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;
using std::map;
using std::make_pair;
using std::max;
using std::min;

using nanogui::Screen;
using nanogui::Window;
using nanogui::GroupLayout;
using nanogui::Button;
using nanogui::CheckBox;
using nanogui::Vector2f;
using nanogui::Vector2i;
using nanogui::MatrixXu;
using nanogui::MatrixXf;
using nanogui::Label;
using nanogui::Arcball;
using nanogui::Vector3f;
using nanogui::Vector3i;
using nanogui::Vector4f;
using nanogui::Matrix4f;
using nanogui::ortho;



struct W_edge;
struct W_vertex;
struct W_face;

struct W_edge {
  W_vertex *start, *end;
  W_face *left, *right;
  W_edge *left_prev, *left_next;
  W_edge *right_prev, *right_next;
  W_edge *opposite;
};
struct W_vertex {
  float x, y, z;
  float nx, ny, nz;
  W_edge *edge;
  Matrix4f quadric;
  int index;
};
struct W_face {
  W_vertex *a, *b, *c;
  float nx, ny, nz;
  W_edge *edge;
  Matrix4f quadric;
};

enum meshShading_t {FLAT, SMOOTH, WIREFRAME, MESH_EDGES};


class MyGLCanvas : public nanogui::GLCanvas {
  public:
    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent) {
          using namespace nanogui;


      // Initialize the shaders
    	mShader.initFromFiles("a_smooth_shader", "StandardShading.vertexshader", "StandardShading.fragmentshader");
      mShader.bind();

      // Load a default obj file
      string objFile = "horse.obj";

      // Read the obj file
      readOBJLines(objFile);

      maxWidth = max(max((maxX - minX), (maxY - minY)), (maxZ - minZ));

      // Allocate memory for the winged edge data structure
      W_edges = (W_edge*)malloc(sizeof(W_edge) * faces.cols() * 3);
      W_vertices = (W_vertex*)malloc(sizeof(W_vertex) * vertices.cols());
      W_faces = (W_face*)malloc(sizeof(W_face) * faces.cols());

      // Initialize the winged edge data structure for the mesh
      initWingedEdge();

      // Set the vertices to draw for the mesh
      setDrawnVertices();

    }

    //flush data on call
    ~MyGLCanvas() {
      mShader.free();
    }

    void getOBJNumber(string line) {
      int numberStart = -1;
      int numberLength = 0;

      string vertexNumber_s = "\0          ";
      string faceNumber_s = "\0          ";

      for (int i = 0; i < line.length(); i++) {
        if (isdigit(line[i])) {
          if (numberStart < 0) {
            numberStart = i;
          }
          numberLength++;
        }
        if (!isdigit(line[i]) && numberStart >= 0 && vertexNumber_s.length() == 0) {
          vertexNumber_s = line.substr(numberStart, numberLength);
          numberStart = -1;
          numberLength = 0;
        }
        if (!isdigit(line[i]) && numberStart >= 0 && faceNumber_s.length() == 0) {
          faceNumber_s = line.substr(numberStart, numberLength);
          numberStart = -1;
          numberLength = 0;
        }
      }
      if (faceNumber_s.length() == 0) {
        faceNumber_s = line.substr(numberStart, numberLength);
      }

      vertexNumber = std::stoi(vertexNumber_s);
      faceNumber = std::stoi(faceNumber_s);
    }

    vector <float> splitOnSpaceF(string line) {
      int numberStart = -1;
      int numberLength = 0;
      vector <float> output;
      for (int i = 1; i < line.length(); i++) {
        if (line[i] != ' ') {
          if (numberStart < 0) {
            numberStart = i;
          }
          numberLength++;
        } else if (numberStart >= 0) {
          output.push_back(std::stof(line.substr(numberStart, numberLength)));
          numberStart = -1;
          numberLength = 0;
        }
      }
      if (numberStart != -1) {
        output.push_back(std::stof(line.substr(numberStart, numberLength)));
      }
      return output;
    }

    vector <int> splitOnSpaceI(string line) {
      int numberStart = -1;
      int numberLength = 0;
      int inputs = 0;
      vector <int> output;
      for (int i = 1; i < line.length(); i++) {
        if (inputs == 3) {
          break;
        }
        if (line[i] != ' ') {
          if (numberStart < 0) {
            numberStart = i;
          }
          numberLength++;
        } else if (numberStart >= 0) {
          output.push_back(std::stoi(line.substr(numberStart, numberLength)));
          numberStart = -1;
          numberLength = 0;
          inputs++;
        }
      }
      if (numberStart != -1) {
        output.push_back(std::stoi(line.substr(numberStart, numberLength)));
      }
      return output;
    }

    void readVertexLine(string line) {
      vector <float> vertexCoordinates = splitOnSpaceF(line);

      vertices.conservativeResize(vertices.rows(), vertices.cols()+1);
      vertices.col(vertices.cols()-1) <<  vertexCoordinates[0], vertexCoordinates[1], vertexCoordinates[2];

      maxX = max(maxX, vertexCoordinates[0]);
      minX = min(minX, vertexCoordinates[0]);
      maxY = max(maxY, vertexCoordinates[1]);
      minY = min(minY, vertexCoordinates[1]);
      maxZ = max(maxZ, vertexCoordinates[2]);
      minZ = min(minZ, vertexCoordinates[2]);
    }

    void readFaceLine(string line) {
      vector <int> vertexIndexes = splitOnSpaceI(line);

      faces.conservativeResize(faces.rows(), faces.cols()+1);
      faces.col(faces.cols()-1) <<  vertexIndexes[0] - 1, vertexIndexes[1] - 1, vertexIndexes[2] - 1;
    }

    void readOBJLines(string objFile) {
      string line;
      std::ifstream myfile (objFile);

      if (myfile.is_open()) {

        vertices.resize(3, 0);
        faces.resize(3, 0);

        int vertexCounter = 0;
        int faceCounter = 0;

        while ( getline (myfile,line) )
        {
          if (line[0] == 'v') {
            readVertexLine(line);
          }
          if (line[0] == 'f') {
            readFaceLine(line);
          }
        }

        vertexNumber = vertices.cols();
        faceNumber = faces.cols();

        myfile.close();
      }

      else cout << "Unable to open file";
    }

    void writeOBJLines(string objFile) {
      std::ofstream myfile (objFile);
      if (myfile.is_open())
      {
        myfile << "# " << vertexNumber << " " << faceNumber << "\n";

        int vertexCounter = 0;
        for (int i = 0; i < vertices.cols(); i++) {
          if (W_vertices[i].edge == NULL) {
            continue;
          }
          myfile << "v " << W_vertices[i].x << " " << W_vertices[i].y << " " << W_vertices[i].z << "\n";
          W_vertices[i].index = vertexCounter;
          vertexCounter++;
        }

        for (int i = 0; i < faces.cols(); i++) {
          if (W_faces[i].edge == NULL) {
            continue;
          }
          myfile << "f " << W_faces[i].a->index + 1 << " " << W_faces[i].b->index + 1 << " " << W_faces[i].c->index + 1 << "\n";
        }

        myfile.close();
      } else {
        cout << "Unable to save the file\n";
      }
    }

    void openOBJFile(string objFile) {

      maxX = std::numeric_limits<float>::min();
      minX = std::numeric_limits<float>::max();
      maxY = std::numeric_limits<float>::min();
      minY = std::numeric_limits<float>::max();
      maxZ = std::numeric_limits<float>::min();
      minZ = std::numeric_limits<float>::max();

      free(W_edges);
      free(W_vertices);
      free(W_faces);
      W_edges_map.clear();

      readOBJLines(objFile);

      maxWidth = max(max((maxX - minX), (maxY - minY)), (maxZ - minZ));

      W_edges = (W_edge*)malloc(sizeof(W_edge) * faces.cols() * 3);
      W_vertices = (W_vertex*)malloc(sizeof(W_vertex) * vertices.cols());
      W_faces = (W_face*)malloc(sizeof(W_face) * faces.cols());

      initWingedEdge();

      setDrawnVertices();
    }

    void saveOBJFile(string objFile) {
      writeOBJLines(objFile);
    }

      //Method to update the rotation on each axis
    void setRotation(nanogui::Vector3f vRotation) {
        mRotation = vRotation;
    }

      //Method to update the translation on each axis
    void setTranslation(nanogui::Vector3f vTranslation) {
        mTranslation = vTranslation;
    }

      //Method to update the zoom
    void setZoom(float vZoom) {
      mZoom = vZoom;
    }

      //Method to update how the mesh is displayed
    void setMeshShading(meshShading_t selection) {
      if (meshShading == FLAT && selection != FLAT) {
        changeToSurfaceNormals();
      }
      if (meshShading != FLAT && selection == FLAT) {
        changeToFaceNormals();
      }

      meshShading = selection;
    }

    Vector3f computeFaceNormal(Vector3f vertex1, Vector3f vertex2, Vector3f vertex3) {
      Vector3f u = vertex2 - vertex1;
      Vector3f v = vertex3 - vertex1;
      return (u.cross(v)).normalized();
    }

    Matrix4f computePlaneQuadric(Vector3f normal, Vector3f point) {
      float d = -(normal.x() * point.x() + normal.y() * point.y() + normal.z() * point.z());
      MatrixXf plane = MatrixXf(4, 1);
      plane.col(0) << normal.x(), normal.y(), normal.z(), d;
      Matrix4f quadric = plane * plane.transpose();
      return quadric;
    }

    void initWingedEdge() {
      int primitiveSize = 3;

      // Init vertices
      Matrix4f identity;
      identity.setIdentity();
      for (int i = 0; i < vertices.cols(); i++) {
        W_vertices[i] = {vertices(0, i), vertices(1, i), vertices(2, i), NAN, NAN, NAN, NULL, identity, -1};
        // cout << W_vertices[i].x << ": " << W_vertices[i].y << ": " << W_vertices[i].z << '\n';
      }

      // Init faces
      for (int i = 0; i < faces.cols(); i++) {
        Vector3f normal = computeFaceNormal(vertices.col(faces(0, i)), vertices.col(faces(1, i)), vertices.col(faces(2, i)));
        Matrix4f quadric = computePlaneQuadric(normal, vertices.col(faces(2, i)));
        // cout << "Quadric:\n" << quadric << "\n";
        W_faces[i] = {&W_vertices[faces(0, i)], &W_vertices[faces(1, i)], &W_vertices[faces(2, i)], normal.x(), normal.y(), normal.z(), NULL, quadric};
      }

      // Init edges
      int edgeCounter = 0;
      for (int i = 0; i < faces.cols(); i++) {
        for (int j = 0; j < primitiveSize; j++) {

          int previousVertex = faces((j + 1) % primitiveSize, i);
          int nextVertex = faces(j % primitiveSize, i);

          W_edges[(i * 3) + j] = {&W_vertices[previousVertex], &W_vertices[nextVertex], NULL, &W_faces[i], NULL, NULL, NULL, NULL, NULL};


          if (W_vertices[previousVertex].edge == NULL) {
            W_vertices[previousVertex].edge = &W_edges[(i * 3) + j];
          }
          if (W_vertices[nextVertex].edge == NULL) {
            W_vertices[nextVertex].edge = &W_edges[(i * 3) + j];
          }
          if (W_faces[i].edge == NULL) {
            W_faces[i].edge = &W_edges[(i * 3) + j];
          }


          W_edges_map.insert(make_pair(make_pair(previousVertex, nextVertex), &(W_edges[(i * 3) + j])));
        }
      }

      // Fill in the rest of the edge info
      for (int i = 0; i < faces.cols(); i++) {
        for (int j = 0; j < primitiveSize; j++) {

          int previousVertex = faces((j + 1) % primitiveSize, i);
          int nextVertex = faces(j % primitiveSize, i);

          W_edge *currEdge = W_edges_map.at(make_pair(previousVertex, nextVertex));
          W_edge *oppositeEdge = W_edges_map.at(make_pair(nextVertex, previousVertex));

          currEdge->left = oppositeEdge->right;

          currEdge->opposite = oppositeEdge;

          int otherVertex = faces((j + 2) % primitiveSize, i);

          // the right adjacent edges can be obtained from the current face
          currEdge->right_prev = W_edges_map.at(make_pair(otherVertex, previousVertex));
          currEdge->right_next = W_edges_map.at(make_pair(nextVertex, otherVertex));

          // the left adjacent edges need to be obtained from the opposite face
          // it is simpler to set the left adjacent edges for the opposite edge instead, since we already have these
          oppositeEdge->left_prev = W_edges_map.at(make_pair(previousVertex, otherVertex));
          oppositeEdge->left_next = W_edges_map.at(make_pair(otherVertex, nextVertex));
        }
      }

      // Visit every face adjacent to each vertex to compute vertex normals and initial vertex quadrics
      for (int i = 0; i < vertices.cols(); i++) {

        Vector3f normal = Vector3f(0.0f, 0.0f, 0.0f);
        Matrix4f quadric = Eigen::Matrix4f::Zero();

        W_edge *edge0 = W_vertices[i].edge;
        W_edge *currEdge = edge0;

        do {
          if (currEdge->end == &W_vertices[i]) {
            normal = normal + Vector3f(currEdge->right->nx, currEdge->right->ny, currEdge->right->nz);
            quadric = quadric + currEdge->right->quadric;
            currEdge = currEdge->right_next;
          } else {
            normal = normal + Vector3f(currEdge->left->nx, currEdge->left->ny, currEdge->left->nz);
            quadric = quadric + currEdge->left->quadric;
            currEdge = currEdge->left_next;
          }
        } while (currEdge != edge0);

        normal = normal.normalized();

        W_vertices[i].nx = normal.x();
        W_vertices[i].ny = normal.y();
        W_vertices[i].nz = normal.z();
        W_vertices[i].quadric = quadric;

        // cout << "Quadric:\n" << quadric << "\n";
      }
    }

    void setDecimateK(int vDecimateK) {
      mDecimateK = vDecimateK;
    }

    void collapseEdge(W_edge* collapseEdge, Vector4f bestVertex) {
      // Collapse the edge
      // The end vertex, left_prev edge, and right_next edge are the same location as the original edge

      W_vertex* origStart = collapseEdge->start;
      W_vertex* origEnd = collapseEdge->end;

      W_vertex newVertex = {bestVertex.x(), bestVertex.y(), bestVertex.z(), NAN, NAN, NAN, NULL, origStart->quadric + origEnd->quadric, -1};

      // Create the new edges

      W_edge newEdgeLeft = {collapseEdge->left_prev->end, collapseEdge->left_prev->start,
                            collapseEdge->left_prev->right, collapseEdge->left_next->right,

                            collapseEdge->left_prev->right_prev->opposite, collapseEdge->left_prev->right_next->opposite,
                            collapseEdge->left_next->right_prev, collapseEdge->left_next->right_next,

                            collapseEdge->left_prev};


      W_edge newEdgeRight = {collapseEdge->right_next->start, collapseEdge->right_next->end,
                            collapseEdge->right_next->left, collapseEdge->right_prev->left,

                            collapseEdge->right_next->left_prev, collapseEdge->right_next->left_next,
                            collapseEdge->right_prev->left_prev->opposite, collapseEdge->right_prev->left_next->opposite,

                            collapseEdge->right_next->opposite};


      W_edge newEdgeLeftOpp = {collapseEdge->left_prev->start, collapseEdge->left_prev->end,
                            collapseEdge->left_next->right, collapseEdge->left_prev->right,

                            collapseEdge->left_next->right_prev->opposite, collapseEdge->left_next->right_next->opposite,
                            collapseEdge->left_prev->right_prev, collapseEdge->left_prev->right_next,

                            collapseEdge->left_prev->opposite};


      W_edge newEdgeRightOpp = {collapseEdge->right_next->end, collapseEdge->right_next->start,
                            collapseEdge->right_prev->left, collapseEdge->right_next->left,

                            collapseEdge->right_prev->left_prev, collapseEdge->right_prev->left_next,
                            collapseEdge->right_next->left_prev->opposite, collapseEdge->right_next->left_next->opposite,

                            collapseEdge->right_next};


      W_edge *edge0 = origStart->edge;
      W_edge *currEdge = edge0;
      W_face *currFace;
      W_vertex *currVertex;

      // visit every face adjacent to origStart, replace origStart with origEnd
      edge0 = origStart->edge;
      currEdge = edge0;
      do {
        if (currEdge->end == origStart) {
          currFace = currEdge->right;
          if (currFace->a == origStart) currFace->a = origEnd;
          if (currFace->b == origStart) currFace->b = origEnd;
          if (currFace->c == origStart) currFace->c = origEnd;
          currEdge = currEdge->right_next;
        } else {
          currFace = currEdge->left;
          if (currFace->a == origStart) currFace->a = origEnd;
          if (currFace->b == origStart) currFace->b = origEnd;
          if (currFace->c == origStart) currFace->c = origEnd;
          currEdge = currEdge->left_next;
        }
      } while (currEdge != edge0);

      // visit every edge adjacent to origStart, replace origStart with origEnd
      // find all of the edges adjacent to origStart
      edge0 = origStart->edge;
      currEdge = edge0;
      vector<W_edge*> adjacentEdges;
      do {
        adjacentEdges.push_back(currEdge);
        if (currEdge->end == origStart) {
          currEdge = currEdge->right_next;
        } else {
          currEdge = currEdge->left_next;
        }
      } while (currEdge != edge0);
      // replace origStart with origEnd for both the edge and the opposite
      for (std::vector<W_edge*>::iterator it = adjacentEdges.begin(); it != adjacentEdges.end(); it++) {
        currEdge = *it;
        if (currEdge->start == origStart) {
          currEdge->start = origEnd;
          if (currEdge->opposite->end != origStart) {
            std::cout << "Opposite edge is corrupted" << '\n';
            while(true);
          }
          currEdge->opposite->end = origEnd;
        }
        if (currEdge->end == origStart) {
          currEdge->end = origEnd;
          if (currEdge->opposite->start != origStart) {
            std::cout << "Opposite edge is corrupted" << '\n';
            while(true);
          }
          currEdge->opposite->start = origEnd;
        }
      }

      // insert the new edges (still using the same array location as left_prev and right_next)
      *(collapseEdge->left_prev) = newEdgeLeftOpp;
      *(collapseEdge->right_next) = newEdgeRight;
      *(collapseEdge->left_prev->opposite) = newEdgeLeft;
      *(collapseEdge->right_next->opposite) = newEdgeRightOpp;

      // delete the old edges
      // replace the location in the W_edges array with some value to indicatie the edge is "deleted"
      collapseEdge->left_next->start = NULL;
      collapseEdge->right_prev->start = NULL;
      collapseEdge->left_next->opposite->start = NULL;
      collapseEdge->right_prev->opposite->start = NULL;
      collapseEdge->start = NULL;
      collapseEdge->opposite->start = NULL;

      collapseEdge->left_next->end = NULL;
      collapseEdge->right_prev->end = NULL;
      collapseEdge->left_next->opposite->end = NULL;
      collapseEdge->right_prev->opposite->end = NULL;
      collapseEdge->end = NULL;
      collapseEdge->opposite->end = NULL;

      // insert the newVertex into the array location of origEnd
      newVertex.edge = collapseEdge->left_prev;
      *origEnd = newVertex;

      // delete the old vertex
      // replace the location in the W_vertices array with some value to indicate the vertex is "deleted"
      origStart->edge = NULL;
      vertexNumber = vertexNumber - 1;

      // delete the two old faces
      // replace the location in the W_faces array with some value to indicate the face is "deleted"
      collapseEdge->left->edge = NULL;
      collapseEdge->right->edge = NULL;
      faceNumber = faceNumber - 2;


      // update the edges adjacent to right_prev and left_next to point at the new edges (left_prev and right_next)
      collapseEdge->left_next->right_prev->right_next = collapseEdge->left_prev->opposite;
      collapseEdge->left_next->right_next->right_prev = collapseEdge->left_prev->opposite;
      collapseEdge->right_prev->left_prev->left_next = collapseEdge->right_next->opposite;
      collapseEdge->right_prev->left_next->left_prev = collapseEdge->right_next->opposite;

      collapseEdge->left_next->right_prev->opposite->left_next = collapseEdge->left_prev;
      collapseEdge->left_next->right_next->opposite->left_prev = collapseEdge->left_prev;
      collapseEdge->right_prev->left_prev->opposite->right_next = collapseEdge->right_next;
      collapseEdge->right_prev->left_next->opposite->right_prev = collapseEdge->right_next;


      // update face normals
      edge0 = origEnd->edge;
      currEdge = edge0;
      do {
        // if (currEdge == collapseEdge || currEdge == collapseEdge->right_prev || currEdge == collapseEdge->left_next || currEdge == collapseEdge->right_prev->opposite || currEdge == collapseEdge->left_next->opposite) {
        //   std::cout << "BAD!!!!!!!!!!!" << '\n';
        //   break;
        // }

        if (currEdge->end == origEnd) {
          currFace = currEdge->right;

          Vector3f normal = computeFaceNormal(Vector3f(currFace->a->x, currFace->a->y, currFace->a->z), Vector3f(currFace->b->x, currFace->b->y, currFace->b->z), Vector3f(currFace->c->x, currFace->c->y, currFace->c->z));
          currFace->nx = normal.x();
          currFace->ny = normal.y();
          currFace->nz = normal.z();

          currFace->edge = currEdge;

          currEdge = currEdge->right_next;
        } else {
          currFace = currEdge->left;

          Vector3f normal = computeFaceNormal(Vector3f(currFace->a->x, currFace->a->y, currFace->a->z), Vector3f(currFace->b->x, currFace->b->y, currFace->b->z), Vector3f(currFace->c->x, currFace->c->y, currFace->c->z));
          currFace->nx = normal.x();
          currFace->ny = normal.y();
          currFace->nz = normal.z();

          currFace->edge = currEdge;

          currEdge = currEdge->left_next;
        }
      } while (currEdge != edge0);


      // update vertex normals
      edge0 = origEnd->edge;
      currEdge = edge0;
      do {
        // if (currEdge == collapseEdge || currEdge == collapseEdge->right_prev || currEdge == collapseEdge->left_next || currEdge == collapseEdge->right_prev->opposite || currEdge == collapseEdge->left_next->opposite) {
        //   std::cout << "BAD!!!!!!!!!!!" << '\n';
        //   break;
        // }

        if (currEdge->end == origEnd) {

          currVertex = currEdge->start;

          currVertex->edge = currEdge;

          Vector3f normal = computeVertexNormal(currVertex);
          currVertex->nx = normal.x();
          currVertex->ny = normal.y();
          currVertex->nz = normal.z();

          currEdge = currEdge->right_next;
        } else {

          currVertex = currEdge->end;

          currVertex->edge = currEdge;

          Vector3f normal = computeVertexNormal(currVertex);
          currVertex->nx = normal.x();
          currVertex->ny = normal.y();
          currVertex->nz = normal.z();

          currEdge = currEdge->left_next;
        }
      } while (currEdge != edge0);

      Vector3f normal = computeVertexNormal(origEnd);
      origEnd->nx = normal.x();
      origEnd->ny = normal.y();
      origEnd->nz = normal.z();
    }

    Vector3f computeVertexNormal(W_vertex* vertex) {
      W_edge *edge0 = vertex->edge;
      W_edge *currEdge = edge0;
      Vector3f normal = Vector3f(0.0f, 0.0f, 0.0f);

      do {
        if (currEdge->end == vertex) {
          normal = normal + Vector3f(currEdge->right->nx, currEdge->right->ny, currEdge->right->nz);
          currEdge = currEdge->right_next;
        } else {
          normal = normal + Vector3f(currEdge->left->nx, currEdge->left->ny, currEdge->left->nz);
          currEdge = currEdge->left_next;
        }
      } while (currEdge != edge0);

      normal = normal.normalized();
      return normal;
    }


    bool safeToCollapse(W_edge* collapseEdge) {
      // Ensure that the edge is save to collapse
      // ie. the start and end vertices of the edge have only two mutual adjacent vertices

      std::set<W_vertex*> startAdjacentVertices;
      int mutualAdjacentVertices = 0;

      W_edge *edge0 = collapseEdge->start->edge;
      W_edge *currEdge = edge0;

      do {
        if (currEdge->end == collapseEdge->start) {
          startAdjacentVertices.insert(currEdge->start);
          currEdge = currEdge->right_next;
        } else {
          startAdjacentVertices.insert(currEdge->end);
          currEdge = currEdge->left_next;
        }
      } while (currEdge != edge0);

      edge0 = collapseEdge->end->edge;
      currEdge = edge0;
      do {
        if (currEdge->end == collapseEdge->end) {
          if (startAdjacentVertices.find(currEdge->start) != startAdjacentVertices.end()){
            mutualAdjacentVertices++;
          }
          currEdge = currEdge->right_next;
        } else {
          if (startAdjacentVertices.find(currEdge->end) != startAdjacentVertices.end()){
            mutualAdjacentVertices++;
          }
          currEdge = currEdge->left_next;
        }
      } while (currEdge != edge0);

      if (mutualAdjacentVertices == 2) {
        return true;
      } else {
        return false;
      }
    }

    bool edgeAlreadyInList(W_edge* edgeList, int edgeListSize, W_edge* newEdge) {
      W_vertex* previousVertexNew = newEdge->start;
      W_vertex* nextVertexNew = newEdge->end;
      for (int i = 0; i < edgeListSize; i++) {
        W_vertex* previousVertexOld = edgeList[i].start;
        W_vertex* nextVertexOld = edgeList[i].end;
        if (previousVertexNew == previousVertexOld && nextVertexNew == nextVertexOld) {
          return true;
        }
        if (previousVertexNew == nextVertexOld && nextVertexNew == previousVertexOld) {
          return true;
        }
      }
      return false;
    }

    void decimateMesh() {
      // Randomly choose k edges

      W_edge* randomEdges = (W_edge*)malloc(sizeof(W_edge) * mDecimateK);
      for (int decimateCounter = 0; decimateCounter < mDecimateK; decimateCounter++) {

        std::random_device rd; // obtain a random number from hardware
        std::mt19937 eng(rd()); // seed the generator
        std::uniform_int_distribution<> distr(0, (faces.cols() * 3) - 1); // initialiaze a unifrom distribution with the desired range

        W_edge* randEdge;
        bool reroll = true;
        for(int i = 0; i < mDecimateK; i++){
          reroll = true;
          while (reroll) {
            randEdge = &W_edges[distr(eng)];
            // Check whether the edge has already been chosen or if it has already been deleted
            if (!edgeAlreadyInList(randomEdges, i, randEdge) && randEdge->start != NULL && randEdge->start->edge != NULL) {
              reroll = false;
            }
          }
          randomEdges[i] = *randEdge;
        }

        W_edge* bestEdge = &randomEdges[0];
        W_edge* currentEdge;
        float bestError = std::numeric_limits<float>::max();
        Matrix4f mergedQuadric;
        Matrix4f inverseQuadric;
        Vector4f minVertex;
        Vector4f bestVertex;
        Vector4f minimumVec = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
        for (int i = 0; i < mDecimateK; i++) {
          currentEdge = &randomEdges[i];
          mergedQuadric = currentEdge->start->quadric + currentEdge->end->quadric;

          inverseQuadric.setIdentity();
          inverseQuadric.topRightCorner<3,4>() = mergedQuadric.topRightCorner<3,4>();
          inverseQuadric = inverseQuadric.inverse();

          minVertex = inverseQuadric * minimumVec;

          float error = minVertex.transpose() * mergedQuadric * minVertex;

          if (error < bestError) {
            bestError = error;
            bestEdge = currentEdge;
            bestVertex = minVertex;
          }

        }

        if (safeToCollapse(bestEdge)) {
          collapseEdge(bestEdge, bestVertex);
        } else {
          decimateCounter--;
        }

        // Need to update Winged Edge data structure, faces, vertices, normals, etc.
        // Similar to opening a new file

      }
      free(randomEdges);

      std::cout << "Decimation complete, updating drawn vertices." << '\n';

      setDrawnVertices();

      std::cout << "Drawn vertices updated." << '\n';

    }

    void changeToFaceNormals() {
      int vertexCounter = 0;
      for (int i = 0; i < faces.cols(); i++) {
        if (W_faces[i].edge == NULL) {
          continue;
        }

        normals.col(vertexCounter) << Vector3f(W_faces[i].nx, W_faces[i].ny, W_faces[i].nz).normalized();
        normals.col(vertexCounter + 1) << Vector3f(W_faces[i].nx, W_faces[i].ny, W_faces[i].nz).normalized();
        normals.col(vertexCounter + 2) << Vector3f(W_faces[i].nx, W_faces[i].ny, W_faces[i].nz).normalized();

        vertexCounter += 3;
      }
      mShader.uploadAttrib("vertexNormal_modelspace", normals);
    }

    void changeToSurfaceNormals() {
      int vertexCounter = 0;
      for (int i = 0; i < faces.cols(); i++) {
        if (W_faces[i].edge == NULL) {
          continue;
        }

        W_vertex vertex1 = *(W_faces[i].a);
        W_vertex vertex2 = *(W_faces[i].b);
        W_vertex vertex3 = *(W_faces[i].c);

        normals.col(vertexCounter) << Vector3f(vertex1.nx, vertex1.ny, vertex1.nz).normalized();
        normals.col(vertexCounter + 1) << Vector3f(vertex2.nx, vertex2.ny, vertex2.nz).normalized();
        normals.col(vertexCounter + 2) << Vector3f(vertex3.nx, vertex3.ny, vertex3.nz).normalized();

        vertexCounter += 3;
      }
      mShader.uploadAttrib("vertexNormal_modelspace", normals);
    }

    void setDrawnVertices() {
      normals.resize(3, (faceNumber * 3) + (faceNumber * 3 * 2));
      colors.resize(3, (faceNumber * 3) + (faceNumber * 3 * 2));
      drawnVertices.resize(3, (faceNumber * 3) + (faceNumber * 3 * 2));

      float adjustX = (maxX + minX) / 2.0f;
      float adjustY = (maxY + minY) / 2.0f;
      float adjustZ = (maxZ + minZ) / 2.0f;
      float fit = 2.0f / maxWidth;

      // std::cout << "maxX: " << maxX << " minX: " << minX << '\n';
      // std::cout << "maxY: " << maxY << " minY: " << minY << '\n';
      // std::cout << "maxZ: " << maxZ << " minZ: " << minZ << '\n';
      // std::cout << "X: " << adjustX << '\n';
      // std::cout << "Y: " << adjustY << '\n';
      // std::cout << "Z: " << adjustZ << '\n';

      // Set vertices to draw
      int vertexCounter = 0;
      for (int i = 0; i < faces.cols(); i++) {
        if (W_faces[i].edge == NULL) {
          continue;
        }

        W_vertex vertex1 = *(W_faces[i].a);
        W_vertex vertex2 = *(W_faces[i].b);
        W_vertex vertex3 = *(W_faces[i].c);

        drawnVertices.col(vertexCounter) << Vector3f(vertex1.x - adjustX, vertex1.y - adjustY, vertex1.z - adjustZ) * fit;
        drawnVertices.col(vertexCounter + 1) << Vector3f(vertex2.x - adjustX, vertex2.y - adjustY, vertex2.z - adjustZ) * fit;
        drawnVertices.col(vertexCounter + 2) << Vector3f(vertex3.x - adjustX, vertex3.y - adjustY, vertex3.z - adjustZ) * fit;

        if (meshShading == FLAT) {
          normals.col(vertexCounter) << Vector3f(W_faces[i].nx, W_faces[i].ny, W_faces[i].nz).normalized();
          normals.col(vertexCounter + 1) << Vector3f(W_faces[i].nx, W_faces[i].ny, W_faces[i].nz).normalized();
          normals.col(vertexCounter + 2) << Vector3f(W_faces[i].nx, W_faces[i].ny, W_faces[i].nz).normalized();
        } else {
          normals.col(vertexCounter) << Vector3f(vertex1.nx, vertex1.ny, vertex1.nz).normalized();
          normals.col(vertexCounter + 1) << Vector3f(vertex2.nx, vertex2.ny, vertex2.nz).normalized();
          normals.col(vertexCounter + 2) << Vector3f(vertex3.nx, vertex3.ny, vertex3.nz).normalized();
        }

        colors.col(vertexCounter) << 1, 0, 0;
        colors.col(vertexCounter + 1) << 1, 0, 0;
        colors.col(vertexCounter + 2) << 1, 0, 0;

        vertexCounter += 3;
      }

      if (vertexCounter != faceNumber * 3) {
        std::cout << "Not the right number of vertices to draw for faces!" << '\n';
      }

      for (int i = 0; i < faces.cols(); i++) {
        if (W_faces[i].edge == NULL) {
          continue;
        }

        W_vertex vertex1 = *(W_faces[i].a);
        W_vertex vertex2 = *(W_faces[i].b);
        W_vertex vertex3 = *(W_faces[i].c);

        float lineOffset = 0.001f;

        drawnVertices.col(vertexCounter) << Vector3f(vertex1.x - adjustX, vertex1.y - adjustY, vertex1.z - adjustZ) * fit + lineOffset * Vector3f(vertex1.nx, vertex1.ny, vertex1.nz).normalized();
        drawnVertices.col(vertexCounter + 1) << Vector3f(vertex2.x - adjustX, vertex2.y - adjustY, vertex2.z - adjustZ) * fit + lineOffset * Vector3f(vertex2.nx, vertex2.ny, vertex2.nz).normalized();

        drawnVertices.col(vertexCounter + 2) << Vector3f(vertex2.x - adjustX, vertex2.y - adjustY, vertex2.z - adjustZ) * fit + lineOffset * Vector3f(vertex2.nx, vertex2.ny, vertex2.nz).normalized();
        drawnVertices.col(vertexCounter + 3) << Vector3f(vertex3.x - adjustX, vertex3.y - adjustY, vertex3.z - adjustZ) * fit + lineOffset * Vector3f(vertex3.nx, vertex3.ny, vertex3.nz).normalized();

        drawnVertices.col(vertexCounter + 4) << Vector3f(vertex3.x - adjustX, vertex3.y - adjustY, vertex3.z - adjustZ) * fit + lineOffset * Vector3f(vertex3.nx, vertex3.ny, vertex3.nz).normalized();
        drawnVertices.col(vertexCounter + 5) << Vector3f(vertex1.x - adjustX, vertex1.y - adjustY, vertex1.z - adjustZ) * fit + lineOffset * Vector3f(vertex1.nx, vertex1.ny, vertex1.nz).normalized();

        normals.col(vertexCounter) << Vector3f(vertex1.nx, vertex1.ny, vertex1.nz).normalized();
        normals.col(vertexCounter + 1) << Vector3f(vertex2.nx, vertex2.ny, vertex2.nz).normalized();

        normals.col(vertexCounter + 2) << Vector3f(vertex2.nx, vertex2.ny, vertex2.nz).normalized();
        normals.col(vertexCounter + 3) << Vector3f(vertex3.nx, vertex3.ny, vertex3.nz).normalized();

        normals.col(vertexCounter + 4) << Vector3f(vertex1.nx, vertex1.ny, vertex1.nz).normalized();
        normals.col(vertexCounter + 5) << Vector3f(vertex3.nx, vertex3.ny, vertex3.nz).normalized();

        colors.col(vertexCounter) << 0, 0, 0;
        colors.col(vertexCounter + 1) << 0, 0, 0;
        colors.col(vertexCounter + 2) << 0, 0, 0;
        colors.col(vertexCounter + 3) << 0, 0, 0;
        colors.col(vertexCounter + 4) << 0, 0, 0;
        colors.col(vertexCounter + 5) << 0, 0, 0;

        vertexCounter += 6;
      }

      if (vertexCounter != (faceNumber * 3) + (faceNumber * 3 * 2)) {
        std::cout << "Not the right number of vertices to draw for lines!" << '\n';
      }

      mShader.uploadAttrib("vertexPosition_modelspace", drawnVertices);
      mShader.uploadAttrib("color", colors);
    	mShader.uploadAttrib("vertexNormal_modelspace", normals);

    }



    //OpenGL calls this method constantly to update the screen.
    virtual void drawGL() override {
      using namespace nanogui;

      mShader.bind();


      Matrix4f rotation;
      rotation.setIdentity();
      rotation.topLeftCorner<3,3>() = Eigen::Matrix3f(Eigen::AngleAxisf(mRotation[2], Vector3f::UnitX()) *
                                                      Eigen::AngleAxisf(mRotation[1], Vector3f::UnitY()) *
                                                      Eigen::AngleAxisf(mRotation[0], Vector3f::UnitZ())) * 0.25f;

      Matrix4f translation;
      translation.setIdentity();
      translation.topRightCorner<3,1>() = mTranslation;// - Vector3f((maxX - minX) / 2.0, (maxY - minY) / 2.0, (maxZ - minZ) / 2.0);

      Matrix4f zoom;
      zoom.setIdentity();
      zoom(0,0) = mZoom;
      zoom(1,1) = mZoom;
      zoom(2,2) = mZoom;


      Matrix4f V;
      V.setIdentity();
      // V = V * translation * rotation;
      mShader.setUniform("V", V);

    	Matrix4f M;
    	M.setIdentity();
      // M = M * fit * zoom;
    	mShader.setUniform("M", M);

      Matrix4f mvp;
      mvp.setIdentity();
      mvp = ortho(-1.0f, 1.0f, -1.0f, 1.0f, -200.0f, 200.0f);
      mvp = mvp * translation * zoom * rotation;
      mShader.setUniform("MVP", mvp);

      mShader.setUniform("LightPosition_worldspace", Vector3f(-12,2,-8));


      glEnable(GL_DEPTH_TEST);

      glLineWidth(2);

      if (meshShading != WIREFRAME) {
        // mShader.drawArray(GL_TRIANGLES, 0, faces.cols() * 3);
        mShader.drawArray(GL_TRIANGLES, 0, faceNumber * 3);
      }
      if (meshShading == WIREFRAME || meshShading == MESH_EDGES) {
    	   // mShader.drawArray(GL_LINES, faces.cols() * 3, faces.cols() * 3 * 2);
         mShader.drawArray(GL_LINES, faceNumber * 3, faceNumber * 3 * 2);
      }

      glDisable(GL_DEPTH_TEST);
    }

  //Instantiation of the variables that can be acessed outside of this class to interact with the interface
  private:
    int vertexNumber = 0;
    int faceNumber = 0;

    MatrixXf vertices = MatrixXf(3, 3);
    MatrixXf normals = MatrixXf(3, 3);
    MatrixXu faces = MatrixXu(3, 3);
    MatrixXf colors = MatrixXf(3, 3);
    MatrixXf drawnVertices = MatrixXf(3, 3);

    W_edge *W_edges;
    map<pair<int, int>, W_edge*> W_edges_map;
    W_vertex *W_vertices;
    W_face *W_faces;

    nanogui::GLShader mShader;
    Vector3f mRotation = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f mTranslation =  Vector3f(0.0f, 0.0f, 0.0f);
    float mZoom = 1.0f;
    int mDecimateK = 30;
    float maxWidth = 0.0f;

    float maxX = std::numeric_limits<float>::min();
    float minX = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();
    float minY = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::min();
    float minZ = std::numeric_limits<float>::max();

    meshShading_t meshShading = FLAT;
};

float radiansX = 0.0f;
float radiansY = 0.0f;
float radiansZ = 0.0f;
float translationX = 0.0f;
float translationY = 0.0f;
float translationZ = 0.0f;
float zoom = 0.0f;
int kValue = 30;

class ExampleApplication : public nanogui::Screen {
public:
    ExampleApplication() : nanogui::Screen(Eigen::Vector2i(900, 630), "NanoGUI Cube and Menus", false) {
        using namespace nanogui;

	//OpenGL canvas demonstration

	//First, we need to create a window context in which we will render both the interface and OpenGL canvas
        Window *window = new Window(this, "GLCanvas Demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

	//OpenGL canvas initialization, we can control the background color and also its size
        mCanvas = new MyGLCanvas(window);
        mCanvas->setBackgroundColor({100, 100, 100, 255});
        mCanvas->setSize({400, 400});

	//This is how we add widgets, in this case, they are connected to the same window as the OpenGL canvas
        Widget *tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 5));

	//then we start adding elements one by one as shown below
        Button *b0 = new Button(tools, "Random Color");
        b0->setCallback([this]() { mCanvas->setBackgroundColor(Vector4i(rand() % 256, rand() % 256, rand() % 256, 255)); });

        Button *b1 = new Button(tools, "Random Rotation");
        b1->setCallback([this]() { mCanvas->setRotation(nanogui::Vector3f((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f)); });

        //widgets demonstration
        nanogui::GLShader mShader;

	//Then, we can create another window and insert other widgets into it
	Window *anotherWindow = new Window(this, "Basic widgets");
        anotherWindow->setPosition(Vector2i(500, 15));
        anotherWindow->setLayout(new GroupLayout());




  //This is how to implement a combo box, which is important in A1
  new Label(anotherWindow, "Mesh Shading", "sans-bold");
  ComboBox *combo = new ComboBox(anotherWindow, { "Flat Shaded", "Smooth Shaded", "Wireframe", "Shaded with Edges"} );
  combo->setCallback([&](int value) {
    cout << "Combo box selected: " << value << endl;
    mCanvas->setMeshShading(static_cast<meshShading_t>(value));
  });

  new Label(anotherWindow, "X Rotation", "sans-bold");

	Widget *panelRotX = new Widget(anotherWindow);
        panelRotX->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 0));
	Slider *rotSliderX = new Slider(panelRotX);
        rotSliderX->setValue(0.5f);
        rotSliderX->setFixedWidth(150);
        rotSliderX->setCallback([&](float value) {
      radiansX = (value - 0.5f)*2*2*M_PI;
	    mCanvas->setRotation(nanogui::Vector3f(radiansX, radiansY, radiansZ));
        });

    new Label(anotherWindow, "Y Rotation", "sans-bold");

    Widget *panelRotY = new Widget(anotherWindow);
        panelRotY->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 0));
	Slider *rotSliderY = new Slider(panelRotY);
        rotSliderY->setValue(0.5f);
        rotSliderY->setFixedWidth(150);
        rotSliderY->setCallback([&](float value) {
	    radiansY = (value - 0.5f)*2*2*M_PI;
	    mCanvas->setRotation(nanogui::Vector3f(radiansX, radiansY, radiansZ));
        });

    new Label(anotherWindow, "Z Rotation", "sans-bold");

    Widget *panelRotZ = new Widget(anotherWindow);
        panelRotZ->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 0));
    Slider *rotSliderZ = new Slider(panelRotZ);
          rotSliderZ->setValue(0.5f);
          rotSliderZ->setFixedWidth(150);
          rotSliderZ->setCallback([&](float value) {
        radiansZ = (value - 0.5f)*2*2*M_PI;
        mCanvas->setRotation(nanogui::Vector3f(radiansX, radiansY, radiansZ));
          });

    new Label(anotherWindow, "X Translation", "sans-bold");
    Widget *panelTransX = new Widget(anotherWindow);
        panelTransX->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 0));
    Slider *transSliderX = new Slider(panelTransX);
    transSliderX->setValue(0.5f);
    transSliderX->setFixedWidth(150);
    transSliderX->setCallback([&](float value) {
      translationX = (value - 0.5f) * 3;
      mCanvas->setTranslation(nanogui::Vector3f(translationX, translationY, translationZ));
    });

    new Label(anotherWindow, "Y Translation", "sans-bold");
    Widget *panelTransY = new Widget(anotherWindow);
        panelTransY->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 0));
    Slider *transSliderY = new Slider(panelTransY);
    transSliderY->setValue(0.5f);
    transSliderY->setFixedWidth(150);
    transSliderY->setCallback([&](float value) {
      translationY = (value - 0.5f) * 3;
      mCanvas->setTranslation(nanogui::Vector3f(translationX, translationY, translationZ));
    });

    new Label(anotherWindow, "Z Translation", "sans-bold");
    Widget *panelTransZ = new Widget(anotherWindow);
       panelTransZ->setLayout(new BoxLayout(Orientation::Horizontal,
                                      Alignment::Middle, 0, 0));
    Slider *transSliderZ = new Slider(panelTransZ);
    transSliderZ->setValue(0.5f);
    transSliderZ->setFixedWidth(150);
    transSliderZ->setCallback([&](float value) {
      translationZ = (value - 0.5f) * 3;
      mCanvas->setTranslation(nanogui::Vector3f(translationX, translationY, translationZ));
    });

    new Label(anotherWindow, "Zoom", "sans-bold");
    Widget *panelZoom = new Widget(anotherWindow);
        panelZoom->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 0));
    Slider *zoomSliderZ = new Slider(panelZoom);
    zoomSliderZ->setValue(0.5f);
    zoomSliderZ->setFixedWidth(150);
    zoomSliderZ->setCallback([&](float value) {
      // zoom = (value - 0.5) * 5.0f;
      zoom = pow(50.0f, (value - 0.5) * 2);
      mCanvas->setZoom(zoom);
    });

	//Message dialog demonstration, it should be pretty straightforward
        Button *b = new Button(tools, "Info");

	//Here is how you can get the string that represents file paths both for opening and for saving.
	//you need to implement the rest of the parser logic.
        new Label(anotherWindow, "File dialog", "sans-bold");
        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        b = new Button(tools, "Open");
        b->setCallback([&] {
          string objFile = file_dialog({ {"obj", "Object file"}, {"txt", "Text file"} }, false);
          cout << "File dialog result: " << objFile << endl;
          mCanvas->openOBJFile(objFile);
        });
        b = new Button(tools, "Save");
        b->setCallback([&] {
          string objFile = file_dialog({ {"obj", "Object file"}, {"txt", "Text file"} }, true);
          cout << "File dialog result: " << objFile << endl;
          mCanvas->saveOBJFile(objFile);
        });

        // Demonstrates how a button called "New Mesh" can update the positions matrix.
        // This is just a demonstration, you actually need to bind mesh updates with the open file interface
        new Label(anotherWindow, "Quit", "sans-bold");
        Button *quitButton = new Button(anotherWindow, "Quit");
        quitButton->setCallback([&] {
          nanogui::leave();
        });

      Window *advancedWindow = new Window(this, "Advanced widgets");
            advancedWindow->setPosition(Vector2i(750, 15));
            advancedWindow->setLayout(new GroupLayout());

        new Label(advancedWindow, "Decimation", "sans-bold");

        TextBox *decimateKBox = new TextBox(advancedWindow, "30");
        decimateKBox->setEditable(true);
        decimateKBox->setFormat("^\\d+$");
        decimateKBox->setCallback([](const std::string &value) {
          kValue = std::stoi(value);
          return true;
        });

        Button *decimateButton = new Button(advancedWindow, "Decimate");
        decimateButton->setCallback([&] {
          std::cout << "Decimating" << "\n";
          mCanvas->setDecimateK(kValue);
          mCanvas->decimateMesh();
        });

	//Method to assemble the interface defined before it is called
        performLayout();
    }

    //This is how you capture mouse events in the screen. If you want to implement the arcball instead of using
    //sliders, then you need to map the right click drag motions to suitable rotation matrices
    virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Vector2i &rel, int button, int modifiers) override {
        if (button == GLFW_MOUSE_BUTTON_3 ) {
	    //Get right click drag mouse event, print x and y coordinates only if right button pressed
	    cout << p.x() << "     " << p.y() << "\n";
            return true;
        }
        return false;
    }

    virtual void drawContents() override {
        // ... put your rotation code here if you use dragging the mouse, updating either your model points, the mvp matrix or the V matrix, depending on the approach used
    }

    virtual void draw(NVGcontext *ctx) {

        /* Draw the user interface */
        Screen::draw(ctx);
    }


private:
    MyGLCanvas *mCanvas;
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

            /* scoped variables */ {
            nanogui::ref<ExampleApplication> app = new ExampleApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}
