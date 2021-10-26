//
//  cue.h
//  multi-choice_decision_geometry
//
//  Created by Vivek Hari Sridhar on 27/08/17.
//  Copyright © 2017 Vivek Hari Sridhar. All rights reserved.
//

#ifndef cue_h
#define cue_h
#include "vector2D.h"
#include "random.h"

const int number_of_cues = 3;

class cue
{
public:
    cue(void);
    ~cue(void);
    
    void Setup(const CVec2D& set_centre);
    
    CVec2D centre;              // position for centre of cue
};

#endif /* cue_h */
