#include "yagpio.h"
#include "gpio.h"
#include "mik32_memory_map.h"
#include "types.h"
#include "led.h"
#include "W5100.h"
#include <string.h>
#include "config.h"
#include "timers.h"

retv ensureRelayPosition(uint8_t relay, uint8_t condition);
static void clockTicksForTelemetry(uint8_t numberOfTicks);
void checkCurrentTelemetryOfTheRelay(uint8_t relay, uint8_t* telemetryBufForOneRELAY);
static retv toggleRELAY(uint8_t relay, uint8_t condition);
static retv sendPositionData(uint8_t relay, uint8_t* telemetryBuf);

void gpio_init(void){
    //Put initial values
    GPIO_0->SET = CTRL_NENABLE_BUFFER1_PART1 | CTRL_NENABLE_BUFFER1_PART2 | CTRL_NENABLE_BUFFER2_PART1;
    GPIO_0->CLEAR = TM_CLK_PIN;

    GPIO_1->SET = TM_PL_PIN | TM_CE_PIN | CTRL_NENABLE_BUFFER2_PART2 | CTRL_NENABLE_BUFFER3_PART1 | CTRL_NENABLE_BUFFER3_PART2;


    GPIO_0->DIRECTION_OUT = (TM_CLK_PIN|CTRL_NENABLE_BUFFER1_PART1|CTRL_NENABLE_BUFFER1_PART2|CTRL_NENABLE_BUFFER2_PART1);

    GPIO_1->DIRECTION_OUT = (TM_PL_PIN|TM_CE_PIN|CTRL_NENABLE_BUFFER2_PART2|CTRL_NENABLE_BUFFER3_PART1|CTRL_NENABLE_BUFFER3_PART2);
    GPIO_1->DIRECTION_IN  = TM_DATA_PIN; //Вход телеметрии

}

void gpio_errorIndicator(retv resOfOperation){
    if(resOfOperation != retv::Ok){
        // Если есть ошибка, то просто зажжём светодиод
        GPIO_0->CLEAR = BIT(INDICATE_DIOD_PORT);  // включить (активный 0) 
    }
    else{
        //Если нет ошибок, то помигаем 5 раз
        for(int i = 0; i < MISTAKE_BLINK; i++){  // выключить
            GPIO_0->CLEAR = BIT(INDICATE_DIOD_PORT);  // включить (активный 0) 
            for(volatile int i = 0; i < 1000000; i++);
            GPIO_0->SET   = BIT(INDICATE_DIOD_PORT);
            for(volatile int i = 0; i < 1000000; i++);
        }
    }
}


