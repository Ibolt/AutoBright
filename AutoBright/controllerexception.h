#pragma once
#include <iostream>
#include "controllerexception.h"


class ControllerException : public std::exception {
    const char* message;

public:
    ControllerException(const char* msg);
    const char* what();
};