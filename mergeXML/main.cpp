#include <QtCore/QCoreApplication>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <QtXml>
#include <QDomDocument>
#include <QGeoCoordinate>

using namespace std;

struct airline
{
    // airlineid, name, iata
    QString airlineInfo[3];
};

struct route
{
    // sourceAirportID, destinationAirportID
    QString routeInfo[4];

    // airlineMap's key is identified by the airlineid (not used) <-> airlineid (info[0])
    map<int, airline*> airlineMap;
    double distance;
};

struct airport
{
    // id, name, iata, icao, city, country lat, lon;
    QString airportInfo[8];
    float fare = -1;

    // routeMap's key is identified by the airport id (info[0]) <-> route's sourceID (info[0])
    map<int, route*> routeMap;
};

struct xmlInfo
{
    string opening;
    string closing;
};


void readFromXML(QDomDocument &doc, const QString &filename);
void loadXMLHeaders(xmlInfo port[8], xmlInfo rout[6], xmlInfo line[2]);
void loadAirports(map<int, airport*> &airportMap, QDomDocument &airports);
void loadFares(map<int, airport*> &airportMap, string farePath);
void loadRoutes(map<int, airport*> &airportMap, QDomDocument &routes);
void loadAirlines(map<int, airline *> &airlineMap, QDomDocument &airlines);
void linkRouteWithAirline(map<int, airport*> &airportMap, map<int, airline*> &airlineMap);
void writeXML(ostream& out, map<int, airport*> &airportMap, xmlInfo port[], xmlInfo rout[], xmlInfo line[]);
void writeRouteXML(ostream& out, airport* currPort, xmlInfo rout[], xmlInfo line[]);
void writeAirlineXML(ostream& out, route* currRoute, xmlInfo line[]);
double calculateDistance(airport* source, airport* destination);

int main()
{
    // Files and XML
    QString filePath = "../resources/xml/";
    QString xmlNames[] = {"airports.xml", "routes.xml", "airlines.xml"};
    string farePath = "/home/allen/School/CS8/finaldata/Resources/RAW/USfares.dat";
    xmlInfo airportHeaders[9];
    xmlInfo routeHeaders[6];
    xmlInfo airlineHeaders[3];
    QDomDocument airports, routes, airlines;

    // I/O
    ofstream out;
    map<int, airport*> airportMap;
    map<int, airline*> airlineMap;

    // Load required elements
    readFromXML(airports, (filePath + xmlNames[0]));
    readFromXML(routes, (filePath + xmlNames[1]));
    readFromXML(airlines, (filePath + xmlNames[2]));
    loadXMLHeaders(airportHeaders, routeHeaders, airlineHeaders);

    // Load data structure used with I/O
    loadAirports(airportMap, airports);
    loadFares(airportMap, farePath);
    loadRoutes(airportMap, routes);
    loadAirlines(airlineMap, airlines);
    linkRouteWithAirline(airportMap, airlineMap);

    // Create and open the final XML
    out.open(filePath.toStdString() + "finalData.xml");
    writeXML(out, airportMap, airportHeaders, routeHeaders, airlineHeaders);
    out.close();
}

void readFromXML(QDomDocument &doc, const QString &filename)
{
    QFile file(filename);
    cout<<"Reading "<<filename.toStdString().substr(filename.toStdString().find_last_of('/')+1)<<endl;
    if(!file.open(QIODevice::ReadOnly))
    {
        cout<<"Cannot open XML file!"<<endl;
        return;
    }
    if(!doc.setContent(&file))
    {
        cout<<"Cannot parse XML file!"<<endl;
        return;
    }
    cout<<"Finished "<<filename.toStdString().substr(filename.toStdString().find_last_of('/')+1)<<endl;
}

void loadXMLHeaders(xmlInfo port[8], xmlInfo rout[6], xmlInfo line[3])
{
    port[0].opening = "<id>";
    port[0].closing = "</id>";
    port[1].opening = "<name>";
    port[1].closing = "</name>";
    port[2].opening = "<iata>";
    port[2].closing = "</iata>";
    port[3].opening = "<icao>";
    port[3].closing = "</icao>";
    port[4].opening = "<city>";
    port[4].closing = "</city>";
    port[5].opening = "<country>";
    port[5].closing = "</country>";
    port[6].opening = "<latitude>";
    port[6].closing = "</latitude>";
    port[7].opening = "<longitude>";
    port[7].closing = "</longitude>";
    port[8].opening = "<fare>";
    port[8].closing = "</fare>";

    rout[0].opening = "<sourceAirportID>";
    rout[0].closing = "</sourceAirportID>";
    rout[1].opening = "<sourceAirport>";
    rout[1].closing = "</sourceAirport>";
    rout[2].opening = "<destinationAirportID>";
    rout[2].closing = "</destinationAirportID>";
    rout[3].opening = "<destinationAirport>";
    rout[3].closing = "</destinationAirport>";
    rout[4].opening = "<distance>";
    rout[4].closing = "</distance>";

    line[0].opening = "<airlineid>";
    line[0].closing = "</airlineid>";
    line[1].opening = "<name>";
    line[1].closing = "</name>";
    line[2].opening = "<iata>";
    line[2].closing = "</iata>";
}

