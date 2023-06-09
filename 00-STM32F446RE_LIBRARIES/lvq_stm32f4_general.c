/**
  ******************************************************************************

  GENERAL For SLVQ32F446RE
  Author:   LVQ
  Updated:  4st July 2022

  ******************************************************************************
*/

#include "lvq_stm32f4_general.h"

/* System speed in MHz */
uint16_t GENERAL_SystemSpeedInMHz = 0;
static uint16_t InterruptDisabledCount = 0;

/* Private functions */
static uint32_t x_na_y(uint32_t x, uint8_t y) 
{
    uint32_t output = 1;

    /* Make a "power" multiply */
    while (y--) 
		{
        output *= x;
    }

    /* Return output value */
    return output;
}

void LVQ_GENERAL_DisableInterrupts(void) 
{
    /* Disable interrupts */
    __disable_irq();

    /* Increase number of disable interrupt function calls */
    InterruptDisabledCount++;
}

uint8_t LVQ_GENERAL_EnableInterrupts(void)	
{
    /* Decrease number of disable interrupt function calls */
    if (InterruptDisabledCount) 
		{
        InterruptDisabledCount--;
    }

    /* Check if we are ready to enable interrupts */
    if (!InterruptDisabledCount) 
		{
        /* Enable interrupts */
        __enable_irq();
    }

    /* Return interrupt enabled status */
    return !InterruptDisabledCount;
}

void LVQ_GENERAL_SystemReset(void) 
{
    /* Call user callback function */
    LVQ_GENERAL_SystemResetCallback();

    /* Perform a system software reset */
    NVIC_SystemReset();
}

uint32_t LVQ_GENERAL_GetClockSpeed(LVQ_GENERAL_Clock_t clock) 
{
    static RCC_ClocksTypeDef RCC_Clocks;
    uint32_t c = 0;

    /* Check RCC clocks */
    if (RCC_Clocks.HCLK_Frequency == 0) 
		{
        /* Read clocks */
        RCC_GetClocksFreq(&RCC_Clocks);
    }

    /* Return clock speed */
    switch (clock) 
		{
        case LVQ_GENERAL_Clock_HSI:
            c = HSI_VALUE;
            break;
        case LVQ_GENERAL_Clock_HSE:
            c = HSE_VALUE;
            break;
        case LVQ_GENERAL_Clock_HCLK:
            c = RCC_Clocks.HCLK_Frequency;
            break;
        case LVQ_GENERAL_Clock_PCLK1:
            c = RCC_Clocks.PCLK1_Frequency;
            break;
        case LVQ_GENERAL_Clock_PCLK2:
            c = RCC_Clocks.PCLK2_Frequency;
            break;
        case LVQ_GENERAL_Clock_SYSCLK:
            c = RCC_Clocks.SYSCLK_Frequency;
            break;
        default:
            break;
    }

    /* Return clock */
    return c;
}

LVQ_GENERAL_ResetSource_t LVQ_GENERAL_GetResetSource(uint8_t reset_flags) 
{
    LVQ_GENERAL_ResetSource_t source = LVQ_GENERAL_ResetSource_None;

    /* Check bits */
    if (RCC->CSR & RCC_CSR_LPWRRSTF) 
		{
        source = LVQ_GENERAL_ResetSource_LowPower;
    } 
		else if (RCC->CSR & RCC_CSR_WWDGRSTF) 
		{
        source = LVQ_GENERAL_ResetSource_WWDG;
    } 
		else if (RCC->CSR & RCC_CSR_WDGRSTF) 
		{
        source = LVQ_GENERAL_ResetSource_IWDG;
    } 
		else if (RCC->CSR & RCC_CSR_SFTRSTF) 
		{
        source = LVQ_GENERAL_ResetSource_Software;
    } 
		else if (RCC->CSR & RCC_CSR_PORRSTF) 
		{
        source = LVQ_GENERAL_ResetSource_POR;
    } 
		else if (RCC->CSR & RCC_CSR_BORRSTF) 
		{
        source = LVQ_GENERAL_ResetSource_BOR;
    }
		else if (RCC->CSR & RCC_CSR_PADRSTF) 
		{
        source = LVQ_GENERAL_ResetSource_PIN;
    }

    /* Check for clearing flags */
    if (reset_flags) 
		{
        RCC->CSR = RCC_CSR_RMVF;
    }

    /* Return source */
    return source;
}

uint8_t LVQ_GENERAL_DWTCounterEnable(void) 
{
    uint32_t c;

    /* Set clock speed if not already */
    if (GENERAL_SystemSpeedInMHz == 0) 
		{
        /* Get clock speed in MHz */
        GENERAL_SystemSpeedInMHz = LVQ_GENERAL_GetClockSpeed(LVQ_GENERAL_Clock_SYSCLK) / 1000000;
    }

    /* Enable TRC */
    CoreDebug->DEMCR &= ~0x01000000;
    CoreDebug->DEMCR |=  0x01000000;

    /* Enable counter */
    DWT->CTRL &= ~0x00000001;
    DWT->CTRL |=  0x00000001;

    /* Reset counter */
    DWT->CYCCNT = 0;

    /* Check if DWT has started */
    c = DWT->CYCCNT;

    /* 2 dummys */
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");

    /* Return difference, if result is zero, DWT has not started */
    return (DWT->CYCCNT - c);
}

void LVQ_GENERAL_ConvertFloat(LVQ_GENERAL_Float_t* Float_Struct, float Number, uint8_t decimals) 
{
    /* Check decimals */
    if (decimals > 9) 
		{
        decimals = 9;
    }

    /* Get integer part */
    Float_Struct->Integer = (int32_t) Number;

    /* Get decimal part */
    if (Number < 0) 
		{
        Float_Struct->Decimal = (int32_t)((float)(Float_Struct->Integer - Number) * x_na_y(10, decimals));
    } 
		else 
		{
        Float_Struct->Decimal = (int32_t)((float)(Number - Float_Struct->Integer) * x_na_y(10, decimals));
    }
}

float LVQ_GENERAL_RoundFloat(float Number, uint8_t decimals) 
{
    float x;

    /* Check decimals */
    if (decimals > 9) 
		{
        decimals = 9;
    }

    x = x_na_y(10, decimals);

    /* Make truncating */
    if (Number > 0) 
		{
        return (float)(Number * x + (float)0.5) / x;
    }
    if (Number < 0) 
		{
        return (float)(Number * x - (float)0.5) / x;
    }

    /* Return number */
    return 0;
}

uint32_t LVQ_GENERAL_NextPowerOf2(uint32_t number) 
{
    /* Check number */
    if (number <= 1) 
		{
        return 1;
    }

    /* Do some bit operations */
    number--;
    number |= number >> 1;
    number |= number >> 2;
    number |= number >> 4;
    number |= number >> 8;
    number |= number >> 16;
    number++;

    /* Return calculated number */
    return number;
}

void LVQ_GENERAL_ForceHardFaultError(void) 
{
    /* Create hard-fault-function typedef */
    typedef void (*hff)(void);
    hff hf_func = 0;

    /* Call function at zero location in memory = HARDFAULT */
    hf_func();
}

__weak void LVQ_GENERAL_SystemResetCallback(void) 
{
    /* NOTE: This function should not be modified, when the callback is needed,
            the LVQ_GENERAL_SystemResetCallback could be implemented in the user file
    */
}
