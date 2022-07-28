#include <msp430.h>
#include <stdint.h>
#include <intrinsics.h>

/**
 * Botao P2.1 (pressionado P2.1 = 0) altera estado do Led P1.0 (ativo-alto).
 * Ao pressionar S1 o Led1 altera estado. Com "debouncing".
 */

int IOconfig(void) {
    P1OUT &= ~(BIT0);           // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);            // pino P1.0 como saida

    P2DIR &= ~(BIT1);           // pino P2.1 como entrada
    P2REN |= (BIT1);            // habilita resistor
    P2OUT |= (BIT1);            // resistor pull-up

    P2IE  |= (BIT1);            // P2.1 interrupcao habilitada
    P2IES |= (BIT1);            // P2.1 Hi/lo edge
    P2IFG &= ~(BIT1);           // P2.1 IFG = 0

    return 0;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    IOconfig();                         // Configura pinos de entrada e saida

    __enable_interrupt();               // Habilita interrupcoes, GIE bit em SR

    for(;;) {                           // Loop principal
    }

    return 0;
}

// Rotina do Servico de interrupcao da porta 1
#pragma vector = PORT2_VECTOR
__interrupt void P2ISR() {
    volatile uint16_t delay_loops = 1000; // MCLOCK em 1MHz, delay ~= 10ms
    switch (P2IV) {
        case 0x4:                       // P2.1 botao S1
            while(--delay_loops);       // Debouncing
            if((P2IN & BIT1) == 0)      // Caso botao esteja pressionado
                P1OUT ^= BIT0;          // LED1 alterna de estado
        break;
        default: break;
    }
}

