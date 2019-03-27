#ifndef SIGNAL_GENERATOR_FREQUENCYINCREMENT_H
#define SIGNAL_GENERATOR_FREQUENCYINCREMENT_H

#endif //SIGNAL_GENERATOR_FREQUENCYINCREMENT_H

typedef struct {
    int num;
    int32_t step;
    char* description;
} FrequencyIncrement;

#define INCREMENTS_SIZE 8
const FrequencyIncrement increments[8] {
        {0, 100, "100 Hz" },
        {1, 500, "500 Hz" },
        {2, 1000, "1 KHz" },
        {3, 2500, "2.5 KHz" },
        {4, 5000, "5 KHz" },
        {5, 10000, "10 KHz" },
        {6, 100000, "100 KHz" },
        {7, 1000000, "1 MHz" }
};

void formatFrequency(char* buf, int32_t n) {
    if (n < 1000) {
        buf[0] = '\0';
        sprintf(buf+strlen(buf), "%d", n);
        return;
    }
    formatFrequency(buf, n / 1000);
    sprintf(buf+strlen(buf), ",%03d", n %1000);
    return;
}