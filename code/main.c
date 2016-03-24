/*
 * File:   main.c
 * Author: Jason
 *
 * Created on February 13, 2016, 10:43 AM
 */


#include <xc.h>
#include "libmathq15.h"
#include "task.h"
#include "frame.h"
#include "dio.h"

/********************* CONFIGURATION BIT SETTINGS *****************************/
// FBS
#pragma config BWRP = OFF               // Boot Segment Write Protect (Disabled)
#pragma config BSS = OFF                // Boot segment Protect (No boot program flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Write Protect (General segment may be written)
#pragma config GCP = OFF                // General Segment Code Protect (No Protection)

// FOSCSEL
#pragma config FNOSC = PRI              // Oscillator Select (Primary Oscillator (XT, HS, EC))
#pragma config SOSCSRC = ANA            // SOSC Source Type (Analog Mode for use with crystal)
#pragma config LPRCSEL = LP             // LPRC Oscillator Power and Accuracy (Low Power, Low Accuracy Mode)
#pragma config IESO = ON                // Internal External Switch Over bit (Internal External Switchover mode enabled (Two-speed Start-up enabled))

// FOSC
#pragma config POSCMOD = EC             // Primary Oscillator Configuration bits (External clock mode selected)
#pragma config OSCIOFNC = CLKO          // CLKO Enable Configuration bit (CLKO output signal enabled)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary Oscillator configured for high-power operation)
#pragma config FCKSM = CSECME           // Clock Switching and Monitor Selection (Both Clock Switching and Fail-safe Clock Monitor are enabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32768)
#pragma config FWPSA = PR128            // WDT Prescaler bit (WDT prescaler ratio of 1:128)
#pragma config FWDTEN = ON              // Watchdog Timer Enable bits (WDT enabled in hardware)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected(windowed WDT disabled))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware, SBOREN bit disabled)
#pragma config RETCFG = OFF             //  (Retention regulator is not available)
#pragma config PWRTEN = ON              // Power-up Timer Enable bit (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Use Default SCL1/SDA1 Pins For I2C1)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset set to lowest voltage (1.8V))
#pragma config MCLRE = ON               // MCLR Pin Enable bit (RA5 input pin disabled, MCLR pin enabled)

// FICD
#pragma config ICS = PGx3               // ICD Pin Placement Select bits (EMUC/EMUD share PGC3/PGD3)

/****************************** END CONFIGURATION *****************************/

/*********** Useful defines and macros ****************************************/
#define OMEGA_MIN_TO_FAST_SINE      2000
#define OMEGA_MAX_TO_NORMAL_SINE    (OMEGA_MIN_TO_FAST_SINE >> 1)

#define LD_VOLTAGE_1_AN 0x0101
#define LD_VOLTAGE_0_AN 0x0202
#define HZ_VOLTAGE_1_AN 0x0f0f
#define HZ_VOLTAGE_2_AN 0x1010
#define CURRENT_VOLTAGE_AN  0x1414

#define NUM_OF_SAMPLES 200

/*********** Variable Declarations ********************************************/
volatile q16angle_t omega = 1333;

volatile q15_t loadVoltageL = 0;
volatile q15_t loadVoltage[NUM_OF_SAMPLES] = {0};
volatile q15_t loadCurrent[NUM_OF_SAMPLES] = {0};
volatile q15_t sampleIndex = 0;
volatile q15_t hz1Voltage = 0;
volatile q15_t hz2Voltage = 0;

/*********** Function Declarations ********************************************/
void initOsc(void);
void initLowZAnalogOut(void);
void initInterrupts(void);
void initPwm(void);
void initAdc(void);
void setDutyCycleHZ1(q15_t dutyCycle);
void setDutyCycleHZ2(q15_t dutyCycle);

void timed(void);

/*********** Function Implementations *****************************************/
int main(void) {
    /* setup the hardware */
    initOsc();
    initLowZAnalogOut();
    initInterrupts();
    initPwm();
    initAdc();
    
    FRM_init();
    
    /* initialize the task manager */
    TASK_init();
    
    /* set the initial duty cycles */
    setDutyCycleHZ1(16384);
    setDutyCycleHZ2(8192);
    
    /* add necessary tasks */
    TASK_add(&timed, 4000);
    
    TASK_manage();
    
    return 0;
}

void timed(void){
    uint8_t data[] = {0,0xf7,0,0x7f,0,0xf6,6,7};
    
    uint16_t length = 8;
    
    FRM_push(data, length);
}

void initOsc(void){
    /* for the moment, initialize the oscillator
     * on the highest internal frequency;  this will likely
     * change soon */
    CLKDIV = 0;

    return;
}

void initLowZAnalogOut(void){
    /* both DACs must be initialized, but do not
     * need to be connected to external pins */
    
    /* Pin config:
     * according to datasheet, opamp pins
     * should be in 'analog' mode as 'inputs' */
    DIO_makeInput(DIO_PORT_B, 3);
    DIO_makeInput(DIO_PORT_B, 15);
    DIO_makeInput(DIO_PORT_B, 14);
    DIO_makeAnalog(DIO_PORT_B, 3);
    DIO_makeAnalog(DIO_PORT_B, 15);
    
    
    /* DAC config:
     * trigger on write, DAC available to internal
     * peripherals only, sleep behavior doesn't matter,
     * left-aligned input (fractional) */
    DAC1CON = DAC2CON = 0x0802;
    //DAC1CONbits.DACOE = DAC2CONbits.DACOE = 1;
    DAC1CONbits.DACEN = DAC2CONbits.DACEN = 1; // enable after configured
    
    /* Opamp config:
     * higher bandwidth/response, voltage follower config,
     * positive input connected to DAC */
    AMP1CON = AMP2CON = 0x002D;
    AMP1CONbits.AMPEN = AMP2CONbits.AMPEN = 1;
    
    return;
}

