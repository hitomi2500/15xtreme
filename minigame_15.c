#include <yaul.h>
#include "svin.h"
#include "minigame_15.h"

#define MENU_ENTRY_COUNT 16

//extern uint32_t JumpLinks[512];
extern uint32_t MenuLinks[512];
extern int _svin_filelist_size;
extern char *_svin_filelist;

#define UNUSED(x) (void)(x)


extern vdp1_cmdt_list_t *_svin_cmdt_list;

typedef struct {
        char filename[256];
        int Month;
        int Day;
} _entry_type;

//_entry_type offtopic_entries[1024];
_entry_type *offtopic_entries;
int iOfftopicEntriesNumber = 0;

uint16_t lfsr = 0;

uint16_t lfsr_fib(void)
{
    
    uint16_t bit;                    /* Must be 16-bit to allow bit<<15 later in the code */

    /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
    bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1u;
    lfsr = (lfsr >> 1) | (bit << 15);

    return lfsr;
}

void _15xtreme_set_background(char * filename)
{
    _svin_background_set(filename);
    uint8_t buf[100];
    uint8_t * p1;
    uint8_t * p2;

        vdp1_vram_partitions_t vdp1_vram_partitions;
        vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //blocks
    for (int i=0;i<15;i++)
    {
        for (int y=0;y<48;y++)  
        {
            for (int x=0;x<96;x++)    
            {
                p2 = (uint8_t *)(vdp1_vram_partitions.texture_base + 77 * 2048 + i*96*48 + y*96 + x);
                if (160+i*64+x < 352)
                        p1 = (uint8_t *)(vdp1_vram_partitions.texture_base + 160+i*64+x + y*352);
                else
                        p1 = (uint8_t *)(vdp1_vram_partitions.texture_base + 244 * 352 + (160+i*64+x-352) + y*352);
                p2[0] = p1[0];
            }
        }
    }

    //cut space for blocks
    for (int y=16;y<16+4*48;y++)  
    {
        memset(vdp1_vram_partitions.texture_base + y*352 + 160, 0, 352-160);
        memset(vdp1_vram_partitions.texture_base + (y+223)*352, 0, 384+160-352);
    }

}

