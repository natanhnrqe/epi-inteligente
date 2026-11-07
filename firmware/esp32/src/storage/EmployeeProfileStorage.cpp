
#include "EmployeeProfileStorage.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

struct EmergencyProfile;

bool EmployeeProfileStorage::exists() const {
    return LittleFS.exists(PROFILE_PATH);
}

bool EmployeeProfileStorage::load(EmergencyProfile& profile) const {
    if (!exists()) {
        return false;
    }

    File file = LittleFS.open(PROFILE_PATH, FILE_READ);

    if (!file) {
        return false;
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);

    file.close();

    if (error) {
        return false;
    }

    profile.name = doc["nome"] | "";
    profile.contactName = doc["contato_nome"] | "";
    profile.contactPhone = doc["contato_telefone"] | "";
    profile.blood = doc["tipo_sanguineo"] | "";
    profile.allergies = doc["alergias"] | "";
    profile.observations = doc["observacoes"] | "";

    return true;
}

bool EmployeeProfileStorage::save(
    const EmergencyProfile& profile
) const {

    if (exists()) {
        return false;
    }

    JsonDocument doc;

    doc["nome"] = profile.name;
    doc["contato_nome"] = profile.contactName;
    doc["contato_telefone"] = profile.contactPhone;
    doc["tipo_sanguineo"] = profile.blood;
    doc["alergias"] = profile.allergies;
    doc["observacoes"] = profile.observations;

    File file = LittleFS.open(PROFILE_PATH, FILE_WRITE);

    if (!file) {
        return false;
    }

    const size_t expected = measureJson(doc);
    const size_t written = serializeJson(doc, file);

    file.close();

    return written == expected;
}

bool EmployeeProfileStorage::remove() const {
    if (!exists()) {
        return false;
    }

    return LittleFS.remove(PROFILE_PATH);
}
