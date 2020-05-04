/***********************************************************************************
 * File              :TS_SysMng_ADC.c
 *
 * Title             :
 *
 * Author            :Tarik SEMRADE
 *
 * Created on        :Mar 19, 2020
 *
 * Version           :
 *
 * Description       : ADC offset adjustment using post processing bloc
 *
 * Copyright (c) 2020 Tarik SEMRADE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 **********************************************************************************/
#include "TS_SysMng_Adc.h"
#include "F2837xD_Adc_defines.h"
#include "TS_SysMng/TS_SysMng_X.h"
#include "adc.h"
/**********************************************************************************
 * \function:       TS_SysMng_AdcConfig
 * \brief           main `0` numbers
 * \param[in]       void
 * \return          void
 **********************************************************************************/

void
TS_SysMng_AdcConfig(void)
{
    /* Enable EALLOW protected register access */
    EALLOW;

    /* Interrupt pulse position when end of conversion occurs */
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 0x1;
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 0x1;

    /* set ADCCLK divider to /4 */
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcbRegs.ADCCTL2.bit.PRESCALE = 6;

    /* Call AdcSetMode() to configure the resolution and signal mode. */
    /* This also performs the correct ADC calibration for the configured mode. */
    AdcSetMode(ADC_ADCA, ADC_BITRESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCB, ADC_BITRESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

    /*
     * => SOC0 and SOC1 configuration
     * => Trigger using Timer0
     * => Convert channel ADCINA0 (Ch. 0)
     * => Acquisition window set to (19+1)=20 cycles (100 ns with 200 MHz SYSCLK)
     * */
    /* ADC-A */
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIGGER_CPU1_TINT0;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = ADC_CH_ADCIN0;
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;

    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIGGER_SW_ONLY;
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = ADC_CH_ADCIN0;
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 14;

    /* ADC-B */
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIGGER_CPU1_TINT0;
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = ADC_CH_ADCIN2;
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = 14;

    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIGGER_SW_ONLY;
    AdcbRegs.ADCSOC1CTL.bit.CHSEL = ADC_CH_ADCIN2;
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = 14;

    /*
     * => ADCA0 interrupt configuration
     * => Interrupt pulses regardless of flag state
     * => Enable the interrupt in the ADC
     * => EOC1 triggers the interrupt
     * */
    /* ADC-A INT1 et 2 Enable */
    AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 1;
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = ADC_SOC_NUMBER0;

    AdcaRegs.ADCINTSEL1N2.bit.INT2CONT = 1;
    AdcaRegs.ADCINTSEL1N2.bit.INT2E = 1;
    AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = ADC_SOC_NUMBER1;

    /* ADC-B INT1 & 2 Enable */
    AdcbRegs.ADCINTSEL1N2.bit.INT1CONT = 1;
    AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1;
    AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = ADC_SOC_NUMBER0;

    AdcbRegs.ADCINTSEL1N2.bit.INT2CONT = 1;
    AdcbRegs.ADCINTSEL1N2.bit.INT2E = 1;
    AdcbRegs.ADCINTSEL1N2.bit.INT2SEL = ADC_SOC_NUMBER1;

    /* Make sure INT1 flag is cleared */
    /* ADC-A */
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT2 = 1;

    /* ADC-B */
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT2 = 1;

    /*
     * => Power up ADC-A & ADC-B
     * => Wait 1 ms after power-up before using the ADC
     * => Disable EALLOW protected register access
     * */
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    DELAY_US(1000);
    EDIS;

}
/**********************************************************************************
 * \function:       TS_SysMng_ConfiPpbOffset
 * \brief           TS_SysMng_ConfiPpbOffset `2` numbers
 * \param[in]       p_i16AbcAOffset, p_i16AbcBOffset
 * \return          void
 **********************************************************************************/
void
TS_SysMng_ConfiPpbOffset(int16 p_i16AbcAOffset, int16 p_i16AbcBOffset)
{
    EALLOW;
    /*
     * PPB1 is associated with SOC1
     * PPB1 will subtract OFFCAL value
     * PPB1 is associated with SOC1
     * */
    AdcaRegs.ADCPPB1CONFIG.bit.CONFIG = 1;
    AdcaRegs.ADCPPB1OFFCAL.all = p_i16AbcAOffset;

    /*
     * PPB1 is associated with SOC1
     * PPB1 will subtract OFFCAL value
     * to associated SOC
     * */
    AdcbRegs.ADCPPB1CONFIG.bit.CONFIG = 1;
    AdcbRegs.ADCPPB1OFFCAL.all = p_i16AbcBOffset;

    EDIS;
}

