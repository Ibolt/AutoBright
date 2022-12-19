#include <iostream>

#include <opencv2/core.hpp>

#include "controllerexception.h"
#include "displaycontroller.h"

int main() {
    try {
        while (1) {
            DisplayController ctrl = DisplayController();
            ctrl.captureScreen();
            std::array<uchar, 3> mainColour{ ctrl.findMainColour() };
            std::cout << +mainColour[0] << " " << +mainColour[1] << " " << +mainColour[2] << std::endl;

            const float brightness = (mainColour[0] * 0.299f + mainColour[0] * 0.587f + mainColour[0] * 0.114f) / 256;
            if (brightness > 0.7) {
                ctrl.setCurrBrightness(70);
            }
            else {
                ctrl.setCurrBrightness(90);
            }
        }
    }
    catch (ControllerException e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}