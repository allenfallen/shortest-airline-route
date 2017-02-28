#ifndef VERTEX_H
#define VERTEX_H
#include <string>
#include <vector>
#include "route.h"

struct vertex
{
    vertex()
    {
        airportID = visited = fare = NULL;
        name = city = iata = std::string();
    }

    ~vertex()
    {
        airportID = visited = fare = NULL;
        name = city = iata = std::string();
        routes.clear();
    }

    int airportID;
    std::string name, city, iata;
    float fare;
    bool visited;
    std::vector<route*> routes;
};

#endif // VERTEX_H
