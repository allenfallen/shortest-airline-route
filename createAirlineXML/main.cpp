#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <QXmlStreamWriter>
#include <QFile>

using namespace std;

void removeCharacter(string &line, char item = ',', bool removeReplace = true);
void readLineIntoArray(const string &line,string airlineInfo[]);

int main()
{
    string line, airlineInfo[8],
           input = "../resources/raw/airlines.dat";
    QFile out("../resources/xml/airlines.xml");
    QXmlStreamWriter xmlWriter(&out);
    out.open(QIODevice::WriteOnly);
    ifstream in(input);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("airlines");
    while(getline(in, line))
    {
        readLineIntoArray(line, airlineInfo);
        if(airlineInfo[0] == "-1" || airlineInfo[3] == "-"  || airlineInfo[4] == "N/A" || airlineInfo[7] == "N")
            continue;
        xmlWriter.writeStartElement("airline");
        xmlWriter.writeTextElement("airlineid", airlineInfo[0].c_str());
        xmlWriter.writeTextElement("name",airlineInfo[1].c_str());
        xmlWriter.writeTextElement("iata",airlineInfo[3].c_str());
        xmlWriter.writeTextElement("icao",airlineInfo[4].c_str());
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndDocument();
    in.close();
    out.close();
    return 0;
}


void readLineIntoArray(const string &line,string airlineInfo[])
{
    stringstream ss;
    ss<<line;
    for(int i = 0; i < 8; i++)
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
