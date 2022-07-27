#include <msp430.h>
#include <stdint.h>

/**
 * Botao P2.1 (pressionado P2.1 = 0) altera estado do Led P1.0 (ativo-alto)
 * Ao pressionar S1 o Led1 altera estado. Com "debouncing".
 */

int IOconfig(void) {
    P1OUT &= ~(BIT0);           // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);            // pino P1.0 como saida

    P2DIR &= ~(BIT1);           // pino P2.1 como entrada
    P2REN |= (BIT1);            // habilita resistor
    P2OUT |= (BIT1);            // resistor pull-up
    return 0;
}

int debounce(void) {
    volatile uint16_t delay_loops = 10000; // MCLOCK em 1MHz, delay ~= 10ms
    while(--delay_loops);

    return 0;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    IOconfig();

    for(;;) {
        if((P2IN & BIT1) == 0) {
            P1OUT ^= (BIT0);    // altera estado do led
            debounce();
            while((P2IN & BIT1) == 0); // Caso botao nao tenha sido solto
            debounce();                // prende a execucao
        }
    }
    return 0;
}
