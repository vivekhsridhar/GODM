//
//  main.cpp
//  multi-choice_decision_geometry
//
//  Created by Vivek Sridhar on 27/08/17.
//  Copyright © 2017 Vivek Sridhar. All rights reserved.
//

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "parameteres.h"

using namespace rnd;
using namespace cv;

std::ofstream outputFile1;      // to get trajectories over replicates
std::ofstream outputFile2;      // to replicate Fig 1 - Couzin et al. 2005 Nature
std::ofstream outputFile3;      // for results similar to Fig 3 - Couzin et al. 2005 Nature (alternative to outputFile1)
std::ofstream outputFile4;      // get proportion of total trials that individuals go to each cue

//**************************************************************************************************
//**	MAIN	************************************************************************************
//**************************************************************************************************

int main()
{
    // Random generator engine from a time-based seed
    unsigned seed = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    rnd::set_seed(seed);
    
    // Set parameters
    timestep_inc = 0.1;
    arena_size = 1000;
    top_left.x = 0.0;
    top_left.y = 0.0;
    bottom_right.x = arena_size;
    bottom_right.y = arena_size;
    
    total_agents = 60;
    informed_group_size = 20;
    assert(informed_group_size * number_of_cues <= total_agents);
    
    angular_error_sd = 0.0;
    max_turning_rate = 114.591559;
    zod = 1.0;
    zop = 36.0;
    speed = 1.0;
    
    set_omega = 0.3;
    angular_thresh = 20.0;
    omega_inc = 0.012;
    omega_dec = 0.0008;
    omega_max = 0.4;
    dist_thresh = 10.0;
    start_dist = 450.0;
    overall_angle = 360.0;  // used for the symmetric case ('overall_angle' is split into 'number_of_cues' equal angles)
    max_angle = 4*Pi/9;       // used for the asymmetric case ('max_angle' is split into 'number_of_cues' - 1 equal angles)
    
    rep_done = false;
    time_up = false;
    symmetric = false;
    
    agent = new individual[total_agents];
    CS = new cue[number_of_cues];
    
    int num_replicates = 500;
    int num_timesteps = 50000;
    
    CVec2D centroid, polarisation;
    
    // Setup simulation
    Size S(static_cast<int>(arena_size), static_cast<int>(arena_size));
    std::fill_n(centres, number_of_cues, CVec2D(0.0, 0.0));
    
    // Open output file
    std::string filename_dg;
    filename_dg = "decision_geometry.csv";
    outputFile1.open(filename_dg.c_str());
    
    std::string filename_gp;
    filename_gp = "group_properties.csv";
    outputFile2.open(filename_gp.c_str());
    
    std::string filename_dg2;
    filename_dg2 = "dGeom_fig3.csv";
    outputFile3.open(filename_dg2.c_str());
    
    std::string filename_cr;
    filename_cr = "cue_reached.csv";
    outputFile4.open(filename_cr.c_str());
    
    // Output file headers
    outputFile1 << "replicate" << ", " << "time" << ", " << "good_run" << ", " << "x" << ", " << "y" << ", " << "cue_reached" << "\n";
    outputFile2 << "replicate" << ", " << "group_size" << ", " << "n_informed" << ", " << "accuracy" << "\n";
    outputFile3 << "replicate" << ", " << "good_run" << ", " << "angle0" << ", " << "angle1" << "\n";
    outputFile4 << "replicate" << ", " << "time_up" << ", " << "group_size" << ", " << "n_informed" << ", " << "cue_reached" << "\n";
    
    /*
    //**    use lines between these marks to measure group level properties only   **********************
    //**    (silence the bottom part of the loop as well if decision geometry needs to be examined) *****
    for (total_agents = 10; total_agents <= 150; )
    {
        assert(informed_group_size * number_of_cues <= total_agents);
    //***************************************************************************************************
     */
        for (int i = 0; i != num_replicates; ++i)
        {
            bool result = true;
            SetupSimulation();
                
            CVec2D temp_vec;
            // Time loop
            for(int j = 0; j != num_timesteps; ++j)
            {
                MoveAgents(centroid, polarisation);
                temp_vec = (CS[0].centre - centroid).normalise();
                accuracy += polarisation.dot(temp_vec);
                    
                if (j % 50 == 0)
                {
                    result = GroupTogether();
                    outputFile1 << i << ", " << j << ", " << result << ", " << centroid.x << ", " << centroid.y << ", " << cue_reached << "\n";
                        
                    CVec2D tmp1, tmp2;
                    tmp1 = (CS[0].centre - centroid).normalise();
                    tmp2 = (CS[number_of_cues - 1].centre - centroid).normalise();
                    outputFile3 << i << ", " << result << ", " << tmp1.smallestAngleTo(tmp2) << ", " << polarisation.smallestAngleTo(tmp1) << "\n";
                        
                    //Graphics();
                }
                    
                ++timestep_number;
                    
                if (!result || rep_done)
                {
                    outputFile1 << i << ", " << j << ", " << result << ", " << centroid.x << ", " << centroid.y << ", " << cue_reached << "\n";
                    break;
                }
                if (j == num_timesteps - 1) time_up = true;
            }
                
            if (rep_done)
            {
                outputFile2 << i << ", " << total_agents << ", " << informed_group_size << ", " << accuracy / timestep_number << "\n";
                    
                outputFile4 << i << ", " << time_up << ", " << total_agents << ", " << informed_group_size << ", " << cue_reached << "\n";
            }
                
            if (i % 50 == 0) std::cout << i << "\n";
            rep_done = false;
            time_up = false;
        }
        /*
    //**    this is the bottom part of the loops to be used for group properties    *********************
    //**    (silence this part along the with the chunk above to examine decision geometry) *************
        
        total_agents += 20;
    }
    //***************************************************************************************************
         */
    echo("simulation end");
    return 0;
}

