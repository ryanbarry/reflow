#ifndef _LEARN_H
#define _LEARN_H

#include "temp.h"

#define LEARNED_POWER_WHOLE_OVEN 12
#define LEARNED_POWER_BOTTOM 28
#define LEARNED_POWER_TOP 23
#define LEARNED_INERTIA_WHOLE_OVEN 43
#define LEARNED_INERTIA_BOTTOM 111
#define LEARNED_INERTIA_TOP 56
#define LEARNED_INSULATION 154

void learn(Thermocouple &tc);

#endif
