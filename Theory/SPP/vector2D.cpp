//
//  vector2D.cpp
//  multi-choice_decision_geometry
//
//  Created by Vivek Hari Sridhar on 27/08/17.
//  Copyright © 2017 Vivek Hari Sridhar. All rights reserved.
//

#include "vector2D.h"

//  Default Constructor
CVec2D::CVec2D(void)
{
    x = 0.0f; y = 0.0f;
}

//  Destructor
CVec2D::~CVec2D(void)
{
}

// Alternative Constructor - specified position
CVec2D::CVec2D(double x1, double y1)
{
    x = x1; y = y1;
}

//  Copy Constructor
CVec2D::CVec2D(CVec2D& other_vector)
{
    x = other_vector.x; y = other_vector.y;
}

//  Add Vectors
CVec2D CVec2D::operator+(const CVec2D& vec)
{
    CVec2D result;
    result.x = x + vec.x;
    result.y = y + vec.y;
    return result;
}

CVec2D CVec2D::operator+=(const CVec2D& vec)
{
    x += vec.x;
    y += vec.y;
    return *this;
}

//  Subtract Vectors
CVec2D CVec2D::operator-(const CVec2D& vec)
{
    CVec2D result;
    result.x = x - vec.x;
    result.y = y - vec.y;
    return result;
}

CVec2D CVec2D::operator-=(const CVec2D& vec)
{
    x -= vec.x;
    y -= vec.y;
    return *this;
}

//  Negative Vector
CVec2D CVec2D::operator-()
{
    CVec2D result;
    result.x = -x;
    result.y = -y;
    return result;
}

//  Equate to vector
CVec2D CVec2D::operator=(const CVec2D& vec)
{
    x = vec.x;
    y = vec.y;
    return *this;
}

//  Multiply Vectors - elementwise
CVec2D CVec2D::operator*(const CVec2D& vec)
{
    CVec2D result;
    result.x = x * vec.x;
    result.y = y * vec.y;
    return result;
}

CVec2D CVec2D::operator*=(const CVec2D& vec)
{
    x *=vec.x;
    y *=vec.y;
    return *this;
}

//  Multiply vector with scalar
CVec2D CVec2D::operator*(double val)
{
    CVec2D result;
    result.x = x * val;
    result.y = y * val;
    return result;
}

CVec2D CVec2D::operator*=(double val)
{
    x *= val;
    y *= val;
    return *this;
}

// Divide Vectors - elementwise
CVec2D CVec2D::operator/(const CVec2D& vec)
{
    CVec2D result;
    result.x = x / vec.x;
    result.y = y / vec.y;
    return result;
}

// Divide vector by scalar
CVec2D CVec2D::operator/(double val)
{
    CVec2D result;
    result.x = x / val;
    result.y = y / val;
    return result;
}

CVec2D CVec2D::operator/=(double val)
{
    x /= val;
    y /= val;
    return *this;
}

// Normalise
CVec2D CVec2D::normalise()
{
    CVec2D result;
    double dist = this->length();
    
    if(dist != 0.0)
    {
        result.x = x / dist;
        result.y = y / dist;
        return result;
    }
    else
    {
        // make no change
        return *this;
    }
}

// Dot Product
double CVec2D::dot(const CVec2D& vec)
{
    double result;
    result = x*vec.x + y*vec.y;
    return result;
}

// Cross Product
double CVec2D::cross(const CVec2D& vec)
{
    double result;
    result = x*vec.y - vec.x*y;
    
    return result;
}

// Rotate vector
void CVec2D::rotate(double degrees)
{
    double temp_x = x;
    double temp_y = y;
    temp_x = ( (x * cos(degrees * PiOver180)) - (y * sin(degrees * PiOver180)) );
    temp_y = ( (x * sin(degrees * PiOver180)) + (y * cos(degrees * PiOver180)) );
    x = temp_x;
    y = temp_y;
}

//  Length
double CVec2D::length()
{
    //	return (sqrt(Num.SqrFP(x) + Num.SqrFP(y)));	// too slow
    return sqrt((x*x) + (y*y));
}

// Distance to another vector
double CVec2D::distanceTo(const CVec2D& to_point)
{
    double dist_x = to_point.x - x;
    double dist_y = to_point.y - y;
    //  return (sqrt(dist_x * dist_x + dist_y * dist_y));
    return (dist_x * dist_x + dist_y * dist_y);
}

// Angle from x-axis (east) to vector - counterclockwise        OR
// Angle from vector to x-axis (east) - clockwise
double CVec2D::polarAngle()
{
    if((x == 0.0f) && (y == 0.0f)) return -1.0f;

    if(x == 0.0f) return ((y > 0.0f) ? 90.0f : 270.0f);
    
    double theta = atan(y / x);	// in radians
    theta *= PiUnder180;
    
    if(x > 0.0)	return ((y >= 0) ? theta : 360.0 + theta);  // quadrants 1 and 4
    else return (180.0 + theta);  // quadrants 2 and 3
}

//  Angle from vector to y-axis (north) - counterclockwise      OR
//  Angle from y-axis (north) to vector - clockwise
double CVec2D::polarAngleZeroNorth()
{
    if((x == 0.0f) && (y == 0.0f)) return -1.0f;
    
    if(x == 0.0f) return ((y > 0.0f) ? 90.0f : 270.0f);
    
    double theta = atan(y / x);	// in radians
    theta *= PiUnder180;
    
    if(x > 0.0) return (90.0 - theta);  // quadrants 1 and 4
    else return (270.0 - theta);  // quadrants 2 and 3
}

// Smallest angle to another vector
double CVec2D::smallestAngleTo(CVec2D other_vector)
{
    // this function calculates the smaller of the two angles between two unit vectors that begin at the origin
    CVec2D vec = *this;
    vec = vec.normalise();	// the speed is unimportant so the vector can be normalised
    
    double dot_product = vec.dot(other_vector);
    
    // important to have these lines below in
    //	if(dot_product > 1.0f)	{	dot_product = 1.0f;	}
    //	if(dot_product < 0.0f)	{	dot_product = 0.0f;	}
    
    double val = (acos(dot_product));
    return (val/PiOver180);
}
