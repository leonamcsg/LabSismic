#include <msp430.h> 


/**
 * Botao P2.1 (pressionado P2.1 = 0) altera estado do Led P1.0 (ativo-alto)
 * Ao pressionar S1 o Led1 altera estado. Sem "debouncing".
 */

int IOconfig(void) {
    P1OUT &= ~(BIT0);           // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);            // pino P1.0 como saida

    P2DIR &= ~(BIT1);           // pino P2.1 como entrada
    P2REN |= (BIT1);            // habilita resistor
    P2OUT |= (BIT1);            // resistor pull-up
    return 0;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    IOconfig();

    for(;;) {
        if((P2IN & BIT1) == 0)
            P1OUT ^= (BIT0);    // altera estado do led
    }
    return 0;
}