void loadAirports(map<int,airport*> &airportMap, QDomDocument &airports)
{
    // Obtain a list of of each <airport> element
    QDomNodeList elements = airports.documentElement().elementsByTagName("airport");

    // Look through every <airport>
    for(size_t i = 0; i < elements.size(); i++)
    {
        // Current <airport> element in the list
        QDomNode curr = elements.item(i);

        // Current airport to insert into map
        airport* port = new airport();

        // Load the corresponding information and insert into the map
        port->airportInfo[0] = curr.firstChildElement("id").text(),
        port->airportInfo[1] = curr.firstChildElement("name").text(),
        port->airportInfo[2] = curr.firstChildElement("iata").text(),
        port->airportInfo[3] = curr.firstChildElement("icao").text();

        // Not an airport: Do not need its information
        if( port->airportInfo[1].contains("Bus") ||
            port->airportInfo[1].contains("Ferry") ||
            port->airportInfo[1].contains("Nolf") ||
            port->airportInfo[1] == "Wright Aaf" ||
            port->airportInfo[1] == "New River Mcas" ||
            port->airportInfo[1].contains("\"") ||
            port->airportInfo[1] == "Fresh Creek" ||
            port->airportInfo[1].contains("Transit") ||
            port->airportInfo[1].contains("Aerodrome") ||
            port->airportInfo[0].toUInt() >= 9349 ||
            port->airportInfo[2].length() < 3)
                continue;

        port->airportInfo[4] = curr.firstChildElement("city").text(),
        port->airportInfo[5] = curr.firstChildElement("country").text(),
        port->airportInfo[6] = curr.firstChildElement("latitude").text(),
        port->airportInfo[7] = curr.firstChildElement("longitude").text();

        airportMap.insert(std::pair<int, airport*>(port->airportInfo[0].toInt(), port));
    }
}

void loadFares(map<int, airport *> &airportMap, string farePath)
{
    cout<<"Loading fares"<<endl;
    ifstream in;
    string line, iata, cost;
    in.open(farePath);
    while(getline(in, line))
    {
        iata = line.substr(0, 3);
        cost = line.substr(4);

        // Remove commas
        if(cost.find(',') < cost.length())
        {
            cost = cost.substr(0, cost.find(',')) + cost.substr(cost.find(',')+1);
        }

        for(map<int, airport*>::iterator it = airportMap.begin(); it != airportMap.end(); it++)
        {
            airport* ptr = it->second;
            if(ptr->airportInfo[2].toStdString() == iata)
            {
                ptr->fare = stof(cost);
            }
        }
    }
    cout<<"Finished loading fares"<<endl;
    in.close();
}

void loadRoutes(map<int,airport*> &airportMap, QDomDocument &routes)
{
    // Obtain a list of of each <route> element
    QDomNodeList elements = routes.documentElement().elementsByTagName("route");

    // Look through every <route>
    for(size_t i = 0; i < elements.size(); ++i)
    {
        // Current <route> element in the list
        QDomNode curr = elements.item(i);

        // Current airport to insert into map
        route* routePtr = new route();

        // Load the corresponding information
        routePtr->routeInfo[0] = curr.firstChildElement("sourceAirportID").text(),
        routePtr->routeInfo[1] = curr.firstChildElement("sourceAirport").text();
        routePtr->routeInfo[2] = curr.firstChildElement("destinationAirportID").text(),
        routePtr->routeInfo[3] = curr.firstChildElement("destinationAirport").text();

        // A route's source airport ID matches its corresponding airportMap's key
        int sourceID = routePtr->routeInfo[0].toInt();
        int destID = routePtr->routeInfo[2].toInt();

        // Airports do not exist
        if(airportMap.find(sourceID) == airportMap.end() ||
           airportMap.find(destID) == airportMap.end() ||
           sourceID == destID)
            continue;

        // Calculate great circle distance using the IDs
        QGeoCoordinate source, dest;
        source.setLatitude(airportMap.at(sourceID)->airportInfo[6].toDouble());
        source.setLongitude(airportMap.at(sourceID)->airportInfo[7].toDouble());
        dest.setLatitude(airportMap.at(destID)->airportInfo[6].toDouble());
        dest.setLongitude(airportMap.at(destID)->airportInfo[7].toDouble());
        routePtr->distance = source.distanceTo(dest);

        // A route's airline ID is the key of the routeMap
        int airlineID = curr.firstChildElement("airlineID").text().toInt();

        // If route not found, insert new route. Insert a blank airline (by its ID) no matter what.
        if(airportMap.at(sourceID)->routeMap.find(destID) == airportMap.at(sourceID)->routeMap.end())
        {
            airportMap.at(sourceID)->routeMap.insert(pair<int, route*>(destID, routePtr));
            airportMap.at(sourceID)->routeMap.at(destID)->airlineMap.insert(pair<int, airline*>(airlineID, NULL));
        }

        // if airport exists AND route exists in its route map, insert airline (id, null)
        else
        {
            airportMap.at(sourceID)->routeMap.at(destID)->airlineMap.insert(pair<int, airline*>(airlineID, NULL));
        }
    }
}

