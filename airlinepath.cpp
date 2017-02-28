#include "airlinepath.h"

airlinePath::airlinePath()
{
    finalXML = QDomDocument();
    filePath = QString();
    beginCode = endCode = std::string();
    beginID = endID = distance = err = NULL;
    errMsg[0] = "FATAL ERROR";
    errMsg[1] = "Cannot open XML file!";
    errMsg[2] = "Cannot parse XML file!";
    errMsg[3] = "Invalid beginning IATA code!";
    errMsg[4] = "Invalid ending IATA code!";
    errMsg[5] = "Invalid IATA codes!";
    errMsg[6] = "Cannot travel to the same location!";
}

airlinePath::~airlinePath()
{
    table.clear();
    finalXML = QDomDocument();
    filePath = QString();
    beginCode = endCode = std::string();
    beginID = endID = distance = totalCost = err = NULL;
}

void airlinePath::openXML(const QString &filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        err = 1;    // Can't open file
        return;
    }
    if(!finalXML.setContent(&file))
    {
        err = 2;    // Can't parse file
        return;
    }
    if(!err)
        loadMap();
}

void airlinePath::run()
{
    if(err)
        displayError();
    else
        do
        {
            getInput();
            findInitialPath();
            findShortestPath();
            display();
        }while(doAgain());
}

void airlinePath::loadMap()
{
    // Obtain a list of of each <airport> element
    QDomNodeList portElements = finalXML.documentElement().elementsByTagName("airport");
    for(size_t i = 0; i < portElements.size(); i++)
    {
        // Current <airport>
        QDomNode currAirport = portElements.item(i);
        vertex* portPtr = new vertex();

        // Load necessary <airport> information
        portPtr->airportID = currAirport.firstChildElement("id").text().toInt();
        portPtr->name = currAirport.firstChildElement("name").text().toStdString();
        portPtr->city = currAirport.firstChildElement("city").text().toStdString();
        portPtr->iata = currAirport.firstChildElement("iata").text().toStdString();
        portPtr->fare = currAirport.firstChildElement("fare").text().toFloat();

        // Obtain a list of each <route> element
        QDomNodeList routeElements = currAirport.firstChildElement("routes").elementsByTagName("route");
        for(size_t j = 0; j < routeElements.size(); j++)
        {
            // Current <route>
            QDomNode currRoute = routeElements.at(j);
            route* routePtr = new route();

            // Load necessary <route> information
            routePtr->destinationID = currRoute.firstChildElement("destinationAirportID").text().toInt();
            routePtr->distance = currRoute.firstChildElement("distance").text().toDouble();

            // Obtain a list of each <airline> element
            QDomNodeList lineElements = currRoute.firstChildElement("airlines").elementsByTagName("airline");
            for(size_t k = 0; k < lineElements.size(); k++)
            {
                // Current <airline>
                QDomNode currLine = lineElements.at(k);
                airline* linePtr = new airline();

                // Load necessary <airlines> information
                linePtr->id = currLine.firstChildElement("airlineid").text().toInt();
                linePtr->name = currLine.firstChildElement("name").text().toStdString();
                linePtr->iata = currLine.firstChildElement("iata").text().toStdString();

                // Route contains a number of airlines to choose from
                routePtr->airlines.push_back(linePtr);
            }
            // Airport contains a number of routes
            portPtr->routes.push_back(routePtr);
        }
        // Adjacency table contains the vertex and subsequently the destinations
        table.insert(std::pair<int, vertex*>(portPtr->airportID, portPtr));
    }
}

void airlinePath::clear()
{
    beginCode = endCode = std::string();
    beginID = endID = distance = totalCost = err = NULL;
    path.clear();
    for(std::map<int, vertex*>::iterator it = table.begin(); it != table.end(); it++)
    {
        vertex* ptr = it->second;
        ptr->visited = false;
    }
}

void airlinePath::getInput()
{
    do
    {
        clear();
        getIATA();
        if(!validInput())
            displayError();
    }while(err);
}

void airlinePath::getIATA()
{
    std::cout<<"Where are you flying from?: ";
    getline(std::cin, beginCode);
    std::cout<<"Where are you flying to?: ";
    getline(std::cin, endCode);
    std::cout<<std::endl;
}

bool airlinePath::doAgain()
{
    std::string str;
    do
    {
        std::cout<<"Do you want to do this again? (Y/N): ";
        getline(std::cin, str);
        str[0] = toupper(str[0]);
        std::cout<<std::endl;
    }while(str.length() != 1 && (str[0] != 'Y' || str[0] != 'N'));
    return str[0] == 'Y';

}

void airlinePath::trimAndUpper(std::string &line)
{
    // Remove leading spaces
    while(line[0] == ' ')
        line.erase(0,1);

    // Remove trailing spaces
    while(line[line.size()-1] == ' ')
        line.erase(line.size()-1,1);

    // To Upper
    for(int i = 0; i < line.length(); i++)
        line.at(i) = toupper(line.at(i));
}

bool airlinePath::validInput()
{
    trimAndUpper(beginCode);
    trimAndUpper(endCode);

    // Match codes with ID
    for(std::map<int, vertex*>::iterator it = table.begin(); it != table.end(); it++)
    {
        if(((vertex*)it->second)->iata == beginCode)
        {
            beginID = it->first;
            if(endID)
                break;
        }
        if(((vertex*)it->second)->iata == endCode)
        {
            endID = it->first;
            if(beginID)
                break;
        }
    }

    err = beginID == 0 ? 3 : err;               // Invalid begin id
    err = endID == 0 ? 4 : err;                 // Invalid end id
    err = beginID == endID ? 5 : err;           // Both invalid id
    err = err == 5 && beginID > 0 ? 6 : err;    // Travel to same place

    return err == 0;
}

