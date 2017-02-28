#ifndef AIRLINE_H
#define AIRLINE_H
#include <string>

struct airline
{
    airline()
    {
        id = 0;
        name = iata = std::string();
    }

    ~airline()
    {
        id = 0;
        name = iata = std::string();
    }

    int id;
    std::string name, iata;
};

#endif // AIRLINE_H
