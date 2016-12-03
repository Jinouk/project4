#include <cstdlib> 
#include <cstdio> 
#include <cmath> 
#include <fstream> 
#include <vector> 
#include <iostream> 
#include <string>
#include <sstream>

#if defined __linux__ || defined __APPLE__ 
// "Compiled for Linux
#else 
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793 
#define INFINITY 1e8 
#endif 

using namespace std;

class Vec {
public:
  float x, y, z;
  Vec() : x(0), y(0), z(0) {}
  Vec(float xx) : x(xx), y(xx), z(xx) {}
  Vec(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
  Vec operator - (const Vec &v) const {return Vec(x-v.x, y-v.y, z-v.z);}
  Vec operator + (const Vec &v) const {return Vec(x+v.x, y+v.y, z+v.z);}
  Vec operator / (float d) {return Vec(x/d, y/d, z/d);}
  Vec operator * (float d) {return Vec(x*d, y*d, z*d);}
  Vec operator * (const Vec &v) const {return Vec(x* v.x, y* v.y, z* v.z);}
  Vec operator += (const Vec &v) {x += v.x, y += v.y, z += v.z; return *this;}
  bool operator == (const Vec &v) {return (x == v.x) && (y == v.y) && (z == v.z);}
  float dot(const Vec &v) const { return x * v.x + y * v.y + z * v.z; }
  void normalize() {
    float mg = sqrt(x * x + y * y + z * z); 
    x = x / mg;
    y = y / mg;
    z = z / mg;
  }
  void print() {
    cout << " < x: " << x << " y: " << y << " z: " << z << " >" << endl;
  }
};

Vec crossProduct(const Vec &v1, const Vec &v2) {
  return Vec(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

class Sphere {
public:
  Vec center;
  Vec color;
  float radius;
  float reflectivity;
  string texture;

  Sphere(){
    center.x = 0, center.y = 0, center.z= 0;
    color.x = 0, color.y = 0, color.z = 0;
    radius = 0;
    reflectivity = 0;
    texture = "";
  }

  bool intersect(const Vec &origin, const Vec &dir, float &t0, float &t1) const {
    Vec co = center - origin; 
    float tca = co.dot(dir); 
    if (tca < 0) return false; 
    float d2 = co.dot(co) - tca * tca; 
    if (d2 > radius * radius) return false; 
    float thc = sqrt(radius * radius - d2); 
    t0 = tca - thc; 
    t1 = tca + thc; 

    return true;  
  }
};

class Plane {
public:
  Vec center;
  Vec color;
  Vec normal;
  Vec headup;
  float width, height;
  float reflectivity;
  string texture;

  Plane(){
    center.x = 0, center.y = 0, center.z= 0;
    color.x = 0, color.y = 0, color.z = 0;
    normal.x = 0, normal.y = 0, normal.z = 0;
    headup.x = 0, headup.y = 0, headup.z = 0;
    width = 0, height = 0;
    reflectivity = 0;
    texture = "";
  }

  bool intersect(const Vec &origin, const Vec &dir, float &t) const {
    //Vec p0 = center - origin;
    //cout << dir.x << " " << dir.y << " " <<dir.z << endl;
    float denom = normal.dot(dir);
    //cout << abs(denom) << endl; 
    if (abs(denom) > 1e-6) { 
        Vec co = center - origin; 
        t = co.dot(normal) / denom; 
        //cout << t << endl;

        Vec p = origin + dir * t;
        
        Vec xAxis = crossProduct(headup, normal);
        xAxis.normalize();
        Vec yAxis = crossProduct(normal, xAxis);
        yAxis.normalize();
        
        Vec pToc = p - center;
        ////pToc.normalize();

        float x1, y1;
        //xAxis.print();
        //yAxis.print();
        // Normal 0 1 0
        if(normal.x == 0 && normal.y == 1 && normal.z == 0){
          y1 = (xAxis.x * pToc.z - pToc.x * xAxis.z) / (xAxis.x * yAxis.z - yAxis.x * xAxis.z);
          x1 = (pToc.x - y1 * yAxis.x) / xAxis.x;

          //cout << x1 << " " << y1 << endl;
        }

        else if(normal.x == 1 && normal.y == 0 && normal.z == 0) {

        }

        // General 
        else {
          y1 = (xAxis.x * pToc.y - pToc.x * xAxis.y) / (xAxis.x * yAxis.y - yAxis.x * xAxis.y);
          x1 = (pToc.x - y1 * yAxis.x) / xAxis.x;
        }

        //cout << x1  << " " << y1 << endl;
        //cout << center.x + width/2 << " " << width << endl;
        if(x1 < (center.x + width/2) && x1 > (center.x - width/2))
          if(y1 < (center.y+ height / 2) && y1 > (center.y - height/2))
            return (t >= 0); 
    } 
 
    return false; 
  }
};

class Light {
public:
  Vec location;
  Vec color;

  Light() {
    location.x = 0, location.y = 0, location.z = 0;
    color.x = 0, color.y = 0, color.z = 0;
  }
};

float mix(const float &a, const float &b, const float &mix) 
{ 
    return b * mix + a * (1 - mix); 
} 

Vec trace(const Vec &origin,
          const Vec &dir,
          const vector<Sphere> &spheres,
          const vector<Plane> &planes,
          const vector<Light> &lights,
          const int depth
        ) {
  
  float tnear = INFINITY;
  const Sphere* sphere = NULL;
  const Plane* plane = NULL;

  for(int i = 0; i < spheres.size(); ++i) {
    float t0 = INFINITY, t1 = INFINITY;
    if(spheres[i].intersect(origin, dir, t0, t1)){
      if(t0 < 0) t0 = t1;
      if(t0 < tnear) {
        tnear = t0;
        sphere = &spheres[i];
      }
    }
  }

  
  for(int i = 0; i< planes.size(); ++i){
    float t = INFINITY;
    if(planes[i].intersect(origin,dir,t)){
      if(t < tnear) {
        tnear = t;
        plane = &planes[i];
        sphere = NULL;
      }
    }
  }



  if(!sphere && !plane) return Vec(0);
  
  Vec surfaceColor = 0;
  Vec pi = origin + dir * tnear;
  if(sphere){
    //cout << "sphere" << endl;
    Vec normal = pi - sphere->center;
    normal.normalize();

    float bias = 1e-4;
    //cout << sphere->reflectivity << endl;
    if(sphere->reflectivity > 0 && depth < 5){
      Vec refl_dir = dir - normal * 2 * dir.dot(normal);
      refl_dir.normalize();
      Vec reflection = trace(pi + normal * bias, refl_dir, spheres, planes, lights, depth + 1);
    
      // Color_final = surface_color + lamda * refl_color
      if(reflection == Vec(0)){

        //surfaceColor = sphere->color;
        
        for(int i = 0; i < lights.size(); ++i) {
          Vec transmission = 1;
          Vec lightDirection = lights[i].location - pi;
          lightDirection.normalize();

          for(int j = 0; j < spheres.size(); ++j){
            float t0, t1; 
            if(spheres[j].intersect(pi + normal * bias, lightDirection, t0, t1)) {
              transmission = 0;
              break;
            }
          }

          surfaceColor += sphere->color * transmission * max(float(0), normal.dot(lightDirection)) * lights[i].color;
          //surfaceColor.print();
        }
      }
      else{
        for(int i = 0; i < lights.size(); ++i) {
          Vec transmission = 1;
          Vec lightDirection = lights[i].location - pi;
          lightDirection.normalize();

          for(int j = 0; j < spheres.size(); ++j){
            float t0, t1; 
            if(spheres[j].intersect(pi + normal * bias, lightDirection, t0, t1)) {
              transmission = 0;
              break;
            }
          }

          surfaceColor += reflection * sphere->reflectivity + sphere->color * transmission * max(float(0), normal.dot(lightDirection)) * lights[i].color;
          //surfaceColor.print();
        }

        //surfaceColor = reflection * sphere->reflectivity + sphere->color;
      }
    }
    else{
      for(int i = 0; i < lights.size(); ++i) {
        Vec transmission = 1;
        Vec lightDirection = lights[i].location - pi;
        lightDirection.normalize();

        for(int j = 0; j < spheres.size(); ++j){
          float t0, t1; 
          if(spheres[j].intersect(pi + normal * bias, lightDirection, t0, t1)) {
            transmission = 0;
            break;
          }
        }

        surfaceColor += sphere->color * transmission * max(float(0), normal.dot(lightDirection)) * lights[i].color;
      }
    }
  }
  if(plane){
    //cout << "plane" << endl;
    Vec normal = plane->normal;
    //cout << "normal" << endl;
    normal.normalize();

    // Plane reflectivity 
    //cout << plane->reflectivity << endl;
    if(plane->reflectivity > 0 && depth < 5){
      Vec refl_dir = dir - normal * 2 * dir.dot(normal);
      refl_dir.normalize();
      //cout << "reflectivity" << endl;
      Vec reflection = trace(pi, refl_dir, spheres, planes, lights, depth + 1);
      
      if(reflection == Vec(0)){
        for(int i = 0; i < lights.size(); ++i) {
          Vec transmission = 1;
          Vec lightDirection = lights[i].location - pi;
          lightDirection.normalize();

          for(int j = 0; j < spheres.size(); ++j) {
            float t0, t1;
            if(spheres[j].intersect(pi, lightDirection, t0, t1)) {
              transmission = 0;
              break;
            }
          }

          surfaceColor += plane->color * transmission * max(float(0), normal.dot(lightDirection)) * lights[i].color;
        }
      }
      else {
        for(int i = 0; i < lights.size(); ++i) {
          Vec transmission = 1;
          Vec lightDirection = lights[i].location - pi;
          lightDirection.normalize();

          for(int j = 0; j < spheres.size(); ++j) {
            float t0, t1;
            if(spheres[j].intersect(pi, lightDirection, t0, t1)) {
              transmission = 0;
              break;
            }
          }

          surfaceColor += /*reflection * plane->reflectivity + */plane->color * transmission * max(float(0), normal.dot(lightDirection)) * lights[i].color;
        }
      }

    }
    else{
      for(int i = 0; i < lights.size(); ++i) {
        Vec transmission = 1;
        Vec lightDirection = lights[i].location - pi;
        lightDirection.normalize();

        for(int j = 0; j < spheres.size(); ++j) {
          float t0, t1;
          if(spheres[j].intersect(pi, lightDirection, t0, t1)) {
            transmission = 0;
            break;
          }
        }

        surfaceColor += plane->color * transmission * max(float(0), normal.dot(lightDirection)) * lights[i].color;
      }
    }
  }


  return surfaceColor;
}

void render(int size,
            const string &of,
            const vector<Sphere> &spheres,
            const vector<Plane> &planes,
            const vector<Light> &lights) {

  Vec *image = new Vec[size * size], *pixel = image;
  float invWidth = 1 / float(size), invHeight = 1 / float(size);
  float fov = 45, aspectratio = 1;
  float angle = tan(M_PI * 0.5 * fov / 180.0);

  for (int y = 0; y < size; ++y){
    for (int x = 0; x < size; ++x, ++pixel) {
      float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio; 
      float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle; 
      Vec raydir(xx, yy, -1); 
      raydir.normalize(); 
      *pixel = trace(Vec(0,0,0), raydir, spheres, planes, lights, 0); 
    }
  }

  ofstream image_file(of); 
  image_file << "P6\n" << size << " " << size << "\n255\n"; 
  for (unsigned i = 0; i < size * size; ++i) { 
      image_file << (unsigned char)(min(float(1), image[i].x) * 255) << 
             (unsigned char)(min(float(1), image[i].y) * 255) << 
             (unsigned char)(min(float(1), image[i].z) * 255); 
  } 
  image_file.close(); 
  delete [] image;
}


int main (int argc, char ** argv) {

  if(argc != 3){
    cout << "invalid input" << endl;
    return -1;
  }

  string of = argv[2];

  vector<Plane> planes;
  vector<Sphere> spheres;
  vector<Light> lights;
  int imageSize;                  //image width and height

  string line;
  ifstream descriptionFile;
  descriptionFile.open(argv[1]);
  if(descriptionFile.is_open()){
    while(getline(descriptionFile, line)){
      
      string object;
      istringstream iss(line);
      iss >> object;

      // Set image width and height
      if(object.compare("camera") == 0){
        iss >> imageSize;
      }
      // Read sphere specification
      if(object.compare("sphere") == 0){
        
        Sphere s;
        while(getline(descriptionFile, line)){
          if(line.compare("")==0){
            spheres.push_back(s);
            break;
          }
          string detail;
          istringstream iss2(line);

          iss2 >> detail;

          if(detail.compare("dimension") == 0){
            float radius;
            iss2 >> radius;
            s.radius = radius;
          } 
          else if(detail.compare("center") == 0){
            float x, y, z;
            iss2 >> x >> y >> z;
            s.center.x = x;
            s.center.y = y;
            s.center.z = z;
          }
          else if(detail.compare("color") == 0){
            float r, g, b;
            iss2 >> r >> g >> b;
            s.color.x = r;
            s.color.y = g;
            s.color.z = b;

          }
          else if(detail.compare("reflectivity") == 0){
            float r;
            iss2 >> r;
            s.reflectivity = r;
            cout << r << endl;
          }
          else{
            string t;
            iss2 >> t;
            s.texture.assign(t);
          }     
        }
      }

      // Read plane specification
      if(object.compare("plane") == 0){

        Plane p;
        while(getline(descriptionFile, line)){
          if(line.compare("")==0){
            planes.push_back(p);
            break;
          }

          string detail;
          istringstream iss2(line);

          iss2 >> detail;

          if(detail.compare("dimension") == 0){
            float w, h;
            iss2 >> w >> h;
            p.width = w;
            p.height = h;
          } 
          else if(detail.compare("center") == 0){
            float x, y, z;
            iss2 >> x >> y >> z;
            p.center.x = x;
            p.center.y = y;
            p.center.z = z;
          }
          else if(detail.compare("color") == 0){
            float r, g, b;
            iss2 >> r >> g >> b;
            p.color.x = r;
            p.color.y = g;
            p.color.z = b;

          }
          else if(detail.compare("normal") == 0){
            float x, y, z;
            iss2 >> x >> y >> z;
            p.normal.x = x;
            p.normal.y = y;
            p.normal.z = z;

          }
          else if(detail.compare("headup") == 0){
            float x, y, z;
            iss2 >> x >> y >> z;
            p.headup.x = x;
            p.headup.y = y;
            p.headup.z = z;

          }

          else if(detail.compare("reflectivity") == 0){
            float r;
            iss2 >> r;
            p.reflectivity = r;
          }
          else{
            string t;
            iss2 >> t;
            p.texture.assign(t);
          } 
        }
      }

      // Read light specification
      if(object.compare("light") == 0){
        
        Light l;
        string detail;
        float x, y, z; // or r, g, b
        getline(descriptionFile, line);
        istringstream iss2(line);

        iss2 >> detail >> x >> y >> z;

        l.location.x = x;
        l.location.y = y;
        l.location.z = z;
        getline(descriptionFile, line);
        istringstream iss3(line);

        iss3 >> detail >> x >> y >> z;

        l.color.x = x;
        l.color.y = y;
        l.color.z = z;

        lights.push_back(l);
      }
    }
  }

  descriptionFile.close();

  render(imageSize, of, spheres, planes, lights);

  return 0;
}