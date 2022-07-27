#include <msp430.h> 


/**
 * main.c
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
        if((P2IN & BIT1) == 0) {
            P1OUT |= (BIT0);    // liga led com botao pressionado
        } else {
            P1OUT &= ~(BIT0);   // desliga led com botao solto
        }
    }
    return 0;
}
