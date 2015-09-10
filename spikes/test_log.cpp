#include <iostream>
#include "spdlog/spdlog.h"

int main(int, char* [])
{
    namespace spd = spdlog;
    try
    {
        //Create console, multithreaded logger
        auto console = spd::stdout_logger_mt("console");
        console->info("Welcome to spdlog!") ;
        console->info("An info message example {}..", 1);
        console->info() << "Streams are supported too  " << 1;

        //Formatting examples
        console->info("Easy padding in numbers like {:08d}", 12);
        console->info("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
        console->info("Support for floats {:03.2f}", 1.23456);
        console->info("Positional args are {1} {0}..", "too", "supported");

        console->info("{:<30}", "left aligned");
        console->info("{:>30}", "right aligned");
        console->info("{:^30}", "centered");

        //
        // Runtime log levels
        //
        spd::set_level(spd::level::info); //Set global log level to info
        console->debug("This message shold not be displayed!");
        console->set_level(spd::level::debug); // Set specific logger's log level
        console->debug("Now it should..");

        //
        // Create a file rotating logger with 5mb size max and 3 rotated files
        //
        auto file_logger = spd::rotating_logger_mt("file_logger", "logs/mylogfile", 1048576 * 5, 3);
        for(int i = 0; i < 10; ++i)
              file_logger->info("{} * {} equals {:>10}", i, i, i*i);

        //
        // Create a daily logger - a new file is created every day on 2:30am
        //
        auto daily_logger = spd::daily_logger_mt("daily_logger", "logs/daily", 2, 30);

        //
        // Customize msg format for all messages
        //
        spd::set_pattern("*** [%H:%M:%S %z] [thread %t] %v ***");
        file_logger->info("This is another message with custom format");

        spd::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name) function");

    }
    catch (const spd::spdlog_ex& ex)
    {
        std::cout << "Log failed: " << ex.what() << std::endl;
    }
}


// Example of user defined class with operator<<
class some_class {};
std::ostream& operator<<(std::ostream& os, const some_class& c) { return os << "some_class"; }

void custom_class_example()
{
    some_class c;
    spdlog::get("console")->info("custom class with operator<<: {}..", c);
    spdlog::get("console")->info() << "custom class with operator<<: " << c << "..";
}
