#include "yagpio.h"
#include "gpio.h"
#include "mik32_memory_map.h"
#include "types.h"
#include "led.h"
#include "W5100.h"
#include <string.h>
#include "config.h"

#define PINS_6_TO_11_MASK (BIT(6)|BIT(7)|BIT(8)|BIT(9)|BIT(10)|BIT(11))

retv toggleAppropriateChannel(uint8_t channel, uint8_t condition);

void gpio_init(void){
    GPIO_0->DIRECTION_IN = BIT(5);
    GPIO_0->DIRECTION_OUT = BIT(10);
    
    // Пины 0-5 и 12 остаются выходами, пины 6-11 — входы
    GPIO_1->DIRECTION_OUT = (BIT(13) - 1) & ~(BIT(6)|BIT(7)|BIT(8)|BIT(9)|BIT(10)|BIT(11));
    GPIO_1->DIRECTION_IN  = (BIT(6)|BIT(7)|BIT(8)|BIT(9)|BIT(10)|BIT(11));

    GPIO_2->DIRECTION_OUT = (BIT(8) - 1) & ~BIT(0); // на выход с 7-го по 1-й включительно
}

void gpio_errorIndicator(retv resOfOperation){
    if(resOfOperation != retv::Ok){
        // Если есть ошибка, то просто зажжём светодиод
        GPIO_0->CLEAR = BIT(SUCCESS_DIOD_PIN);  // включить (активный 0) 
    }
    else{
        //Если нет ошибок, то помигаем 5 раз
        //Dыключить ERROR? включить SUCCESS
        GPIO_0->SET = BIT(ERROR_DIOD_PIN); 
        
        for(int i = 0; i < 5; i++){  // выключить
            GPIO_0->CLEAR = BIT(SUCCESS_DIOD_PIN);  // включить (активный 0) 
            for(volatile int i = 0; i < 1000000; i++);
            GPIO_0->SET   = BIT(SUCCESS_DIOD_PIN);
            for(volatile int i = 0; i < 1000000; i++);
        }
    }
}


retv makeConnectionsBetweenRelay(char* out_buf, uint16_t* out_size){
    w5100_send("OK\r\n", 4);
    if(out_buf[0] != ':') return retv::NonValideData;

    if(strncmp(out_buf + 1, "SWIT", strlen("SWIT")) == 0){
        w5100_send("I see SWIT\r\n", 12);
        char digit_char = out_buf[5];
        uint8_t relay_num = digit_char - '0'; 

        //We have only three rele: number 0m number 1, number 2
        if(relay_num > 2  || relay_num < 0) return retv::NonValideData;

        char releCondition = out_buf[7];
        uint8_t releCondition_num = releCondition - '0';
        if(releCondition_num != 1 && releCondition_num != 2) return retv::NonValideData;

        toggleAppropriateChannel(relay_num, releCondition_num);

        if(strncmp(out_buf + 8, ";SWIT", strlen(";SWIT")) == 0){
            char second_relay_digit_char = out_buf[13];
            uint8_t second_relay_relay_num = second_relay_digit_char - '0'; 

            //We have only three rele: number 0m number 1, number 2
            if(second_relay_relay_num > 2  || second_relay_relay_num < 0) return retv::NonValideData;

            char second_relay_releCondition = out_buf[15];
            uint8_t second_relay_releCondition_num = second_relay_releCondition - '0';
            if(second_relay_releCondition_num != 1 || second_relay_releCondition_num != 2) return retv::NonValideData;

            toggleAppropriateChannel(second_relay_relay_num, second_relay_releCondition_num);
        }
        else return retv::NonValideData;
    }
    else return retv::NonValideData;

    return retv::Ok;
}

retv toggleAppropriateChannel(uint8_t channel, uint8_t condition){
    w5100_send("I'm inside toggle\r\n", 19);
    if(channel == 0){
        uint8_t currentPosition = 1;
        //GPIO1_6 - телеметрия position 1 для реле 0
        //GPIO1_7 - телеметрия position 2 для реле 0
        //Считываем текущее положение реле с телеметрии
        uint32_t state_gpio1 = GPIO_1->STATE; //Считыванием состояние регистра
        if(!(state_gpio1 & BIT(6))){
                currentPosition = 1;
                w5100_send("Telemetriya for reley 0 in position 1\r\n", 39);
        }
        else if(!(state_gpio1 & BIT(7))){
             w5100_send("Telemetriya for reley 0 in position 2\r\n", 39);
            currentPosition = 2;
        }
        else return retv::Fail;

        if(condition == 1){//Т.е., если нужно переключить реле в положение 1
            if(currentPosition == 1) return retv::Ok;
            GPIO_1->SET = BIT(0); //Переключаем реле в положение 1
            w5100_send("Check the GPIO 1 port 0\r\n",26);
            delay_ms(1000);
            GPIO_1->CLEAR = BIT(0);
        }
        else{
            if(currentPosition == 2) return retv::Ok;
            GPIO_1->SET = BIT(1); //Переключаем реле в положение 1
            delay_ms(IMPULSE_DURATION_FOR_RELE_MS);
            GPIO_1->CLEAR = BIT(1);
        }
    }

    else if(channel == 1){
        uint8_t currentPosition = 1;
        //GPIO1_8 - телеметрия position 1 для реле 1
        //GPIO1_9 - телеметрия position 2 для реле 1
        //Считываем текущее положение реле с телеметрии
        uint32_t state_gpio1 = GPIO_1->STATE; //Считыванием состояние регистра
        if(!(state_gpio1 & BIT(8))){
                currentPosition = 1;
        }
        else if(!(state_gpio1 & BIT(9))){
            currentPosition = 2;
        }
        else return retv::Fail;

        if(condition == 1){//Т.е., если нужно переключить реле в положение 1
            if(currentPosition == 1) return retv::Ok; //Если оно уже в положении 1
            GPIO_1->SET = BIT(2); //Переключаем реле в положение 1
            delay_ms(IMPULSE_DURATION_FOR_RELE_MS);
            GPIO_1->CLEAR = BIT(2);
        }
        else{
            if(currentPosition == 2) return retv::Ok;
            GPIO_1->SET = BIT(3); //Переключаем реле в положение 1
            delay_ms(IMPULSE_DURATION_FOR_RELE_MS);
            GPIO_1->CLEAR = BIT(3);
        }
    }

    else{
        uint8_t currentPosition = 1;
        //GPIO1_10 - телеметрия position 1 для реле 2
        //GPIO1_11 - телеметрия position 2 для реле 2
        //Считываем текущее положение реле с телеметрии
        uint32_t state_gpio1 = GPIO_1->STATE; //Считыванием состояние регистра
        if(!(state_gpio1 & BIT(10))){
                currentPosition = 1;
        }
        else if(!(state_gpio1 & BIT(11))){
            currentPosition = 2;
        }
        else return retv::Fail;

        if(condition == 1){//Т.е., если нужно переключить реле в положение 1
            if(currentPosition == 1) return retv::Ok; //Если оно уже в положении 1
            GPIO_1->SET = BIT(4); //Переключаем реле в положение 1
            delay_ms(IMPULSE_DURATION_FOR_RELE_MS);
            GPIO_1->CLEAR = BIT(4);
        }
        else{
            if(currentPosition == 2) return retv::Ok;
            GPIO_1->SET = BIT(5); //Переключаем реле в положение 1
            delay_ms(IMPULSE_DURATION_FOR_RELE_MS);
            GPIO_1->CLEAR = BIT(5);
        }
    }

    return retv::Ok;
}