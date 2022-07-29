#include <msp430.h>
#include <stdint.h>

/**
 * Piscando Led2 em aproximadamente 1Hz ultilizando TAIFG polling.
 */

int IOconfig(void);

void initTimerA0(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    IOconfig();                         // Configura pinos de entrada e saida

    initTimerA0();                      // Configura TimerA, a 1Mhz/8 = 125Khz

    TA0CCR0 = 62500 - 1;                // TimerA com 62500Ticks/125Khz = 500ms

    for(;;) {                           // Loop eterno
        while((TA0CTL & TAIFG) == 0);   // Espera overflow do Timer A
        TA0CTL &= ~TAIFG;
        P4OUT ^= BIT7;
    }

    return 0;
}

int IOconfig(void) {
    P4OUT &= ~(BIT7);                   // inicia led desligado (ativo-alto)
    P4DIR |= (BIT7);                    // pino P1.0 como saida

    return 0;
}

void initTimerA0(void) {
    TA0CCR0 = 0;                        //Inicialmente, para o timer
    TA0CCTL0 |= CCIE;                   //Habilita interrupcao para CCR0
    TA0CTL = MC_1|ID_3|TASSEL_2|TACLR;  // Configura e inicia o Timer A
}
