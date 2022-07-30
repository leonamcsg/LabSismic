#include <msp430.h>
#include <stdint.h>

/**
 * Leonam C. S. Gomes, 2022
 *
 * Piscando Led1 em aproximadamente 50Hz ultilizando interrupcoes do TimerA.
 */

//
//-------     -------     -------
//|     |     |     |     |         Periodo do ciclo, Timer a partir de SMCLOCK
//      -------     -------         1Mhrz, CCR0 = 20000, T0 = 20ms,
//
//----  ----  ----  ----  ----  -
//|  |  |  |  |  |  |  |  |  |  |   50% do ciclo de trabalho, 50Hz, modo alternado,
//   ----  ----  ----  ----  ----   T1 = 10ms.
//

void IOconfig(void);

void initTimerA0(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    IOconfig();                         // Configura pinos de entrada e saida

    initTimerA0();                      // Configura TimerA, a 1Mhz

    __enable_interrupt();               // Habilita interrupcoes

    TA0CCR0 = 20000 - 1;                // TimerA com 20000Ticks/1Mhz = 20ms
    TA0CCR1 = 10000 - 1;                // TimerA com 10000Ticks/1Khz = 10ms

    __bis_SR_register(LPM0_bits);       // Ativa modo de baixo consumo
    __no_operation();                   // Aguarda interrupcao

    return 0;
}

void IOconfig(void) {
    P1OUT &= ~(BIT0);                   // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);                    // pino P1.0 como saida
}

void initTimerA0(void) {
    TA0CCTL0 |= CCIE;
    TA0CCTL1 |= CCIE;        // Habilita interrupcao para CCR1, modo "toggle"
    TA0CTL = MC_1|ID_0|TASSEL_2|TACLR;  // Configura e inicia o Timer A

    TA0CCTL0 &= ~CCIFG;                 // Limpa flags de interrupcao
    TA0CCTL1 &= ~CCIFG;
}

//TimerA Rotina do Servico de Interrupcao

#pragma vector = TIMER0_A0_VECTOR       // Tratamento para interrupcao CCR0IFG
__interrupt void TIMERA0_CCR0_ISR(void) {
    TA0CCTL0 &= ~CCIFG;                 // Limpa flag de overflow
    P1OUT ^= BIT0;                      // Altera estado led1
}

#pragma vector = TIMER0_A1_VECTOR       // Tratamento para interrupcao TA0IVIFG
__interrupt void TIMERA1_TA0IV_ISR(void) {
    switch (TA0IV) {                    // Verificando qual CCR ativou interrupcao
        case 0:
            break;
        case TA0IV_TA0CCR1:             // Se CCR1 ativou a interrupcao,
            P1OUT ^= BIT0;              // Altera estado led1
            break;
        case TA0IV_TA0IFG:              // Possui tratamento especial
            break;
        default:
            break;
    }
}

