/* 
 * Bárbara Alves de Paiva Barbosa
 * 2020003139
 */

#include "lcd.h"
#include "atraso.h"
#include "keypad.h"
#include "config.h"
#include "bits.h"
#include "ssd.h"
#include "pwm.h"
#include "serial.h"
#include <pic18f4520.h>

int limiteSuperior = 8, limiteInferior = 2, auxiliar;
float temperatura = 22.0, ponto1 = 22, ponto2 = 22, tempLiquido = 22.0, tempPontos = 22, maximo = 22, minimo = 22;
char str[6];
unsigned int estoque = 0;

//Funcao auxiliar para impressao no lcd

void itoa(unsigned int val, char* str) {
    str[0] = (val / 10000) + 0x30;
    str[1] = ((val % 10000) / 1000) + 0x30;
    str[2] = ((val % 1000) / 100) + 0x30;
    str[3] = ((val % 100) / 10) + 0x30;
    str[4] = (val % 10) + 0x30;
    str[5] = 0;
}

//Leitura do serial

int serial(void) {
    char i;
    unsigned char tmp = 10;
    atraso_ms(2000);
    while (tmp > 9) {
        serial_tx_str(" Digite:\r\n");
        lcd_cmd(L_CLR);
        lcd_cmd(L_L1);
        lcd_str("Digite:");
        for (i = 0; i < 16; i++) {
            tmp = serial_rx(2000);
        }
        tmp -= 48;
        if (tmp < 0 || tmp > 9) {
            lcd_cmd(L_CLR);
            lcd_cmd(L_L1);
            lcd_str("Valor invalido.");
            atraso_ms(500);
        }
    }
    serial_tx_str("FIM");
    serial_tx(tmp);
    return (int) tmp;
}

//Impressao da quantidade de vacinas no display de 7seg

void ssdEstoque(void) {
    for (auxiliar = 0; auxiliar < 50; auxiliar++);
    ssdDigit(14, 0);
    ssdDigit(((estoque / 10) % 10), 2);
    ssdDigit(((estoque) % 10), 3);
}

//Controlar a quantidade de entrada e saida e vacinas

void controleEstoque() {
    unsigned int tecla = 16, opc = 0;
    int aux = 0;
    char i;

    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("ESTOQUE:");
    lcd_cmd(L_L2);
    lcd_str("6 - Retirar");
    lcd_cmd(L_L3);
    lcd_str("7 - Adicionar");
    lcd_cmd(L_L4);
    lcd_str("# - Cancelar");
    kpInit();
    for (;;) {
        kpDebounce();
        if (kpRead() != tecla) {
            tecla = kpRead();
            if (bitTst(kpRead(), 10)) { //Retirar vacinas
                opc = 1;
                break;
            }
            if (bitTst(kpRead(), 1)) { //Adicionar vacinas
                opc = 2;
                break;
            }
            if (bitTst(kpRead(), 8)) { //Cancelar
                opc = 0;
                break;
            }
        }
        for (i = 0; i < 250; i++);
    }
    if (opc != 0) {
        lcd_cmd(L_CLR);
        lcd_cmd(L_L1);
        lcd_str("Maximo de 5 und.");
        lcd_cmd(L_L2);
        lcd_str("por abertura.");
        lcd_cmd(L_L3);
        lcd_str("Digite a quanti-");
        lcd_cmd(L_L4);
        lcd_str("dade no serial.");
        aux = serial();
        if (aux < 0 || aux > 5) {
            lcd_cmd(L_CLR);
            lcd_cmd(L_L1);
            lcd_str("Valor invalido.");
            atraso_ms(500);
            opc = 0;
        }
        lcd_cmd(L_CLR);
        if (opc == 1) {
            //Controle para caso de quantidades negativas
            if (estoque < aux) {
                lcd_cmd(L_CLR);
                lcd_cmd(L_L1);
                lcd_str("#NAO HA VACINAS#");
                atraso_ms(2000);
            } else
                estoque -= aux;

        }
        if (opc == 2)
            estoque += aux;

        ssdEstoque();
    }
}

//Funcao auxilar para movimentar o desenho

void shift_texto(int tempo) {
    char i;
    for (i = 0; i < 4; i++) {
        atraso_ms(tempo);
        lcd_cmd(0x1C); //lcd desliza pra direita
    }
    for (i = 0; i < 4; i++) {
        atraso_ms(tempo);
        lcd_cmd(0x18); //lcd desliza pra esquerda
    }
    for (i = 0; i < 4; i++) {
        atraso_ms(tempo);
        lcd_cmd(0x1C); //lcd desliza pra direita
    }
    for (i = 0; i < 4; i++) {
        atraso_ms(tempo);
        lcd_cmd(0x18); //lcd desliza pra esquerda
    }
    atraso_ms(1000);
}

