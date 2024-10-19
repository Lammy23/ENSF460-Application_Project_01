/*
 * File:   main.c
 * Author: UPDATE THIS WITH YOUR GROUP MEMBER NAMES OR POTENTIALLY LOSE POINTS
 *
 * Created on: USE THE INFORMATION FROM THE HEADER MPLAB X IDE GENERATES FOR YOU
 */

// FBS
#pragma config BWRP = OFF               // Table Write Protect Boot (Boot segment may be written)
#pragma config BSS = OFF                // Boot segment Protect (No boot program Flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Code Flash Write Protection bit (General segment may be written)
#pragma config GCP = OFF                // General Segment Code Flash Code Protection bit (No protection)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC oscillator (FRC))
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-Speed Start-up disabled))

// FOSC
#pragma config POSCMOD = NONE           // Primary Oscillator Configuration bits (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Enable Configuration bit (CLKO output disabled; pin functions as port I/O)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8 MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary oscillator configured for high-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor Selection (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (WDT prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected; windowed WDT disabled)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = ON              // Power-up Timer Enable bit (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default location for SCL1/SDA1 pins)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset set to lowest voltage (1.8V))
#pragma config MCLRE = ON               // MCLR Pin Enable bit (MCLR pin enabled; RA5 input pin disabled)

// FICD
#pragma config ICS = PGx2               // ICD Pin Placement Select bits (PGC2/PGD2 are used for programming and debugging the device)

// FDS
#pragma config DSWDTPS = DSWDTPSF       // Deep Sleep Watchdog Timer Postscale Select bits (1:2,147,483,648 (25.7 Days))
#pragma config DSWDTOSC = LPRC          // DSWDT Reference Clock Select bit (DSWDT uses LPRC as reference clock)
#pragma config RTCOSC = SOSC            // RTCC Reference Clock Select bit (RTCC uses SOSC as reference clock)
#pragma config DSBOREN = ON             // Deep Sleep Zero-Power BOR Enable bit (Deep Sleep BOR enabled in Deep Sleep)
#pragma config DSWDTEN = ON             // Deep Sleep Watchdog Timer Enable bit (DSWDT enabled)

// #pragma config statements should precede project file includes.

#include <xc.h>
#include <p24F16KA101.h>
#include "clkChange.h"
#include "UART2.h"
#include "IOS.h"


uint8_t PB1_pressed = 0;
uint8_t PB2_pressed = 0;
uint8_t PB3_pressed = 0;
uint8_t anyPressed = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
uint8_t timePB2Pressed = 0;
uint8_t clkStatus = 0;
uint8_t DEBUG = 0;

/**
 * You might find it useful to add your own #defines to improve readability here
 */
void printTime() {
    if(!DEBUG) Disp2String("\r\033[K"); // Clears out previous lines and starts a new one
    
    if (clkStatus == 2) { // if in completed status print FIN first
        Disp2String("FIN");
    }
    else if (clkStatus == 0 && (seconds || minutes)) { // if in set status and seconds and minutes are not zero
        Disp2String("SET");
    }
    else if(clkStatus == 1) { // if in counting status
        Disp2String("CNT");
    }
    else if (seconds == 0 && minutes == 0) { // if seconds and minutes are zero and not in completed status
        Disp2String("CLR");
    }
    Disp2Dec(minutes); // display minutes
    Disp2String("m");
    Disp2String(":");
    Disp2Dec(seconds);
    Disp2String("s");
    if (clkStatus == 2) { // if in completed status add -- ALARM to end
        Disp2String(" -- ALARM");
    }
    
    if(DEBUG) Disp2String("\n\r");

}

int main(void) {

    /** This is usually where you would add run-once code
     * e.g., peripheral initialization. For the first labs
     * you might be fine just having it here. For more complex
     * projects, you might consider having one or more initialize() functions
     */



    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */

    newClk(500); // 500kHz Clock

    /* Initializations*/
    IOinit();
    /* Let's set up our UART */
    InitUART2();

    while (1) {
        printTime();// print time everytime before going into idle
        Idle();
        IOcheck(); // perform io checks based on current state

    }

    return 0;
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    // For counting down the timer every second during state CNT
    
    if(DEBUG) Disp2String("T1_Interrupt\n\r");
    IFS0bits.T1IF = 0; // Clear timer 1 flag

    if (seconds == 0) { // if seconds are zero decrement minutes and set seconds to 59
        minutes--;
        seconds = 59;
    } else seconds--; // otherwise decrement seconds as normal
    T1CONbits.TON = 0; // turn off timer 1
}


// Timer 2 interrupt subroutine

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void) {
    // For measuring how long PB3 has been pressed during state SET
    
    if(DEBUG) Disp2String("T2_Interrupt\n\r");
    IFS0bits.T2IF = 0; // Clear timer 2 flag
    TMR2 = 0; // reset TMR2 to zero

    if (PB3_pressed) { // if still pressed when timer interrupts
        minutes = 0; // reset minutes
        seconds = 0; // reset seconds
        if(DEBUG) printTime();
    }

    T2CONbits.TON = 0; // turn off timer 2

}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    // For incrementing seconds and minutes in SET Mode
    
    IFS0bits.T3IF = 0; // clear timer 3 interrupt flag
    TMR3 = 0; // reset TMR3 to zero
    
    if (PB1_pressed && minutes < 59) minutes += 1; // if PB1 is still pressed when interrupt and minutes less than 59: increment minutes
    if (PB2_pressed && seconds < 59) { // if PB2 is still pressed when interrupt and seconds less than 59: increment timePB2Pressed
        timePB2Pressed += 1;

        seconds = (timePB2Pressed >= 5) ? ((seconds / 5) + 1)*5 : seconds + 1; // increment seconds to either next integer or next multiple of 5 based on timePB2Pressed
        if (seconds == 60) seconds = 59; // sets seconds to 59 if previous line sets it to 60
    }
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void) {
    // For checking the state of each button on rising and falling edges of inputs
    
    
    if(DEBUG) Disp2String("CN_Interrupt\n\r");
    if(DEBUG) Disp2Dec(clkStatus);    // If any button is pressed
    if(DEBUG) Disp2String("\n\r");
    
    if (PORTBbits.RB2 == 0 || PORTBbits.RB4 == 0 || PORTAbits.RA4 == 0) { // if any button currently pressed set anyPressed to 1
        anyPressed = 1;
    } else {
        anyPressed = 0;
    }
    IFS1bits.CNIF = 0; // Clear CN interrupt flag

    // Set PB flags to inverse of read value (because of pull down on inputs)
    PB1_pressed = !PORTBbits.RB2; 
    PB2_pressed = !PORTBbits.RB4;
    PB3_pressed = !PORTAbits.RA4;
}