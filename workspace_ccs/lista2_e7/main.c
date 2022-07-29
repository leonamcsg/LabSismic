#include <msp430.h>
#include <stdint.h>

/**
 * Piscando Led1 em aproximadamente 1Hz ultilizando loop.
 */

int IOconfig(void) {
    P1OUT &= ~(BIT0);           // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);            // pino P1.0 como saida

    return 0;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    IOconfig();                         // Configura pinos de entrada e saida

    volatile uint16_t delay_loops = 50000 ; // MCLOCK em 1.5MHz USB, 14 ciclos * 50000 / 1.5mhz ~= 467 ms

    for(;;) {                           // Loop eterno
        while(delay_loops--);           // 5 + 2 + 2 + 5 = 14 ciclos de clock
        P1OUT ^= BIT0;                  // Altera Led1
        delay_loops = 50000;
    }

    return 0;
}
