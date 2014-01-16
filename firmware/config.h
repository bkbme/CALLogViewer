/*
 * config.h
 *
 * Created: 4/4/2013 5:43:38 PM
 *  Author: mlohse
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

// keep build targts consistent with enum in versionmessage.h
#define VICTUS_FS_TARGET 1
#define EXCIMER_FS_TARGET 2
#define VICTUS_DOCK_TARGET 3
#define EITECH_DOCK_TARGET 4

#define TARGET VICTUS_DOCK_TARGET	// build target

/***********************************
 * IO Ports                        *
 ***********************************/
#define IO_DDRA					0xF3 // PortA [4..7] LED output, [0,1] servo output, [2,3] sensor input
#if TARGET == VICTUS_DOCK_TARGET
#define IO_DDRB					0xFF // PortB output for led board
#else
#define IO_DDRB					0x80 // PortB input, except PB7 (output for ext. interlock / Servo 1)
#endif
#define IO_DDRC					0xC3 // PortC 6,7 LED output 0,1 Servo out
#define IO_DDRD					0x30 // PortD 4,5 output for footswitch

// servo
#define PORT_SERVO0				PORTA
#define PIN_SERVO0				1
#define PORT_SERVO1				PORTA
#define PIN_SERVO1				0

//uart signals
#define PORT_UART0_DTR			PIND
#define PIN_UART0_DTR			6

// sensors
#define PORT_PHOTO_SENSOR_BOT	PINA
#define PORT_PHOTO_SENSOR_TOP	PINA
#define PIN_PHOTO_SENSOR_BOT		2
#define PIN_PHOTO_SENSOR_TOP		3

// footswitch
#define PORT_FOOTSWITCH			PORTD
#define PIN_FOOTSWITCH0			5
#define PIN_FOOTSWITCH1			4

// leds
#if TARGET == VICTUS_DOCK_TARGET				// use external led board
#define PORT_LED1				PORTB
#define PORT_LED2				PORTB
#define PORT_LED3				PORTB
#define PORT_LED4				PORTB
#define PORT_LED5				PORTB
#define PORT_LED6				PORTB
#define PORT_LED7				PORTB
#define PORT_LED8				PORTB
#define PIN_LED1					0    // green
#define PIN_LED2					1    // red
#define PIN_LED3					2    // red
#define PIN_LED4					3    // red
#define PIN_LED5					4    // red
#define PIN_LED6					5    // red
#define PIN_LED7					6    // red
#define PIN_LED8					7    // red
#else										// use onboard leds
#define PORT_LED1				PORTA
#define PORT_LED2				PORTA
#define PORT_LED3				PORTA
#define PORT_LED4				PORTA
#define PORT_LED5				PORTC
#define PORT_LED6				PORTC
#define PIN_LED1					4    // red
#define PIN_LED2					5    // red
#define PIN_LED3					6    // red
#define PIN_LED4					7    // red
#define PIN_LED5					7    // red
#define PIN_LED6					6    // green
#endif

/***********************************
 * Initialization                  *
 ***********************************/
#define INIT_PORTA				0x00 // switch off LEDs, turn off pullups on PortA
#if TARGET == VICTUS_DOCK_TARGET
#define INIT_PORTB				0x00 // switch off leds
#else
#define INIT_PORTB				0xFF // enable pullups and close ext. interlock
#endif
#define INIT_PORTC				0x00
#define INIT_PORTD				0x08 // PD3 (UART1-TX) output

#define INIT_LOWER_DOCKING		0
#define INIT_UPPER_DOCKING		1023
#define INIT_SERVO_ZAXIS			0	//116 // DOCK_SERVO_STOP
#define INIT_SERVO_XAXIS			0

#if TARGET == EXCIMER_FS_TARGET
#define INIT_FOOTSWITCH_STATE	0x00 // ExcimerFS
#else
#define INIT_FOOTSWITCH_STATE	0x02 // VictusFS/Dock
#endif

/***********************************
 * Servo Timings                   *
 ***********************************/
#define TMG_SERVO_START			145 // min on_cycle: 1000 ms / (clk / 64 / TMG_SERVO_START) -> 0.4479 ms @ 18.4321MHz
#define TMG_SERVO_PWM			1   // variable duty cycle: 0 .. 1000 ms / (clk / 64 / 256 / TMG_SERVO_PWM) -> 1.77777 ms @ 18.4321MHz
#define TMG_SERVO_STOP			254 // min off_cycle: 1000 ms / (clk / 64 / ((TMG_SERVO_STOP + 1) * TMG_SERVO_STOP_MULT)) -> 17.7777 ms @ 18.4321MHz
#define TMG_SERVO_STOP_MULT		19


