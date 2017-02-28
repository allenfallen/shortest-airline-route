#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <QXmlStreamWriter>
#include <QFile>

using namespace std;

void removeCharacter(string &line, char item = ',', bool removeReplace = true);
void readLineIntoArray(const string &line,string info[]);

int main()
{
    string routeInfo[9], line,
           input = "../resources/raw/routes.dat";
    QFile out("../resources/xml/routes.xml");
    QXmlStreamWriter xmlWriter(&out);
    out.open(QIODevice::WriteOnly);
    ifstream in(input);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("routes");
    while(getline(in, line))
    {
        readLineIntoArray(line, routeInfo);
        if(routeInfo[4].empty())
            continue;
        xmlWriter.writeStartElement("route");
        xmlWriter.writeTextElement("airline",routeInfo[0].c_str());
        xmlWriter.writeTextElement("airlineID",routeInfo[1].c_str());
        xmlWriter.writeTextElement("sourceAirport",routeInfo[2].c_str());
        xmlWriter.writeTextElement("sourceAirportID",routeInfo[3].c_str());
        xmlWriter.writeTextElement("destinationAirport",routeInfo[4].c_str());
        xmlWriter.writeTextElement("destinationAirportID",routeInfo[5].c_str());
        xmlWriter.writeEndElement();


    }
    xmlWriter.writeEndDocument();
    out.close();
    in.close();
    return 0;
}


void readLineIntoArray(const string &line,string airlineInfo[])
{
    stringstream ss;
    ss << line;
    for(int i = 0; i < 6; i++)
    {
        getline(ss, airlineInfo[i], ',');
        removeCharacter(airlineInfo[i],'\"');
    }
}

void removeCharacter(string &line, char item, bool removeReplace)
{
    unsigned int pos = 0;
    while((pos = line.find(item)) < line.size())
        if(removeReplace)
            line.erase(pos,1);
        else
            line[pos] = ' ';
}
