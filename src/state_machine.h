#ifndef STATE_MACHINE_HEADER
#define STATE_MACHINE_HEADER

#include "Arduino.h"
#include "EEPROM.h"

#define EEPROM_HEIGHT_1_INDEX 0
#define EEPROM_HEIGHT_2_INDEX 1
#define EEPROM_HEIGHT_3_INDEX 2

enum state_t {
    WAITING_FOR_COMMAND_STATE,
    ADJUST_TO_PRESET_1_HEIGHT_STATE,
    ADJUST_TO_PRESET_2_HEIGHT_STATE,
    ADJUST_TO_PRESET_3_HEIGHT_STATE,
    ADJUST_UP_STATE,
    ADJUST_DOWN_STATE,
    SAVE_CURRENT_HEIGHT_TO_PRESET_1_STATE,
    SAVE_CURRENT_HEIGHT_TO_PRESET_2_STATE,
    SAVE_CURRENT_HEIGHT_TO_PRESET_3_STATE,
};

class StateMachine {
private:
    state_t current_state;
    uint8_t height_preset_1, height_preset_2, height_preset_3;
    uint8_t up_pwm_channel;
    uint8_t down_pwm_channel;
    uint8_t* current_desk_height;

    uint8_t microsecondsToCentimeters(long microseconds);
    void saveDeskHeightToEEPROM(uint8_t eeprom_index, uint8_t height);
    void raiseDeskToHeight(uint8_t new_desk_height);
    void lowerDeskToHeight(uint8_t new_desk_height);

public:
    void begin(uint8_t* current_desk_height, uint8_t up_pwm_channel, uint8_t down_pwm_channel);
    uint8_t* getHeightPreset1();
    uint8_t* getHeightPreset2();
    uint8_t* getHeightPreset3();
    bool requestStateChange(state_t requested_state);
    void processCurrentState();
};

#endif
