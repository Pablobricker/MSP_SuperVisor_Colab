#include <msp430.h>

//Drivers para comunicacion UART con la memoria Flash del UC maestro
//Notas de aplicacion AN3155 y AN2606
//https://stm32duinoforum.com/forum/resource/en/application_note/cd00264342.pdf
//https://www.st.com/resource/en/application_note/cd00167594-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf


//Lista de comandos Nota AN3155 (p.7)
#define GET                 0x00
#define GET_V_RPS           0x01
#define GET_ID              0x02
#define READ_MEMORY         0x11
#define GO                  0x21
#define WRITE_MEMORY        0x31
#define E_ERASE             0x44
#define WRITE_PROTECT       0x63
#define WRITE_UNPROTECT     0x73
#define READOUT_PROTECT     0x82
#define READOUT_UNPROTECT   0x92
#define GET_CHEKSUM         0xA1

uint32_t complement_command;

static unsigned int i;
static int ACK;

<<<<<<< HEAD
//Puerto encargado de establecer los niveles para los pines de acceso al bootloader
=======
//Esta funcion habilita puertos GPIO para la secuencia del bootloader en los pines especiales
>>>>>>> origin/pablo
void P1_Init(){
    PM5CTL0 &= ~LOCKLPM5;   //Disable the GPIO power-on default high-impedance mode (slau367p.pdf, p.639)
    P1DIR |= BIT0 | BIT3 | BIT4 | BIT5; //Los puertos definidos son salidas (slau367p.pdf p.389)
    P1OUT |= BIT5;  //Reset siempre esta en alto.
}

