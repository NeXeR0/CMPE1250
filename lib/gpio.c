// gpio.c
//
// Basic GPIO library implementation for STM32G0
//

#include "gpio.h"

static int _GPIO_IsValidPin(int PinNumber)
{
    return (PinNumber >= 0) && (PinNumber <= 15);
}

// turn on port clock for desired port
void _GPIO_ClockEnable (GPIO_TypeDef * pPort)
{
    if (pPort == GPIOA)
    {
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    }
    else if (pPort == GPIOB)
    {
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    }
    else if (pPort == GPIOC)
    {
        RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    }
    else if (pPort == GPIOD)
    {
        RCC->IOPENR |= RCC_IOPENR_GPIODEN;
    }
#ifdef GPIOF
    else if (pPort == GPIOF)
    {
        RCC->IOPENR |= RCC_IOPENR_GPIOFEN;
    }
#endif
}

// fast-ish versions with basic pin check

void _GPIO_PinToggle (GPIO_TypeDef * pPort, int PinNumber)
{
    if (!pPort || !_GPIO_IsValidPin(PinNumber))
        return;

    pPort->ODR ^= (1u << PinNumber);
}

void _GPIO_PinSet (GPIO_TypeDef * pPort, int PinNumber)
{
    if (!pPort || !_GPIO_IsValidPin(PinNumber))
        return;

    pPort->BSRR = (1u << PinNumber);
}

void _GPIO_PinClear (GPIO_TypeDef * pPort, int PinNumber)
{
    if (!pPort || !_GPIO_IsValidPin(PinNumber))
        return;

    pPort->BSRR = (1u << (PinNumber + 16));
}

// set the mode of an individual pin
void _GPIO_SetPinMode (GPIO_TypeDef * pPort,
                       int PinNumber,
                       _GPIO_PinMode mode,
                       _GPIO_ClockOption clkop)
{
    if (!pPort || !_GPIO_IsValidPin(PinNumber))
        return;

    if (clkop == _GPIO_Clock_On)
    {
        _GPIO_ClockEnable(pPort);
    }

    if (mode < _GPIO_PinMode_Input || mode > _GPIO_PinMode_Analog)
        return;

    uint32_t shift = (uint32_t)PinNumber * 2u;
    uint32_t mask  = (0x3u << shift);

    pPort->MODER &= ~mask;
    pPort->MODER |= ((uint32_t)mode << shift);
}

// read input state from IDR
int _GPIO_GetPinIState (GPIO_TypeDef * pPort, int PinNumber)
{
    if (!pPort || !_GPIO_IsValidPin(PinNumber))
        return 0;

    uint32_t mask = (1u << PinNumber);
    return ( (pPort->IDR & mask) != 0u );
}

// read output state from ODR
int _GPIO_GetPinOState (GPIO_TypeDef * pPort, int PinNumber)
{
    if (!pPort || !_GPIO_IsValidPin(PinNumber))
        return 0;

    uint32_t mask = (1u << PinNumber);
    return ( (pPort->ODR & mask) != 0u );
}

// set the output type as push-pull or open-drain
int _GPIO_Set_OutputType (GPIO_TypeDef * pPort,
                          int PinNumber,
                          _GPIO_OutputType otype)
{
    if (!pPort || !_GPIO_IsValidPin(PinNumber))
        return -1;

    uint32_t mask = (1u << PinNumber);

    if (otype == _GPIO_OutputType_OpenDrain)
    {
        pPort->OTYPER |= mask;
    }
    else
    {
        pPort->OTYPER &= ~mask;
    }

    return 0;
}

// set the pin pull configuration (none, up, down)
int _GPIO_Set_PullType (GPIO_TypeDef * pPort,
                        int PinNumber,
                        _GPIO_PullType ptype)
{
    if (!pPort || !_GPIO_IsValidPin(PinNumber))
        return -1;

    if (ptype < _GPIO_PullType_None || ptype > _GPIO_PullType_Down)
        return -1;

    uint32_t shift = (uint32_t)PinNumber * 2u;
    uint32_t mask  = (0x3u << shift);

    pPort->PUPDR &= ~mask;
    pPort->PUPDR |= ((uint32_t)ptype << shift);

    return 0;
}

// set the alternate function for the specified pin
int _GPIO_SetPinAlternateFunction (GPIO_TypeDef * pPort,
                                   unsigned int PinNumber,
                                   unsigned int FunctionNumber,
                                   _GPIO_ClockOption clkop)
{
    if (!pPort || PinNumber > 15 || FunctionNumber > 7)
        return -1;

    if (clkop == _GPIO_Clock_On)
    {
        _GPIO_ClockEnable(pPort);
    }

    // AFR[0] for pins 0–7, AFR[1] for pins 8–15
    uint32_t index = (PinNumber > 7u) ? 1u : 0u;
    uint32_t pinLocal = PinNumber & 0x7u;  // 0–7 in each AFR
    uint32_t shift = pinLocal * 4u;
    uint32_t mask  = (0xFu << shift);

    pPort->AFR[index] &= ~mask;
    pPort->AFR[index] |= ((uint32_t)FunctionNumber << shift);

    // ensure pin is in Alternate Function mode in MODER
    _GPIO_SetPinMode(pPort, (int)PinNumber,
                     _GPIO_PinMode_AlternateFunction,
                     _GPIO_Clock_NoChange);

    return 0;
}
