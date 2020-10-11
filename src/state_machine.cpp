#include "state_machine.h"

void StateMachine::begin(uint8_t* current_desk_height, uint8_t up_pwm_channel, uint8_t down_pwm_channel) {
    this->up_pwm_channel = up_pwm_channel;
    this->down_pwm_channel = down_pwm_channel;
    this->current_desk_height = current_desk_height;

    this->current_state = WAITING_FOR_COMMAND_STATE;

    this->height_preset_1 = EEPROM.read(EEPROM_HEIGHT_1_INDEX);
    this->height_preset_2 = EEPROM.read(EEPROM_HEIGHT_2_INDEX);
    this->height_preset_3 = EEPROM.read(EEPROM_HEIGHT_3_INDEX);

    Serial.println("Current saved heights are as follows:");
    Serial.print("Height 1: ");
    Serial.println(this->height_preset_1);
    Serial.print("Height 2: ");
    Serial.println(this->height_preset_2);
    Serial.print("Height 3: ");
    Serial.println(this->height_preset_3);
}

uint8_t* StateMachine::getHeightPreset1() {
    return &(this->height_preset_1);
}

uint8_t* StateMachine::getHeightPreset2() {
    return &(this->height_preset_2);
}

uint8_t* StateMachine::getHeightPreset3() {
    return &(this->height_preset_2);
}

bool StateMachine::requestStateChange(state_t requested_state) {
    if (requested_state == WAITING_FOR_COMMAND_STATE) {
        Serial.println("Requesting to change to WAITING_FOR_COMMAND is a NOOP");
        return false;
    }

    if (this->current_state == requested_state) {
        Serial.println("Allow the state to remain the same");
        return true;
    }

    if (current_state != WAITING_FOR_COMMAND_STATE) {
        Serial.println("State transitions can only be made when in the WAITING_FOR_COMMAND state");
        return false;
    }

    this->current_state = requested_state;
    return true;
}

void StateMachine::saveDeskHeightToEEPROM(uint8_t eeprom_index, uint8_t height) {
    EEPROM.write(eeprom_index, height);
    EEPROM.commit();
}

void StateMachine::raiseDeskToHeight(uint8_t new_desk_height) {
    ledcWrite(this->down_pwm_channel, 0);

    for (uint16_t i = 0; i < 500; i++) {
        uint8_t pwm_level = (- ((16 * pow(i, 2))/15625)) + ((128 * i) / 125);
        ledcWrite(this->up_pwm_channel, pwm_level);
        delay(1);
    }

    Serial.print("Raise desk: current - ");
    Serial.print(*this->current_desk_height);
    Serial.print(" = new -");
    Serial.println(new_desk_height);
    while(*this->current_desk_height < new_desk_height) {
        ledcWrite(this->up_pwm_channel, 256);
        delay(1);
    }

    for (uint16_t i = 0; i < 500; i++) {
        uint8_t pwm_level = (- ((16 * pow(i, 2))/15625)) + ((128 * i) / 125);
        ledcWrite(this->up_pwm_channel, (256 - pwm_level));
        delay(1);
    }

    ledcWrite(this->up_pwm_channel, 0);
}

void StateMachine::lowerDeskToHeight(uint8_t new_desk_height) {
    ledcWrite(this->up_pwm_channel, 0);

    for (uint16_t i = 0; i < 500; i++) {
        uint8_t pwm_level = (- ((16 * pow(i, 2))/15625)) + ((128 * i) / 125);
        ledcWrite(this->down_pwm_channel, pwm_level);
        delay(1);
    }

    Serial.print("Lower desk: current - ");
    Serial.print(*this->current_desk_height);
    Serial.print(" = new - ");
    Serial.println(new_desk_height);
    while(*this->current_desk_height > new_desk_height) {
        ledcWrite(this->down_pwm_channel, 256);
        delay(1);
    }

    for (uint16_t i = 0; i < 500; i++) {
        uint8_t pwm_level = (- ((16 * pow(i, 2))/15625)) + ((128 * i) / 125);
        ledcWrite(this->down_pwm_channel, (256 - pwm_level));
        delay(1);
    }

    ledcWrite(this->down_pwm_channel, 0);
}

void StateMachine::processCurrentState() {
    if (this->current_state == SAVE_CURRENT_HEIGHT_TO_PRESET_1_STATE) {
        uint8_t height = *this->current_desk_height;
        saveDeskHeightToEEPROM(EEPROM_HEIGHT_1_INDEX, height);
        this->current_state = WAITING_FOR_COMMAND_STATE;

        Serial.print("Setting height preset 1 to ");
        Serial.println(height);
    }

    if (this->current_state == SAVE_CURRENT_HEIGHT_TO_PRESET_2_STATE) {
        uint8_t height = *this->current_desk_height;
        saveDeskHeightToEEPROM(EEPROM_HEIGHT_2_INDEX, height);
        this->current_state = WAITING_FOR_COMMAND_STATE;

        Serial.print("Setting height preset 2 to ");
        Serial.println(height);
    }

    if (this->current_state == SAVE_CURRENT_HEIGHT_TO_PRESET_3_STATE) {
        uint8_t height = *this->current_desk_height;
        saveDeskHeightToEEPROM(EEPROM_HEIGHT_3_INDEX, height);
        this->current_state = WAITING_FOR_COMMAND_STATE;

        Serial.print("Setting height preset 3 to ");
        Serial.println(height);
    }

    if (this->current_state == ADJUST_TO_PRESET_1_HEIGHT_STATE) {
        uint8_t starting_desk_height = *this->current_desk_height;
        uint8_t new_desk_height = 85;

        if (starting_desk_height < new_desk_height) {
            raiseDeskToHeight(new_desk_height);
        } else if (starting_desk_height > new_desk_height) {
            lowerDeskToHeight(new_desk_height);
        }

        this->current_state = WAITING_FOR_COMMAND_STATE;
    }

    if (this->current_state == ADJUST_TO_PRESET_2_HEIGHT_STATE) {
        uint8_t starting_desk_height = *this->current_desk_height;
        uint8_t new_desk_height = 102;

        if (starting_desk_height < new_desk_height) {
            raiseDeskToHeight(new_desk_height);
        } else if (starting_desk_height > new_desk_height) {
            lowerDeskToHeight(new_desk_height);
        }

        this->current_state = WAITING_FOR_COMMAND_STATE;
    }

    if (this->current_state == ADJUST_TO_PRESET_3_HEIGHT_STATE) {
        uint8_t starting_desk_height = *this->current_desk_height;
        uint8_t new_desk_height = 108;

        if (starting_desk_height < new_desk_height) {
            raiseDeskToHeight(new_desk_height);
        } else if (starting_desk_height > new_desk_height) {
            lowerDeskToHeight(new_desk_height);
        }

        this->current_state = WAITING_FOR_COMMAND_STATE;
    }

    if (this->current_state == ADJUST_UP_STATE) {

        this->current_state = WAITING_FOR_COMMAND_STATE;
    }

    if (this->current_state == ADJUST_DOWN_STATE) {

        this->current_state = WAITING_FOR_COMMAND_STATE;
    }

    // Serial.print("Currently processing state ");
    // Serial.println(this->current_state);
}
