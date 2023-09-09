#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#define STR_SIZE 32

void print_names(){
    printf("Информация о системе:\n");
    uint8_t buff[STR_SIZE];
    gethostname(buff, STR_SIZE);
    printf("\tИмя компьютера: %s\n", buff);
    memset(buff, 0, STR_SIZE);
    getlogin_r(buff, STR_SIZE);
    printf("\tИмя пользователя: %s\n", buff);
    struct utsname utsn;
    uname(&utsn);
    printf("\tВерсия системы: %s\n", utsn.version);
}

void print_sysmetrics(){
    printf("Системные метрики:\n");
    printf("\tКоличество ядер процессора: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    printf("\tКоличество потоков процессора: %ld\n", sysconf(_SC_NPROCESSORS_CONF));
    printf("\tЧастота системного таймера: %ld\n", sysconf(_SC_CLK_TCK));
}

//Возвращает кол-во наносекунд в формате UTC в переменную 64 бита
uint64_t timestamp(){
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)(ts.tv_sec)*1000000000ULL + (uint64_t)(ts.tv_nsec);
}

//Структура для хранения времени
struct TIME_INFO {
  uint32_t YYYY, MM, DD, w, hh, mm, ss, ns;
};

//Функция преобразующая timestamp в формат YYYY.MM.DD-HH:MM:SS.NS
int utc2str(uint8_t* dst, int cbDstMax, uint64_t utc) {
  struct TIME_INFO ti;
  struct tm tm = {}; ti.ns = utc%1000000000ULL; time_t t = utc/1000000000ULL; gmtime_r(&t, &tm);
  ti.YYYY = 1900+tm.tm_year; ti.MM = 1+tm.tm_mon; ti.DD = tm.tm_mday;
  ti.hh = tm.tm_hour; ti.mm = tm.tm_min; ti.ss = tm.tm_sec; ti.w = tm.tm_wday;
  return snprintf(dst, cbDstMax, "%04u.%02u.%02u-%02u:%02u:%02u.%06u"
    , ti.YYYY, ti.MM, ti.DD, ti.hh, ti.mm, ti.ss, ti.ns/1000
  );
}

void print_time(){
    printf("Взаимодействие с временем:\n");
    printf("\tВременная отметка UTC: %llu нс.\n", timestamp());
    uint8_t buff[STR_SIZE];
    utc2str(buff, STR_SIZE, timestamp());
    printf("\tПреобразованная временная отметка: %s\n", buff);
}

void print_extra(){
    printf("Дополнительные вызовы:\n");
    double load[3];
    getloadavg(load, 3);
    printf("\tЗагрузка процессора за 5 минут: %.2f\n", load[0]);

    struct sysinfo sinfo;
    if(sysinfo(&sinfo) < 0){
        printf("Ошибка получения информации о системе!\n");
        return;
    }
    printf("\tСистема работает %ld секунд\n", sinfo.uptime);


    FILE* file;
    uint64_t rx;
    file = fopen("/sys/class/net/enp5s0/statistics/rx_bytes", "r");
    if(file == NULL){   
        printf("Ошибка открытия файла!\n");
    }
    fscanf(file, "%llu", &rx);
    printf("\tКол-во байт входящих на интерфейсе enp5s0: %llu\n", rx);
    fclose(file);


    uint8_t buff[STR_SIZE];
    file = fopen("/sys/class/net/enp5s0/address", "r");
    if(file == NULL){
        printf("Ошибка открытия файла!\n");
    }
    fscanf(file, "%s", buff);
    printf("\tMAC-адрес интерфейса enp5s0: %s\n", buff);
    fclose(file);
    
}


int main(){
    print_names();
    print_sysmetrics();
    print_time();
    print_extra();
    return 0;
}