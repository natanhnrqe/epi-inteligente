
#include "OledDisplay.h"


OledDisplay::OledDisplay(TwoWire &bus)
    :bus(bus), display(128, 64, &bus, -1){
}

bool OledDisplay::begin(int sdaPin, int sclPin) {
    bus.begin(sdaPin, sclPin);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, false)) {
        return false;
    }

    display.clearDisplay();
    display.display();

    return true;
}

void OledDisplay::showBattery(int percentage) {

    percentage = constrain(percentage, 0, 100);

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(15, 3);
    display.print("EPI-INTELIGENTE");

    display.drawRoundRect(
        8, 18,
        105, 34,
        5,
        SSD1306_WHITE
    );

    display.fillRect(
        113, 27,
        7, 16,
        SSD1306_WHITE
    );

    int fillWidth = (97 * percentage) / 100;

    if (fillWidth > 0) {
        display.fillRoundRect(
            12, 22,
            fillWidth, 26,
            3,
            SSD1306_WHITE
        );
    }

    display.setTextSize(2);
    display.setTextColor(SSD1306_BLACK);

    display.setCursor(40, 28);
    display.print(percentage);
    display.print("%");

    display.display();
}

void OledDisplay::showEmergencyPage(const EmergencyProfile& profile, int page) {
    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextWrap(true);

    // Cabeçalho
    display.setCursor(0, 0);
    display.print("MODO DE EMERGENCIA");

    display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

    switch (page) {

        case 0:
            display.setCursor(0, 20);
            display.print("NOME COMPLETO:");

            display.setCursor(0, 34);
            display.print(profile.name);
            break;

        case 1:
            display.setCursor(0, 20);
            display.print("TIPO SANGUINEO:");

            display.setTextSize(3);
            display.setCursor(0, 32);
            display.print(profile.blood);
            break;

        case 2:
            display.setCursor(0, 19);
            display.print("CONTATO:");

            display.setCursor(0, 30);
            display.print(profile.contactName);

            display.setCursor(0, 44);
            display.print(profile.contactPhone);
            break;

        case 3:
            display.setCursor(0, 18);
            display.print("ALERGIAS:");

            display.setCursor(0, 28);
            display.print(profile.allergies);

            display.setCursor(0, 44);
            display.print("OBS: ");
            display.print(profile.observations);
            break;

        default:
            display.setCursor(0, 25);
            display.print("Pagina invalida");
            break;
    }

    // Indicador da página atual
    display.setTextSize(1);
    display.setTextWrap(false);
    display.setCursor(112, 56);
    display.print(page + 1);
    display.print("/4");

    display.display();
}