//**************************************************************************************************
//**    OTHER GROUP LEVEL FUNCTIONS ****************************************************************
//**************************************************************************************************

void MoveAgents(CVec2D& centroid, CVec2D& polarisation)
{
    CalculateSocialForces();
    CalculateGroupProperties(centroid, polarisation);
    for (int i = 0; i != number_of_cues; ++i)
    {
        if (centroid.distanceTo(CS[i].centre) < dist_thresh * dist_thresh)
        {
            rep_done = true;
            cue_reached = i;
        }
    }
    
    for(int i = 0; i != total_agents; ++i)
    {
        if (agent[i].informed != 100)
        {
            agent[i].Feedback(CS[agent[i].informed].centre, angular_thresh, omega_inc, omega_dec, omega_max);
            agent[i].AddPersonalPreference(CS[agent[i].informed].centre);
        }
        
        // now each fish has a unit vector which is its desired direction of travel in the next timestep
        double dev_angle = 360.0f * normal(0.0, angular_error_sd);
        agent[i].Move(timestep_inc, arena_size, dev_angle);
    }
}

void CalculateSocialForces()
{
    double dist;
    CVec2D temp_vector;
    
    double zop_length = agent[0].zone_of_perception;
    double zod_length = agent[0].zone_of_deflection;
    
    for(int clear = 0; clear != total_agents; ++clear)
    {
        agent[clear].zod_count = 0;
        agent[clear].zop_count = 0;
        agent[clear].total_zod.Clear();
        agent[clear].total_zop.Clear();
    }
    
    for (int i = 0; i != total_agents; ++i)
    {
        for (int j = (i+1); j != total_agents; ++j)
        {
            temp_vector = (agent[j].r_centre - agent[i].r_centre);
            
            // check to see if it is reasonable that you may be interacting
            if (temp_vector.x * temp_vector.x > zop_length || temp_vector.y * temp_vector.y > zop_length)
            {
                // cannot be close enough to interact with
            }
            else
            {
                dist = agent[j].r_centre.distanceTo(agent[i].r_centre);
                
                if(dist < zod_length)	// this has highest priority
                {
                    agent[i].zod_count++;
                    agent[j].zod_count++;
                    
                    temp_vector = temp_vector.normalise();
                    
                    agent[i].total_zod+= (-temp_vector);
                    agent[j].total_zod+= (temp_vector);
                }
                
                else if(dist < zop_length)
                {
                    agent[i].zop_count++;
                    agent[j].zop_count++;
                    
                    temp_vector = temp_vector.normalise();
                    
                    agent[i].total_zop += temp_vector;
                    agent[i].total_zop += agent[j].direction;
                    agent[j].total_zop += (-temp_vector);
                    agent[j].total_zop += agent[i].direction;
                }
            }
        }
    }
    
    // now have total_zod and total_zop calculated for all individuals
    for(int k = 0; k != total_agents; ++k)
    {
        if(agent[k].zod_count > 0)
        {
            if(fabs(agent[k].total_zod.x) < FLT_EPSILON && fabs(agent[k].total_zod.y) < FLT_EPSILON)	// if they completely cancelled out
            {
                // do a correlated random walk
                agent[k].desired_direction = agent[k].direction;
            }
            else
            {
                agent[k].desired_direction = agent[k].total_zod.normalise();
            }
        }
        else if(agent[k].zop_count > 0)
        {
            if(fabs(agent[k].total_zop.x) < FLT_EPSILON && fabs(agent[k].total_zop.y) < FLT_EPSILON)	// if they completely cancelled out
            {
                // do a correlated random walk
                agent[k].desired_direction = agent[k].direction;
            }
            else
            {
                agent[k].desired_direction = agent[k].total_zop.normalise();
            }
        }
    }
}

