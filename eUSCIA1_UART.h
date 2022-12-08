#include <msp430.h>
//Drivers de comunicacion UART para manipular el bootloader del STM32
//Variables auxiliares para la recepcion de datos
int eUSCIA1_UART_availableData = 0;
int eUSCIA1_UART_data = 0;

void eUSCIA1_UART_Init(){
    //Configura puertos (pag.90) MSPFR59 datasheet
    P2SEL0 &= ~(BIT5 | BIT6);  //P2SEL0.x = 0
    P2SEL1 |= BIT5 | BIT6;     //P2SEL1.x = 1; Selecciona la funcion de UART en P2.5 y P2.6
                               //(pag.369) manual slau367p.pfd ; (pag.13) msp430fr5969.pdf

    //El reloj se configura en TimerA0
    UCA1CTLW0 = UCSWRST;       //Deshabilita modulo de harware para poder configurar
                               //(pag.788) manual slau367p.pdf BIT0

    //ConfiguraUART
    UCA1CTLW0 |= UCSSEL__SMCLK; //Selecciona SM subsytem master clock como reloj del modulo UART 1 MHz/8
                                //(pag.787) manual slau367p.pdf BIT 7 10b=SMCLK

    UCA1CTLW0 |= UCMODE_0 | UCPAR | UCPEN; //| UCMSB; //Configura UART habilitando la paridad
                                                      //y configurandola como paridad par. BIT 14,15
                                                      //(pag.787) manual slau367p.pdf

    //Configura baudrate a 9600;
    //UCOS16 = 1; UCBRx = 6; UCBRF = 8 = 1000; UCBRSx = 0x20 = 100000;
    UCA1BRW = 6;                //(pag.789) manual slau367p.pdf preescalador del reloj maestro elegido
    UCA1MCTLW = UCOS16 | UCBRS5 | UCBRF3; //(pag.789) manual slau367p.pdf selecciona patrones de modulacion
    UCA1CTLW0 &= ~UCSWRST;      //Habilita modulo de hardware eUSCI

    //Habilita interrupciones
    UCA1IE |= UCRXIE;           //Habilita interrupción de recepción (pag. 812) manual slau367p.pdf
    __enable_interrupt();       //Habilita la las interrupciones enmascarables.
                                //(pag.52) manual slau367p.pdf
    UCA1IFG &= ~UCRXIFG;        //Limpia la bandera de UCA1RX que se activa cuando
                                //Se recibe un caracter completo (pag.813) manual slau367p.pdf
}

void eUSCIA1_UART_send(int data_Tx){
    timer_Wait_ms(10);
    UCA1TXBUF = data_Tx;        //Dato a enviar (pag.791) manual slau367p.pdf
}

int eUSCIA1_UART_receiveACK_eerase(){
    while(eUSCIA1_UART_availableData == 0){}
        eUSCIA1_UART_data = UCA1RXBUF & 0xFF; //Byte recibido en el registro
                                              //de desplazamiento (pag.791) manual slau367p.pdf
    eUSCIA1_UART_availableData = 0;
    return eUSCIA1_UART_data;
}

int eUSCIA1_UART_receive(){
    timer_Wait_ms(10); //Espera 10 ms
    if(eUSCIA1_UART_availableData == 1)        //Si el buffer tiene un valor.
        eUSCIA1_UART_data = UCA1RXBUF & 0xFF;  //Se recibe el byte 0x79 de confirmacion

    else eUSCIA1_UART_data = 0x00;             //Si no, llena el dato en ceros por defecto
    eUSCIA1_UART_availableData = 0;            //para no quedarse esperando en la misma
    return eUSCIA1_UART_data;                  //instruccion. Nota de aplicación AN3155 (pag. 7,8)
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
    eUSCIA1_UART_availableData = 1;
    UCA1IFG = 0;                                //limpia bandera de interrupcion pendiente(pag. 813) slau367.pdf;
}