//Subir leds

void sobe() {
    unsigned char i, j, k;
    unsigned char before;
    TRISB = 0b00000000;
    before = 0b10000000;
    for (k = 0; k < 8; k++) {
        PORTB = before;
        for (i = 0; i < 255; i++) {
            for (j = 0; j < 255; j++);
            for (j = 0; j < 255; j++);
        }
        before = before >> 1;
    }
}

//Descer leds

void desce() {
    unsigned char i, j, k;
    unsigned char before;
    TRISB = 0b00000000;
    before = 0b00000001;
    for (k = 0; k < 8; k++) {
        PORTB = before;
        for (i = 0; i < 255; i++) {
            for (j = 0; j < 255; j++);
            for (j = 0; j < 255; j++);
        }
        before = before << 1;
    }
}

//Impressao de desenho inicial

void inicia(void) {
    char i;
    char desenho[16] = {
        0x00, 0x05, 0x02, 0x07, 0x05, 0x07, 0x03, 0x05,
        0x00, 0x1A, 0x1C, 0x1E, 0x0A, 0x1E, 0x0C, 0x1A,
    };
    lcdCommand(L_CLR);
    lcdCommand(0x40);

    for (i = 0; i < 16; i++) {
        lcdData(desenho[i]);
    }

    //Desenho linha 1

    lcdCommand(L_L1);
    lcdData(0);
    lcdCommand(L_L1 + 1);
    lcdData(1);

    lcdCommand(L_L1 + 2);
    lcdData(0);
    lcdCommand(L_L1 + 3);
    lcdData(1);

    lcdCommand(L_L1 + 4);
    lcdData(0);
    lcdCommand(L_L1 + 5);
    lcdData(1);

    lcdCommand(L_L1 + 6);
    lcdData(0);
    lcdCommand(L_L1 + 7);
    lcdData(1);

    lcdCommand(L_L1 + 8);
    lcdData(0);
    lcdCommand(L_L1 + 9);
    lcdData(1);

    lcdCommand(L_L1 + 10);
    lcdData(0);
    lcdCommand(L_L1 + 11);
    lcdData(1);

    lcdCommand(L_L1 + 12);
    lcdData(0);
    lcdCommand(L_L1 + 13);
    lcdData(1);

    lcdCommand(L_L1 + 14);
    lcdData(0);
    lcdCommand(L_L1 + 15);
    lcdData(1);


    //Desenho linha 4

    lcdCommand(L_L4);
    lcdData(0);
    lcdCommand(L_L4 + 1);
    lcdData(1);

    lcdCommand(L_L4 + 2);
    lcdData(0);
    lcdCommand(L_L4 + 3);
    lcdData(1);

    lcdCommand(L_L4 + 4);
    lcdData(0);
    lcdCommand(L_L4 + 5);
    lcdData(1);

    lcdCommand(L_L4 + 6);
    lcdData(0);
    lcdCommand(L_L4 + 7);
    lcdData(1);

    lcdCommand(L_L4 + 8);
    lcdData(0);
    lcdCommand(L_L4 + 9);
    lcdData(1);

    lcdCommand(L_L4 + 10);
    lcdData(0);
    lcdCommand(L_L4 + 11);
    lcdData(1);

    lcdCommand(L_L4 + 12);
    lcdData(0);
    lcdCommand(L_L4 + 13);
    lcdData(1);

    lcdCommand(L_L4 + 14);
    lcdData(0);
    lcdCommand(L_L4 + 15);
    lcdData(1);

    sobe();
    shift_texto(300);
    desce();
}

//Impressao inicial do programa

void ligar(void) {
    char i, logo[16] = {
        0x1F, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x0E, 0x0E,
        0x0E, 0x0E, 0x0E, 0x0E, 0x04, 0x04, 0x04, 0x00,
    };
    lcd_cmd(L_CLR);
    lcdCommand(0x40);

    for (i = 0; i < 16; i++) {
        lcdData(logo[i]);
    }

    lcdCommand(L_L2);
    lcdData(0);
    lcdCommand(L_L3);
    lcdData(1);
    lcdCommand(L_L2 + 15);
    lcdData(0);
    lcdCommand(L_L3 + 15);
    lcdData(1);

    lcd_cmd(L_L3 + 5);
    lcd_str("Vac-14");

    atraso_ms(15000);
}