retv parseTheCommand(char* out_buf, uint16_t* out_size){
    if(out_buf[CMD_START_SYMBOL_OFFSET] != CMD_START_SYMBOL) return retv::NonValideData;

    if(STR_MATCH(out_buf, CMD_TOGGLE_OFFSET, CMD_TOGGLE)){
        //проваливаемся в условие, значит команда верная
        char digit_char = out_buf[CMD_FIRST_RELE_NUMBER_OFFSET]; //Считываем номер реле
        uint8_t relay_num = digit_char - CHAR_ZERO; 

        //We have only three RELAY: number 0, number 1, number 2
        if(relay_num > RELAY_2) return retv::NonValideData;

        if(out_buf[CMD_SPACE_SYMBOL_OFFSET] != CMD_SPACE_SYMBOL) return retv::NonValideData;

        char RELAYCondition = out_buf[CMD_FIRST_RELE_POSITION_OFFSET];
        uint8_t RELAYCondition_num = RELAYCondition - CHAR_ZERO;
        if(RELAYCondition_num != RELAY_POSITION_1 && RELAYCondition_num != RELAY_POSITION_2) return retv::NonValideData;

        if(ensureRelayPosition(relay_num, RELAYCondition_num) == retv::Fail){
            W5100_SEND_STR(MSG_MISTAKE);
            return retv::Fail;
        }
        if(STR_MATCH(out_buf, CMD_SECOND_TOGGLE_OFFSET, CMD_SECOND_TOGGLE)){
            char second_relay_digit_char = out_buf[CMD_SECOND_RELE_NUMBER_OFFSET];
            uint8_t second_relay_relay_num = second_relay_digit_char - CHAR_ZERO; 

            //We have only three RELAY: number 0m number 1, number 2
            if(second_relay_relay_num > RELAY_2) return retv::NonValideData;

            char second_relay_RELAYCondition = out_buf[CMD_SECOND_RELE_POSITION_OFFSET];
            uint8_t second_relay_RELAYCondition_num = second_relay_RELAYCondition - CHAR_ZERO;
            if(second_relay_RELAYCondition_num != RELAY_POSITION_1 && second_relay_RELAYCondition_num != RELAY_POSITION_2) return retv::NonValideData;

            if(ensureRelayPosition(second_relay_relay_num, second_relay_RELAYCondition_num) == retv::Fail){
                W5100_SEND_STR(MSG_MISTAKE);
                return retv::Fail;
            }
        }
        else{
            W5100_SEND_STR(MSG_MISTAKE);
            return retv::NonValideData;
        }
    }
    else if(STR_MATCH(out_buf, CMD_CHECK_OFFSET, CMD_CHECK)){
        uint8_t telemetryBuf[TELEMETRY_BUF_SIZE];
        if(STR_MATCH(out_buf, CMD_RELAY_NUMBER_OFFSET, CMD_RELAY_0)){
            if(sendPositionData(RELAY_0, telemetryBuf) == retv::Fail) return retv::Fail;
        }
        else if(STR_MATCH(out_buf, CMD_RELAY_NUMBER_OFFSET, CMD_RELAY_1)){
            if(sendPositionData(RELAY_1, telemetryBuf) == retv::Fail) return retv::Fail;
        }

        else if(STR_MATCH(out_buf, CMD_RELAY_NUMBER_OFFSET, CMD_RELAY_2)){
            if(sendPositionData(RELAY_2, telemetryBuf) == retv::Fail) return retv::Fail;
        }
        else if(STR_MATCH(out_buf, CMD_RELAY_NUMBER_OFFSET, CMD_CHECK_ALL)){
            for(int i = 0; i < NUMBER_OF_RELAY; i++){
                if(sendPositionData(i, telemetryBuf) == retv::Fail) return retv::Fail;
            }
        }
    }
    else{
        W5100_SEND_STR(MSG_MISTAKE);
        return retv::NonValideData;
    }

    W5100_SEND_STR(MSG_OK);
    return retv::Ok;
}

retv ensureRelayPosition(uint8_t channel, uint8_t condition){
    uint8_t currentPosition = RELAY_POSITION_1;
    uint8_t telemetryBufForOneRELAY[TELEMETRY_BUF_SIZE];
    checkCurrentTelemetryOfTheRelay(channel, telemetryBufForOneRELAY);

    if(telemetryBufForOneRELAY[TELEMETRU_BUF_POSITON_1]){
        currentPosition = RELAY_POSITION_1;
    }
    else if(telemetryBufForOneRELAY[TELEMETRU_BUF_POSITON_2]){
        currentPosition = RELAY_POSITION_2;
    }
    else return retv::Fail;

    if(condition == RELAY_POSITION_1){//Т.е., если нужно переключить реле в положение 1
        if(currentPosition == RELAY_POSITION_1) return retv::Ok;
        if(toggleRELAY(channel, condition) == retv::Fail) return retv::Fail;
    }
    else{
        if(currentPosition == RELAY_POSITION_2) return retv::Ok;
        if(toggleRELAY(channel, condition) == retv::Fail) return retv::Fail;
    }
    return retv::Ok;
}



