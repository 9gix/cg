//============================================================
// STUDENT NAME: <your name>
// MATRIC NO.  : <matric no.>
// NUS EMAIL   : <your NUS email address>
// COMMENTS TO GRADER:
// <comments to grader, if any>
//
// ============================================================
//
// FILE: Sphere.cpp



#include <cmath>
#include <algorithm>
#include "Sphere.h"

using namespace std;



bool Sphere::hit( const Ray &r, double tmin, double tmax, SurfaceHitRecord &rec ) const 
{
	//***********************************************
    //*********** WRITE YOUR CODE HERE **************
    //***********************************************

    Vector3d rayOrigin = r.origin() - center; // origin with respect to the sphere
    double a = 1;
    double b = 2 * dot(r.direction(), rayOrigin);
    double c = dot(rayOrigin, rayOrigin) - pow(radius, 2);
    double discriminant = pow(b, 2) - 4 * a * c;
    if (discriminant < 0) {
        return false;
    }

    double t1 = (-b + sqrt(discriminant)) / (2 * a);
    double t2 = (-b - sqrt(discriminant)) / (2 * a);
    double t0 = min(t1, t2);

    if (t0 < tmin || t0 > tmax) {
        return false;
    }

    rec.mat_ptr = matp;
    rec.p = r.pointAtParam(t0);
    rec.normal = (rec.p - center) / (rec.p - center).length();
    rec.t = t0;
    return true;
}




bool Sphere::shadowHit( const Ray &r, double tmin, double tmax ) const 
{
	//***********************************************
    //*********** WRITE YOUR CODE HERE **************
    //***********************************************
    Vector3d rayOrigin = r.origin() - center; // origin with respect to the sphere
    double a = 1;
    double b = 2 * dot(r.direction(), rayOrigin);
    double c = dot(rayOrigin, rayOrigin) - pow(radius, 2);
    double discriminant = pow(b, 2) - 4 * a * c;
    if (discriminant < 0) {
        return false;
    }

    double t1 = -b + sqrt(discriminant) / (2 * a);
    double t2 = -b - sqrt(discriminant) / (2 * a);
    double t0 = min(t1, t2);

    if (t0 < tmin || t0 > tmax) {
        return false;
    }
    return t0 <= tmax;
}

