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
int Program_NBytes;                     //pendiente
uint16_t Program_StoreAdd[4]= {0x01, 0x02, 0x03, 0x04};
//uint32_t Program_StoreAdd = 0x08000000;     //pendiente

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
            while(Rx_done == 0){}
            FRAM_REPROG(FRAM_Start_Write);              //Probar la parte de FRAM**
            
    }
    //Recibido el primer paquete de datos

    //Repetir el procedimiento hasta que se envie el byte de fin    
            while (eUSCIA0_UART_receiveACK_eerase()&&0xF0==0){
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
            while (Rx_done == 0){}
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
            FRAM_Start+=Program_NBytes;
            FRAM_write((FRAM_Start>>16)&0xFF,(FRAM_Start>>8)&0xFF,FRAM_Start&0xFF,&data_chk,1);
        
}


void masterReprogramationRutine(uint32_t FRAM_initialAddress,uint32_t Flash_initialAddress){
    //Esta funcion asume que el respaldo del programa del master ya ha sido cargado en la FRAM.
    int i;
    uint16_t FRAMvectorBuffer[sizeBufferVector] = {0}; //Inicializa vector en 0.
    uint8_t vectorBuffer[sizeBufferVector] = {0};
    //int FRAMvectorBufferSize = sizeof(FRAMvectorBuffer)/sizeof(FRAMvectorBuffer[0]); 
    unsigned int j;
    uint32_t FRAM_actualAddress = FRAM_initialAddress;
    uint32_t Flash_actualAddress = Flash_initialAddress;
    ACK= BootloaderAccess();
    for (i = 0; i < masterProgramSize; i++) {
        FRAM_read(((FRAM_actualAddress)>>16)&0xFF,((FRAM_actualAddress)>>8)&0xFF, FRAM_actualAddress & 0xFF, FRAMvectorBuffer, sizeBufferVector);
        //Ejecutar alguna rutina para verificar la integridad de los datos (No se tiene que desarrollar ahora).
        //Hacer la conversion de 16 a 8 bits para que se puedan enviar bien los datos
        //Para mas optimizazion modificar las funciones de escritura y lectura de la FRAM a 8 bits
        //Aunque es poco probable que esto suceda ya que se necesita vaciar o llenar el buffer SPI de 16 bits para que la funcion termine.
        for (j=0; j<=sizeBufferVector; j++){
            vectorBuffer[j] = FRAMvectorBuffer[j]&0xFF;
        }
        
        writeMemoryCommand(((Flash_actualAddress)>>16)&0xFFFF,(Flash_actualAddress)&0xFFFF , vectorBuffer, sizeBufferVector);
        FRAM_actualAddress = FRAM_actualAddress + sizeBufferVector;
        Flash_actualAddress = Flash_actualAddress + sizeBufferVector;
    }
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


void receivePrincipalComputerData(uint8_t *IncAdd){
    uint8_t dataCheck;
    uint8_t checksum;
    uint32_t FRAM_nextWAddress;
    uint8_t dataX[NumDataRx];
    //mientras la entrada de control sea 1 :
    while (P4IN == BIT2){//??????
        dataX[0] = eUSCIA0_UART_receive();//Se recibe dato 1
        dataX[1] = eUSCIA0_UART_receive();//Se recibe dato 2
        dataX[2] = eUSCIA0_UART_receive();//Se recibe dato 3
        dataX[3] = eUSCIA0_UART_receive();//Se recibe dato 4
        checksum = eUSCIA0_UART_receive();//Se recibe checksum
        //comprobar checksum
        dataCheck = dataX[0] + dataX[1] + dataX[2] + dataX[3] + checksum;
        //Si los datos se recibieron correctamente data check tiene que ser 0xFF
        if(dataCheck == 0xFF){
            FRAM_nextWAddress=FRAM_startAddress+IncAdd;
            FRAM_write((FRAM_nextWAddress>>24)&0xFF,(FRAM_nextWAddress>>16)&0xFF,FRAM_nextWAddress&0xFF,dataX,NumDataRx);

            eUSCIA0_UART_send(0X79); //contesta bit de ACK
            //La computadora principal debera de enviar los cuatro bytes siguientes
        }else{
            eUSCIA0_UART_send(0X7F); //bit NACK
        }
    }

}