void airlinePath::findInitialPath()
{
    int fromID = beginID;

    // Continue finding the a path until we have reached the destination
    while(fromID != endID)
    {
        // Push starting destination into the path stack
        path.push_back(fromID);

        // When at an airport, set it as visited
        vertex* currPort = table[fromID];
        currPort->visited = true;

        // List of all possible routes from the airport
        std::vector<route*> options;
        for(int i = 0; i < currPort->routes.size(); i++)
            options.push_back(((route*)currPort->routes[i]));

        // Find best route (shortest distance)
        int bestID = -1;
        double distance = DBL_MAX;
        for(int i = 0; i < options.size(); i++)
        {
            double dist = ((route*)options.at(i))->distance;
            int id = ((route*)options.at(i))->destinationID;

            // Best route must have a shorter distance, unvisited, and route options from it
            if(distance > dist && !table.at(id)->visited && table.at(id)->routes.size())
            {
                distance = dist;
                bestID = id;
            }
        }

        // No (best) route: go back and pick another destination
        if(bestID == -1)
        {
            path.pop_back();
            fromID = path.at(path.size()-1);
            path.pop_back();
            continue;
        }
        fromID = bestID;
    }
    // Insert destination ID
    path.push_back(fromID);
}

void airlinePath::findShortestPath()
{
    /*      S 1 2 3 4 5 E
            ---------
            if 4 -> E    direct: remove everything in between 5
            ---------
            if 3 -> E    direct: remove everything in between 4, 5
       else if 3 -> 5    direct: remove everything in between 4
            ---------
            if 2 -> E    direct: remove everything in between 3, 4, 5
       else if 2 -> 5    direct: remove everything in between 3, 4
       else if 2 -> 4    direct: remove everything in between 3
            ---------
            and so on...
    */

    int toIndex,    // Index of "E" to fromIndex+1
        fromIndex;  // Index from "5" to "S"

    // fromIndex starts before toIndex and only decrements when it meets toIndex
    for(fromIndex = path.size()-2; fromIndex != -1; fromIndex--)
    {
        // toIndex starts at the end, and decrements down to meet fromIndex
        for(toIndex = path.size()-1; toIndex > fromIndex; toIndex--)
        {
            // Already a direct route
            if(fromIndex == toIndex-1)
                continue;

            vertex *fromV = table.at(path.at(fromIndex)),
                   *toV = table.at(path.at(toIndex));

            // Check source routes for a direct route from fromIndex to toIndex
            for(int i = 0; i < fromV->routes.size(); i++)
            {
                if(((route*)fromV->routes.at(i))->destinationID == path.at(toIndex))
                {
                    // Erase everying in between the newly found direct route
                    path.erase(path.begin() + fromIndex+1, path.begin() + toIndex);

                    // Restart the process
                    fromIndex = path.size()-2;
                    toIndex = path.size()-1;
                    break;
                }
            }
        }
    }
}

void airlinePath::display()
{
    std::string toPortMsg[] = { "- Leave ", "- Board ", "- Enter " },
                toLineMsg[] = { " on ", " and board ", " onto " },
                distMsg[] = { " and fly ", " for ", " after flying " },
                multiLine = " and choose between ",
                destPort = "- Finally, you will arrive at ";
    vertex *airport, *next;
    bool costError = 0;

    // Display cities from and to
    std::cout<<"From "<<table.at(beginID)->city<<" to "<<table.at(endID)->city<<std::endl;

    for(int i = 0; i < path.size()-1; i++)
    {
        // Current airports
        airport = table.at(path.at(i));
        next = table.at(path.at(i+1));

        // Display airport information
        std::cout<<toPortMsg[i%3]<<"("<<airport->iata<<") "<<airport->name<<", "<<airport->city;

        // Find matching route to the next destination
        for(int i = 0; i < airport->routes.size(); i++)
        {
            // Display airline information the given route
            route* rPtr = airport->routes.at(i);
            if(rPtr->destinationID == next->airportID)
            {
                // Multiple airlines
                if(rPtr->airlines.size() >= 2)
                {
                    airline* aPtr = rPtr->airlines[0];
                    std::cout<<multiLine;
                    for(int i = 0; i < rPtr->airlines.size()-1; i++)
                    {
                        std::cout<<"("<<aPtr->iata<<") "<<aPtr->name<<", ";
                    }
                    aPtr = rPtr->airlines[rPtr->airlines.size()-1];
                    std::cout<<"and ""("<<aPtr->iata<<") "<<aPtr->name;
                }

                // Single airline
                else
                {
                    airline* aPtr = rPtr->airlines[0];
                    std::cout<<toLineMsg[i%3]<<"("<<aPtr->iata<<") "<<aPtr->name;
                }

                // Display distance
                std::cout<<distMsg[i%3]<<rPtr->distance * (float)0.0006<<" miles. ";

                // Display cost
                if(airport->fare > 0)
                    std::cout<<"($"<<airport->fare<<")";
                else
                    costError = 1;
                std::cout<<std::endl;

                // Add to total distance and cost
                distance += rPtr->distance;
                totalCost += airport->fare;
                break;
            }
        }
    }
    // Final destination
    std::cout<<destPort<<"("<<next->iata<<") "<<next->name<<std::endl<<std::endl;

    // Total distance and cost
    std::cout<<"There will be a total of "<<path.size()<<" airports to stop at."<<std::endl;
    std::cout<<"You will have traveled a total of: "<<distance * (float)0.0006<<" miles."<<std::endl;
    if(!costError)
        std::cout<<"The total cost for this trip will be $"<<totalCost<<std::endl<<std::endl;
}

void airlinePath::displayError()
{
    std::cout<<errMsg[err]<<std::endl<<std::endl;
}
