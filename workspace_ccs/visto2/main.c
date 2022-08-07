// Aluno: Leonam Christian Silva Gomes, Matr.: 190043318, Universidade de Brasilia

#include <msp430.h>
#include <stdint.h>
#include <intrinsics.h>

/**
 * Sensor ultrassonico HC-SR04 com mostrador analogico utilizando
 * servo motor SG90 Tower Pro e led1 e led2.
 *
 * P2.5 - Saida PWM mostrador analogico
 * P1.5 - Sinal "trigger" sensor ultrassonico
 * P2.0 - Sinal "echo" sensor ultrassonico
 */

static const uint16_t lowlimit = 500 - 1;     //  0 graus
static volatile uint16_t newdistance;
static volatile uint16_t PRONTO = 0;

void IOconfig(void);

void initTimers(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;                 // Para WDT

    static const uint16_t uplimit = 2500 - 1; //  180 gaus
    static volatile uint16_t newTA1CCR1;

    IOconfig();                             // Configura pinos de entrada e saida
    initTimers();                           // Inicia temporizadores
    __enable_interrupt();                   // Habilita interrupcoes, GIE bit em SR;

    for(;;) {
        while(!PRONTO);                     // Aguarda sinal de TA1CCR1
        PRONTO = 0;

        newTA1CCR1 = newdistance / 1.45 + lowlimit; // Faz conversao para medidas do
                                                    // mostrador analogico.
        // Atualiza PWM P2.5 com nova distancia
        if(newTA1CCR1 >= uplimit) {          // Caso TA1CCR1 maior que limite superior
            TA2CCR2 = uplimit;               // Limitando maior valor de TA2CCR2
        } else {
            TA2CCR2 = newTA1CCR1;            // Atualiza TACCR2 com novo tempo
        }

        // LEDs
        if(TA2CCR2 >= uplimit) {             // Maior que 2500: ~50cm
            P1OUT &= ~BIT0; // Desliga Led1
            P4OUT &= ~BIT7; // Desliga Led2
        } else if (TA2CCR2 >= 1700 - 1) {    // Entre 2500 e 1700: ~50cm - ~30cm
            P1OUT &= ~BIT0; // Desliga Led1
            P4OUT |= BIT7;  // Liga Led2
        } else if (TA2CCR2 >= 900 - 1){      // Entre 900 e 1700: ~30cm - ~10cm
            P1OUT |= BIT0;  // Liga Led1
            P4OUT &= ~BIT7; // Desliga Led2
        } else {                             // Menor que 900: ~10cm
            P1OUT |= BIT0;  // Liga Led1
            P4OUT |= BIT7;  // Liga Led2
        }
    }
}

void IOconfig(void) {
    P1OUT &= ~BIT0;             // Inicia LED2 desligado
    P4OUT &= ~BIT7;             // Inicia LED1 desligado
    P4DIR |= BIT7;              // P4.7(LED2) como saida
    P1DIR |= BIT0;              // P1.0(LED1) como saida

    P2DIR |= BIT5;              // P2.5 como saida
    P2SEL |= BIT5;              // Seleciona P2.5 como saida PWM

    P1DIR |= BIT5;              // Sinal trigger(saida)
    P2DIR &= ~BIT0;             // Sinal echo(captura)
    P1SEL |= BIT5;              // P1.5 como TA0CCR4
    P2SEL |= BIT0;              // P2.0 como TA1CCR1
}

void initTimers(void) {

    // TimerA2 Servo Motor, P2.5(PWM)
    TA2CCR0 = 20000 - 1;                      // PWM periodo
    TA2CCTL2 = OUTMOD_7;                      // CCR2 reset/set
    TA2CCR2 = lowlimit;                       // CCR2 PWM duty cycle
    TA2CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, limpa TAR

    // TimerA0 Sensor Ultrassonico, Trigger -> P1.5(PWM)
    TA0CCR0 = 12000 - 1;                      // Periodo maximo, trigger-echo = 12ms
    TA0CCR4 = 100  - 1;                       // Pulso trigger dura 100us (min - 10us)
    TA0CCTL4 |= OUTMOD_7;                     // Modo "set/reset"

    TA0CTL |= TASSEL__SMCLK |                 // SMCLK = 1Mhz
              MC__UP        |                 // Modo "up"
              TACLR;                          // Limpa TA0R

    // TimerA1 Sensor Ultrassonico, Echo -> P2.0(CAP)
    TA1CCR0 = 12000 - 1;                      // Periodo total 12ms
    TA1CCTL1 |= CAP  |                        // Modo captura
                CM_3 |                        // Modo de subida e descida do CLK
                CCIE;                         // Habilita interrupcao TA1CCR1

    TA1CTL |= TASSEL__SMCLK |                 // SMCLK = 1Mhz
              MC__UP        |                 // Modo "up"
              TACLR;                          // Limpa TA0R
}

// Servico de inrrupcao de rotina TimerA1
#pragma vector = TIMER1_A1_VECTOR
__interrupt void timerA1_TA1IV_ISR(void) {
    static volatile uint16_t count = 0;
    static volatile uint16_t lastcaptured = 0;

    if(count != 0) {                  // Conta subida e descida
        PRONTO = 1;                   // Sinaliza processo para main()
        count = 0;
    } else {
        count = 1;                    // Contagem
    }
    newdistance = TA1CCR1 - lastcaptured; // 0 a 12ms
    lastcaptured = TA1CCR1;

    TA1CCTL1 &= ~CCIFG;               // Limpa flag de interrupcao de CCR1
}
