#include "W5100.h"
#include "spi_functions.h"
#include "types.h"
#include "spi.h"
#include "mik32_memory_map.h"
#include "yagpio.h"

extern volatile bool itFromW5200;
retv w5100_receive(char* out_buf, uint16_t* out_size);
static retv w5100_reopen_socket(void);

retv w5100_write(uint16_t addr, uint8_t data){
    spi_0_cs_low();
    spi_0_transfer(0xF0, nullptr);
    spi_0_transfer(addr >> 8, nullptr);
    spi_0_transfer(addr & 0xFF, nullptr);
    retv result = spi_0_transfer(data, nullptr);
    spi_0_cs_high();
    return result;
}

retv w5100_read(uint16_t addr, uint8_t* rx){
    spi_0_cs_low();
    spi_0_transfer(0x0F, nullptr); //Байт показывает, что идёт чтение
    spi_0_transfer(addr >> 8, nullptr); //Старшие 8 бит шестнадцатибитного адреса
    spi_0_transfer(addr & 0xFF, nullptr); //Младшие 8 бит шестнадцатибитного адреса
    retv result = spi_0_transfer(0x00, rx); //Отправляем в качестве данных заглушку 0x00, чтобы просто принять данные
    spi_0_cs_high();
    return result;
}

retv w5100_init(void){
    volatile uint8_t rx = 0;   
    w5100_read(W5100_MR, (uint8_t*)&rx);
    volatile uint8_t whoami = 0;
    w5100_read(0x001A, (uint8_t*)&whoami);

    CHECK(w5100_write(W5100_MR, W5100_MR_RST));
    // Задержка после сброса
    for(volatile int i = 0; i < 100000; i++);
    w5100_read(0x001A, (uint8_t*)&whoami);

    //Регистр RMSR (RX Memory Size Register)
    //и TMSR - (TX Memory Size Register)
    CHECK(w5100_write(W5100_RMSR, W5100_RMSR_S0_8KB));
    CHECK(w5100_write(W5100_TMSR, W5100_TMSR_S0_8KB));

    //Регистр SHAR (Source Hardware Address Register) хранит MAC-адресс устройства
    CHECK(w5100_write(W5100_SHAR0 + 0, MAC_ADDR_0));
    CHECK(w5100_write(W5100_SHAR0 + 1, MAC_ADDR_1));
    CHECK(w5100_write(W5100_SHAR0 + 2, MAC_ADDR_2));
    CHECK(w5100_write(W5100_SHAR0 + 3, MAC_ADDR_3));
    CHECK(w5100_write(W5100_SHAR0 + 4, MAC_ADDR_4));
    CHECK(w5100_write(W5100_SHAR0 + 5, MAC_ADDR_5));

    // IP МК
    CHECK(w5100_write(W5100_SIPR0 + 0, IP_ADDR_0)); // 192
    CHECK(w5100_write(W5100_SIPR0 + 1, IP_ADDR_1)); // 168
    CHECK(w5100_write(W5100_SIPR0 + 2, IP_ADDR_2)); // 1
    CHECK(w5100_write(W5100_SIPR0 + 3, IP_ADDR_3)); // 100

    // Шлюз
    CHECK(w5100_write(W5100_GAR0 + 0, GW_ADDR_0));  // 192
    CHECK(w5100_write(W5100_GAR0 + 1, GW_ADDR_1));  // 168
    CHECK(w5100_write(W5100_GAR0 + 2, GW_ADDR_2));  // 1
    CHECK(w5100_write(W5100_GAR0 + 3, GW_ADDR_3));  // 1

    // Маска подсети
    CHECK(w5100_write(W5100_SUBR0 + 0, SUB_ADDR_0)); // 255
    CHECK(w5100_write(W5100_SUBR0 + 1, SUB_ADDR_1)); // 255
    CHECK(w5100_write(W5100_SUBR0 + 2, SUB_ADDR_2)); // 255
    CHECK(w5100_write(W5100_SUBR0 + 3, SUB_ADDR_3)); // 0

    //Разрешаем прерывание от сокета 0
    CHECK(w5100_write(W5100_IMR, W5100_IMR_S0_INT_EN));
    
     // ==== Инициализация сокета 0 как TCP-сервера ====
     CHECK(w5100_reopen_socket());

    return retv::Ok;
}


