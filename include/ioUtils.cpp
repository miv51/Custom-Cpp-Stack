
#include "ioUtils.h"

long long convertUTC(const std::string& timestamp)
{
    if (timestamp.size() < 19) throw std::runtime_error("Invalid format for UTC timestamp.");

    const char* c = timestamp.c_str() + 10;

    //check hour value
    if (*(c++) != 'T') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*c < '0' || *c > '9') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 1) < '0' || *(c + 1) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");

    if (10LL * (*c - '0') + *(c + 1) - '0' > 23LL) throw std::runtime_error("Hour cannot be greater than 23 for UTC timestamp.");

    //check minute value
    if (*(c + 2) != ':') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 3) < '0' || *(c + 3) > '5') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 4) < '0' || *(c + 4) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");

    //check second value
    if (*(c + 5) != ':') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 6) < '0' || *(c + 6) > '5') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 7) < '0' || *(c + 7) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");

    //convert hours, minutes, and whole seconds (not the fractional part) to nanoseconds
    int64_t t = (10LL * (*c - '0') + *(c + 1) - '0') * 3600000000000LL + (10LL * (*(c + 3) - '0') + *(c + 4) - '0') * 60000000000LL\
        + (10LL * (*(c + 6) - '0') + *(c + 7) - '0') * 1000000000LL;

    //convert the fractional part of seconds to nanoseconds
    if (*(c += 8) == '.')
    {
        int64_t f = 100000000LL;

        while (*(++c) != 'Z' && *c != '\0')
        {
            if (*c < '0' || *c > '9') throw std::runtime_error("Invalid format for UTC timestamp.");

            t += f * (*c - '0');
            f = (f * 0xCCCCCCCDLL) >> 35; //roughly twice as fast as dividing by 10 - only works for 64-bit integers
        }
    }

    if (*c != 'Z') throw std::runtime_error("Invalid format for UTC timestamp.");

    return t;
}

long long getDaysSinceEpoch(const std::string& timestamp)
{
    //days passed over a given year since a given month
    constexpr const long long days_since_new_years[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    constexpr const long long days_in_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (timestamp.size() < 10) throw std::runtime_error("Invalid format for UTC timestamp.");

    const char* c = timestamp.c_str();

    //check year value
    if (*c < '0' || *c > '9') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 1) < '0' || *(c + 1) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 2) < '0' || *(c + 2) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 3) < '0' || *(c + 3) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");

    long long year = 1000LL * (*c - '0') + 100LL * (*(c + 1) - '0') + 10LL * (*(c + 2) - '0') + *(c + 3) - '0';
    bool is_leap = (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);

    if (year < 1970) throw std::runtime_error("Year cannot be less than 1970.");

    //check month value
    if (*(c + 4) != '-') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 5) < '0' || *(c + 5) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 6) < '0' || *(c + 6) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");

    long long month = 10LL * (*(c + 5) - '0') + *(c + 6) - '0';

    if (month < 1LL) throw std::runtime_error("Month cannot be less than 1.");
    if (month > 12LL) throw std::runtime_error("Month cannot be greater than 12.");

    //check day value
    if (*(c + 7) != '-') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 8) < '0' || *(c + 8) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");
    if (*(c + 9) < '0' || *(c + 9) > '9') throw std::runtime_error("Invalid format for UTC timestamp.");

    long long day_of_month = 10LL * (*(c + 8) - '0') + *(c + 9) - '0';

    if (day_of_month < 1LL) throw std::runtime_error("Day cannot be less than 1.");
    if (day_of_month > days_in_month[month - 1])
    {
        //if the month is february and the year is a leap year
        if (month == 2 && is_leap)
        {
            if (day_of_month > 29) throw std::runtime_error("Day is too large for the given month.");
        }
        else throw std::runtime_error("Day is too large for the given month.");
    }

    /*
    a leap year is a year that is divisible by 4 and either not divisible by 100 or divisible by 400
    so in addition to the 365 days present in each year, we add 1 day for each year divisible by 4

    this includes all years that are divisible by 100 so we need to subtract 1 day for each year divisible by 100
    the centurial years also include years divisible by 400 so we need to then add an aditional day for each year divisible by 400
    */

    long long days_since_epoch = 365LL * (year - 1970) + (year - 1) / 4 - ((1970LL - 1) / 4) - (year - 1) / 100 + ((1970LL - 1) / 100)\
        + (year - 1) / 400 - ((1970LL - 1) / 400) + days_since_new_years[month - 1] + day_of_month - 1;

    //if this is a leap year and we are past february then we need to add an extra day
    if (month > 2 && is_leap) return days_since_epoch + 1;

    return days_since_epoch;
}