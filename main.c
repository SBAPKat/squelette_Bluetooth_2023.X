
/*
 * File:   main.c
 * programmation de l'UART du PIC18F45K20 par interruption
 * Author: MD TRAN
 * TP EN51: Mise en oeuvre technologie ZigBee et Bluetooth
 * Created on 13 septembre 2018, 08:51
 * Modifié le 24/11/2023
 *  - Détection message : <LF>OK<CR> idem \nOK\r\n
 *              trame complète : <CR><LF>OK<CR><LF>
 *  - Détection message : CONNECT  "0012-6F-00C726"<CR>
 *              trame complète : <CR><LF>"0012-6F-00C726"<CR><LF>
 */

#include <xc.h>
#include "tmd_lib_v0.h"     //Bibliothèque de MD Tran
#include "configFuse.h"     //Configuration du PIC
#define _XTAL_FREQ 10000000 //Quartz 10 MHz utlisé

//Déclaration des variables globales
bit flag_CONNECT=0;
bit flag_OK=0;
bit flag_DISCONNECT=0;
//suite de vos variables
//............



//Paramètres de gestion UART
#define buffin_size 60              // taille du buffer
char Data=0, Data_1=0, Data_2=0;    // initialisation tampon départ 
char buffin[buffin_size];           // buffer de réception usart
int  rw_ptr = 0;                    // pointeur réception usart
int NbrInt=0;
short int TrameStart, TrameFin, TrameERROROK;// drapeaux de détection
char Start,Start1,Start2;           // caractères début trame

//Déclaration des prototypes
void Delay1Second();
void Delay200_ms();
void Delay100_ms();
void wait_OK();
void wait_CONNECT();        // attente connexion

//vos prototypes
void wait_DISCONNECT();     // routine à faire
void wait_PASSKEY_CFM();    // routine à faire
void wait_PASSKEY_DSP();    // routine à faire
void wait_PASSDSP_REQ();    // routine à faire

//Programme principal
void main(void)
{
    //déclaration des variables locales
    int i=0;    
    /* Initialisation Timer, ports et UART*/
    iniPorts();
    InitUSART_19200();

    /*Configuration de l'UART en interruption*/
    RCIF = 0; //reset RX pin flag
    RCIP = 0; //Not high priority
    RCIE = 1; //Enable RX interrupt
    PEIE = 1; //Enable pheripheral interrupt (serial port is a pheripheral)
    GIE = 1; //interruption générale

    //Reset matérield du module bluetooth
    //Broche nRESET reliée au port RA3 du PIC18f4580
        RBPU=0;                     //PORTB pull-ups are enabled by individual port latch values
        TRISBbits.RB3 = 0;          //set RB3 as Output
        //nRESET=0 pendant 200ms
        LATB3 = 0;          //nReset=0
        Delay200_ms();
        LATB3 = 1;          //nReset=0
    LATA4 = 0;          //LED D6 allumée
    for(i=0;i<3;i++)
    {
        Delay1Second();
    }
    LATA4 = 1;          //LED D6 étiente : fin démarrage du système


    //1. Envoi de commande AT
    printf("at+addr?\r");                 //@Mac du composant ?
    //2. Attente réponse "\nOK\r"
    wait_OK(); 
 
     printf("at+dcov+\r");                 //@Mac du composant ?
    wait_OK(); 
    
    //suite de votre programme est ici
    //.........
    wait_CONNECT();
    printf("n'importe koi");  
    

    
    //boucle infinie pour test
    while(1){
        LATA4 = 0;      //la LED d6 est connecté sur le port RA4
        __delay_ms(20); //délai de 20 ms
        LATA4 = 1;
        Delay1Second(); //délai de 1s
    }
}


/* Fonction d'interruption */
void interrupt isr(void)
{
    int i=0; 
    if(PIR1bits.RCIF == 1) {    //interruption déclenchée par l'UART
        PIR1bits.RCIF = 0;      //initialisation de la valeur du drapeau
        LATA4=~LATA4;           //inversion l'état de la Led pour test
        Data= RCREG;            //lecture de la valeur du buffer RX
        NbrInt++;               //incrémentation du nombre de caractères reçus

        //Détection début trame
        //Exemple ( Data=='K' && Data_1=='O' && Data_2=='\n'))
        if( Data==Start2 && Data_1==Start1 && Data_2==Start) {
             TrameStart=1;
             buffin[1]=Data_2;
             buffin[2]=Data_1;
             rw_ptr=2;
             //debug : printf("debut Trame detectee\n");
            }
        
        if (TrameStart) {
             rw_ptr++;
             buffin[rw_ptr]=Data;
             //Détection fin trame '\r' (Carriage Return)
             if( Data == '\r') {    
                 TrameStart=0;
                 TrameFin=1;
                 switch (rw_ptr) {

                    case 4: //\nOK\r    4 caractères détectés
                            flag_OK=1;
                    break;

                    case 26: //CONNECT  "0012-6F-00C726"\r soit 26 caractères détectés
                            flag_CONNECT=1;
                    break;

                    case 28: //CONNECT  "0012-6F-00C726"\r soit 26 caractères détectés
                            flag_DISCONNECT=1;
                    break;
                    
                    default: //erreur de détection
                            //printf("Error\r\n");
                            TrameERROROK=1;
                    break; 

                 Data=0;    
                 Data_1=0;
                 }//fin switch

                 //initialisation valeurs du buffer
                 for(i=0; i<=buffin_size ;i++){ 
                    buffin[i]=0;   //reset valeurs
                 }
                 rw_ptr=0;
           }//fin If if( Data == '\r')    
        }//fin If (TrameStart)
     Data_2=Data_1;
     Data_1=Data;
    }//fin de la routine
    
}

void Delay1Second()
{
    int i = 0;
    for(i=0;i<100;i++)
    {
         __delay_ms(10);
    }
}

void Delay200_ms()
{
    int i = 0;
    for(i=0;i<20;i++)   //200ms
    {
         __delay_ms(10);
    }
}

void Delay100_ms()
{
    int i = 0;
    for(i=0;i<10;i++)   //200ms
    {
         __delay_ms(10);
    }
}

void wait_OK()
{
    //attente réponse :"\r\nOK\r\n"
    Start='\n';
    Start1='O';
    Start2='K';
    flag_OK=0;
    LATA4 = 0;  //led allumée
    while(!flag_OK){   //prévoir une sortie de la boucle avec une variavle de retour!
    }
    flag_OK=0;
    LATA4 = 1;  //led éteinte, confirmation de réception
}

void wait_CONNECT(){
    //attente connexion avec réponse : CONNECT  "0012-6F-00C726"\r
    Start='C';
    Start1='O';
    Start2='N';
    flag_CONNECT=0;
    while(!flag_CONNECT){   //prévoir une sortie de cette boucle
        flag_CONNECT=0;
        LATA4 = 0;
        Delay200_ms();
        LATA4 = 1;
        Delay200_ms();
     }

}

// à faire
void wait_DISCONNECT(){  
    Start='D';
    Start1='I';
    Start2='S';
    flag_DISCONNECT=0;
    while(!flag_DISCONNECT){   //DISCONNECT"0012-6F-00C698"
        flag_DISCONNECT=0;
        LATA4 = 0;
        Delay200_ms();
        LATA4 = 1;
        Delay200_ms();
     }//votre code
}     
void wait_PASSKEY_CFM(){  
    //votre code
}         
void wait_PASSKEY_DSP(){ 
    //votre code
}         
void wait_PASSDSP_REQ(){ 
    //votre code
}        

