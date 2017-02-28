#ifndef ROUTE_H
#define ROUTE_H
#include <string>
#include <vector>
#include "airline.h"


struct route
{
    route()
    {
        destinationID = distance = 0;
    }

    ~route()
    {
        destinationID = distance = 0;
        airlines.clear();
    }

    int destinationID;
    double distance;
    std::vector<airline*> airlines;
};


#endif // ROUTE_H
