#include <iostream>

#include "uuid.h"

using std::cout;
using std::endl;

// sudo apt-get install uuid-dev

int main()
{
    cout << "genrate_uuid: " << UUID::genrate_uuid() << endl;
    cout << "genrate_uuid_by_random: " << UUID::genrate_uuid_by_random() << endl;
    cout << "genrate_uuid_by_time: " << UUID::genrate_uuid_by_time() << endl;
}