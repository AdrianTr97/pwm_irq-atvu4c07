#include <stdio.h> //biblioteca padrão da linguagem C
#include "pico/stdlib.h" //subconjunto central de bibliotecas do SDK Pico
#include "pico/time.h" //biblioteca para gerenciamento de tempo
#include "hardware/irq.h" //biblioteca para gerenciamento de interrupções
#include "hardware/pwm.h" //biblioteca para controlar o hardware de PWM

#define LEDPin 12 //pino do LED conectado a GPIO como PWM

//tratamento da interrupção do PWM
void wrapHandler(){ 
    static int fade = 0; //nível de iluminação
    static bool rise = true; //flag para elevar ou reduzir a iluminação
    pwm_clear_irq(pwm_gpio_to_slice_num(LEDPin)); //resetar o flag de interrupção

    if(rise){ //caso a iluminação seja elevada
        fade++; //aumenta o nível de brilho
        if(fade > 255){ //caso o fade seja maior que 255
            fade = 255; //iguala fade a 255
            rise = false; //muda o flag rise para redução do nível de iluminação
        }
    }
    else{ //caso a iluminação seja reduzida
        fade--; //reduz o nível de brilho
        if(fade < 0){ //Icaso o fade seja menor que 0
            fade = 0; //iguala fade a 0
            rise = true; //muda o flag rise para elevação no nível de iluminação
        }
    }

    pwm_set_gpio_level(LEDPin, fade * fade); //define o ciclo ativo (Ton) de forma quadrática, para acentuar a variação de luminosidade.
}

//Configuração do PWM com interrupção
uint pwm_setup_irq(){

    gpio_set_function(LEDPin, GPIO_FUNC_PWM); //habilitar o pino GPIO como PWM
    uint sliceNum = pwm_gpio_to_slice_num(LEDPin); //obter o canal PWM da GPIO

    pwm_clear_irq(sliceNum); //resetar o flag de interrupção para o slice
    pwm_set_irq_enabled(sliceNum, true); //habilitar a interrupção de PWM para um dado slice
    irq_set_exclusive_handler(PWM_IRQ_WRAP, wrapHandler); //Definir um tipo de interrupção.
    // Esta interrupção (PWM_IRQ_WRAP) é gerada quando um contador de um slice atinge seu valor de wrap
    irq_set_enabled(PWM_IRQ_WRAP, true); //Habilitar ou desabilitar uma interrupção específica

    pwm_config config = pwm_get_default_config(); //obtem a configuração padrão para o PWM
    pwm_config_set_clkdiv(&config, 4.f); //define o divisor de clock do PWM
    pwm_init(sliceNum, &config, true); //inicializa o PWM com as configurações do objeto

    return sliceNum;
}

int main(){

    uint sliceNum = pwm_setup_irq(); //função que inicializa o PWM com interrupção por wrap

    while(1){

        printf("Interrupção do PWM ativa");
        pwm_set_irq_enabled(sliceNum, true); //habilita a interrupção
        sleep_ms(5000);
        printf("Interrupção do  PWM desativada"); //desabilita a interrupção
        pwm_set_irq_enabled(sliceNum, false);
        sleep_ms(5000);

    }
}