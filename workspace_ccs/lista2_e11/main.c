#include <msp430.h>
#include <stdint.h>

/**
 * Leonam C. S. Gomes, 2022
 *
 * Piscando Led1 em aproximadamente 1Hz e Led2 em 2hz ultilizando
 * interrupcoes do TimerA.
 */

//
//-------     -------     -------
//|     |     |     |     |         Periodo do ciclo, Timer a partir de SMCLOCK
//      -------     -------         1Mhrz/8 = 125khz, CCR0 = 62500, T0 = T1 = 500ms
//
//----  ----  ----  ----  ----  -
//|  |  |  |  |  |  |  |  |  |  |   50% do ciclo de trabalho, 2Hz, modo alternado,
//   ----  ----  ----  ----  ----   T2 = 250ms.
//

void IOconfig(void);

void initTimerA0(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    IOconfig();                         // Configura pinos de entrada e saida

    initTimerA0();                      // Configura TimerA, a 1Mhz/8 = 125Khz

    __enable_interrupt();               // Habilita interrupcoes

    TA0CCR0 = 62500 - 1;                // TimerA com 62500Ticks/125Khz = 500ms
    TA0CCR1 = 62500 - 1;                // TimerA com 62500Ticks/125Khz = 500ms
    TA0CCR2 = 31250 - 1;                // TimerA com 31250Ticks/125Khz = 250ms

    __bis_SR_register(LPM0_bits);       // Ativa modo de baixo consumo
    __no_operation();                   // Aguarda interrupcao

    return 0;
}

void IOconfig(void) {
    P4OUT &= ~(BIT7);                   // inicia led desligado (ativo-alto)
    P4DIR |= (BIT7);                    // pino P4.7 como saida

    P1OUT &= ~(BIT0);                   // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);                    // pino P1.0 como saida
}

void initTimerA0(void) {
    TA0CCTL0 |= CCIE;
    TA0CCTL1 |= OUTMOD_4 | CCIE;        // Habilita interrupcao para CCR1, modo "toggle"
    TA0CCTL2 |= OUTMOD_4 | CCIE;        // Habilita interrupcao para CCR2, modo "toggle"
    TA0CTL = MC_1|ID_3|TASSEL_2|TACLR;  // Configura e inicia o Timer A

    TA0CCTL0 &= ~CCIFG;                 // Limpa flags de interrupcao
    TA0CCTL1 &= ~CCIFG;
    TA0CCTL2 &= ~CCIFG;
}

//TimerA Rotina do Servico de Interrupcao

#pragma vector = TIMER0_A0_VECTOR       // Tratamento para interrupcao CCR0IFG
__interrupt void TIMERA0_CCR0_ISR(void) {
    TA0CCTL0 &= ~CCIFG;                 // Limpa flag de overflow
}

#pragma vector = TIMER0_A1_VECTOR       // Tratamento para interrupcao TA0IVIFG
__interrupt void TIMERA1_TA0IV_ISR(void) {
    switch (TA0IV) {                    // Verificando qual CCR ativou interrupcao
        case 0:
            break;
        case TA0IV_TA0CCR1:             // Se CCR1 ativou a interrupcao
            P1OUT ^= BIT0;              // Altera estado led1
            TA0CCTL1 &= ~CCIFG;
        case TA0IV_TA0CCR2:             // Se CCR2 ativou a interrupcao
            P4OUT ^= BIT7;              // Altera estado Led2 novamente, em 50% DC
            TA0CCTL2 &= ~CCIFG;         // gerando dobro da frequencia do Led1
            break;
        case TA0IV_TA0IFG:              // Possui tratamento especial
            break;
        default:
            break;
    }

}

