/* Estamos usando o oscilador interno a 1Mhz */
#define F_CPU 1000000

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h> 
#include <util/delay.h>

/* Função de divisão unsigned da libgcc. */
extern div_t udiv(int __num, int __denom) __asm__("__udivmodhi4") __ATTR_CONST__;

/* Macros para ligar, desligar e ler bits */
#define ligar_bit(porta, bit) do {      \
    porta |=  (1<<bit);                 \
} while (0)
#define desligar_bit(porta, bit) do {   \
    porta &= ~(1<<bit);                 \
} while (0)
#define ler_bit(pin, bit) (pin & (1<<bit))

/* Número de cervejas tomadas dividido em 4 dígitos. */
static volatile uint8_t cervejas[4] = { 0 };

/* O LED do botão está na porta C neste pino. */
static const int pino_LED = 5;

void main(void) __attribute__((noreturn));
void main(void)
{
    /* Número de cervejas tomadas - EEPROM
     * Valor inicial (quando EEPROM é gravada) */
    static uint16_t cervejas_eeprom EEMEM = 42;
    /* Número de cervejas tomadas - RAM */
    uint16_t cervejas_ram;

    uint16_t cervejas_temp;
    uint16_t botao_apertado;
    div_t div_ret;

    /* O número de cervejas tomadas fica gravado na EEPROM no endereço 0.
     * No começo do programa, esse valor é lido e guardado em uma variável
     * na memória RAM. A cada cerveja tomada, a variável é incrementada e
     * seu valor é atualizado na EEPROM.
     */
    cervejas_ram = eeprom_read_word(&cervejas_eeprom);

    cervejas_temp = cervejas_ram;

    /* Dividir com udiv() para poder usar tanto o quociente quanto o resto
     * com uma só execução da função de divisão. */
    div_ret       = udiv(cervejas_temp, 10);
    cervejas[0]   = div_ret.rem;
    cervejas_temp = div_ret.quot;
    div_ret       = udiv(cervejas_temp, 10);
    cervejas[1]   = div_ret.rem;
    cervejas_temp = div_ret.quot;
    div_ret       = udiv(cervejas_temp, 10);
    cervejas[2]   = div_ret.rem;
    cervejas[3]   = div_ret.quot;

    /* Os displays estão interligados assim:
     *     (a)     (b)
     *    /---\   /---\
     *    _   _   _   _ 
     *  1|_|2|_|3|_|4|_|
     *   |_| |_| |_| |_|
     *    |   |   |   |
     * (c)\-------/   |
     *        \-------/(d)
     *
     * Os displays 1 e 2 estão ligados sempre no mesmo número (nas mesmas
     * saídas do microcontrolador) através da conexão (a). Os displays 3 e 4
     * também estão ligados sempre no mesmo número através da conexão (b).
     * O negativo dos displays 1 e 3 estão ligados através da conexão (c)
     * e são chaveado por um transistor. O negativo dos displays 2 e 4 estão
     * ligados através da conexão (d) e também são chaveados por um
     * transistor.
     * As chaves ficam alternando o ligamento do par de displays 1 e 3 e do
     * par de displays 2 e 4 a uma frequência de aproximadamente 100Hz.
     * Assim, cada display fica aceso só metade de cada período, mas isso
     * acontece a uma frequência tão alta que nós não percebemos e parece
     * que os quatro displays estão sempre ligados.
     */

    /* Configura o timer1 para acionar uma função de interrupção (no final do
     * código) que fica alternando o ligamento dos displays. */
    cli();
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11);
    OCR1A  = 625;
    TIMSK |= (1 << OCIE1A);
    sei();

    /* Pinos de saída e entrada de cada porta:
     * B0 a B7 => (saída  ) números para os BCDs (divido em dois nibbles)
     * D6 e D7 => (saída  ) chave para os BCDs
     * D5      => (entrada) chave do botão
     * C5      => (saída  ) LED do botão
     */
    DDRB = 0xff;
    DDRC = 0x20;
    DDRD = 0xc0;

    /* Primeiro limpamos todas as saídas. */
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;

    /* Ligamos o LED do botão. */
    ligar_bit(PORTC, pino_LED);

    /* Loop infinito que verifica se o botão foi apertado. */
    while (1) {
        /* Para evitar que o "bouncing" do botão seja considerado como várias
         * apertadas, considera-se que o botão só é efetivamente apertado se
         * estiver continuamente ligado por aproximadamente 50ms. Para isso,
         * uma variável conta quantas iterações do loop o botão esteve
         * continuamente apertado. Cada vez que o botão é considerado não
         * apertado, a variável é reiniciada.
         */
        if (ler_bit(PIND, 5))
            botao_apertado++;
        else
            botao_apertado = 0;

        /* aproximadamente 50 ms */
        if (botao_apertado == 3277) {
            uint8_t i;

            /* O botão foi apertado! */

            /* Incrementar contador de cervejas */
            cervejas_ram++;
            /* Agora, um dígito por vez. */
            for (i = 0; i < 4; i++) {
                cervejas[i]++;
                if (cervejas[i] == 10) {
                    cervejas[i] = 0;
                } else {
                    break;
                }
            }

            /* Gravar valor na EEPROM. */
            eeprom_write_word(&cervejas_eeprom, cervejas_ram);

            /* Piscar LED do botão 3 vezes. */
            for (i = 3; i; i--) {
                desligar_bit(PORTC, pino_LED);
                _delay_ms(167);
                ligar_bit(PORTC, pino_LED);
                _delay_ms(166);
            }

            /* Reiniciar contador do botão. */
            botao_apertado = 0;
        }
    }
}

/* Esta função de interrupção é chamada 100 vezes por segundo. A cada vez que
 * é chamada, os displays ligados são alternados. */
ISR(TIMER1_COMPA_vect)
{
    static uint8_t idx = 0;
    uint8_t valor_saida = cervejas[!idx] | (cervejas[!idx+2] << 4);

    desligar_bit(PORTD, (6 + idx));
    PORTB = valor_saida;
    idx = !idx;
    ligar_bit(PORTD, (6 + idx));
}
