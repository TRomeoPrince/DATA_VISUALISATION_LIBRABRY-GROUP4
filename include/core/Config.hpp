#pragma once
struct Config {
    int width=900,height=560;
    double left=80,right=40,top=60,bottom=85;
    void validate() const;
};