int
main(void)
{
        offtopic_entries = (_entry_type * )LWRAM(0x80000);

        //using 704x244 progressive mode
        _svin_init(_SVIN_X_RESOLUTION_704,_SVIN_Y_RESOLUTION_224,true);
        _svin_textbox_disable(); //filling textbox tiles with invisible data
        //_svin_background_set_no_filelist("BOOTLOGO.BG");

        bool bCD_Ok = _svin_filelist_fill(); //move this to init probably
		if (false == bCD_Ok)
		{
				_svin_textbox_init();
				_svin_textbox_print("","This game does not work in Yabause except romulo builds. Get one from https://github.com/razor85/yabause/releases/latest","Lato_Black15",7,7);
				while (1);
		}
        
        //_svin_background_fade_to_black();

        //-------------- resetup VDP1 -------------------
        _svin_cmdt_list = vdp1_cmdt_list_alloc(_MINIGAME_15_VDP1_ORDER_COUNT);

        static const int16_vec2_t local_coord_ul =
                INT16_VEC2_INITIALIZER(0,
                                0);

        static const vdp1_cmdt_draw_mode_t sprite_draw_mode = {
                .raw = 0x0000,
                .bits.pre_clipping_disable = true};

        assert(_svin_cmdt_list != NULL);

        vdp1_cmdt_t *cmdts;
        cmdts = &_svin_cmdt_list->cmdts[0];

        (void)memset(&cmdts[0], 0x00, sizeof(vdp1_cmdt_t) * _MINIGAME_15_VDP1_ORDER_COUNT);

        _svin_cmdt_list->count = _MINIGAME_15_VDP1_ORDER_COUNT;

        vdp1_cmdt_normal_sprite_set(&cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_0_INDEX]);
        vdp1_cmdt_param_draw_mode_set(&cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_0_INDEX], sprite_draw_mode);
        vdp1_cmdt_normal_sprite_set(&cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_1_INDEX]);
        vdp1_cmdt_param_draw_mode_set(&cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_1_INDEX], sprite_draw_mode);
        for (int i=0; i<15; i++)
        {
                vdp1_cmdt_normal_sprite_set(&cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)]);
                vdp1_cmdt_param_draw_mode_set(&cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)], sprite_draw_mode);
        }


        vdp1_cmdt_system_clip_coord_set(&cmdts[_MINIGAME_15_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX]);

        vdp1_cmdt_local_coord_set(&cmdts[_MINIGAME_15_VDP1_ORDER_LOCAL_COORDS_INDEX]);
        vdp1_cmdt_vtx_local_coord_set(&cmdts[_MINIGAME_15_VDP1_ORDER_LOCAL_COORDS_INDEX], local_coord_ul);

        vdp1_cmdt_end_set(&cmdts[_MINIGAME_15_VDP1_ORDER_DRAW_END_INDEX]);
        #define VDP1_FBCR_DIE (0x0008)
        if (_svin_videomode_y_res > 256)
                MEMORY_WRITE(16, VDP1(FBCR), VDP1_FBCR_DIE);
        
        vdp1_vram_partitions_set(64,//VDP1_VRAM_DEFAULT_CMDT_COUNT,
                                0x7F000, //  VDP1_VRAM_DEFAULT_TEXTURE_SIZE,
                                0,//  VDP1_VRAM_DEFAULT_GOURAUD_COUNT,
                                0);//  VDP1_VRAM_DEFAULT_CLUT_COUNT);

        static vdp1_env_t vdp1_env = {
                .erase_color = RGB1555(1, 0, 0, 0),
                .erase_points[0] = {
                        .x = 0,
                        .y = 0
                },
                .bpp = VDP1_ENV_BPP_8,
                .rotation = VDP1_ENV_ROTATION_0,
                .color_mode = VDP1_ENV_COLOR_MODE_PALETTE,
                .sprite_type = 0xC
        };

        vdp1_env.erase_points[1].x = _svin_videomode_x_res-1;
        vdp1_env.erase_points[1].y = _svin_videomode_y_res-1;

        vdp1_env_set(&vdp1_env);

        vdp1_vram_partitions_t vdp1_vram_partitions;
        vdp1_vram_partitions_get(&vdp1_vram_partitions);

        vdp1_cmdt_color_bank_t dummy_bank;
        dummy_bank.raw = 0;

        vdp1_cmdt_t *cmdt_sprite;
        cmdt_sprite = &cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_0_INDEX];
        cmdt_sprite->cmd_xa = 0;
        cmdt_sprite->cmd_ya = 0;
        cmdt_sprite->cmd_size = 0x2CE0;
        cmdt_sprite->cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0) ) / 8;
        vdp1_cmdt_param_color_mode4_set(cmdt_sprite, dummy_bank);
        vdp1_cmdt_param_color_bank_set(cmdt_sprite, dummy_bank);
        cmdt_sprite->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
        cmdt_sprite = &cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_1_INDEX];
        cmdt_sprite->cmd_xa = 352;
        cmdt_sprite->cmd_ya = 0;
        cmdt_sprite->cmd_size = 0x2CE0;
        cmdt_sprite->cmd_srca = ((int)vdp1_vram_partitions.texture_base - VDP1_VRAM(0) + _svin_videomode_x_res*_svin_videomode_y_res/2 ) / 8;
        vdp1_cmdt_param_color_mode4_set(cmdt_sprite, dummy_bank);
        vdp1_cmdt_param_color_bank_set(cmdt_sprite, dummy_bank);
        cmdt_sprite->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now

        for (int i=0;i<15;i++)
        {
                cmdt_sprite = &cmdts[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)];
                cmdt_sprite->cmd_xa = 164+96*(i%4);
                cmdt_sprite->cmd_ya = 16+48*(i/4);
                cmdt_sprite->cmd_size = 0x0C30;
                cmdt_sprite->cmd_srca = ((int)vdp1_vram_partitions.texture_base - VDP1_VRAM(0) + _svin_videomode_x_res*_svin_videomode_y_res + i*96*48 ) / 8;
                vdp1_cmdt_param_color_mode4_set(cmdt_sprite, dummy_bank);
                vdp1_cmdt_param_color_bank_set(cmdt_sprite, dummy_bank);        
                //vdp1_cmdt_jump_assign(cmdt_sprite, _SVIN_VDP1_ORDER_DRAW_END_B_INDEX * 4);//skipping B2 and B3
                cmdt_sprite->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
        }


        vdp1_cmdt_t *cmdt_system_clip_coords;
        cmdt_system_clip_coords = &cmdts[_MINIGAME_15_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX];

        cmdt_system_clip_coords->cmd_xc = _svin_videomode_x_res - 1;
        cmdt_system_clip_coords->cmd_yc = _svin_videomode_y_res - 1;

        
        vdp1_sync_cmdt_list_put(_svin_cmdt_list, 0);
        vdp1_sync();
        //-------------- resetup VDP1 done -------------------

        _svin_set_vdp1_cmdlist_toggle_at_vblank(false); //no need in progressive mode

		
	//move blocks away
        uint16_t * p16 = (uint16_t*)VDP1_VRAM(0);
        for (int i=0;i<15;i++)
        {
                p16[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)*16+6] = -100;
                p16[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)*16+7] = -100;
        }

        _svin_menu_init("SCRIPT_ENG.MNU"); //this requires filelist to be loaded first

        //fill entries list
        for (int i=0;i<_svin_filelist_size;i++)
        {
                if (memcmp("img/",(char*)(_svin_filelist+i*256+8),4) == 0)
                {
                        strcpy(offtopic_entries[iOfftopicEntriesNumber].filename,(char*)(_svin_filelist+i*256+8));
                        offtopic_entries[iOfftopicEntriesNumber].Month = (offtopic_entries[iOfftopicEntriesNumber].filename[6] - '0')*10;
                        offtopic_entries[iOfftopicEntriesNumber].Month += (offtopic_entries[iOfftopicEntriesNumber].filename[7] - '0');
                        offtopic_entries[iOfftopicEntriesNumber].Day = (offtopic_entries[iOfftopicEntriesNumber].filename[8] - '0')*10;
                        offtopic_entries[iOfftopicEntriesNumber].Day += (offtopic_entries[iOfftopicEntriesNumber].filename[9] - '0');
                        iOfftopicEntriesNumber++;
                }
        }

        while(1)
        {

        for (int i=0;i<64;i++)
                MenuLinks[i] = i;
        char buf[64];
        _entry_type entry;

        _svin_menu_populate(0,"Random date");
        _svin_menu_populate(1,"Select date");
        int iYear;
        int iMonth;
        int iDays;
        UNUSED(iYear);
        UNUSED(iMonth);
        UNUSED(iDays);
        int iRand;

        if (0 == (uint32_t)_svin_menu_activate())
        {
                //random chosen
                iRand = lfsr_fib();
                entry = offtopic_entries[iRand % iOfftopicEntriesNumber];
        }
        else
        {
                //select month
                int iMonthCounts[12];
                for (int mon=0; mon <12; mon++)
                       iMonthCounts[mon] = 0; 
                for (int mon=0; mon <12; mon++)
                    {
                                for (int i=0;i<iOfftopicEntriesNumber; i++)
                                {
                                        if (offtopic_entries[i].Month == mon )
                                                iMonthCounts[mon]++;
                                }
                    }
                char MonthNames[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
                for (int mon=6; mon <12; mon++)
                {
                        sprintf(buf,"%s (%i)",MonthNames[mon],iMonthCounts[mon]);
                        _svin_menu_populate(mon,buf);
                }
                iMonth = _svin_menu_activate();
                //select part (optional)
                int iPartsNumber = ((iMonthCounts[iMonth]-1)/12)+1;
                int iPart = 0;
                int iPartsCounts[10];
                if (iPartsNumber>1)
                {
                        for (int i=0;i<(iPartsNumber);i++)
                                iPartsCounts[i] = 12;
                        iPartsCounts[iPartsNumber-1] = iMonthCounts[iMonth]%12;
                        for (int par=0; par < iPartsNumber; par++)
                        {
                                sprintf(buf,"Part %i (%i)",par,iPartsCounts[par]);
                                _svin_menu_populate(par,buf);
                        }
                        iPart = _svin_menu_activate();
                }
                //select entry
                int iCount = 0;
                int iMenuID = 0;
                _entry_type * entries[12];

                        for (int i=0;i<iOfftopicEntriesNumber; i++)
                        {
                                if (offtopic_entries[i].Month == iMonth)
                                {
                                        iCount++;
                                        if (iCount >= iPart*12)
                                        {
                                                iMenuID++;
                                                _svin_menu_populate(iMenuID,offtopic_entries[i].filename);
                                                entries[iMenuID] = &(offtopic_entries[i]);
                                        }
                                }
                        }
                int iRes = _svin_menu_activate();
                entry = entries[iRes][0];
        }

        _15xtreme_set_background(entry.filename);

        bool bComplete = false;
        bool bMoving = false;
        
        //initial fill
        int Blocks[4][4];
        int * Blocks_Linear = (int*) Blocks;
        for (int i=0;i<16;i++)
               Blocks_Linear[i] = i;

        //now shuffling
        int iHoleX,iHoleY;
        int iSelectedX,iSelectedY;
        int iBuf;
        int iKeyCode = 0;
        
        iHoleX = 0;
        iHoleY = 0;
        for (int i=0;i<1000;i++)
        {
                //searching for hole, it have id 15
                for (int i=0;i<4;i++)
                        for (int j=0;j<4;j++)
                                if (Blocks[i][j] == 15)
                                {
                                        iHoleX = i;
                                        iHoleY = j;
                                }
                //choosing random direction for hole, 0=top,1=bot,2=left,3=right
                iRand = lfsr_fib();
                switch(iRand%4)
                {
                        case 0:
                                if (iHoleY > 0)
                                {
                                        iBuf = Blocks[iHoleX][iHoleY-1];
                                        Blocks[iHoleX][iHoleY-1] = 15;
                                        Blocks[iHoleX][iHoleY] = iBuf;
                                }
                                break;
                        case 1:
                                if (iHoleY < 3)
                                {
                                        iBuf = Blocks[iHoleX][iHoleY+1];
                                        Blocks[iHoleX][iHoleY+1] = 15;
                                        Blocks[iHoleX][iHoleY] = iBuf;
                                }
                                break;
                        case 2:
                                if (iHoleX > 0)
                                {
                                        iBuf = Blocks[iHoleX-1][iHoleY];
                                        Blocks[iHoleX-1][iHoleY] = 15;
                                        Blocks[iHoleX][iHoleY] = iBuf;
                                }
                                break;
                        case 3:
                                if (iHoleX < 3)
                                {
                                        iBuf = Blocks[iHoleX+1][iHoleY];
                                        Blocks[iHoleX+1][iHoleY] = 15;
                                        Blocks[iHoleX][iHoleY] = iBuf;
                                }
                                break;
                }
        }
        
        //searching for hole, it have id 15
        for (int i=0;i<4;i++)
                for (int j=0;j<4;j++)
                        if (Blocks[i][j] == 15)
                        {
                                iHoleX = j;
                                iHoleY = i;
                        }

        iSelectedX=1;
        iSelectedY=1;
        UNUSED(iSelectedX);
        UNUSED(iSelectedY);
        bMoving = false;
        int iDivider = 0;
        int iShiftX = 0;
        int iShiftY = 0;
        rgb1555_t bs_color;
        bool bKeyPressed = false;
        //char buf[256];
        //acivate main loop
        while (false == bComplete)
        {
                //if move is in progress, process move, ignore keys
                if (true == bMoving)
                {
                        //TODO: process moving
                        //now simply switch 
                        if ((iSelectedX == iHoleX) && (iSelectedY == iHoleY+1))
                        {
                                //swap blocks
                                iBuf = Blocks[iHoleY+1][iHoleX];
                                Blocks[iHoleY+1][iHoleX] = Blocks[iHoleY][iHoleX];
                                Blocks[iHoleY][iHoleX] = iBuf;
                                //move coords for hole and selected
                                iSelectedY--;
                                iHoleY++;
                        }
                        else if ((iSelectedX == iHoleX) && (iSelectedY == iHoleY-1))
                        {
                                //swap blocks
                                iBuf = Blocks[iHoleY-1][iHoleX];
                                Blocks[iHoleY-1][iHoleX] = Blocks[iHoleY][iHoleX];
                                Blocks[iHoleY][iHoleX] = iBuf;
                                //move coords for hole and selected
                                iSelectedY++;
                                iHoleY--;
                        }
                        else if ((iSelectedX == iHoleX+1) && (iSelectedY == iHoleY))
                        {
                                //swap blocks
                                iBuf = Blocks[iHoleY][iHoleX+1];
                                Blocks[iHoleY][iHoleX+1] = Blocks[iHoleY][iHoleX];
                                Blocks[iHoleY][iHoleX] = iBuf;
                                //move coords for hole and selected
                                iSelectedX--;
                                iHoleX++;
                        }
                        else if ((iSelectedX == iHoleX-1) && (iSelectedY == iHoleY))
                        {
                                //swap blocks
                                iBuf = Blocks[iHoleY][iHoleX-1];
                                Blocks[iHoleY][iHoleX-1] = Blocks[iHoleY][iHoleX];
                                Blocks[iHoleY][iHoleX]= iBuf;
                                //move coords for hole and selected
                                iSelectedX++;
                                iHoleX--;
                        }
                        bMoving = false;
                }
                else
                {
                        //check keys
                        iKeyCode =  _svin_get_keys_state();
                        if (false == bKeyPressed)
                        {
                                switch (iKeyCode)
                                {
                                        case PERIPHERAL_DIGITAL_UP:
                                                if (iSelectedY > 0) iSelectedY--;
                                                break;
                                        case PERIPHERAL_DIGITAL_DOWN:
                                                if (iSelectedY < 3) iSelectedY++;
                                                break;
                                        case PERIPHERAL_DIGITAL_LEFT:
                                                if (iSelectedX > 0) iSelectedX--;
                                                break;
                                        case PERIPHERAL_DIGITAL_RIGHT:
                                                if (iSelectedX < 3) iSelectedX++;
                                                break;
                                        case PERIPHERAL_DIGITAL_A:
                                        case PERIPHERAL_DIGITAL_C:
                                                bMoving = true;
                                                break;
                                }
                        }
                        if (iKeyCode != 0)
                                bKeyPressed = true;
                        else
                                bKeyPressed = false;                        
                }

                //check if complete
                bComplete = true;
                for (int i=0;i<16;i++)
                        if (Blocks_Linear[i] != i)
                                bComplete = false;

                //dump table to lwram
                /*int * p32 = (int*)LWRAM(0);
                UNUSED(p32);
                for (int i=0;i<16;i++)
                        //for (int j=0;j<4;j++)
                                p32[i]=Blocks_Linear[i];//Blocks[i][j];

                p32[16] = iHoleX;
                p32[17] = iHoleY;
                p32[18] = iSelectedX;
                p32[19] = iSelectedY;
                p32[20] =  iOfftopicEntriesNumber;*/

                //redraw data
                uint16_t * p16 = (uint16_t*)VDP1_VRAM(0);
                for (int i=0;i<15;i++)
                {
                        //finding block
                        int j=-1;
                        for (int k=0;k<16;k++)
                                if (Blocks_Linear[k] == i) j=k;
                        assert(j>=0);
                        //getting x and y
                        int x,y;
                        x = 160 + (j%4)*96;
                        y = 16 + (j/4)*48;
                        if ((iSelectedX  == j%4) && (iSelectedY == j/4))
                        {
                                x = x + iShiftX;
                                y = y + iShiftY;
                        }
                        p16[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)*16+6] = x;
                        p16[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)*16+7] = y;
                }
				int col = lfsr_fib();
				col = col%16;
                if ((iSelectedX == iHoleX) && (iSelectedY == iHoleY))
                        bs_color = RGB1555(1, col, col, col);
                else
                        bs_color = RGB1555(1, 0, 0, 0);
                vdp2_scrn_back_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE), bs_color);

                iDivider++;
                if (iDivider%5 == 0)
                {
                        //iShiftX = (lfsr_fib()%7)-4;
                        //iShiftY = (lfsr_fib()%7)-4;
                        iShiftX += (lfsr_fib()%3)-1;
                        iShiftY += (lfsr_fib()%3)-1;
                        iShiftX = (iShiftX+7) % 7 - 4;
                        iShiftY = (iShiftY+7) % 7 - 4;
                }

                //wait for vsync
                vdp2_sync();
                vdp2_sync_wait();
        }

        _svin_background_set(entry.filename);//"offtopic/210613.bg");
        bs_color = RGB1555(1, 0, 0, 0);
        vdp2_scrn_back_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE), bs_color);
        //move blocks away
        uint16_t * p16 = (uint16_t*)VDP1_VRAM(0);
        for (int i=0;i<15;i++)
        {
                p16[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)*16+6] = -100;
                p16[_MINIGAME_15_VDP1_ORDER_SPRITE_BLOCK_INDEX(i)*16+7] = -100;
        }

        _svin_wait_for_key_press_and_release();


        }

}
