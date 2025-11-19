// GPIO Library
//
// Functions perform basic GPIO operations and configurations
//
// Simon Walker
// Created September 2024, initial build, ported from STM32L476 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#include "stm32g0xx.h"

// from STM32 derivative file
//typedef struct
//{
//  __IO uint32_t MODER;       /*!< GPIO port mode register,               Address offset: 0x00      */
//  __IO uint32_t OTYPER;      /*!< GPIO port output type register,        Address offset: 0x04      */
//  __IO uint32_t OSPEEDR;     /*!< GPIO port output speed register,       Address offset: 0x08      */
//  __IO uint32_t PUPDR;       /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
//  __IO uint32_t IDR;         /*!< GPIO port input data register,         Address offset: 0x10      */
//  __IO uint32_t ODR;         /*!< GPIO port output data register,        Address offset: 0x14      */
//  __IO uint32_t BSRR;        /*!< GPIO port bit set/reset  register,     Address offset: 0x18      */
//  __IO uint32_t LCKR;        /*!< GPIO port configuration lock register, Address offset: 0x1C      */
//  __IO uint32_t AFR[2];      /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
//  __IO uint32_t BRR;         /*!< GPIO Bit Reset register,               Address offset: 0x28      */
//} GPIO_TypeDef;

// Pin Modes
typedef enum
{
  _GPIO_PinMode_Input = 0x00,              // digital input
  _GPIO_PinMode_Output = 0x01,             // digital output
  _GPIO_PinMode_AlternateFunction = 0x02,  // alternate function (peripheral use)
  _GPIO_PinMode_Analog = 0x03 // default   // analog input (default, unmapped)
} _GPIO_PinMode;

// Output Types (default is Push-Pull)
typedef enum
{
  _GPIO_OutputType_PushPull, 
  _GPIO_OutputType_OpenDrain
} _GPIO_OutputType;

// pull configuration (7.5.4)
typedef enum
{
  _GPIO_PullType_None = 0b00,
  _GPIO_PullType_Up   = 0b01,
  _GPIO_PullType_Down = 0b10
} _GPIO_PullType;

// used for pin startup functions _GPIO_SetPinMode and _GPIO_SetPinAlternateFunction
// provide option to turn on port clock for the specified port
typedef enum
{
  _GPIO_Clock_NoChange = 0, // leave clock in same state
  _GPIO_Clock_On = 1        // turn the clock on for this port
} _GPIO_ClockOption;

// fast, no error checking versions of set, clear, and toggle for outputs
//#define _GPIO_PinToggle(p, num) ((p)->ODR ^= (1u << (num)))
//#define _GPIO_PinSet(p, num) ((p)->BSRR |= (1u << (num)))
//#define _GPIO_PinClear(p, num) ((p)->BSRR |= (0x10000u << (num)))

// turn on port clock for desired port
void _GPIO_ClockEnable (GPIO_TypeDef * pPort);

// changed macros to inline implementations
void _GPIO_PinToggle (GPIO_TypeDef * pPort, int PinNumber);
void _GPIO_PinSet (GPIO_TypeDef * pPort, int PinNumber);
void _GPIO_PinClear (GPIO_TypeDef * pPort, int PinNumber);

// set the mode of an individual pin, includes mode only Input, Output, AlternateFunction, Analog
// option to turn on clock (can't turn port clock off here)
void _GPIO_SetPinMode (GPIO_TypeDef * pPort, int PinNumber, _GPIO_PinMode mode, _GPIO_ClockOption clkop);

// get the state of the specified pin (0 == clear, !0 == set)
// from IDR (IDR is read only, assumes input pin)
int _GPIO_GetPinIState (GPIO_TypeDef * pPort, int PinNumber);

// get the state of the specified pin (0 == clear, !0 == set)
// from ODR (IDR is R/W, assumes buffered output state)
int _GPIO_GetPinOState (GPIO_TypeDef * pPort, int PinNumber);

// set the output type as push-pull or open-drain
int _GPIO_Set_OutputType (GPIO_TypeDef * pPort, int PinNumber, _GPIO_OutputType otype);

// set the pin pull configuration (none, up, down)
int _GPIO_Set_PullType (GPIO_TypeDef * pPort, int PinNumber, _GPIO_PullType ptype);

// set the alternate function for the specified pin on the specified port
int _GPIO_SetPinAlternateFunction (GPIO_TypeDef * pPort, unsigned int PinNumber, unsigned int FunctionNumber, _GPIO_ClockOption clkop);