void loadAirlines(map<int, airline*> &airlineMap, QDomDocument &airlines)
{
    // Obtain a list of of each <airline> element
    QDomNodeList elements = airlines.documentElement().elementsByTagName("airline");

    // Look through every <airline>
    for(size_t i = 0; i < elements.size(); ++i)
    {
        // Current <airline> element in the list
        QDomNode curr = elements.item(i);

        // Current airline to insert into map
        airline* airlinePtr = new airline();

        // Load the corresponding information and insert into the route's airline vector
        airlinePtr->airlineInfo[0] = curr.firstChildElement("airlineid").text();
        airlinePtr->airlineInfo[1] = curr.firstChildElement("name").text();
        airlinePtr->airlineInfo[2] = curr.firstChildElement("iata").text();

        if(airlinePtr->airlineInfo[1].contains("School") ||
           airlinePtr->airlineInfo[1].contains("Army") ||
           airlinePtr->airlineInfo[1].contains("Corporation") ||
           airlinePtr->airlineInfo[1].contains("Training") ||
           airlinePtr->airlineInfo[1].contains("Survey") ||
           airlinePtr->airlineInfo[1].contains("School"))
            continue;

        airlineMap.insert(pair<int, airline*>(airlinePtr->airlineInfo[0].toInt(), airlinePtr));
    }
}

void linkRouteWithAirline(map<int, airport*> &airportMap, map<int, airline*> &airlineMap)
{
    for(map<int, airport*>::iterator it = airportMap.begin(); it != airportMap.end(); it++)
    {
        // current airport to link with
        airport* currAirport = it->second;

        for(map<int, route*>::iterator rit = currAirport->routeMap.begin(); rit != currAirport->routeMap.end(); rit++)
        {
            // current airport's current route to link with
            route* currRoute = rit->second;

            for(map<int, airline*>::iterator ait = currRoute->airlineMap.begin(); ait != currRoute->airlineMap.end(); ait++)
            {
                int airlineID = ait->first;
                if(currRoute->airlineMap.find(airlineID) != currRoute->airlineMap.end() &&
                   airlineMap.find(airlineID) != airlineMap.end())
                    currRoute->airlineMap.at(airlineID) = airlineMap.at(airlineID);
            }
        }
    }
}

void writeXML(ostream& out, map<int, airport*> &airportMap, xmlInfo port[], xmlInfo rout[], xmlInfo line[])
{
    out<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
    out<<"<airports>"<<endl;
    for(map<int, airport*>::iterator it = airportMap.begin(); it != airportMap.end(); it++)
    {
        airport* currPort = it->second;

        out<<"\t<airport>"<<endl;
        for(int i = 0; i < 8; i++)
            out<<"\t\t"<<port[i].opening<<currPort->airportInfo[i].toStdString()<<port[i].closing<<endl;
        out<<"\t\t"<<port[8].opening<<currPort->fare<<port[8].closing<<endl;

        writeRouteXML(out, currPort, rout, line);
        out<<"\t</airport>"<<endl;
    }
    out<<"</airports>"<<endl;
    cout<<"done!"<<endl;
}

void writeRouteXML(ostream& out, airport* currPort, xmlInfo rout[], xmlInfo line[])
{
    out<<"\t\t<routes>"<<endl;
    for(map<int, route*>::iterator rit = currPort->routeMap.begin(); rit != currPort->routeMap.end(); rit++)
    {
        route* currRoute = rit->second;

        out<<"\t\t\t<route>"<<endl;
        for(int i = 0; i < 4; i++)
            out<<"\t\t\t\t"<<rout[i].opening<<currRoute->routeInfo[i].toStdString()<<rout[i].closing<<endl;
        out<<"\t\t\t\t"<<rout[4].opening<<currRoute->distance<<rout[4].closing<<endl;
        writeAirlineXML(out, currRoute, line);
        out<<"\t\t\t</route>"<<endl;
    }
    out<<"\t\t</routes>"<<endl;
}

void writeAirlineXML(ostream& out, route* currRoute, xmlInfo line[])
{
    out<<"\t\t\t\t<airlines>"<<endl;
    for(map<int, airline*>::iterator ait = currRoute->airlineMap.begin(); ait != currRoute->airlineMap.end(); ait++)
    {
        airline* currLine = ait->second;

        if(!currLine)
            continue;

        out<<"\t\t\t\t\t<airline>"<<endl;
        for(int i = 0; i < 3; i++)
            out<<"\t\t\t\t\t\t"<<line[i].opening<<currLine->airlineInfo[i].toStdString()<<line[i].closing<<endl;
        out<<"\t\t\t\t\t</airline>"<<endl;
    }
    out<<"\t\t\t\t</airlines>"<<endl;
}