//Impressao das medidas do sistema

void imprime(void) {
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("####MEDIDAS#####");

    //Temperatura média
    lcd_cmd(L_L2);
    itoa(temperatura * 10, str);
    lcd_dat('T');
    lcd_dat(str[2]);
    lcd_dat(str[3]);
    lcd_dat(',');
    lcd_dat(str[4]);
    lcd_cmd(L_L2 + 5);
    lcd_str("C");

    //Temperatura no liquido
    lcd_cmd(L_L3);
    itoa(tempLiquido * 10, str);
    lcd_dat('L');
    lcd_dat(str[2]);
    lcd_dat(str[3]);
    lcd_dat(',');
    lcd_dat(str[4]);
    lcd_cmd(L_L3 + 5);
    lcd_str("C");

    //Limites
    lcd_cmd(L_L4);
    itoa(limiteInferior, str);
    lcd_dat('I');
    lcd_cmd(L_L4 + 2);
    lcd_dat(str[3]);
    lcd_dat(str[4]);
    lcd_cmd(L_L4 + 4);
    lcd_str("C");

    lcd_cmd(L_L4 + 7);
    itoa(limiteSuperior, str);
    lcd_dat('S');
    lcd_cmd(L_L4 + 9);
    lcd_dat(str[3]);
    lcd_dat(str[4]);
    lcd_cmd(L_L4 + 11);
    lcd_str("C");
}

//Impressao no lcd para itens do menu

void instrucoes(void) {
    lcd_cmd(L_L1);
    lcd_str("OPCOES:");

    lcd_cmd(L_L3);
    lcd_str("1 - Medidas");
    lcd_cmd(L_L4);
    lcd_str("2 - Abrir Porta");
    atraso_ms(3000);
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("3 - Fechar Porta");
    lcd_cmd(L_L2);
    lcd_str("4 - Max/Min");
    lcd_cmd(L_L3);
    lcd_str("# - Voltar");
    atraso_ms(3000);
    lcd_cmd(L_CLR);
}

//Atualiza os valores das temperaturas e mantem um controle dos limites

void atualizar(void) {
    ponto1 = temperatura;
    ponto2 = temperatura + 1;
    tempPontos = (ponto1 + ponto2) / 2;
    tempLiquido = (temperatura + tempPontos) / 2;

    if (temperatura > maximo)
        maximo = temperatura;
    if (temperatura < minimo)
        minimo = temperatura;
}

//Diminuir temperatura

void resfria(void) {
    int cont = 0;
    while (temperatura >= limiteSuperior || tempPontos >= limiteSuperior || tempLiquido >= limiteSuperior) {
        lcd_cmd(L_L1);
        lcd_str("##RESFRIANDO");
        pwmSet1(96);
        temperatura -= 0.25;
        atraso_ms(150);
        atualizar();
        cont++;
    }
    if (cont != 0) {
        lcd_cmd(L_CLR);
        lcd_cmd(L_L1);
        lcd_str("ESCOLHA");
        lcd_cmd(L_L3);
        lcd_str("# - Menu");
    }
    pwmSet1(0);
}

//Aumentar temperatura

void aquece(void) {
    while (temperatura <= limiteInferior || tempPontos <= limiteInferior || tempLiquido <= limiteInferior) {

        PORTCbits.RC5 = 1;
        temperatura += 0.5;
        atraso_ms(150);
        atualizar();
    }
    PORTCbits.RC5 = 0;
}

//Funcao para recalcular a temperatura quando a porta eh fechada

void fecharPorta(int cont) {
    temperatura += cont * 0.25;
    PORTCbits.RC5 = 1;
    atraso_ms(350);
    PORTCbits.RC5 = 0;
}

//Impressao dos maximos e  minimos atingidos

void maxEmin(void) {
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("#####MAXIMA#####");

    //Temperatura maxima
    lcd_cmd(L_L2);
    itoa(maximo * 10, str);
    lcd_dat('M');
    if (maximo < 0) {
        lcd_cmd(L_L2 + 1);
        lcd_str("-");
    }
    lcd_cmd(L_L2 + 2);
    lcd_dat(str[2]);
    lcd_dat(str[3]);
    lcd_dat(',');
    lcd_dat(str[4]);
    lcd_cmd(L_L2 + 6);
    lcd_str("C");

    lcd_cmd(L_L3);
    lcd_str("#####MINIMO#####");

    //Temperatura minima
    lcd_cmd(L_L4);
    itoa(minimo * 10, str);
    lcd_dat('m');
    if (minimo < 0) {
        lcd_cmd(L_L4 + 1);
        lcd_str("-");
    }
    lcd_cmd(L_L4 + 2);
    lcd_dat(str[2]);
    lcd_dat(str[3]);
    lcd_dat(',');
    lcd_dat(str[4]);
    lcd_cmd(L_L4 + 6);
    lcd_str("C");
}

