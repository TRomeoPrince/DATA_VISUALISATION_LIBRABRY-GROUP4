#pragma once
#include <string>
struct Theme {
    std::string background="#ffffff",foreground="#243447",grid="#dce4eb",
                accent="#2374ab",annotation="#b43346";
    static Theme light();
    static Theme dark();
};