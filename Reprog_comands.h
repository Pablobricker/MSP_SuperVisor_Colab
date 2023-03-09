#include <msp430.h>
#include "eUSCIA0_UART.h"

static unsigned int i;
static int ACK;
static int StartByte;
//Checksum de datos propio que se escribe en FRAM
uint8_t data_chk;
//Bandera de recepcion exitosa
uint8_t Rx_done; 
uint32_t FRAM_Start_Write = 0x00000000;     //pendiente
uint8_t Program_NBytes;                     //pendiente


//Proceso de Reprogramacion
//Estructura Tx del PC
// Add_MSB -- Add_LSB -- NumDatos -- Dato1 -- Daton -- Checksum

//En esta funcion se hace el manejo de direcciones de FRAM y de Flash
//Trabaja por poleo cuando toca recibir byte de inicio y final

void Start_Reprog(){
    

    if (eUSCIA0_UART_receiveACK_eerase() == 0x0F){      //Startbyte
            
            
            receive_PC_Frame();
            //Aqi ya debio haber cambiado la bandera
            while (Rx_done == 0){
            //Datos Corrompidos
            eUSCIA0_UART_send(0x7F);        //NACK

            receive_PC_Frame();
        
        }
        //Empieza a escribir el codigo recibido en FRAM
        
            FRAM_REPROG(FRAM_Start_Write);              //Probar la parte de FRAM**
            
    }
    //Recibido el primer paquete de datos
            
            while (eUSCIA0_UART_receiveACK_eerase()=!0xF0){
                FRAM_Start_Write = FRAM_Start_Write + Program_NBytes;
                //Listo para recibir la Siguiente linea de codigo
                eUSCIA0_UART_send(0x79);      //ACK
                receive_PC_Frame();
                while (Rx_done == 0){
            //Datos Corrompidos
            eUSCIA0_UART_send(0x7F);        //NACK

            receive_PC_Frame();
        
        }
        //Empieza a escribir el codigo recibido en FRAM
        
            FRAM_REPROG(FRAM_Start_Write);              
            }
            

    
}

//Recibir datos en crudo
void receive_PC_Frame(){
    while(Rx_done==0){                  //Check
    //Repetir la trasmision
            //uint8_t Program_NBytes;
        uint8_t Hx_address_1;
        uint8_t Hx_address_2;
        uint8_t CurrentChecksum;
            //Recibe Dirección
            Hx_address_1 = eUSCIA0_UART_receiveACK_eerase();
            Hx_address_2 = eUSCIA0_UART_receiveACK_eerase();
            //Recibe número de bytes
            Program_NBytes = eUSCIA0_UART_receiveACK_eerase();
            //Calcula lo que llevamos recibido para el checkssum total
            CurrentChecksum = Hx_address_1 ^ Hx_address_2 ^ Program_NBytes;
            receive_ProgramdataRx(Program_StoreAdd,Program_NBytes,CurrentChecksum);
    }
}

//Recibir los datos que se guardan en FRAM
void receive_ProgramdataRx(uint8_t* arrayRx, int DataSize, uint8_t CurrentChecksum){
Rx_done=0; 
uint16_t checksum = CurrentChecksum;
uint8_t Hx_chksum;
data_chk=DataSize;
    for (i=0;i<=DataSize-1;i++){                        //Aqui se puede cambiar la posicion en la que se va a leer el checsum -1,-2-3
        *(arrayRx+i) = eUSCIA0_UART_receiveACK_eerase();   //*(array-1-i)
    checksum= checksum ^ *(arrayRx+i);
    data_chk=data_chk ^ *(arrayRx+i); }

    Hx_chksum = eUSCIA0_UART_receiveACK_eerase();   //Checksum recibido 
    //Comprobacion del checksum total
    if (checksum == Hx_chksum){                    //HAcer el propio checsum

        Rx_done =1;
        

    }
    //La comprobación del checksum se puede hacer sumando todos los bytes de datos incluyendo el checksum
    //El LSB del resultado debe ser 0x00
}

//Tratamiento de datos para escribir FRAM
void FRAM_REPROG(uint32_t FRAM_Start){

    FRAM_write((FRAM_Start>>16)&0xFF,(FRAM_Start>>8)&0xFF,FRAM_Start&0xFF,&Program_NBytes,1);
            FRAM_Start++;
        //Luego el contenido de los bytes a escribir    INCLUYE DIRECCION Y RECORDTYPE???
            FRAM_write((FRAM_Start>>16)&0xFF,(FRAM_Start>>8)&0xFF,FRAM_Start&0xFF,Program_StoreAdd,Program_NBytes);
        
}


//funcion con la intension de vaciar el buffer vector donde van los datos de programa 
//en caso de que se hallan corrompido y se tenga que volver a escribir.
//O en caso de recibir un nuevo vector de diferente tamaño
//void receive_reset(uint8_t* arrayRx, int DataSize){
//    
//    for(i=0; i<=DataSize-1; i++){
//        *(arrayRx+i)=0;
//    }
//}