//Aciona relés para simbolizar alerta

void alerta(void) {
    TRISCbits.TRISC0 = 0;
    TRISEbits.TRISE0 = 0;
    if (temperatura >= limiteSuperior - 2 || tempPontos >= limiteSuperior - 2 || tempLiquido >= limiteSuperior - 2 || temperatura <= limiteInferior + 2 || tempPontos <= limiteInferior + 2 || tempLiquido <= limiteInferior + 2) {


        PORTCbits.RC0 ^= 1;
        PORTEbits.RE0 ^= 1;

    } else {
        PORTCbits.RC0 = 0;
        PORTEbits.RE0 = 0;
    }
}

//Aciona o buzzer para temperaturas passando os limites

void perigo(void) {
    unsigned char k, i;
    if (temperatura >= limiteSuperior + 1 || tempPontos >= limiteSuperior + 1 || tempLiquido >= limiteSuperior + 1 || temperatura <= limiteInferior - 1 || tempPontos <= limiteInferior - 1 || tempLiquido <= limiteInferior - 1) {
        pwmFrequency(10000);

        for (k = 0; k < 3; k++) {
            for (i = 1; i > 0; i = i * 2) {
                bitSet(TRISC, 1);
                PORTB = i;
                PORTD = i;
                atraso_ms(100);
            }
            bitClr(TRISC, 1);
        }

        PORTB = 0;
        PORTD = 0;
    }
}

void main(void) {
    char i;
    float aux = 0;
    unsigned int tecla = 16, controle = 0, cont = 0;
    TRISA = 0xC3;
    TRISB = 0x03;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    PORTD = 0x00;

    //Inicializacoes
    lcd_init();
    ssdInit();
    pwmInit();
    serial_init();
    lcd_cmd(0x0C); //desligar cursor

    //Inicio do programa com a logo
    inicia();
    ligar();

    //Adequa as temperaturas de acordo com a configuracao
    temperatura = (limiteSuperior + limiteInferior) / 2;
    ponto1 = temperatura;
    ponto2 = temperatura;
    tempLiquido = temperatura;
    tempPontos = temperatura;
    maximo = temperatura;
    minimo = temperatura;

    lcdCommand(L_CLR); //limpa a tela

    //Inicio do menu
    kpInit();

    for (;;) {
        kpDebounce();
        if (controle == 0) {
            controle = 1;
            instrucoes();
            lcd_cmd(L_L1);
            lcd_str("ESCOLHA");
            lcd_cmd(L_L3);
            lcd_str("# - Menu");
        }
        if (kpRead() != tecla) {
            tecla = kpRead();

            if (bitTst(kpRead(), 3)) { //Imprime medidas
                imprime();
            }
            if (bitTst(kpRead(), 7)) { //Controle de estoque
                controleEstoque();
                lcd_cmd(L_CLR);
                lcd_cmd(L_L1);
                lcd_str("##PORTA ABERTA");
                lcd_cmd(L_L3);
                lcd_str("3 - Fechar Porta");
                cont = 3;
            }
            if (bitTst(kpRead(), 11)) { //Fechar a porta e recalcular temperaturas
                if (cont != 0)
                    fecharPorta(cont);
                cont = 0;
                lcd_cmd(L_CLR);
                controle = 0;
                //temperatura = 10; //Para facilitar testes
            }
            if (bitTst(kpRead(), 2)) { //Mostrar temperaturas maximas e minimas
                maxEmin();
            }
            if (bitTst(kpRead(), 8)) { //Volta ao menu
                lcd_cmd(L_CLR);
                controle = 0;
            }
        }
        //Atualiza e mantem no intervalo
        alerta();
        perigo();
        atualizar();
        resfria();
        aquece();
        aux++;
        //Aquecimento natural para motor desligado
        if (aux == 10000) {
            temperatura += 2;
            PORTCbits.RC5 = 1;
            atraso_ms(350);
            PORTCbits.RC5 = 0;
            aux = 0;
        }
        ssdEstoque();
        ssdUpdate();
    }
}