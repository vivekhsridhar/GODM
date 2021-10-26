//
//  individual.cpp
//  multi-choice_decision_geometry
//
//  Created by Vivek Hari Sridhar on 27/08/17.
//  Copyright © 2017 Vivek Hari Sridhar. All rights reserved.
//

#include "individual.h"
#include <limits.h>
#include <float.h>
#include <algorithm>

individual::individual(void) : max_turning_rate(0.0), speed(0.0), zone_of_deflection(0.0), zone_of_perception(0.0), angular_error_sd(0.0), zod_count(0.0), zop_count(0.0)
{
}

individual::~individual(void)
{
}

void individual::Setup(const CVec2D& set_r_centre, const CVec2D& set_direction, const double& set_max_turning_rate,
                       const double& set_speed, const double& set_zone_of_deflection, const double& set_zone_of_perception,
                       const double& set_angular_error_sd, const double& set_omega, const int& set_informed)
{
    r_centre = set_r_centre;	direction = set_direction; 	max_turning_rate = set_max_turning_rate;	speed = set_speed;
    zone_of_deflection = set_zone_of_deflection;    zone_of_perception = set_zone_of_perception;
    angular_error_sd = set_angular_error_sd;    informed = set_informed;    omega = set_omega;
}

void individual::Move(const double& timestep_inc, const double& arena_size, double dev_angle)	// so we have direction (current direction) and a desired direction
{
    if(fabs(desired_direction.x) < FLT_EPSILON && fabs(desired_direction.y) < FLT_EPSILON)
    {
        TurnTowardsVector(direction, timestep_inc, dev_angle);
    }
    else
    {
        TurnTowardsVector(desired_direction, timestep_inc, dev_angle);
    }
    
    MoveMyself(timestep_inc, arena_size);
}

void individual::MoveMyself(const double& timestep_inc, const double& arena_size)
{
    CVec2D velocity = direction;
    velocity *= (speed * timestep_inc);
    r_centre += velocity;
}

void individual::TurnTowardsVector(CVec2D& vector, const double& timestep_inc, const double& dev_angle)
{
    // Nice algorithm
    double max_degrees = max_turning_rate * timestep_inc;
    
    vector.rotate(dev_angle);
    
    double check_angle = direction.smallestAngleTo(vector);
    
    if(check_angle <= max_degrees)
    {
        direction = vector;
    }
    else if(check_angle > max_degrees)  // should be able to go from the cross produce to the dot product
    {
        double cross_product = direction.cross(vector);
        
        if(cross_product > 0) direction.rotate(max_degrees);
        else direction.rotate(-max_degrees);
    }
}

void individual::AddPersonalPreference(CVec2D& cue_centre)
{
    CVec2D temp_vec;
    temp_vec = (cue_centre - r_centre).normalise();
    
    temp_vec *= omega;
    desired_direction += temp_vec;
    desired_direction = desired_direction.normalise();
}

void individual::Feedback(CVec2D& cue_centre, const double& angular_thresh, const double& omega_inc, const double& omega_dec, const double& omega_max)
{
    CVec2D temp_vec;
    temp_vec = (cue_centre - r_centre).normalise();
    
    if (direction.smallestAngleTo(temp_vec) < angular_thresh)
    {
        omega += omega_inc;
        if (omega > omega_max) omega = omega_max;
    }
    else
    {
        omega -= omega_dec;
        if (omega < 0.0) omega = 0.0;
    }
}

void individual::Copy(const individual &other)
{
    r_centre = other.r_centre;	direction = other.direction; 	max_turning_rate = other.max_turning_rate;	speed = other.speed;	zone_of_deflection = other.zone_of_deflection;  zone_of_perception = other.zone_of_perception;  angular_error_sd = other.angular_error_sd;
}

