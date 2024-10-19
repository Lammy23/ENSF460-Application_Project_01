/*
 * File:   IOs.c
 * Author: 
 *      Robert Meyer
 *      Paulo Pineda
 *      Olamikun Aluko
 *
 * Created on September 20, 2024, 3:28 PM
 */


#include "xc.h"
#include "IOs.h"
#include "UART2.h"

extern uint8_t PB1_pressed;
extern uint8_t PB2_pressed;
extern uint8_t PB3_pressed;
extern uint8_t anyPressed;
extern uint8_t minutes;
extern uint8_t seconds;
extern uint8_t timePB2Pressed;
extern uint8_t clkStatus;
extern uint8_t DEBUG;

void IOinit() {
    //T3CON config
    T2CONbits.T32 = 0; // operate timer 2 and 3 as separate 16 bit timers

    T3CONbits.TCKPS = 0b10; // set prescaler to 1:64
    T3CONbits.TCS = 0; // use internal clock
    T3CONbits.TSIDL = 0; //operate in idle mode
    IPC2bits.T3IP = 2; //7 is highest and 1 is lowest pri.
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1; //enable timer interrupt
    PR3 = 1000; // set the count value for 0.5 s (or 500 ms)
    TMR3 = 0;
    T3CONbits.TON = 0; // Leave the timer off until needed

    // Same initialization logic as timer 3, except priority is one higher
    T2CONbits.TCKPS = 0b10;
    T2CONbits.TCS = 0;
    T2CONbits.TSIDL = 0;
    IPC1bits.T2IP = 3;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    PR2 = 1000;
    TMR2 = 0;
    T2CONbits.TON = 0;

    // Same initialization logic as timer 3, except priority is one higher
    T1CONbits.TCKPS = 0b10;
    T1CONbits.TCS = 0;
    T1CONbits.TSIDL = 0;
    IPC0bits.T1IP = 3;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    PR1 = 1000;
    TMR1 = 0;
    T1CONbits.TON = 0;


    /* Let's set up some I/O */
    TRISBbits.TRISB8 = 0;
    LATBbits.LATB8 = 0;

    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;

    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;

    TRISBbits.TRISB2 = 1;
    CNPU1bits.CN6PUE = 1;
    CNEN1bits.CN6IE = 1;

    // Initializing CN interrupt
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
}

void IOcheck() {
    if (anyPressed && clkStatus == 2) clkStatus = 0; // stays in status 2 until any PB is pressed (status 2 is finished)
    if (clkStatus == 0) { // Status SET
        LATBbits.LATB8 = 0; // Turn off LED
        
        if (PB1_pressed) { // If PB1_pressed start timer to check if held
            PR3 = 3906;
            TMR3 = 0;
            T3CONbits.TON = 1;
        }
        
        if (PB2_pressed) { // If PB2_pressed start timer to check if held
            PR3 = 3906;
            TMR3 = 0;
            T3CONbits.TON = 1;

        } else timePB2Pressed = 0; // Resets timePB2Pressed if released
        
        if (PB3_pressed) { // If PB3_pressed start timer to check if held
            PR2 = 11719;
            TMR2 = 0;
            T2CONbits.TON = 1;

        } else if (T2CONbits.TON == 1) { // If released before timer reaches three seconds
            T2CONbits.TON = 0; // stop timer
            if (minutes == 0 && seconds == 0) { // Do nothing if already reset
            } else { // If minutes and seconds not already reset
                if(DEBUG) Disp2String("Take me to status 1\n\r");
                clkStatus = 1; // Move to CNT status

            }
        }
        
        if (!anyPressed) { // If nothing is pressed turn off timer 3
            T3CONbits.TON = 0;
        }
    }
    if (clkStatus == 1) { // CNT status
        if (T1CONbits.TON == 0) { // If timer1 is off start it
            PR1 = 3900;
            TMR1 = 0;
            T1CONbits.TON = 1;
        }

        if(DEBUG) Disp2String("Status1\n\r");
        if (PB3_pressed) { // If PB3 is pressed pause and return to SET status and turn off timer1
            if(DEBUG) Disp2String("Going back to status 0\n\r");
            clkStatus = 0;
            TMR1 = 0;
            T1CONbits.TON = 0;
        }
        
        if (minutes == 0 && seconds == 0){ // If clock completes move to FIN status and turn on LED and turn off timer1
            TMR1 = 0;
            T1CONbits.TON = 0;
            LATBbits.LATB8 = 1;
            clkStatus = 2;
        }
    }
    
}
