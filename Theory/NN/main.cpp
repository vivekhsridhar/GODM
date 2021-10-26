//
//  main.cpp
//  decision_geometry
//
//  Created by Vivek Sridhar on 04/12/19.
//  Copyright © 2019 Vivek Sridhar. All rights reserved.
//

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "parameteres.h"

using namespace rnd;
using namespace cv;

std::ofstream outputFile1;

//**************************************************************************************************
//**    MAIN    ************************************************************************************
//**************************************************************************************************

int main()
{
    echo("simulation started");
    
    // random generator engine from a time-based seed
    unsigned seed = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    rnd::set_seed(seed);
    
    // boolean switches
    rep_done = false;
    symmetric = false;
    distance = false;
    assert(symmetric == false || distance == false);
    
    // time parameters
    if (distance)
    {
        num_replicates = 50;
        num_timesteps = 10000;
    }
    else
    {
        num_replicates = 500;
        num_timesteps = 100000;
    }
    timestep_number = 0;
    trial_time = 0;
    equilibration_time = 1000;
    
    // space parameters
    arena_size = 1000;
    if (number_of_cues == 2) max_angle = PI/3;
    else max_angle = 4*PI/9;
    
    if (distance) start_dist = 100.0;
    else start_dist = 500.0;
    dist_thresh = 10.0;
    left_right_dist = 500.0;
    arena_centre = CVec2D((double)arena_size / 2, (double)arena_size / 2);
    
    // system parameters
    total_agents = 60;
    nu = 0.54;
    A = 1.8;
    h = 0.25;
    c = 1.0;
    dev = 0.02;
    temp_rescale = 0.5;
    system_energy = 0.0;
    system_magnetisation = CVec2D(0.0, 0.0);
    
    // run parameters
    reset_no = 0;
    std::fill_n(n_inds_preference, number_of_cues, 0);
    
    // output parameter
    energy = 0.0;
    magnetisation = CVec2D(0.0, 0.0);
    preference = new CVec2D[total_agents+1];
    state = new bool[total_agents+1];
    
    // output variables
    cue_reached = -1;
    path_length = 0.0;
    centroid = CVec2D(0.0,0.0);
    
    // class vectors
    agent = new spin[total_agents];
    CS = new cue[number_of_cues];
    
    // open output files
    outputFile1 = std::ofstream("geometry.csv");
    
    // output file headers
    if (distance) outputFile1 << "time" << ", " << "x" << ", " << "y" << ", " << "left_right_distance" << ", " << "front_back_distance" << "\n";
    else outputFile1 << "time" << ", " << "x" << ", " << "y" << ", " << "susceptibility" << "\n";
    
    //===================================
    //==    functions in the main   =====
    //===================================
    RunGeneration();
    
    echo("simulation ended");
    return 0;
}

//**************************************************************************************************
//**    WITHIN GENERATIONAL FUNCTIONS   ************************************************************
//**************************************************************************************************

void RunGeneration()
{
    double temp = 0.1;
    for (left_right_dist = 0; left_right_dist <= 500; )
    {
        SetupSimulation(temp);
        for (int rep = 0; rep != num_replicates; ++rep)
        {
            ResetSetup(0, arena_size/2);
            
            while (trial_time != num_timesteps)
            {
                FlipSpins(false);
                MoveAgents(temp);
                if (trial_time % 10 == 0 && trial_time != 0)
                {
                    if (!distance)
                    {
                        ResetStates();
                        for (int i = 0; i != equilibration_time; ++i) FlipSpins(true);
                        CalculateMagnetisation();
                    }
                    
                    //Graphics();
                    GenerationOutput(rep);
                }
                
                ++trial_time;
                ++timestep_number;
                
                // reset agents if target is reached
                if (rep_done) break;
            }
            
            if (rep % 50 == 0) std::cout << rep << " ";
        }
        
        std::cout << left_right_dist << "\n";
        if (distance) left_right_dist += 10;
        else left_right_dist += 1000;
    }
}

void FlipSpins(bool test)
{
    if (distance || !test)
    {
        int id = rnd::integer(total_agents);
        CalculateSystemProperties(id);
        double before = system_energy;
        agent[id].state = !agent[id].state;
        CalculateSystemProperties(id);
        double after = system_energy;
        
        double p_accept = 0.0;
        if (before < after) p_accept = exp(-(after - before) / (temp_rescale * agent[id].temperature));
        else p_accept = 1.0;
        
        if (rnd::uniform() >= p_accept) agent[id].state = !agent[id].state;
    }
    else
    {
        int id = rnd::integer(total_agents+1);
        CalculateEnergy(id);
        double before = energy;
        state[id] = !state[id];
        CalculateEnergy(id);
        double after = energy;
        
        double p_accept = 0.0;
        if (before < after) p_accept = exp(-(after - before) / (temp_rescale * agent[0].temperature));
        else p_accept = 1.0;
        
        if (rnd::uniform() >= p_accept) state[id] = !state[id];
    }
}

