//
//  parameteres.h
//  multi-choice_decision_geometry
//
//  Created by Vivek Hari Sridhar on 27/08/17.
//  Copyright © 2017 Vivek Hari Sridhar. All rights reserved.
//

#ifndef parameteres_h
#define parameteres_h

#include "individual.h"
#include <fstream>

int     timestep_number;    // timestep number
int     cue_reached;
double  timestep_inc;       // time increment (between timesteps)
CVec2D  bottom_right;
CVec2D  top_left;
int     arena_size;
int     total_agents;
int     informed_group_size;

double	angular_error_sd;
double	max_turning_rate;
double	zod;            // zone of deflection
double	zop;            // zone of perception
double	speed;

double  set_omega;
double  angular_thresh;
double  omega_inc;
double  omega_dec;
double  omega_max;
double  dist_thresh;
double  start_dist;
double  overall_angle;
double  max_angle;
double  accuracy;

bool    rep_done;
bool    time_up;
bool    symmetric;

individual* agent;
cue* CS;

CVec2D  centres[number_of_cues];

int main();
void SetupSimulation();
void SetupEnvironmentSymmetric();
void SetupEnvironmentAsymmetric();
void SetupAgents();
void CalculateSocialForces();
void MoveAgents(CVec2D&, CVec2D&);
CVec2D RandomBoundedPoint();
void CalculateGroupProperties(CVec2D&, CVec2D&);
bool GroupTogether();
bool Equivalent(individual&, individual&);	// testing for equivalence classes (school membership)
void EquivalenceClasses();

void Graphics();

#endif /* parameteres_h */
