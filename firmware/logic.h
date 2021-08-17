enum button_e
{
	BUTTON_A = 0,
	BUTTON_B = 1,
	BUTTON_ONOFF = 2,
	BUTTON_SLEEP = 3
};

void process_rf(uint32_t command);
void process_button(void);
void process_buttonhold(void);
void animation_tick(void);
void process_endprg(void);
void process_start(void);