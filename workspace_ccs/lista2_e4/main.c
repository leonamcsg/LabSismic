#include <msp430.h>
#include <stdint.h>

/**
 * Botao P2.1 (pressionado P2.1 = 0) ou botao P1.1 alteram estado do Led P1.0 (ativo-alto).
 * Ao pressionar S1 o Led1 altera estado. Com "debouncing".
 */

int IOconfig(void) {
    P1OUT &= ~(BIT0);           // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);            // pino P1.0 como saida

    P2DIR &= ~(BIT1);           // pino P2.1 como entrada
    P2REN |= (BIT1);            // habilita resistor
    P1OUT |= (BIT1);            // resistor pull-up

    P1DIR &= ~(BIT1);           // pino P1.1 como entrada
    P1REN |= (BIT1);            // habilita resistor
    P1OUT |= (BIT1);            // resistor pull-up
    return 0;
}

int debounce(void) {
    volatile uint16_t delay_loops = 10000; // MCLOCK em 1MHz, delay ~= 10ms
    while(--delay_loops);

    return 0;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    uint8_t BTold1, BTnew1, BTold2, BTnew2;

    IOconfig();                         // Configura pinos de entrada e saida

    for(;;) {
        BTnew1 = (P1IN & BIT1);         // Estado atual do botao em BTnew
        BTnew2 = (P2IN & BIT1);

        // Botao 1 = S1
        if( BTnew1 != 0) {              // Botao solto.
            if(BTold1 != BTnew1)        // Caso o botao estivesse pressionado.
                debounce();
        }else {                         // Botao pressionado.
            if(BTold1 != BTnew1) {       // Caso botao estivesse solto
                debounce();
                P1OUT ^= (BIT0);        // altera estado do led.
            }
        }

        // Botao 2 = S2
        if( BTnew2 != 0) {              // Botao solto.
            if(BTold2 != BTnew2)         // Caso o botao estivesse pressionado.
                debounce();
        }else {                         // Botao pressionado.
            if(BTold2 != BTnew2) {       // Caso botao estivesse solto
                debounce();
                P1OUT ^= (BIT0);        // altera estado do led.
            }
        }

        BTold1 = BTnew1;                // Estado atual e armazenado em BTold
        BTold2 = BTnew2;                // para comparacao
    }
    return 0;
}