void SetupSimulation()
{
    timestep_number = 0;
    cue_reached = -1;
    accuracy = 0.0;
    if (symmetric) SetupEnvironmentSymmetric();
    else SetupEnvironmentAsymmetric();
    SetupAgents();
}

void SetupEnvironmentSymmetric()
{
    CVec2D start;
    start = CVec2D(bottom_right.x / 2, bottom_right.y / 2);
    
    double theta = overall_angle * PiOver180 / number_of_cues;
    
    for (int i = 0; i != number_of_cues; ++i)
    {
        centres[i] = start + CVec2D(start_dist * cos((i-1) * theta), start_dist * sin((i-1) * theta));
        CS[i].Setup(centres[i]);
    }
}

void SetupEnvironmentAsymmetric()
{
    CVec2D start;
    start = CVec2D(0.0, arena_size / 2);
    
    double theta = 0.0;
    if (number_of_cues != 1) theta = max_angle / (number_of_cues - 1);
    
    for (int i = 0; i != number_of_cues; ++i)
    {
        centres[i] = start + CVec2D(start_dist * cos(i * theta - max_angle/2), start_dist * sin(i * theta - max_angle/2));
        CS[i].Setup(centres[i]);
    }
}

void SetupAgents()
{
    CVec2D set_r_centre;
    int set_informed;
    
    for(int i = 0; i != total_agents; ++i)
    {
        CVec2D set_direction(1.0, 0.0);             // need to be set to unit vectors
        set_direction.rotate(uniform() * 360.0);
        set_r_centre = RandomBoundedPoint();
        
        if (i / informed_group_size < number_of_cues) set_informed = i / informed_group_size;
        else set_informed = 100;                    // informed status 100 implies uninformed
        
        agent[i].Setup(set_r_centre, set_direction, max_turning_rate, speed, zod, zop, angular_error_sd, set_omega, set_informed);
    }
}

