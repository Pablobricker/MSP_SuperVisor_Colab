#include <msp430.h>
#include <stdint.h>

uint8_t dataX[4];
#define NumDataRx 4
uint32_t command_dataRx[NumDataRx];              //Aqui antes estaban declarados como uint32_t
uint32_t dataW[]={0x10,0x17,0x22,0x33};

#include <TIMERA0.h>
#include <eUSCIA1_UART.h>
#include <STMF407xx_bootloaderCommands.h>
#include <eUSCIA0_UART.h>
#include <eUSCIB0_SPI.h>
#include <FRAM_commands.h>
#include <RTCB.h>





void receivePrincipalComputerData(){
    uint8_t dataCheck;
    uint8_t checksum;
    //mientras la entrada de control sea 1 :
    while (P4IN == BIT2){
        dataX[0] = eUSCIA0_UART_receive();//Se recibe dato 1
        dataX[1] = eUSCIA0_UART_receive();//Se recibe dato 2
        dataX[2] = eUSCIA0_UART_receive();//Se recibe dato 3
        dataX[3] = eUSCIA0_UART_receive();//Se recibe dato 4
        checksum = eUSCIA0_UART_receive();//Se recibe checksum
        //comprobar checksum
        dataCheck = dataX[0] + dataX[1] + dataX[2] + dataX[3] + checksum;
        //Si los datos se recibieron correctamente data check tiene que ser 0xFF
        if(dataCheck == 0xFF){
            eUSCIA0_UART_send(0X79); //contesta bit de ACK
            //La computadora principal debera de enviar los cuatro bytes siguientes
        }else{
            eUSCIA0_UART_send(0X7F); //bit NACK
        }
    }

}

int main(void)
{

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	//PM5CTL0 &= ~LOCKLPM5;

	P1_Init();
    timer_Init();
    eUSCIA1_UART_Init();
    //eUSCIB0_SPI_init();

    //P1OUT |= BIT0;
    //P4DIR &= ~BIT2; //habilita pin P4.2 como entrada digital
    //P4IES &= ~BIT2; //la bandera de interrupcion se activa con flanco positivo.
    //P4IE |= BIT2;   //Activa la interrupci�n en P4.2

    //RTC_disabling();
    //RTC_setTime(0xA,0x00);
    //RTC_setDate(0x9, 0xA, 0x7E5);
    //RTC_setAlarm(0x01);
    //RTC_enable();
    //_low_power_mode_3();
    //P1OUT &= ~BIT0;

    //FRAM_write(0x02, 0x00,0x00, 4); //Escribe 4 bytes en la localidad 0x00000
    //FRAM_read(0x02, 0x00, 0x00, 4); //Lee 4 bytes de la direcci�n 0x00000
    //x = 5;


    //Comentario pablo


        int FRAM_dataW[]={0x10,0x20,0x30,0x40};
            int FRAM_WdataSize = sizeof(FRAM_dataW)/sizeof(FRAM_dataW[0]);  //sizeof(FRAM_dataW/FRAM_dataW[0]);
            uint16_t FRAM_dataR[4];
            int FRAM_RdataSize = sizeof(FRAM_dataR)/sizeof(FRAM_dataR[0]);  //sizeof(FRAM_dataR/FRAM_dataR[0]);
            //FRAM_RdataSize =4;


            eUSCIB0_SPI_init();
            FRAM_erase(0x00,0x90,0x00,4);
            //FRAM_write(0x00,0x90,0x00,FRAM_dataW,FRAM_WdataSize);
            FRAM_read(0x00,0x90,0x00,FRAM_dataR,FRAM_RdataSize); //Escribe 4 bytes en la localidad 0x00000
//
    static int NumDataTx = sizeof(dataW)/sizeof(dataW[0]);     //sizeof(dataW/dataW[0]); sizeof(dataW);

    ACK = BootloaderAccess();
    eeraseCommand(11);
    //writeMemoryCommand(0x080E,0x0000,dataW,NumDataTx); //Escribe 0x05,0x08,0x2,0x03 en la 0x080E0000
    //readMemoryCommand(0x080E,0x0000,command_dataRx,NumDataRx);
    goCommand(0x0800, 0x0000);

    while(1){
	}
}

#pragma vector = PORT4_VECTOR
__interrupt void PORT4_ISR(void){
    P4IFG = 0; //limpia bandera de interrupcion
    P1OUT ^= BIT0;
    _low_power_mode_off_on_exit();
}
