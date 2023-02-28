#include <msp430.h>
int eUSCIA0_UART_availableData = 0;
int eUSCIA0_UART_data = 0;

/*
 * Módulo utilizado: eUSCIA0 (ver SLAU367P-october 2012-Revised April 2020, pp 767)
 * Configuracion del UART
 * - baudrate = 9600
 * - bit de paridad = impar
 * - LSB primero
 * - 8 bits de datos
 * - 1 bit de stop
*/
void eUSCIA0_UART_Init(){
    //Configura puertos despues de un reset.
    //MSPFR5969 datasheet (pag.90)
    PM5CTL0 &= ~LOCKLPM5;
    P2SEL0 &= ~(BIT0 | BIT1);                   //P2SEL0.x = 0
    P2SEL1 |= BIT0 | BIT1;                      //P2SEL1.x = 1; Selecciona la función de UART en P2.1 y P2.0

    //Configura 2.1 Tx y 2.0 Rx
    UCA0CTLW0 = UCSWRST;                        //Deshabilita modulo de harware
    //ConfiguraUART
    UCA0CTLW0 |= UCSSEL__SMCLK;                 //Selecciona SM subsytem master clock como reloj del modulo UART 1 MHz/8
    UCA0CTLW0 |= UCMODE_0 | UCPAR | UCPEN; //| UCMSB; //Configura UART
                                                //(pag.770) manual slau367p.pdf
    //Configura baudrate a 9600;
    //UCOS16 = 1; UCBRx = 6; UCBRF = 8 = 1000; UCBRSx = 0x20 = 100000;
    UCA0BRW = 6;
    UCA0MCTLW = UCOS16 | UCBRS5 | UCBRF3;       //(pag.789) manual slau367p.pdf Control de modulacion de transmision
    UCA0CTLW0 &= ~UCSWRST;                      //Habilita modulo de hardware eUSCI

    //Habilita interrupciones
    UCA0IE |= UCRXIE;                           //Habilita interrupción de recepción
    __enable_interrupt();                       //Habilita la las interrupciones enmascarables
    UCA0IFG &= ~UCRXIFG;                        //Limpia la bandera de UCA0RX
                                                //(pag.784) manual slau367p.pdf
}

void eUSCIA0_UART_send(int data_Tx){
    __delay_cycles(10000);
    //timer_Wait_ms(10);
    UCA0TXBUF = data_Tx; //Dato a enviar (pag.791) manual slau367p.pdf
}

int eUSCIA0_UART_receiveACK_eerase(){
    while(eUSCIA0_UART_availableData == 0){}
        eUSCIA0_UART_data = UCA0RXBUF & 0xFF;       //Byte recibio en el registro
    eUSCIA0_UART_availableData = 0;                 //de desplazamiento
    return eUSCIA0_UART_data;                       //(pag.791) manual slau367p.pdf
}

int eUSCIA0_UART_receive(){
    __delay_cycles(10000);
    //timer_Wait_ms(10); //Espera 10 ms
    if(eUSCIA0_UART_availableData == 1)             //Si el buffer tiene un valor.
        eUSCIA0_UART_data = UCA0RXBUF & 0xFF;       //Se recibe el byte 0x79 de confirmación
    else eUSCIA0_UART_data = 0x00;                  //Si no, llena el dato en ceros por defecto
    eUSCIA0_UART_availableData = 0;                 //para no quedarse esperando en la misma
    return eUSCIA0_UART_data;                       //instruccion. Nota de aplicación AN3155 (pag. 7,8)
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    eUSCIA0_UART_availableData = 1;
    UCA0IFG = 0;                                    //limpia bandera de interrupcion pendiente(pag. 813) slau367.pdf;
}
