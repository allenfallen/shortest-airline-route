#include <iostream>
#include "airlinepath.h"

using namespace std;

int main()
{
    airlinePath ap;
    ap.openXML("resources/xml/finalData.xml");
    ap.run();
    return 0;
}