void CalculateSystemProperties(int spin_id)
{
    // calculate energy
    system_energy = 0.0;
    for (int i = 0; i != total_agents; ++i)
    {
        double ang = agent[spin_id].preference.smallestAngleTo(agent[i].preference) * PiOver180;
        
        ang = PI * pow(ang / PI, nu);
        double J = cos(ang);
        //double J = A * (1 - h * ang * ang) * exp(-h * ang * ang) - c;
        
        if (i != spin_id) system_energy -=  J * agent[spin_id].state * agent[i].state * agent[spin_id].picked * agent[i].picked;
    }
    system_energy /= total_agents;
    
    // calculate magnetisation
    int total_picked = 0;
    centroid = CVec2D(0.0, 0.0);
    system_magnetisation = CVec2D(0.0, 0.0);
    for (int i = 0; i != total_agents; ++i)
    {
        centroid += agent[i].position;
        system_magnetisation += agent[i].preference * agent[i].state;
        total_picked += agent[i].picked;
    }
    centroid /= total_agents;
    system_magnetisation /= total_picked;
}

void MoveAgents(double temp)
{
    std::fill_n(n_inds_preference, number_of_cues, 0);
    
    for (int i = 0; i != total_agents; ++i)
    {
        if (!distance) agent[i].position += system_magnetisation;
        else agent[i].position.y += system_magnetisation.y;
        agent[i].AddPreference(CS[agent[i].GetInformed()].centre);
        n_inds_preference[agent[i].GetInformed()] += (agent[i].state * agent[i].picked);
        
        double summation = 0.0;
        for (int j = 0; j != number_of_cues; ++j)
        {
            agent[i].GetDeviation(CS[j].centre, j);
            agent[i].probabilities[j] = GetProbability(agent[i].deviations[j], 0.0, dev);
            summation += agent[i].probabilities[j];
        }
        
        for (int j = 0; j != number_of_cues; ++j) agent[i].probabilities[j] /= summation;
        
        if (rnd::uniform() < agent[i].probabilities[agent[i].GetInformed()]) agent[i].picked = true;
        else agent[i].picked = false;
    }
    
    for (int i = 0; i != number_of_cues; ++i)
    {
        if (centroid.distanceTo(CS[i].centre) < dist_thresh * dist_thresh)
        {
            rep_done = true;
            cue_reached = i;
        }
    }
    
    path_length += system_magnetisation.length();
}

void CalculateEnergy(int spin_id)
{
    energy = 0.0;
    for (int i = 0; i != total_agents+1; ++i)
    {
        double ang = preference[spin_id].smallestAngleTo(preference[i]) * PiOver180;
        
        ang = PI * pow(ang / PI, nu);
        double J = cos(ang);
        //double J = A * (1 - h * ang * ang) * exp(-h * ang * ang) - c;
        
        if (i != spin_id) energy -=  J * state[spin_id] * state[i];
    }
    energy /= (total_agents+1);
}

void CalculateMagnetisation()
{
    magnetisation = CVec2D(0.0, 0.0);
    for (int i = 0; i != total_agents+1; ++i) magnetisation += preference[i] * state[i];
    magnetisation /= (total_agents+1);
}

//**************************************************************************************************
//**    SETUP FUNCTIONS ****************************************************************************
//**************************************************************************************************

void SetupSimulation(double temp)
{
    timestep_number = 0;
    trial_time = 0;
    reset_no = 0;
    cue_reached = -1;
    
    path_length = 0.0;
    
    centroid = arena_centre;
    if (symmetric) SetupEnvironmentSymmetric();
    else if (distance) SetupEnvironmentDistances();
    else SetupEnvironmentAsymmetric();
    
    SetupSpins(temp);
}

