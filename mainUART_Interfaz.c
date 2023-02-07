#include <msp430.h>
#include <stdint.h>

uint8_t dataX[4];
#define NumDataRx 4
uint8_t RxByt;
uint8_t IncAdd;                                  //Pasarlo como parámetro
uint32_t command_dataRx[NumDataRx];              //Aqui antes estaban declarados como uint32_t
uint32_t dataW[]={0x10,0x17,0x22,0x33};

#include <TIMERA0.h>
#include <eUSCIA1_UART.h>
#include <STMF407xx_bootloaderCommands.h>
#include <eUSCIA0_UART.h>
#include <eUSCIB0_SPI.h>
#include <FRAM_commands.h>
#include <RTCB.h>





void receivePrincipalComputerData(uint8_t *IncAdd){
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

            FRAM_write(0x00,0x90,IncAdd,dataX,NumDataRx);

            eUSCIA0_UART_send(0X79); //contesta bit de ACK
            //La computadora principal debera de enviar los cuatro bytes siguientes
        }else{
            eUSCIA0_UART_send(0X7F); //bit NACK
        }
    }

}

int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
        //PM5CTL0 &= ~LOCKLPM5;

    eUSCIA0_UART_Init();

    eUSCIB0_SPI_init();

    //Receive Start Byte

    while(eUSCIA0_UART_availableData){

        RxByt = eUSCIA0_UART_receive();
        if (RxByt == 0x0F){
            IncAdd=0x0;                     //Direccion inicial FRAM donde se guardara
            receivePrincipalComputerData();

        }
        else if(RxByt == 0xF0){
            //Aqui inicia la carga del programa al STM32
            //En proceso...(terminar)****
            //Primero leer de FRAM
            //Escribir el bootloader por uart
            //ACK = BootloaderAccess();
        }
        else{
            IncAdd+=0x04;
            receivePrincipalComputerData();
        }
    }
}
