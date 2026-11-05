
#ifndef BUZZER_H
#define BUZZER_H


class Buzzer {
    private:
        int pin;
        bool active;

    static constexpr int FREQUENCY = 2000;
    static constexpr int RESOLUTION = 8;
    static constexpr int DUTY_CYCLE = 128;

    public:
        explicit Buzzer(int pin);

        void begin();
        void activate();
        void deactivate();

        bool isActive() const;
};


#endif