void SetupEnvironmentSymmetric()
{
    CVec2D start;
    start = arena_centre;
    
    double theta = 360.0 * PiOver180 / number_of_cues;
    
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

void SetupEnvironmentDistances()
{
    for (int i = 0; i != number_of_cues; ++i)
    {
        if (number_of_cues == 2)
        {
            centres[i] = CVec2D(start_dist, arena_size / 2 - left_right_dist / 2 + i * left_right_dist);
        }
        else
        {
            centres[i] = CVec2D(start_dist, arena_size / 2 - left_right_dist + i * left_right_dist);
        }
        
        CS[i].Setup(centres[i]);
    }
}

void SetupSpins(double temp)
{
    CVec2D set_position;
    CVec2D set_preference;
    int set_informed;
    bool set_state;
    bool set_picked;
    
    double set_temperature;
    double set_deviation;
    
    set_preference = CVec2D(0.0, 0.0);
    
    for(int i = 0; i != total_agents; ++i)
    {
        set_position = RandomBoundedPoint(0, 0);
            
        if (rnd::uniform() < 0.5) set_state = false;
        else set_state = true;
        set_picked = true;
        
        set_informed = i % number_of_cues;
        ++n_inds_preference[set_informed];
            
        set_temperature = temp;
        set_deviation = rnd::normal(0.0, dev);
            
        agent[i].Setup(set_position, set_temperature, set_informed, set_state, set_deviation, set_picked);
        agent[i].AddPreference(CS[agent[i].GetInformed()].centre);
        
        if (!distance)
        {
            preference[i] = agent[i].preference;
            state[i] = agent[i].state;
        }
    }
    preference[total_agents] = (CS[0].centre - centroid).normalise();
    state[total_agents] = true;
}

void ResetSetup(double x, double y)
{
    for(int i = 0; i != total_agents; ++i)
    {
        agent[i].position = RandomBoundedPoint(x, y);
        
        if (rnd::uniform() < 0.5) agent[i].state = false;
        else agent[i].state = true;
        
        int info = i % number_of_cues;
        agent[i].SetInformed(info);
        agent[i].preference = CVec2D(0.0, 0.0);
        agent[i].prime_deviation = rnd::normal(0.0, dev);
        
        if (!distance)
        {
            preference[i] = agent[i].preference;
            state[i] = true;
        }
        preference[total_agents] = (CS[rnd::integer(number_of_cues)].centre - centroid).normalise();
        state[total_agents] = true;
    }
    
    trial_time = 0;
    path_length = 0.0;
    
    ++reset_no;
    
    rep_done = false;
}

void ResetStates()
{
    for (int i = 0; i != total_agents; ++i)
    {
        preference[i] = agent[i].preference;
        state[i] = agent[i].state;
    }
    preference[total_agents] = (CS[0].centre - centroid).normalise();
    state[total_agents] = true;
}

CVec2D RandomBoundedPoint(double x, double y)
{
    double random_x = x + uniform() - 0.5;
    double random_y = y + uniform() - 0.5;
    
    CVec2D random_point(random_x, random_y);
    if (symmetric) random_point = arena_centre;
    
    return random_point;
}

double GetProbability(double x, double mu, double sigma)
{
    double coeff = 1 / (sigma * sqrt(2*PI));
    double exp_function = exp(-pow(x - mu, 2) / (2 * pow(sigma, 2)));
    
    double prob = coeff * exp_function;
    return prob;
}

//**************************************************************************************************
//**    OUTPUT  ************************************************************************************
//**************************************************************************************************

void GenerationOutput(int rep)
{
    CVec2D v1;
    CVec2D v2;
    v1 = system_magnetisation.normalise();
    v2 = magnetisation.normalise();
    
    CVec2D v3;
    CVec2D v4;
    v3 = (CS[0].centre - centroid).normalise();
    v4 = (CS[number_of_cues-1].centre - centroid).normalise();
    
    if (distance) outputFile1 << trial_time << ", " << centroid.x << ", " << centroid.y << ", " << left_right_dist << ", " << start_dist << "\n";
    else outputFile1 << trial_time << ", " << centroid.x << ", " << centroid.y << ", " << v1.smallestAngleTo(v2) << "\n";
}

//**************************************************************************************************
//**    GRAPHICS   *********************************************************************************
//**************************************************************************************************

void Graphics()
{
    // Colours vector
    Scalar colours[6] = {Scalar(234, 174, 84), Scalar(24, 202, 247), Scalar(60, 76, 231), Scalar(113, 204, 46), Scalar(34, 126, 230), Scalar(241, 240, 236)};
    
    // Draw area
    Mat visualisation = Mat::zeros(arena_size, arena_size, CV_8UC3);
    for (int i = 0; i != number_of_cues; ++i)
    {
        circle(visualisation, Point(CS[i].centre.x, CS[i].centre.y), 8, colours[i], -1, CV_AA);
    }
    
    // Draw spins
    for (int i = 0; i != total_agents; ++i)
    {
        int colour = agent[i].GetInformed();
        circle(visualisation, Point(agent[i].position.x, agent[i].position.y), 2, colours[colour], -1);
    }
    
    // Display timestep number & cue counter on screen
    putText(visualisation, std::to_string(timestep_number), cvPoint(10,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
    
    imshow("ising_model", visualisation);
    waitKey(1);
}
