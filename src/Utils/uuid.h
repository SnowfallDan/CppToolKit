#ifndef CPPTOOLKITS_UUID_H
#define CPPTOOLKITS_UUID_H

// sudo apt-get install uuid-dev

#include <uuid/uuid.h>
#include <string>
#include <cstdio>

using std::string;

class UUID
{
public:
    // The uuid will be generated based on high-quality randomness from /dev/urandom, if available.
    // If it is not available, then uuid_generate will use an alternative algorithm which uses the current time,
    // the local ethernet MAC address (if available), and random data generated using a pseudo-random generator.
    static string genrate_uuid()
    {
        string uuid;
        uuid_t uu = {0};
        uuid_generate(uu);
        char buf[256] = {0};
        uuid2string_(uu, buf);
        return std::move(string(buf));
    }

    //  Function forces the use of the all-random UUID format,
    //  even if a high-quality random number generator (i.e., /dev/urandom) is not available,
    //  in which case a pseudo-random generator will be substituted.
    //  Note that the use of a pseudo-random generator may compromise the uniqueness of UUIDs generated in this fashion.
    static string genrate_uuid_by_random()
    {
        string uuid;
        uuid_t uu = {0};
        uuid_generate_random(uu);
        char buf[256] = {0};
        uuid2string_(uu, buf);
        return std::move(string(buf));
    }

    // Function forces the use of the alternative algorithm
    // which uses the current time and the local ethernet MAC address (if available).
    static string genrate_uuid_by_time()
    {
        string uuid;
        uuid_t uu = {0};
        uuid_generate_time(uu);
        char buf[256] = {0};
        uuid2string_(uu, buf);
        return std::move(string(buf));
    }

    // Compare UUID is same
    static bool compare_uuid(const string &luuid, const string &ruuid)
    {
        return luuid == ruuid;
    }

    UUID() = delete;
    ~UUID() = delete;

private:
    static void uuid2string_(const uuid_t uu, char *str)
    {
        for (int i = 0; i < 16; i++)
            sprintf(str + i * 2, "%02X", uu[i]);
    }
};

#endif //CPPTOOLKITS_UUID_H
