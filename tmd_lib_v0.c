/*
 * File:   tmd_lib.c
 * Author: MD TRAN
 *
 * Created on 13 septembre 2017, 08:52
 */


#include <xc.h>
#include "tmd_lib_v0.h"

 
  void InitUSART_19200(void)
{
    TXSTAbits.SYNC = 0;
    TXSTAbits.TXEN = 1;
    TXSTAbits.BRGH = 1;
    RCSTAbits.CREN = 1;
    BRG16 = 0;
    SPBRG = 32;         //justifier cette valeur
    PIE1bits.RCIE = 1;
    RCSTAbits.SPEN = 1;
}
 
 void iniPorts()
 {
    TRISA4 = 0; //RA4 as Ouput PIN
    TRISD6 = 0; //D6 as Ouput PIN
    LATA4 = 0;  //LED ON
    LATD6 = 1;  //D6=1
 }

void putch(char c)
{
    while (PIR1bits.TXIF!=1);
    TXREG=c;
}
 

