#include "mesh_transforms.h"
#include "utils.h"

void mergeMesh(const ntk::Mesh& a, ntk::Mesh&b) {
  bool useNormals = a.normals.size() > 0 && b.normals.size() > 0;
  bool useCoords = a.texcoords.size() > 0 && b.texcoords.size() > 0;
  for (int i = 0; i < a.vertices.size(); i++) {
    b.vertices.push_back(a.vertices[i]);
    b.colors.push_back(a.colors[i]);
    if (useNormals)
      b.normals.push_back(a.normals[i]);
    if (useCoords)
      b.texcoords.push_back(a.texcoords[i]);
  }
  for (int i = 0; i < a.faces.size(); i++) {
    b.faces.push_back(a.faces[i]);
  }
}

Undulate :: Undulate(cv::Point3f t, cv::Point3f f) : travel(t), flow(f) {
  start_timer();
}

void Undulate :: apply(ntk::Mesh& mesh) {
  float time = get_time();
  for (int i = 0; i < mesh.vertices.size(); i++)
    mesh.vertices[i] += flow * sin(time + mesh.vertices[i].dot(travel));
}

Undulate2 :: Undulate2(cv::Point3f o, cv::Point3f t, int sz) : origin(o), travel(t) {
  for (int i = 0; i < sz; i++) {
    history.push_back(cv::Point3f(0, 0, 0));
  }
}
void Undulate2 :: apply(ntk::Mesh& mesh) {
  for (int i = 0; i < mesh.vertices.size(); i++) {
    int ix = ntk::math::rnd((mesh.vertices[i] - origin).dot(travel));
    if (ix >= 0 && ix < history.size()) {
      mesh.vertices[i] += history[ix];
    }
  }
}
 

Inflate :: Inflate(cv::Point3f o, cv::Point3f b, double t)
   : origin(o), bias(b), thresh(t) {
  start_timer();
}
 
//TODO
void Inflate :: apply(ntk::Mesh& mesh) {
  float time = get_time();
  for (int i = 0; i < mesh.vertices.size(); i++) {
  /*
    cv::Point3f delta = mesh.vertices[i] - origin;
    delta.x *= bias.x ; delta.y *= bias.y; delta.z *= bias.z;
    float invcube = pow(delta.dot(delta), 0.5 * -3);
    invcube *= (1.5 + sin(time));
    if (invcube > thresh)
      */
    cv::Point3f delta = mesh.vertices[i] - origin;
    delta *= 0.1;
    float invcube = 0.3 * pow(delta.dot(delta), -0.3333);
    printf("%f\n", invcube);
    if (invcube > 0.3)
      mesh.vertices[i] += delta * invcube;
  }
}
 
ColorSpheres :: ColorSpheres(float life, float w) :
 origins(0), colors(0), alphas(0), start_times(0),
 lifetime(life), width(w) {
  start_timer();
}

void ColorSpheres :: add(cv::Point3f origin, cv::Point3f color, float alpha) {
  origins.push_back(origin);
  colors.push_back(color);
  alphas.push_back(alpha);
  start_times.push_back(get_time());
}

void ColorSpheres :: apply(ntk::Mesh& mesh) {
  printf("\n%i\n", origins.size());
  float time = get_time();
  for (int i = 0; i < origins.size(); i++) {
    if (time - start_times[i] > lifetime) {
      start_times.erase(i + start_times.begin());
      origins.erase    (i + origins.begin());
      alphas.erase     (i + alphas.begin());
      colors.erase     (i + colors.begin());
      i--;
    }
  }
  for (int i = 0; i < origins.size(); i++) {
    cv::Point3f origin = origins[i];
    float alpha = alphas[i], start_time = start_times[i];
    float scaling = alpha * 255;
    cv::Vec3b color(ntk::math::rnd(colors[i].x * scaling),
      ntk::math::rnd(colors[i].y * scaling),
      ntk::math::rnd(colors[i].z * scaling));
    color *= alpha;
    for (int j = 0; j < mesh.vertices.size(); j++) {
      cv::Point3f delta = mesh.vertices[j] - origin;
      float dist = delta.dot(delta);
      //printf("%f\n", fabs(time - start_time - dist));
      if (fabs(0.2 * (time - start_time) - dist) < width) {
        mesh.colors[j] *= (1-alpha);
        mesh.colors[j] += color;
      }
    }
  }
}

TransformGroup :: TransformGroup() : transforms() {}
void TransformGroup :: push(ITransform* t) { transforms.push_back(t); }
void TransformGroup :: apply(ntk::Mesh& mesh) {
  for (int i = 0; i < transforms.size(); i++) transforms[i]->apply(mesh);
}

