
#ifndef EMPLOYEE_PROFILE_STORAGE_H
#define EMPLOYEE_PROFILE_STORAGE_H

#include "models/EmergencyProfile.h"

class EmployeeProfileStorage {
private:
    static constexpr const char* PROFILE_PATH = "/employee.json";

public:
    bool exists() const;

    bool load(EmergencyProfile& profile) const;

    bool save(const EmergencyProfile& profile) const;

    bool remove() const;
};

#endif
