void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void ADC_init(void);
void input_calibration(void);
void LCD_display(unsigned char batt);

void SPI_send(unsigned char temp);
void output_motor(void);
void motor_init(void);
void motor_calibration(void);
void motor_center(void);
void motor_forward(unsigned char vel);
void motor_reverse(unsigned char vel);
void motor_right(unsigned char dist);
void motor_left(unsigned char dist);
void LCD_debug1(void);
void LCD_debug2(void);
void LCD_debug3(void);
char battery_level(void);
void SPI_init( void );
