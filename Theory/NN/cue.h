//
//  cue.h
//  decision_geometry
//
//  Created by Vivek Sridhar on 04/12/19.
//  Copyright © 2019 Vivek Sridhar. All rights reserved.
//

#ifndef cue_h
#define cue_h
#include "vector2D.h"

const int number_of_cues = 2;

class cue
{
public:
    cue(void);
    ~cue(void);
    
    void Setup(const CVec2D& set_centre);
    
    CVec2D centre;              // position for centre of cue
};

#endif /* cue_h */
