
//custom input and output (io) utilities used to speed up data parsing

#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stdexcept>
#include <string>

template <typename dataType>
constexpr inline const char* typeToString() noexcept { return "unspecified"; }

template <> constexpr inline const char* typeToString<int>() noexcept { return "int"; }
template <> constexpr inline const char* typeToString<long long>() noexcept { return "long long"; }
template <> constexpr inline const char* typeToString<double>() noexcept { return "double"; }
template <> constexpr inline const char* typeToString<long double>() noexcept { return "long double"; }
template <> constexpr inline const char* typeToString<float>() noexcept { return "float"; }

template <typename dataType>
inline void throwRuntimeError(const std::string& reason, const std::string& number)
{
    std::string message = reason;

    message.append(" ");
    message.append(typeToString<dataType>());
    message.append(" for the number ");
    message.append(number);
    message.append(".");

    throw std::runtime_error(message);
}

//calculate current time of day in nanoseconds since midnight - convert the current hour, minute, second, and fraction of the current second
long long convertUTC(const std::string&);

//calculate the number of days passed since 1970-01-01 for a given timestamp
long long getDaysSinceEpoch(const std::string&);

//roughly 3 times as fast as std::stod and std::stoll
template <typename dataType>
constexpr inline dataType convert(const std::string& number)
{
    dataType num = 0;
    const char* c = number.c_str();

    if (*c == '\0') return num;
    if (*c == '-')
    {
        while (*(++c) != '.' && *c != '\0')
        {
            if (*c < '0' || *c > '9') throwRuntimeError<dataType>("Invalid format for data type", number);
            if (10 * num + *c - '0' <= num && num != 0) throwRuntimeError<dataType>("Number is too large for data type", number);

            num = 10 * num + *c - '0';
        }

        if (*c == '.')
        {
            dataType factor = 1;

            while (*(++c) != '\0')
            {
                if (*c < '0' || *c > '9') throwRuntimeError<dataType>("Invalid format for data type", number);

                factor = factor * 0.1;
                num += factor * (*c - '0');
            }

            return -num;

        }

        if (*c == '\0') return -num;

        throwRuntimeError<dataType>("Invalid format for data type", number);
    }

    while (*c != '.' && *c != '\0')
    {
        if (*c < '0' || *c > '9') throwRuntimeError<dataType>("Invalid format for data type", number);
        if (10 * num + *c - '0' <= num && num != 0) throwRuntimeError<dataType>("Number is too large for data type", number);

        num = 10 * num + *(c++) - '0';
    }

    if (*c == '.')
    {
        dataType factor = 1;

        while (*(++c) != '\0')
        {
            if (*c < '0' || *c > '9') throwRuntimeError<dataType>("Invalid format for data type", number);

            factor = factor * 0.1;
            num += factor * (*c - '0');
        }

        return num;

    }

    if (*c == '\0') return num;

    throwRuntimeError<dataType>("Invalid format for data type", number);
}

#endif
