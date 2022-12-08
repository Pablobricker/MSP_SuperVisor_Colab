void RTC_disabling(){
    RTCCTL01_H = RTCHOLD_H;         //Retiene la operación del RTC mientras se configura slau367p (p.701)
}

void RTC_setTime(int hour, int min){
    RTCSEC =  0x00;                 //Ingreso de valor de los segundos formato hex slau367p(p.703)
    RTCHOUR = hour;                 //Valor de las horas  slau367p(p.705)
    RTCMIN = min;                   //Valor de los minutos slau367p(p.704)
}

void RTC_setDate(int day, int month, int year){
    RTCDAY = day;                   //Valor de los días slau367p(p.706)
    RTCMON = month;                 //Valor de los meses slau367p(p.707)
    RTCYEAR = year;                 //Valor del año en formato hex dividido en 2 bytes slau367p(p.708)
}

void RTC_setAlarm(int min_A){
    RTCCTL01 |= RTCAIE;            //Interrupcion habilitada para evento de alarma del RTC slau367p (p.700)
    __enable_interrupt();
    RTCAMIN = min_A;                //Valor de los minutos slau367p(p.709)
    RTCAMIN |= BIT7;                //Activa la alarma de Minutos slau367p(p.709)
    RTCAHOUR = 0x00;                //Valor de las horas slau367p(p.710)
    RTCADOW = 0x00;                 //Valor del día de la semana (0 a 6) slau367p(p.711)
    RTCADAY = 0x00;                 //Valor del día del mes (0 a 31) slau367p(p.712)
}

void RTC_enable(){
    RTCCTL01_H &= ~(RTCHOLD_H);     //Continua con la operación de conteo del
                                    //RTC una vez que termina la configuración slau367p(p.700)
}

#pragma vector =  RTC_VECTOR        //Habilita la interrupción reservada al RTC
__interrupt void RTC_ISR(){
    RTCCTL01 &= ~RTCAIFG;           //Configura el vector de interrupción para atender el evento de alarma slau367p(p.700)
    _low_power_mode_off_on_exit();
}