retv w5100_itHandler(){
    retv result = retv::Ok;
    uint8_t s0_ir = 0;

    // 1. Читаем причину прерывания сокета 0
    CHECK(w5100_read(S0_IR, &s0_ir));

    if (s0_ir & S0_IR_CON_M) {
        // Клиент подключился
    }

    if (s0_ir & S0_IR_DISCON_M) {
        // Клиент отключился
        result = w5100_reopen_socket();
    }

    if (s0_ir & S0_IR_RECV_M) {
        // Пришли данные
        char rx_buf[128];
        uint16_t rx_size = 0;
        w5100_receive(rx_buf, &rx_size);

        makeConnectionsBetweenRelay(rx_buf, &rx_size);
    }

    if (s0_ir & S0_IR_TIMEOUT_M) {
        // Таймаут соединения
        result = w5100_reopen_socket();
    }

    // 2. Сбрасываем обработанные биты (запись '1' сбрасывает соответствующий бит)
    CHECK(w5100_write(S0_IR, s0_ir));

    itFromW5200 = false;
    return result;
}


retv w5100_receive(char* out_buf, uint16_t* out_size){
    //Определяем сколько байт пришло
    uint8_t rsr_hi = 0, rsr_lo = 0;
    CHECK(w5100_read(S0_RX_RSR0, &rsr_hi));
    CHECK(w5100_read(S0_RX_RSR1, &rsr_lo));
    uint16_t size = ((uint16_t)rsr_hi << 8) | rsr_lo;
    *out_size = size;
     if (size == 0) {
        return retv::Ok;
    }

    //Проверяем, не привысил ли размер принятых байт максимальный размер
    if (size > RX_BUF_SIZE - 1) return retv::NonValideData;
    
    // 2. Читаем текущую позицию курсора чтения (2 байта, big-endian)
    uint8_t rd_hi = 0, rd_lo = 0;
    CHECK(w5100_read(S0_RX_RD0, &rd_hi));
    CHECK(w5100_read(S0_RX_RD1, &rd_lo));
    uint16_t rd_ptr = ((uint16_t)rd_hi << 8) | rd_lo;

    //Так как буфер кольцевой, нельзя выходить за его пределы
    uint16_t offset       = rd_ptr & gS0_RX_MASK;   // позиция внутри буфера
    uint16_t physical_addr = gS0_RX_BASE + offset;   // реальный адрес в памяти W5100

    //Учитываем переполнение буфера
    if ((offset + size) > (gS0_RX_MASK + 1)) {
        uint16_t upper_size = (gS0_RX_MASK + 1) - offset;
        uint16_t left_size  = size - upper_size;

         // Первый кусок — от physical_addr до конца буфера
        for (uint16_t i = 0; i < upper_size; i++) {
            CHECK(w5100_read(physical_addr + i, (uint8_t*)&out_buf[i]));
        }
        // Второй кусок — с начала буфера
        for (uint16_t i = 0; i < left_size; i++) {
            CHECK(w5100_read(gS0_RX_BASE + i, (uint8_t*)&out_buf[upper_size + i]));
        }
    } else {
        // Данные лежат непрерывно
        for (uint16_t i = 0; i < size; i++) {
            CHECK(w5100_read(physical_addr + i, (uint8_t*)&out_buf[i]));
        }
    }

    // Завершаем строку нулём
    out_buf[size] = '\0';

    //Двигаем курсор чтения вперёд
    rd_ptr += size;
    CHECK(w5100_write(S0_RX_RD0, (rd_ptr >> 8) & 0xFF));
    CHECK(w5100_write(S0_RX_RD1, rd_ptr & 0xFF));

    //Защёлкиваем изменения RD
    CHECK(w5100_write(S0_CR, S0_CR_RECV));

    return retv::Ok;
}

static retv w5100_reopen_socket(void){
    // 1. Устанавливаем протокол TCP
    CHECK(w5100_write(S0_MR, S0_MR_TCP));

    // 2. Задаём порт для прослушивания
    CHECK(w5100_write(S0_PORT0, (LOCAL_TCP_PORT >> 8) & 0xFF));
    CHECK(w5100_write(S0_PORT1, LOCAL_TCP_PORT & 0xFF));

    // 3. Открываем сокет
    CHECK(w5100_write(S0_CR, S0_CR_OPEN));

    // Проверяем, что сокет перешёл в SOCK_INIT
    volatile uint8_t sr = 0;
    CHECK(w5100_read(S0_SR, (uint8_t*)&sr));
    if (sr != SOCK_INIT) {
        return retv::Fail;
    }
    // 4. Переводим сокет в режим прослушивания (TCP Server)
    CHECK(w5100_write(S0_CR, S0_CR_LISTEN));

    CHECK(w5100_read(S0_SR, (uint8_t*)&sr));
    if (sr != SOCK_LISTEN) {
        return retv::Fail;
    }
    return retv::Ok;
}