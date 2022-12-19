#include <iostream>
#include "controllerexception.h"

ControllerException::ControllerException(const char* msg) : message(msg) {}

const char* ControllerException::what() {
    return message;
}
