#ifndef EMERGENCY_PROFILE_H
#define EMERGENCY_PROFILE_H

#include <Arduino.h>

struct EmergencyProfile {
    String name = "Não informado";
    String blood = "Não informado";
    String contactName = "Não informado";
    String contactPhone = "Não informado";
    String allergies = "Nenhuma informada";
    String observations = "Nenhuma";
};

#endif