/***********************************
 * UART Settings                   *
 ***********************************/
#define PROTOCOL_VERSION_LENGTH	8
#define PROTOCOL_VERSION			"14010801" // <- 2014-01-08 Build 01 (must be 8 Byte!)
#define BUFFER_SIZE_TX0			32         // in bytes (must be between 1 and 255)
#define BUFFER_SIZE_TX1			1
#define BUFFER_SIZE_RX0			48
#define BUFFER_SIZE_RX1			36
#define UART0_ENABLE			           // undefine to disable uart interface(s)
#define UART1_ENABLE

#if TARGET == VICTUS_DOCK_TARGET
#define UART0_DISCONNECT_ON_DTR		// disconnect when DTR goes high (client closes serial port)
#define UART0_LED_STATE			0	// connection state
#else
#define UART0_LED_STATE			5
#endif

/***********************************
 * Footswitch Settings             *
 ***********************************/
#if TARGET == VICTUS_DOCK_TARGET
#define FS_LED_STATE0			6
#define FS_LED_STATE1			7
#else
#define FS_LED_STATE0			0
#define FS_LED_STATE1			1
#endif

/***********************************
 * Docking Settings                *
 ***********************************/
#define DOCK_RANGE_MIN			-1024
#define DOCK_RANGE_MAX			5000 // 500g max. docking pressure
#define DOCK_FORCE_TIMEOUT		3000 // 3 sec
#define DOCK_FORCE_SLOWDOWN_TRH  2000 // 200g - slow down when getting close to docking limit
#define DOCK_FORCE_ZERO_TRH      15   // <1,5g force zero (undocked)
#define DOCK_FORCE_DELTA_TRH_INC 10   //	positive threshold for stopping servo (incresing docking force)
#define DOCK_FORCE_DELTA_TRH_DEC -5   // negative threshold for stopping servo (decresing docking force)
#define DOCK_UPDATE_INTERVAL		500  // 500ms
#define DOCK_SERVO_ZAXIS_ID		0
#define DOCK_SERVO_XAXIS_ID		1
#define DOCK_SERVO_POWER_OFF		0	 // setting this position will power down the servo (no pwm signal)
#define DOCK_SLOT_COUNT_EE		10   // initial number of slots stored in eeprom
#define DOCK_SLOTS1_EE			{ -81, -59, -38, -17, 4, 25, 47, 69, 91, 112 } // initial values for slot position 0..9 stored in eeprom
#define DOCK_SLOTS2_EE			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}                // initial values for slot position 10..19 stored in eeprom
#define DOCK_LIMIT_ZERO_LO_EE	-50	// inital docking limits (in g) stored in eeprom
#define DOCK_LIMIT_ZERO_UP_EE	14
#define DOCK_LIMIT_SOFT_LO_EE	35
#define DOCK_LIMIT_SOFT_UP_EE	75
#define DOCK_LIMIT_REGULAR_LO_EE	310
#define DOCK_LIMIT_REGULAR_UP_EE	395

#if TARGET == EITECH_DOCK_TARGET
// Eitech prototype
#define DOCK_SERVO_UP_EE			165   // initial values for servo speed for eeprom
#define DOCK_SERVO_DOWN_EE		85
#define DOCK_SERVO_UP_SLOW_EE	145
#define DOCK_SERVO_DOWN_SLOW_EE	110
#define DOCK_SERVO_STOP			116
#define DOCK_SERVO_REVERSE_Z
#define DOCK_LED_BOT				2
#define DOCK_LED_TOP				4
#define DOCK_LED_FORCE			3
#else
// RS dock
#define DOCK_SERVO_UP_EE			160   // initial values for servo speed for eeprom
#define DOCK_SERVO_DOWN_EE		85
#define DOCK_SERVO_UP_SLOW_EE	150
#define DOCK_SERVO_DOWN_SLOW_EE	140
#define DOCK_SERVO_STOP			128
#define DOCK_LED_BOT				1
#define DOCK_LED_TOP				3
#define DOCK_LED_FORCE			2
#endif


#endif /* CONFIG_H_ */
