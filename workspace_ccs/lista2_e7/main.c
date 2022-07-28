#include <msp430.h>
#include <stdint.h>
#include <intrinsics.h>

/**
 * Usuario deve pressionar S1 n vezes para acertar um numero aleatorio.
 * Deve confirmar com S2, caso ganhe Led 2 acende, se nao Led 1.
 */



volatile unsigned char pressnum; // Variavel global

int IOconfig(void) {
    P1OUT &= ~(BIT0);           // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);            // pino P1.0 como saida

    P4OUT &= ~(BIT7);           // inicia led desligado (ativo-alto)
    P4DIR |= (BIT7);            // pino P2.0 como saida

    P2DIR &= ~(BIT1);           // pino P2.1 como entrada
    P2REN |= (BIT1);            // habilita resistor
    P2OUT |= (BIT1);            // resistor pull-up

    P2IE  |= (BIT1);            // P2.1 interrupcao habilitada
    P2IES |= (BIT1);            // P2.1 Hi/lo edge
    P2IFG &= ~(BIT1);           // P2.1 IFG = 0

    P1DIR &= ~(BIT1);           // pino P1.1 como entrada
    P1REN |= (BIT1);            // habilita resistor
    P1OUT |= (BIT1);            // resistor pull-up

    P1IE  |= (BIT1);            // P1.1 interrupcao habilitada
    P1IES |= (BIT1);            // P1.1 Hi/lo edge
    P1IFG &= ~(BIT1);           // P1.1 IFG = 0

    return 0;
}

unsigned char rand() {          // Gera numero pseudo aleatorio
    static unsigned char num = 5;
    num = (num * 17) % 7;

    return num;
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    IOconfig();                         // Configura pinos de entrada e saida

    __enable_interrupt();               // Habilita interrupcoes, GIE bit em SR

    pressnum = 0;

    for(;;) {                           // Loop principal
    }

    return 0;
}

// Rotina do Servico de interrupcao da porta 2
#pragma vector = PORT2_VECTOR
__interrupt void P2ISR() {
    volatile uint16_t delay_loops = 2500; // MCLOCK em 1.5MHz USB, delay ~= 10ms
    switch (P2IV) {
        case 0x4:                       // P2.1 botao S1
            while(--delay_loops);       // Debouncing
            if((P2IN & BIT1) == 0)      // Caso botao esteja pressionado
                pressnum++;
        break;
        default: break;
    }
}

// Rotina do Servico de interrupcao da porta 1
#pragma vector = PORT1_VECTOR
__interrupt void P1ISR() {
    volatile uint16_t delay_loops = 2500; // MCLOCK em 1.5MHz USB, delay ~= 10ms
    switch (P1IV) {
        case 0x4:                       // P1.1 botao S2
            while(--delay_loops);       // Debouncing, 6 ciclos * 2500 / 1.5mhz = 10ms
            if((P1IN & BIT1) == 0) {    // Caso botao esteja pressionado
               if(pressnum == rand()) {
                   P4OUT |= BIT7;       // Liga Led2
                   P1OUT &= ~(BIT0);    // Desliga Led1
               } else {
                   P1OUT |= BIT0;       // Led2
                   P4OUT &= ~(BIT7);    // Liga Led1
               }
               pressnum = 0;
            }
        break;
        default: break;
    }
}


