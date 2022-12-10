#include <msp430.h>

//Pruebaaaa Funciona bien

void eUSCIB0_CS1_set_state(uint8_t a);

void eUSCIB0_SPI_init(){        //(slau367p.pdf p.800)
    UCB0CTLW0 = UCSWRST; //resetea el modulo USCI
    UCB0CTLW0 |= UCSSEL__SMCLK; //Selecciona SMCLK como fuente de reloj (1 MHz)
    UCB0CTLW0 |= UCSYNC; //Configura modo sincrono.
    UCB0CTLW0 |=  UCMODE_0; //Se configura para 3 wire SPI.
    UCB0CTLW0 |= UCMST; //Se configura en modo maestro.
    UCB0CTLW0 |= UCMSB; //Configura MSB primero
    UCB0CTLW0 |= UCCKPH;
    //Configuraci�n de la velocidad
    UCB0BRW = 180; //Velocidad de SMCLK, 1 Mbit/s


    //configura puertos
    PM5CTL0 &= ~LOCKLPM5;
    //Configuraci�n de los GPIO
    P1SEL1 |= BIT6 | BIT7;             //Configura SOMI, MISO //La configuraci�n de puertos esta bien
    P2SEL1 |= BIT2;                    //UCB0CLK

    P1DIR |= BIT0; //Configura P1.0 (LED) como salida;
    P1DIR |= BIT2; //Configura P1.2 como salida (CS)

    P1OUT &= ~BIT0; //Apaga LED.
    P1OUT |= BIT2; // (1.2) CS esta en alto, el esclavo esta desactivado.

    UCB0CTLW0 &= ~UCSWRST; //Pone a funcionar el modulo eUSCIB
}

void eUSCIB0_SPI_writeByte(int dato){           
    while((UCB0STATW & UCBUSY) == UCBUSY){} //esperar mientras el buffer de escritura no este vac�o.
        UCB0TXBUF = dato;
    while((UCB0STATW & UCBUSY) == UCBUSY){}
}

uint8_t eUSCIB0_SPI_readByte(){
    uint8_t dato = 0;
    _delay_cycles(100);
    while((UCB0STATW & UCBUSY) == UCBUSY){} //esperar mientras el buffer de escritura no este lleno.
    dato = UCB0RXBUF;
    while((UCB0STATW & UCBUSY) == UCBUSY){} //esperar mientras el buffer de escritura no este lleno.
    return dato;
}

void eUSCIB0_CS1_set_state(uint8_t a){      //El módulo SPI del MSP solo manda tramas de 8 bits
                                            //y se levanta en automático el CS, entonces creamos
                                            //uno artificial.
    if (a==0) P1OUT &= ~BIT2; //CS = LOW
    if (a==1) P1OUT |= BIT2; //CS = HIGH
}