CVec2D RandomBoundedPoint()
{
    // Create randomly distributed co-ordinates in the simulated world space
    double range_x = (double) (bottom_right.x - top_left.x);
    double range_y = (double) (bottom_right.y - top_left.y);
    
    double random_x = uniform();
    double random_y = uniform();
    
    // Individuals start in the centre-left 100th of their world
    random_x *= (range_x / 100.0);
    random_y *= (range_y / 100.0);
    if (symmetric) random_x += (double) (range_x / 2.0 - range_x / 200.0);
    random_y += (double) (range_y / 2.0 - range_y / 200.0);
    CVec2D random_point(random_x, random_y);
    return random_point;
}

//**************************************************************************************************
//**	OUTPUT FUNCTIONS    ************************************************************************
//**************************************************************************************************

void CalculateGroupProperties(CVec2D& centroid, CVec2D& polarisation)
{
    centroid.x = 0.0; centroid.y = 0.0;
    polarisation.x = 0.0; polarisation.y = 0.0;
    
    for(int i = 0; i != total_agents; ++i)
    {
        centroid += agent[i].r_centre;
        polarisation += (agent[i].direction.normalise());
    }
    centroid /= total_agents;
    polarisation /= total_agents;
}

// Group Together(), Equivalent() and EquivalenceClasses() together calculate if group is cohesive
bool GroupTogether()
{
    // need to see if the group is cohesive first - return FALSE is it has split
    EquivalenceClasses();
    
    int group_size = 0;
    int equivalence_class = 0;
    
    equivalence_class = agent[0].equivalence_class;
    
    for(int i = 0; i < total_agents; i++)
    {
        if(agent[i].equivalence_class == equivalence_class) ++group_size;
    }
    
    if(group_size == total_agents) return 1;
    else return 0;
}

bool Equivalent(individual& agent1, individual& agent2)
{
    double dist1;
    
    double largest_zone = (agent1.zone_of_deflection > agent1.zone_of_perception) ? agent1.zone_of_deflection : agent1.zone_of_perception;
    
    dist1 = agent1.r_centre.distanceTo(agent2.r_centre);
    
    if(dist1 < largest_zone)
    {
        return 1;
    }
    return 0;
}

void EquivalenceClasses()
{
    int n = total_agents;
    int *nf = new int[n];
    
    for(int j = 0; j != n; ++j)
    {
        nf[j] = j;
        
        for(int k = 0; k != (j); ++k)
        {
            nf[k] = nf[nf[k]];
            if(Equivalent(agent[j], agent[k])) nf[nf[nf[k]]] = j;
        }
    }
    
    for(int j = 0; j != n; ++j) nf[j] = nf[nf[j]];
    
    for(int m = 0; m != n; ++m)
    {
        agent[m].equivalence_class = nf[m];
    }
}

//**************************************************************************************************
//**	GRAPHICS   *********************************************************************************
//**************************************************************************************************

void Graphics()
{
    // Colours vector
    Scalar colours[10] = {Scalar(0, 0, 213), Scalar(0, 152, 255) , Scalar(0, 234, 174), Scalar(144, 200, 4), Scalar(245, 165, 66), Scalar(255, 77, 124), Scalar(185, 201, 252), Scalar(176, 39, 156), Scalar(167, 151,0), Scalar(0, 255, 255)};
    
    // Draw arena
    Mat visualisation = Mat::zeros(arena_size, arena_size, CV_8UC3);
    for (int i = 0; i != number_of_cues; ++i)
    {
        circle(visualisation, Point(CS[i].centre.x, CS[i].centre.y), 8, colours[i], -1, CV_AA);
    }
    
    // Draw agents
    for (int i = 0; i != total_agents; ++i)
    {
        if (agent[i].informed != 100) circle(visualisation, Point(agent[i].r_centre.x, agent[i].r_centre.y), 2, colours[agent[i].informed], -1, CV_AA);
        else circle(visualisation, Point(agent[i].r_centre.x, agent[i].r_centre.y), 2, Scalar(200, 200, 153), -1, CV_AA);
    }
    
    // Display timestep number & cue counter on screen
    putText(visualisation, std::to_string(timestep_number), cvPoint(10,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
    
    imshow("decision_geometry", visualisation);
    waitKey(1);
}


