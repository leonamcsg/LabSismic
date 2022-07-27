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
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    uint8_t BTold, BTnew;

    IOconfig();                         // Configura pinos de entrada e saida

    for(;;) {
        BTnew = (P2IN & BIT1);          // Estado atual do botao em BTnew
        if( BTnew != 0) {               // Botao solto.
            if(BTold != BTnew)          // Caso o botao estivesse pressionado.
                debounce();
        }else {                         // Botao pressionado.
            if(BTold != BTnew) {        // Caso botao estivesse solto
                debounce();
                P1OUT ^= (BIT0);        // altera estado do led.
            }
        }
        BTold = BTnew;                  // Estado atual e armazenado em BTold
    }
    return 0;
}
