#include <stdio.h> //biblioteca padrão da linguagem C
#include "pico/stdlib.h" //subconjunto central de bibliotecas do SDK Pico
#include "pico/time.h" //biblioteca para gerenciamento de tempo
#include "hardware/irq.h" //biblioteca para gerenciamento de interrupções
#include "hardware/pwm.h" //biblioteca para controlar o hardware de PWM

#define LEDPin 12 //pino do LED conectado a GPIO como PWM
#define ServoPin 22 //pino do servomotor conectado a GPIO como PWM

// Parâmetros do PWM
const uint16_t PWM_MAX = 25000;  // De acordo ao passo 1, definindo o valor do contador PWM para 50Hz
const float PWM_DIV_CLOCK = 100;  // Ajuste do divisor de clock

// Duty Cycles para a angulacao correspondente do servo
const uint16_t ANGLE_0 = 625;    // Passo 4: 0° → 500µs (ciclo ativo)
const uint16_t ANGLE_90 = 1837;  // Passo 3: 90° → 1470µs
const uint16_t ANGLE_180 = 3000; // Passo 2: 180° → 2400µs
const uint16_t ANGLE_STEP = (5 * PWM_MAX / 20000); // Passo 5: Incremento de 5µs

/* funcao wraphandler a ficar desativada depois para teste*/
// Tratamento da interrupção do PWM para LED
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
        if(fade < 0){ //caso o fade seja menor que 0
            fade = 0; //iguala fade a 0
            rise = true; //muda o flag rise para elevação no nível de iluminação
        }
    }

    pwm_set_gpio_level(LEDPin, fade * fade); //define o ciclo ativo (Ton) de forma quadrática, para acentuar a variação de luminosidade.
}
/* funcao pwm_setup_irq a ficar desativada depois para teste*/
//Configuração do PWM
uint pwm_setup_irq(){
    gpio_set_function(LEDPin, GPIO_FUNC_PWM); //habilitar o pino GPIO como PWM
    uint sliceNum = pwm_gpio_to_slice_num(LEDPin); //obter o canal PWM da GPIO

    pwm_clear_irq(sliceNum); //resetar o flag de interrupção para o slice
    pwm_set_irq_enabled(sliceNum, true); //habilitar a interrupção de PWM para um dado slice
    irq_set_exclusive_handler(PWM_IRQ_WRAP, wrapHandler); //Definir um tipo de interrupção.
    irq_set_enabled(PWM_IRQ_WRAP, true); //Habilitar ou desabilitar uma interrupção específica

    pwm_config config = pwm_get_default_config(); //obtem a configuração padrão para o PWM
    pwm_config_set_clkdiv(&config, 4.f); //define o divisor de clock do PWM
    pwm_init(sliceNum, &config, true); //inicializa o PWM com as configurações do objeto

    return sliceNum;
}

// Configuração do PWM para o LED
void pwm_setup_led() {   
    gpio_set_function(LEDPin, GPIO_FUNC_PWM); // Define GPIO como PWM
    uint slice = pwm_gpio_to_slice_num(LEDPin); // Obtém o slice PWM do GPIO

    pwm_set_clkdiv(slice, PWM_DIV_CLOCK); // Configura o divisor de clock
    pwm_set_wrap(slice, PWM_MAX); // Define o valor máximo do contador
    pwm_set_enabled(slice, true); // Habilita o PWM no slice correspondente

}

// Função para Movimentar o led ao inves do servo de forma suave entre 0 e 180 graus 
void rotate_led(uint16_t start, uint16_t end) {
    if (start < end) {
        for (uint16_t duty = start; duty <= end; duty += ANGLE_STEP) {
            pwm_set_gpio_level(LEDPin, duty);
            sleep_ms(10);
        }
    } else {
        for (uint16_t duty = start; duty >= end; duty -= ANGLE_STEP) {
            pwm_set_gpio_level(LEDPin, duty);
            sleep_ms(10);
        }
    }
}

// Configuração do PWM para o Servo
void pwm_setup_servo() {   
    gpio_set_function(ServoPin, GPIO_FUNC_PWM); // Define GPIO como PWM
    uint slice = pwm_gpio_to_slice_num(ServoPin); // Obtém o slice PWM do GPIO

    pwm_set_clkdiv(slice, PWM_DIV_CLOCK); // Configura o divisor de clock
    pwm_set_wrap(slice, PWM_MAX); // Define o valor máximo do contador
    pwm_set_enabled(slice, true); // Habilita o PWM no slice correspondente

}

// Função para Movimentar o servo de forma suave entre 0 e 180 graus 
void rotate_servo(uint16_t start, uint16_t end) {
    if (start < end) {
        for (uint16_t duty = start; duty <= end; duty += ANGLE_STEP) {
            pwm_set_gpio_level(ServoPin, duty);
            sleep_ms(10);
        }
    } else {
        for (uint16_t duty = start; duty >= end; duty -= ANGLE_STEP) {
            pwm_set_gpio_level(ServoPin, duty);
            sleep_ms(10);
        }
    }
}

int main(){
    stdio_init_all();
    char msg_passo4[] = "Servo em 0°";
    char msg_passo3[] = "Servo em 90°";
    char msg_passo2[] = "Servo em 180°";
    pwm_setup_irq();

    while(1){
        pwm_setup_servo(); // Inicializa o PWM na GPIO do servo
        uint slice_num = pwm_gpio_to_slice_num(ServoPin);

        //pwm_setup_led(); // Inicializa o PWM na GPIO do led
        //uint slice_num = pwm_gpio_to_slice_num(LEDPin);

        // Definição das posições iniciais com pausas de 5 segundos
        printf("\n LED no brilho maximo\n");
        pwm_set_gpio_level(ServoPin, ANGLE_180); // <-------------
        //pwm_set_gpio_level(LEDPin, ANGLE_180); // <-------------
        printf("%s\n", msg_passo4);
        sleep_ms(5000);

        printf("\n LED no brilho medio\n");
        rotate_servo(ANGLE_180, ANGLE_90); // <-------------
        //rotate_led(ANGLE_180, ANGLE_90); // <-------------
        printf("%s\n", msg_passo3);
        sleep_ms(5000);

        printf("\n🔄 LED no brilho minimo\n");
        rotate_servo(ANGLE_90, ANGLE_0); // <-------------
        //rotate_led(ANGLE_90, ANGLE_0); // <-------------
        printf("%s\n", msg_passo2);
        sleep_ms(5000);

        // De 0° e 180° suavemente indefinidamente
        printf("\n🔄 Variando entre minimo e máximo...\n");
        uint32_t start_time = to_ms_since_boot(get_absolute_time());

        while (1) { 
            rotate_servo(ANGLE_180, ANGLE_0);
            rotate_servo(ANGLE_0, ANGLE_180);
        }
    }
}