void checkCurrentTelemetryOfTheRelay(uint8_t RELAY, uint8_t* telemetryBufForOneRELAY){
    GPIO_1->CLEAR = TM_PL_PIN; //Открываем параллельную запись
    __asm volatile("nop"); 
    GPIO_1->SET = TM_PL_PIN; //Защёлкиваем данные

    GPIO_1->CLEAR = TM_CE_PIN; //Разрешаем тактирование
    
    if(RELAY == RELAY_2){
        //Даём 2 clock
        clockTicksForTelemetry(2);
         __asm volatile("nop"); 

        uint32_t state_gpio1 = GPIO_1->STATE; //Считыванием состояние регистра
        telemetryBufForOneRELAY[TELEMETRU_BUF_POSITON_2] = (state_gpio1 & TM_DATA_PIN) >> TM_DATA_PIN_S;

        //Затем даём 1 clock
        clockTicksForTelemetry(1);
         __asm volatile("nop"); 

        state_gpio1 = GPIO_1->STATE; //Снова считыванием состояние регистра
        telemetryBufForOneRELAY[TELEMETRU_BUF_POSITON_1] = (state_gpio1 & TM_DATA_PIN) >> TM_DATA_PIN_S;
    }

    else if(RELAY == RELAY_2){
        //Даём 4 clock
        clockTicksForTelemetry(4);
         __asm volatile("nop"); 

        uint32_t state_gpio1 = GPIO_1->STATE; //Считыванием состояние регистра
        telemetryBufForOneRELAY[TELEMETRU_BUF_POSITON_2] = (state_gpio1 & TM_DATA_PIN) >> TM_DATA_PIN_S;

        //Затем даём 1 clock
        clockTicksForTelemetry(1);
         __asm volatile("nop"); 

        state_gpio1 = GPIO_1->STATE; //Снова считыванием состояние регистра
        telemetryBufForOneRELAY[TELEMETRU_BUF_POSITON_1] = (state_gpio1 & TM_DATA_PIN) >> TM_DATA_PIN_S;
    }
    else{
        //Даём 6 clock
        clockTicksForTelemetry(6);
         __asm volatile("nop"); 

        uint32_t state_gpio1 = GPIO_1->STATE; //Считыванием состояние регистра
        telemetryBufForOneRELAY[TELEMETRU_BUF_POSITON_2] = (state_gpio1 & TM_DATA_PIN) >> TM_DATA_PIN_S;

        //Затем даём 1 clock
        clockTicksForTelemetry(1);
         __asm volatile("nop"); 

        state_gpio1 = GPIO_1->STATE; //Снова считыванием состояние регистра
        telemetryBufForOneRELAY[TELEMETRU_BUF_POSITON_1] = (state_gpio1 & TM_DATA_PIN) >> TM_DATA_PIN_S;
    }

    GPIO_1->SET = TM_CE_PIN; //Запрещаем тактирование
}



static void clockTicksForTelemetry(uint8_t numberOfTicks){
    while(numberOfTicks){
        GPIO_0->SET = TM_CLK_PIN;
        __asm volatile ("nop");
        GPIO_0->CLEAR = TM_CLK_PIN;
        __asm volatile ("nop");
        numberOfTicks--;
    }
}

static retv toggleRELAY(uint8_t relay, uint8_t condition){
    if(relay == RELAY_0){
        GPIO_0->CLEAR = CTRL_NENABLE_BUFFER1_PART1;
        __asm volatile ("nop");
        if(condition == RELAY_POSITION_1){
            relay_pulse(CHANNEL_FOR_RELAY_PULSE_0);
        }
        else{
            relay_pulse(CHANNEL_FOR_RELAY_PULSE_1);
        }

        GPIO_0->SET = CTRL_NENABLE_BUFFER1_PART1;
    }
    else if(relay == RELAY_1){
        GPIO_0->CLEAR = CTRL_NENABLE_BUFFER1_PART1;
        __asm volatile ("nop");
        if(condition == RELAY_POSITION_1){
            relay_pulse(CHANNEL_FOR_RELAY_PULSE_2);
        }
        else{
            relay_pulse(CHANNEL_FOR_RELAY_PULSE_3);
        }
        GPIO_0->SET = CTRL_NENABLE_BUFFER1_PART1;
    }

    // Relay 2
    else{
        GPIO_0->CLEAR = CTRL_NENABLE_BUFFER1_PART2;
        __asm volatile ("nop");
        if(condition == RELAY_POSITION_1){
            relay_pulse(CHANNEL_FOR_RELAY_PULSE_0);  
        }
        else{
            relay_pulse(CHANNEL_FOR_RELAY_PULSE_1);
        }
        GPIO_0->SET = CTRL_NENABLE_BUFFER1_PART2;
    }

    //Проверка: всё ли переключилось, как должно было?
    uint8_t telemetryBuf[TELEMETRY_BUF_SIZE];
    checkCurrentTelemetryOfTheRelay(relay, telemetryBuf);
    if(telemetryBuf[condition - 1]) return retv::Ok;
    else return retv::Fail;
}

static retv sendPositionData(uint8_t relay, uint8_t* telemetryBuf){
        checkCurrentTelemetryOfTheRelay(relay, telemetryBuf);
        if(telemetryBuf[TELEMETRU_BUF_POSITON_1]){
            W5100_SEND_STR(MSG_POSITION_1);
        }
        else if(telemetryBuf[TELEMETRU_BUF_POSITON_2]){
            W5100_SEND_STR(MSG_POSITION_2);
        }
        else{
            W5100_SEND_STR(MSG_MISTAKE);
            return retv::Fail;
        }
        return retv::Ok;
    }
