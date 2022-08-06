#include <msp430.h>
#include <stdint.h>
#include <intrinsics.h>
#include <math.h>

/**
 * Calcula tempo de execucao de operacao com alto custo
 */

volatile uint16_t tempo = 0;

void initTimer(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    __enable_interrupt();               // Habilita interrupcoes, GIE bit em SR

    initTimer();                        // Inicia TimerA

    // Conta de alto custo operacional - cerca de 12ms
    volatile double hardVar = 128.43984610923f;
    hardVar = (sqrt(hardVar * 3.14159265359) + 30.3245) / 1020.2331556 - 0.11923;


    tempo = TA0R;                       // Armazenando tempo
    __no_operation();                   // Aguarda interrupcao
}

void initTimer(void) {
    //TIMER_A - principal
    TA0CTL |= TACLR         | // Limpa TA0R
              MC_2          | // Modo continuo
              TASSEL__SMCLK | // CLK = 1Mhz
                      ID_3;   // TA1CLK = 1Mhz/8 = 125Khz
}
