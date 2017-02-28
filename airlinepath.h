#ifndef AIRLINEPATH_H
#define AIRLINEPATH_H
#include <QtCore/QCoreApplication>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <QtXml>
#include <QDomDocument>
#include <stack>
#include <cfloat>
#include "vertex.h"


class airlinePath
{
    public:
        airlinePath();
        ~airlinePath();
        void openXML(const QString &filename);
        void run();

    private:
        QDomDocument finalXML;
        QString filePath;
        std::map<int, vertex*> table;        // <airport ID, airport>
        std::string beginCode, endCode;      // IATA
        int beginID, endID;             // airport IDs
        std::vector<int> path;               // <airport IDs>
        double distance;                // initially in meters
        float totalCost;
        int err;
        std::string errMsg[7];

        // File and data structure handling
        void readFromXML(QDomDocument &doc, const QString &filename);
        void loadMap();
        void clear();

        // Input/Output
        void getInput();
        void getIATA();
        bool doAgain();
        void trimAndUpper(std::string &line);
        bool validInput();
        void display();
        void displayError();

        // Dijkstra's algorithm
        void findInitialPath();
        void findShortestPath();
};

#endif // AIRLINEPATH_H
