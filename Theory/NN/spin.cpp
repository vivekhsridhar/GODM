//
//  spin.cpp
//  decision_geometry
//
//  Created by Vivek Hari Sridhar on 04/12/19.
//  Copyright © 2019 Vivek Hari Sridhar. All rights reserved.
//

#include "spin.h"

spin::spin(void)
{
}

spin::~spin(void)
{
}

void spin::Setup(const CVec2D& set_position, double& set_temperature, int& set_informed, bool& set_state, double& set_deviation, bool& set_picked)
{
    position = set_position;
    informed = set_informed;
    state = set_state;
    
    temperature = set_temperature;
    prime_deviation = set_deviation;
    picked = set_picked;
    
    std::fill_n(deviations, number_of_cues, 0.0);
    std::fill_n(probabilities, number_of_cues, 0.0);
}

void spin::AddPreference(CVec2D& cue_centre)
{
    preference = (cue_centre - position).normalise();
    preference.rotate(prime_deviation * PiUnder180);
}

void spin::GetDeviation(CVec2D& cue_centre, int cue_id)
{
    CVec2D target;
    target = (cue_centre - position).normalise();
    deviations[cue_id] = target.smallestAngleTo(preference)*PiOver180;
}

int spin::GetInformed()
{
    return informed;
}

void spin::SetInformed(int& info)
{
    informed = info;
}

void spin::Copy(spin& source)
{
    preference = source.preference;
    informed = source.informed;
    state = source.state;
    temperature = source.temperature;
    prime_deviation = source.prime_deviation;
    picked = source.picked;
}
