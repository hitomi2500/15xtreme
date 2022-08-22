#ifndef _MINIGAME_15_H_
#define _MINIGAME_15_H_

#include <yaul.h>
#include "svin.h"

//VDP1 command list order
#define _MINIGAME_15_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX  0
#define _MINIGAME_15_VDP1_ORDER_LOCAL_COORDS_INDEX      1
#define _MINIGAME_15_VDP1_ORDER_SPRITE_0_INDEX      2
#define _MINIGAME_15_VDP1_ORDER_SPRITE_1_INDEX      3
#define _MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(x)   (4+x)
#define _MINIGAME_15_VDP1_ORDER_DRAW_END_INDEX          19
#define _MINIGAME_15_VDP1_ORDER_COUNT                     20

#endif