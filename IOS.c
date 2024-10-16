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

uint8_t TMR2Flag = 0;

extern uint16_t PB1_pressed;
extern uint16_t PB2_pressed;
extern uint16_t PB3_pressed;
extern uint16_t noPressedPrinted;
extern uint16_t anyPressed;
extern uint16_t printStatus;

void IOinit() {
    //T3CON config
    T2CONbits.T32 = 0; // operate timer 2 as 16 bit timer
    T3CONbits.TCKPS = 0b10; // set prescaler to 1:8
    T3CONbits.TCS = 0; // use internal clock
    T3CONbits.TSIDL = 0; //operate in idle mode
    IPC2bits.T3IP = 2; //7 is highest and 1 is lowest pri.
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1; //enable timer interrupt
    PR3 = 1000; // set the count value for 0.5 s (or 500 ms)
    TMR3 = 0;
    T3CONbits.TON = 1;

    /* Let's set up some I/O */
    TRISBbits.TRISB8 = 0;
    LATBbits.LATB8 = 1;
    
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;
    
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;
    
    TRISBbits.TRISB2 = 1;
    CNPU1bits.CN6PUE = 1;
    CNEN1bits.CN6IE = 1;
    
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
}

void IOcheck() {     
    
    // If all are pressed
    if(PB1_pressed && PB2_pressed && PB3_pressed){
        PR3 = 1;
        TMR3 = 0;
        T3CONbits.TON = 1;
        
        // If last printed is not all PBs pressed
        if (printStatus != 123) {
            Disp2String("\rAll PBs pressed\033[K");
            printStatus = 123;
        }
        noPressedPrinted = 0;
    }
    
    // If PB1 and PB2 are pressed
    else if(PB1_pressed && PB2_pressed)
    {
        PR3 = 1;
        TMR3 = 0;
        T3CONbits.TON = 1;
        
        // If last printed is not PB1 and PB2 are pressed
        if (printStatus != 12) {
            Disp2String("\rPB1 and PB2 are pressed\033[K");
            printStatus = 12;
        }
        noPressedPrinted = 0;
    }
    
    // If PB1 and PB3 are pressed
    else if(PB1_pressed && PB3_pressed)
    {
        PR3 = 1;
        TMR3 = 0;
        T3CONbits.TON = 1;
        
        // If last printed is not PB1 and PB3 are pressed
        if (printStatus != 13) {
            Disp2String("\rPB1 and PB3 are pressed\033[K");
            printStatus = 13;
        }
        noPressedPrinted = 0;
    }
    
    // If PB2 and PB3 are pressed
    else if(PB2_pressed && PB3_pressed)
    {
        PR3 = 1;
        TMR3 = 0;
        T3CONbits.TON = 1;
        
        // If last printed is not PB2 and PB3 are pressed
        if (printStatus != 23) {
            Disp2String("\rPB2 and PB3 are pressed\033[K");
            printStatus = 23;
        }
        noPressedPrinted = 0;
    }
    
    // If only PB1 is pressed
    else if (PB1_pressed){
        PR3 = 977; // Calculated value for 0.25 second delay
        TMR3 = 0;
        T3CONbits.TON = 1;
        
        // If last printed is not PB1 pressed
        if (printStatus != 1) {
            Disp2String("\rPB1 pressed\033[K");
            printStatus = 1;
        }
        noPressedPrinted = 0;
    }
    
    // If only PB2 is pressed
    else if (PB2_pressed){
        PR3 = 7812; // Calculated value for 2 second delay
        TMR3 = 0;
        T3CONbits.TON = 1;
        
        // If last printed is not PB2 pressed
        if (printStatus != 2) {
            Disp2String("\rPB2 pressed\033[K");
            printStatus = 2;
        }
        noPressedPrinted = 0;
    }
    
    // If only PB3 pressed
    else if (PB3_pressed){
        PR3 = 15627; // Calculated value for 4 second delay
        TMR3 = 0;
        T3CONbits.TON = 1;
        
        // If last printed is not PB3 pressed
        if (printStatus != 3) {
            Disp2String("\rPB3 pressed\033[K");
            printStatus = 3;
        }
        noPressedPrinted = 0;
    }
    
    // If nothing is pressed and Nothing Pressed has not been printed
    else if (!anyPressed && !noPressedPrinted) {
        Disp2String("\rNothing pressed\033[K");
        LATBbits.LATB8 = 0;
        T3CONbits.TON = 0;
        noPressedPrinted = 1;         
        printStatus = 0;
    }
}