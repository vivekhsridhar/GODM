//
//  spin.h
//  decision_geometry
//
//  Created by Vivek Hari Sridhar on 04/12/19.
//  Copyright © 2019 Vivek Hari Sridhar. All rights reserved.
//

#ifndef spin_h
#define spin_h
#include <iostream>
#include "cue.h"

class spin
{
public:
    spin(void);
    ~spin(void);
    
    void Setup(const CVec2D& set_position, double& set_temperature, int& set_informed, bool& set_state, double& set_deviation, bool& set_picked);
    void AddPreference(CVec2D& cue_centre);
    void GetDeviation(CVec2D& cue_centre, int cue_id);
    int GetInformed();
    void SetInformed(int& informed);
    void Copy(spin& source);
    
    CVec2D position;
    CVec2D preference;
    bool state;
    bool picked;
    
    double temperature;
    double prime_deviation;
    double deviations[number_of_cues];
    double probabilities[number_of_cues];   // probability this spin is picked based on the deviations
    
private:
    int informed;
};

#endif /* spin_h */