void initInterrupts(void){
    /* configure the global interrupt conditions */
    /* interrupt nesting disabled, DISI instruction active */
    INTCON1 = 0x8000;
    INTCON2 = 0x4000;
    
    /* timer interrupts */
    T1CON = 0x0000;
    PR1 = 1000;          // based on 12MIPS, 48samples/waveform, 1kHz waveform
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CONbits.TON = 1;
    
    return;
}

void initPwm(void){
    // use OC1C (pin 21, RB10) and OC2A (pin 22, RB11)
    DIO_makeDigital(DIO_PORT_B, 10);
    DIO_makeDigital(DIO_PORT_B, 11);
    
    /* Initialize MCCP module
     *  */
    /* period registers */
    CCP1PRH = CCP2PRH = 0;
    CCP1PRL = CCP2PRL = 1024;
    
    CCP1CON1L = CCP2CON1L = 0x0005;
    CCP1CON1H = CCP2CON1H = 0x0000;
    CCP1CON2L = CCP2CON2L = 0x0000;
    CCP1CON2H = 0x8400; // enable output OC1C
    CCP2CON2H = 0x8100; // enable output 0C2A
    CCP1CON3L = CCP2CON3L = 0;  // dead time disabled
    CCP1CON3H = CCP2CON3H = 0x0000;
    
    CCP1CON1Lbits.CCPON = CCP2CON1Lbits.CCPON = 1;
    
    /* duty cycle registers */
    CCP1RA = CCP2RA = 0;
    CCP1RB = CCP2RB = 0;
    
    return;
}

void initAdc(void){
    /* set up the analog pins as analog inputs */
    DIO_makeInput(DIO_PORT_A, 1);
    DIO_makeInput(DIO_PORT_B, 0);
    DIO_makeInput(DIO_PORT_B, 4);
    DIO_makeInput(DIO_PORT_A, 4);
    DIO_makeInput(DIO_PORT_B, 6);
    
    DIO_makeAnalog(DIO_PORT_A, 1);
    DIO_makeAnalog(DIO_PORT_B, 0);
    DIO_makeAnalog(DIO_PORT_B, 4);
    DIO_makeAnalog(DIO_PORT_A, 4);
    DIO_makeAnalog(DIO_PORT_B, 6);
    
    AD1CON1 = 0x0200;   /* Internal counter triggers conversion
                         * FORM = left justified  */
    AD1CON2 = 0x0000;   /* Set AD1IF after every 1 samples */
    AD1CON3 = 0x0007;   /* Sample time = 1Tad, Tad = 8 * Tcy */
    
    AD1CHS = CURRENT_VOLTAGE_AN;    /* AN1 */
    AD1CSSL = 0;
    
    AD1CON1bits.ADON = 1; // turn ADC ON
    AD1CON1bits.ASAM = 1; // auto-sample
    
    /* analog-to-digital interrupts */
    IFS0bits.AD1IF = 0;
    IEC0bits.AD1IE = 1;
    
    return;
}

void setDutyCycleHZ1(q15_t dutyCycle){
    CCP1RB = q15_mul(dutyCycle, CCP1PRL);
    return;
}

void setDutyCycleHZ2(q15_t dutyCycle){
    CCP2RB = q15_mul(dutyCycle, CCP2PRL);
    return;
}

/**
 * The T1Interrupt will be used to load the DACs and generate the sine wave
 */
void _ISR _T1Interrupt(void){
    static q16angle_t theta = 0;
    static q16angle_t thetaLast = 0;
    thetaLast = theta;
    theta += omega;
    
    DAC1DAT = q15_fast_sin(theta) + 32768;
    DAC2DAT = q15_fast_sin(theta + 32768) + 32768; // theta + 180 deg
    
    IFS0bits.T1IF = 0;
    AD1CON1bits.SAMP = 0;
    
    /* reset sampleIndex on every cycle */
    if((thetaLast > 32768) && (theta < 32768))
        sampleIndex = 0;
    
    return;
}

void _ISR _ADC1Interrupt(void){
    switch(AD1CHS){
        case LD_VOLTAGE_1_AN:
        {
            loadVoltageL = (q15_t)(ADC1BUF0 >> 1);
            AD1CHS = LD_VOLTAGE_0_AN;
            AD1CON1bits.SAMP = 0;

            break;
        }

        case LD_VOLTAGE_0_AN:
        {
            loadVoltage[sampleIndex] = (q15_t)(ADC1BUF0 >> 1) - loadVoltageL;
            AD1CHS = CURRENT_VOLTAGE_AN;
            AD1CON1bits.SAMP = 0;

            break;
        }

        case CURRENT_VOLTAGE_AN:
        {
            loadCurrent[sampleIndex] = (q15_t)(ADC1BUF0 >> 1) - hz1Voltage;

            if(++sampleIndex >= NUM_OF_SAMPLES)
                sampleIndex = 0;

            AD1CHS = HZ_VOLTAGE_1_AN;
            AD1CON1bits.SAMP = 0;
            

            break;
        }

        case HZ_VOLTAGE_1_AN:
        {
            hz1Voltage = (q15_t)(ADC1BUF0 >> 1);
            AD1CHS = HZ_VOLTAGE_2_AN;
            AD1CON1bits.SAMP = 0;

            break;
        }

        case HZ_VOLTAGE_2_AN:
        {
            hz2Voltage = (q15_t)(ADC1BUF0 >> 1);
            AD1CHS = LD_VOLTAGE_1_AN;

            break;
        }

        default: {}
    }
    
    /* clear the flag */
    IFS0bits.AD1IF = 0;
}
