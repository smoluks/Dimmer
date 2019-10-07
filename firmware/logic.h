enum button_e
{
	BUTTON_A = 2,
	BUTTON_B = 8,
	BUTTON_ONOFF = 1,
	BUTTON_SLEEP = 4
};

void process_rf(uint8_t buttonflags);
void process_button();
void process_buttonhold();
void tick(void);