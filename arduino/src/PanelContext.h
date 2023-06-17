//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_PANELCONTEXT_H
#define C_AUNISOMA_PANELCONTEXT_H


#include "Panel.h"

class PanelContext {
public:
    virtual Panel* get_panel_at(int) = 0;
};


#endif //C_AUNISOMA_PANELCONTEXT_H