<<<<<<< HEAD
//Metodo de acceso a Bootloader
//Nota AN2606 (p.127)
//Patron 1 (p.29)
int BootloaderAccess(void){
=======
//Esta funcion lleva a cabo la secuencia de acceso al bootloader
int BootloaderAccess(void){     //(Nota AN2606 Tabla 2 Patron 1 p.24,109)
>>>>>>> origin/pablo
    P1OUT = BIT3;    //Se realiza la secuencia de bootloader y reset.
    timer_Wait_ms(500);
    P1OUT = BIT3 | BIT5; //Sale del reset manteniendo secuencia bootloader
    timer_Wait_ms(500);
    //P1OUT &= ~BIT3;
    P1OUT = BIT5; //Los puertos vuelven a su estado original
    eUSCIA1_UART_send(0x7F);
    ACK = eUSCIA1_UART_receive();
    return ACK;
}

static void sendCommand(int command){   //(Nota AN3155 p.5)

    complement_command = ~command;

    //Env�a comando con su complemento
    eUSCIA1_UART_send(command);
    eUSCIA1_UART_send(complement_command);

    //Espera bit de ACK
    ACK = 0;
    ACK = eUSCIA1_UART_receive();
}

//Esta funcion recibe un numero arrayRxSize de bytes y los guarda en un arreglo definido por el apuntador
//sigue trabajando a 32 bits, aun no se prueban con cambiaro a 8
static void receiveCommand_dataRx(uint32_t* arrayRx2, int arrayRxSize2){
    arrayRx2 = arrayRx2+arrayRxSize2-1;
    for (i=0;i<=arrayRxSize2-1;i++)
        *(arrayRx2-i) = eUSCIA1_UART_receiveACK_eerase();   //*(array-1-i)
}

static void send_startAddress(int ADDRESS_MSB,int ADDRESS_LSB){ 
    //Esta  funcion es identica a send_4bytes_wChecksum()
    //Para ver como funciona ver funciona ver send_4bytes_wChecksum()   
    int ADDRESS_1 = (ADDRESS_MSB & 0x0000FF00) >> 8;
    int ADDRESS_2 = (ADDRESS_MSB & 0x000000FF);
    int ADDRESS_3 = (ADDRESS_LSB & 0x0000FF00) >> 8;
    int ADDRESS_4 = (ADDRESS_LSB & 0x000000FF);

    int checksum = ADDRESS_1 ^ ADDRESS_2 ^ ADDRESS_3 ^ ADDRESS_4;   // Checksum Nota AN3155 p.18
                                                    //


    eUSCIA1_UART_send(ADDRESS_1);
    eUSCIA1_UART_send(ADDRESS_2);
    eUSCIA1_UART_send(ADDRESS_3);
    eUSCIA1_UART_send(ADDRESS_4);
    eUSCIA1_UART_send(checksum);

    ACK = 0x00;
    ACK = eUSCIA1_UART_receive();
}

static void send_4bytes_wChecksum(int WORD_MSB,int WORD_LSB){   //Manejo de memoria de 4 bytes (Nota AN2606 p.37)
    /*Ejemplo: Para enviar la palabra de 4 bytes WORD = 0x80706050
     * WORD_MSB = 0x8070
     * WORD_LSB = 0x6050
     */
    int WORD_1 = (WORD_MSB & 0x0000FF00) >> 8; //WORD_1 = 0x80
    int WORD_2 = (WORD_MSB & 0x000000FF);      //WORD_2 = 0x70
    int WORD_3 = (WORD_LSB & 0x0000FF00) >> 8; //WORD_3 = 0x60
    int WORD_4 = (WORD_LSB & 0x000000FF);      //WORD_4 = 0x50

    int checksum = WORD_1 ^ WORD_2 ^ WORD_3 ^ WORD_4; //Obtiene checksum

    //Env�a los 4 bytes y checksum
    eUSCIA1_UART_send(WORD_1);
    eUSCIA1_UART_send(WORD_2);
    eUSCIA1_UART_send(WORD_3);
    eUSCIA1_UART_send(WORD_4);
    eUSCIA1_UART_send(checksum);

    //Espera bit de ACK
    ACK = 0x00;
    ACK = eUSCIA1_UART_receive();       
}
/*
* Esta función escribe bytes a la memoria Flash del STM 
* Enviados por uart y recibe el ack de confirmacion
* maximo de 256 bytes
*/
static void writeData (uint8_t* arrayTx2, int arrayTxSize2){        //modif a 8
    int j;
    ACK = 0;
    //int checksum = dataW[0] ^ dataW[1] ^ dataW[2] ^ dataW[3] ^ NBYTES; //Obtiene cheksum de los datos a escribir
    //int checksum = 23;
    int checksum=0;
    for (j=0;j<=arrayTxSize2;j++)
        checksum= checksum ^ *(arrayTx2+j);
    checksum = checksum ^ ((arrayTxSize2)&0xFF);
    for (i = 0;i<=arrayTxSize2;i++)
        eUSCIA1_UART_send(*(arrayTx2+i)); //Env�a los datos a escribir
    eUSCIA1_UART_send(checksum); //Env�a checksum
    ACK = eUSCIA1_UART_receive(); //Espera bit de Acknowledge
}

void userSendCommand(int command,uint32_t* arrayRx, int arrayRxSize){
   /*Esta funci�n sirve para los comandos
    * - Get command
    * - Get version & Read Protection Status command
    * - Get ID command
    * - Write Unprotect command
    * - Readout protect command
    * - Readout unprotect command
    *
    */

   sendCommand(command); //Envia el comando
   if (ACK == 0x79){
       receiveCommand_dataRx(arrayRx, arrayRxSize); //Recibe respuesta del microcontrolador principal.
   }
}

<<<<<<< HEAD
//Metodo de lectura de memoria Flash
//Nota AN3155 (p.13)
void readMemoryCommand(int ADDRESS_MSB,int ADDRESS_LSB,uint32_t* arrayRx, int arrayRxSize){
=======
/*
* @brief Esta funcion direcciona y lee bytes de localidad especificada a un arreglo de memoria
* @param ADDRESS_MSB, LSB es la direccion en dos partes donde se va a escribir
* arrayTx: apuntador del arreglo que recibe los datos
* arrayTxSize: numero de elementos que se van a escribir en el arreglo
*sigue trabajando a 32 bits, aun no se prueban con cambiaro a 8
*/ 
void readMemoryCommand(int ADDRESS_MSB,int ADDRESS_LSB,uint32_t* arrayRx, int arrayRxSize){ //Secuencia de lectura (Nota AN3155 p.13)
>>>>>>> origin/pablo
    //NBYTES: n�mero de bytes a leer.
    //Ejemplo: lectura de 4 bytes NBYTES = 4;
    //NBYTES = NBYTES-1; //El dato que se tiene que enviar al principal es NBYTES-1
    sendCommand(READ_MEMORY);//Env�a el comando de lectura de memoria.
    if (ACK == 0x79){ //Espera  bit de acknowledge
        send_startAddress(ADDRESS_MSB,ADDRESS_LSB); //Direccion de inicio
        if (ACK == 0x79){
            eUSCIA1_UART_send(arrayRxSize); //Numero de bytes a leer
            eUSCIA1_UART_send(~arrayRxSize); //Env�a checksum
            ACK = eUSCIA1_UART_receiveACK_eerase(); //Espera Acknowledge para posteriormente recibir los datos
                                            //Se utiliza esta funci�n ya que el principal se tarda en
                                            //mandar el acknowledge.
            if (ACK == 0x79){
                receiveCommand_dataRx(arrayRx, arrayRxSize); //Si todas los pasos fueron correctos se reciben los datos.
            }
        }
    }
}

<<<<<<< HEAD
//Metodo de escritura a memoria Flash
//Nota AN3155 (p.18)
void writeMemoryCommand(int ADDRESS_MSB,int ADDRESS_LSB,uint32_t* arrayTx, int arrayTxSize){
    //NBYTES: n�mero de bytes a escribir.
    //Ejemplo: lectura de 4 bytes NBYTES = 4;
=======
/*
* @brief Esta funcion direcciona y escribe bytes contenidos en un arreglo de memoria en la localidad especificada
* @param ADDRESS_MSB, LSB es la direccion en dos partes donde se va a escribir
* arrayTx: apuntador del arreglo a escribir
* arrayTxSize: numero de elementos que se van a escribir del arreglo
*/    
void writeMemoryCommand(int ADDRESS_MSB,int ADDRESS_LSB,uint8_t* arrayTx, int arrayTxSize){  //modif a 8  //Secuencia de escritura (Nota AN3155 p.18)
>>>>>>> origin/pablo
    arrayTxSize = arrayTxSize-1;//El dato que se tiene que enviar al principal es NBYTES-1
    sendCommand(WRITE_MEMORY);//Env�a el comando de lectura de memoria

    if(ACK == 0x79){//Espera bit de acknowledge
        send_startAddress(ADDRESS_MSB,ADDRESS_LSB); //env�a direcci�n de inicio.
        if (ACK == 0x79){//Espera bit de acknowlegde
            eUSCIA1_UART_send(arrayTxSize); //Numero de bytes a escribir
            writeData(arrayTx, arrayTxSize);
        }
    }
}

<<<<<<< HEAD
//Metodo de salto a direccion para ejecucion de instrucciones
//Nota AN3155 (p.16)
void goCommand(int ADDRESS_MSB,int ADDRESS_LSB){
=======
/*
* Esta funcion sale del modo bootloader para dejar que el stm continue con la ejecucion de su codigo 
* desde una direccion especificada
*/
void goCommand(int ADDRESS_MSB,int ADDRESS_LSB){    //Secuencia para reanudar programa al salir del bootloader (Nota AN3155 p.16)
>>>>>>> origin/pablo
    sendCommand(GO); //Env�a el comando por UART.
    if (ACK == 0x79) //Espera bit de acknowledge
        send_startAddress(ADDRESS_MSB,ADDRESS_LSB); //Envia direccion de inicio.
    //Inicia en la direccion ADDRESS + 4
    //Reinicia los perifericos utilizados por el bootloader.
}

/**
 * @brief Esta funci�n borra un sector X de la memoria FLASH
 * @param FlashSectorCode codigo del sector de memoria a borrar
 */

<<<<<<< HEAD
//Metodo de borrado de memoria Flash
//Nota AN3155 (p.21)
void eeraseCommand(int FlashSectorCode){
=======
void eeraseCommand(int FlashSectorCode){    //secuencia de borrado (Nota AN3155 p.21)
>>>>>>> origin/pablo
    sendCommand(E_ERASE);  //Env�a el comando de extended erase

    if (ACK == 0x79){ //Evalua el bit de acknowledge
        eUSCIA1_UART_send(0x00); //UART7_send(N-1) - N: N�mero de paginas a borrar
        eUSCIA1_UART_send(0x00);
        eUSCIA1_UART_send(0x00);
        eUSCIA1_UART_send(FlashSectorCode); //Env�a el codigo del sector de memoria a borrar.
        eUSCIA1_UART_send(0x00 ^ FlashSectorCode); //Env�a checksum
        ACK = 0;
        ACK = eUSCIA1_UART_receiveACK_eerase(); //Espera bit de acknowledge
    }
}

void getChecksumCommand(int ADDRESS_MSB,int ADDRESS_LSB,
                        int WORD32b_MSB,int WORD32b_LSB,
                        int CRCpolynomial_MSB, int CRCpolynomial_LSB,
                        int CRCinitialValue_MSB, int CRCinitialValue_LSB,uint32_t* arrayRx, int arrayRxSize)
{                               //Secuencia para verificar el checksum de un conjunto información (Nota AN3155 p.35)
    sendCommand(GET_CHEKSUM); //Env�a el comando GET_checksum
    if (ACK == 0x79){   //Evalua bit de acknowledge.
        send_startAddress(ADDRESS_MSB, ADDRESS_LSB); //Env�a direcci�n de inicio
        if(ACK == 0x79){ //Evalua bit de acknowledge.
            send_4bytes_wChecksum(WORD32b_MSB, WORD32b_LSB);
            if(ACK == 0x79){ //Evalua bit de acknowlege.
                send_4bytes_wChecksum(CRCpolynomial_MSB, CRCpolynomial_LSB);
                if(ACK == 0x79){ //Evalua bit de ackwonledge
                    send_4bytes_wChecksum(CRCinitialValue_MSB, CRCpolynomial_LSB);
                    if(ACK == 0x79){ //Evalua bit de acknowledge
                        receiveCommand_dataRx(arrayRx, arrayRxSize); //Si todos los datos son correctos recibe checksum.
                    }
                }
            }
        }
    }
}
