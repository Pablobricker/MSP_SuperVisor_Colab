#include <msp430.h>

/*Drivers para establecer comunicacion UART con modulo UART1 manual del MSP430 slau367p cap30
https://www.ti.com.cn/lit/ug/slau367p/slau367p.pdf*/

uint8_t eUSCIA1_UART_availableData = 0;
uint8_t eUSCIA1_UART_data = 0;

void eUSCIA1_UART_Init(){
    //Configura puertos.
    P2SEL0 &= ~(BIT5 | BIT6);  //P2SEL0.x = 0 
    P2SEL1 |= BIT5 | BIT6;     //P2SEL1.x = 1; Selecciona la funci�n de UART en P2.5 y P2.6 (pp.369)

    //El reloj se configura en TimerA0

    UCA1CTLW0 = UCSWRST;       //Deshabilita modulo de harware.
    //ConfiguraUART (p.770)
    UCA1CTLW0 |= UCSSEL__SMCLK; //Selecciona SM subsytem master clock como reloj del modulo UART 1 MHz/8
    UCA1CTLW0 |= UCMODE_0 | UCPAR | UCPEN; //| UCMSB; //Configura UART (p.787)
    //Configura baudrate a 9600 (p.779); 
    //UCOS16 = 1; UCBRx = 6; UCBRF = 8 = 1000; UCBRSx = 0x20 = 100000;
    UCA1BRW = 6;      //(p.789)
    UCA1MCTLW = UCOS16 | UCBRS5 | UCBRF3; 
    UCA1CTLW0 &= ~UCSWRST;      //Habilita modulo de hardware eUSCI (p.789)
    //Habilita interrupciones (p.784)
    UCA1IE |= UCRXIE; //Habilita interrupci�n de recepci�n  (p.794)
    __enable_interrupt(); //Habilita la las interrupciones enmascarables
    UCA1IFG &= ~UCRXIFG;//Limpia la bandera de UCA1RX (p.795); 
}

void eUSCIA1_UART_send(int data_Tx){ //(p.771)
    timer_Wait_ms(10);
    UCA1TXBUF = data_Tx; //Dato a enviar (p.791)
}

int eUSCIA1_UART_receiveACK_eerase(){
    while(eUSCIA1_UART_availableData == 0){}
        eUSCIA1_UART_data = UCA1RXBUF & 0xFF;   //(p.791)
    eUSCIA1_UART_availableData = 0;
    return eUSCIA1_UART_data;
}

int eUSCIA1_UART_receive(){
    timer_Wait_ms(10); //Espera 10 ms
    if(eUSCIA1_UART_availableData == 1) //Si el buffer tiene un valor.
        eUSCIA1_UART_data = UCA1RXBUF & 0xFF;  //Se recibe el byte 0x79 de confirmaci�n
    else eUSCIA1_UART_data = 0x00;             //Si no, llena el dato en ceros por defecto
    eUSCIA1_UART_availableData = 0;            //para no quedarse esperando en la misma
    return eUSCIA1_UART_data;                  //instruccion. Nota de aplicaci�n AN3155 (pag. 7,8)
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
    eUSCIA1_UART_availableData = 1;
    UCA1IFG = 0;                                //limpia bandera de interrupcion pendiente(p. 813)
}