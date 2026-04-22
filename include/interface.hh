#pragma once

#include <string>

// clang-format off

// Reset
#define COLOR_RESET "\033[0m"

// Regular Colors
#define BLACK     "\033[0;30m"
#define RED       "\033[0;31m"
#define GREEN     "\033[0;32m"
#define YELLOW    "\033[0;33m"
#define BLUE      "\033[0;34m"
#define PURPLE    "\033[0;35m"
#define CYAN      "\033[0;36m"
#define WHITE     "\033[0;37m"

// Bold Colors
#define B_BLACK   "\033[1;30m"
#define B_RED     "\033[1;31m"
#define B_GREEN   "\033[1;32m"
#define B_YELLOW  "\033[1;33m"
#define B_BLUE    "\033[1;34m"
#define B_PURPLE  "\033[1;35m"
#define B_CYAN    "\033[1;36m"
#define B_WHITE   "\033[1;37m"

// Underline Colors
#define U_BLACK   "\033[4;30m"
#define U_RED     "\033[4;31m"
#define U_GREEN   "\033[4;32m"
#define U_YELLOW  "\033[4;33m"
#define U_BLUE    "\033[4;34m"
#define U_PURPLE  "\033[4;35m"
#define U_CYAN    "\033[4;36m"
#define U_WHITE   "\033[4;37m"

// Background Colors
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_PURPLE  "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

// Box elements
#define LIGHT_VERTICAL_LEFT                 "\u2524"
#define VERTICAL_SINGLE_LEFT_DOUBLE         "\u2561"
#define VERTICAL_DOUBLE_LEFT_SINGLE         "\u2562"
#define DOWN_DOUBLE_LEFT_SINGLE             "\u2556"
#define DOWN_SINGLE_LEFT_DOUBLE             "\u2555"
#define DOUBLE_VERTICAL_LEFT                "\u2563"
#define DOUBLE_VERTICAL                     "\u2551"
#define DOUBLE_DOWN_LEFT                    "\u2557"
#define DOUBLE_UP_LEFT                      "\u255D"
#define UP_DOUBLE_LEFT_SINGLE               "\u255C"
#define UP_SINGLE_LEFT_DOUBLE               "\u255B"
#define LIGHT_DOWN_LEFT                     "\u2510"
#define LIGHT_UP_RIGHT                      "\u2514"
#define LIGHT_UP_HORIZONTAL                 "\u2534"
#define LIGHT_DOWN_HORIZONTAL               "\u252C"
#define LIGHT_VERTICAL_RIGHT                "\u251C"
#define LIGHT_HORIZONTAL                    "\u2500"
#define LIGHT_VERTICAL                      "\u2502"
#define LIGHT_VERTICAL_HORIZONTAL           "\u253C"
#define VERTICAL_SINGLE_RIGHT_DOUBLE        "\u255E"
#define VERTICAL_DOUBLE_RIGHT_SINGLE        "\u255F"
#define DOUBLE_UP_RIGHT                     "\u255A"
#define DOUBLE_DOWN_RIGHT                   "\u2554"
#define DOUBLE_UP_HORIZONTAL                "\u2569"
#define DOUBLE_DOWN_HORIZONTAL              "\u2566"
#define DOUBLE_VERTICAL_RIGHT               "\u2560"
#define DOUBLE_HORIZONTAL                   "\u2550"
#define DOUBLE_VERTICAL_HORIZONTAL          "\u256C"
#define UP_SINGLE_HORIZONTAL_DOUBLE         "\u2567"
#define UP_DOUBLE_HORIZONTAL_SINGLE         "\u2568"
#define DOWN_SINGLE_HORIZONTAL_DOUBLE       "\u2564"
#define DOWN_DOUBLE_HORIZONTAL_SINGLE       "\u2565"
#define UP_DOUBLE_RIGHT_SINGLE              "\u2559"
#define UP_SINGLE_RIGHT_DOUBLE              "\u2558"
#define DOWN_SINGLE_RIGHT_DOUBLE            "\u2552"
#define DOWN_DOUBLE_RIGHT_SINGLE            "\u2553"
#define VERTICAL_DOUBLE_HORIZONTAL_SINGLE   "\u256B"
#define VERTICAL_SINGLE_HORIZONTAL_DOUBLE   "\u256A"
#define LIGHT_UP_LEFT                       "\u2518"
#define LIGHT_DOWN_RIGHT                    "\u250C"

// clang-format on

/**
 * @brief Print out a formatted success message
 *
 * @param msg The message to be displayed
 */
void success(const std::string &msg);

/**
 * @brief Print out a formatted debugging message
 *
 * @param msg The message to be displayed
 */
void debug(const std::string &msg);

/**
 * @brief Print out a formatted information
 *
 * @param msg The message to be displayed
 */
void info(const std::string &msg);

/**
 * @brief Print out a formatted warning
 *
 * @param msg The message to be displayed
 */
void warning(const std::string &msg);

void error(const std::string &msg);

/**
 * @brief Ask the user and return the user's answer
 *
 * @param question The question to be asked
 * @return true if the user responded Y and false otherwise
 */
bool ask(const std::string &question);

/**
 * @brief Ask the question to the user and get the answer
 *
 * @param question The question to be asked
 * @return the user's answer
 */
std::string input(const std::string &question);

/**
 * @brief Ask the question to the user and get the answer
 *
 * @param question The question to be asked
 * @return the user's answer
 */
std::string input(const std::string &question, const std::string &default_answer);
