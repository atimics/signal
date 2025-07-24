/**
 * @file json_parser.h
 * @brief Lightweight JSON parser for input action maps
 * 
 * This is a simple JSON parser specifically designed for parsing
 * input action map configuration files. It's not a full-featured
 * JSON parser but handles the subset needed for our use case.
 */

#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdbool.h>
#include <stddef.h>

// JSON value types
typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JSONValueType;

// JSON value structure
typedef struct JSONValue {
    JSONValueType type;
    union {
        bool boolean;
        double number;
        char* string;
        struct {
            struct JSONValue** items;
            size_t count;
        } array;
        struct {
            char** keys;
            struct JSONValue** values;
            size_t count;
        } object;
    } data;
} JSONValue;

// JSON parser context
typedef struct {
    const char* input;
    size_t position;
    size_t length;
    char error_message[256];
} JSONContext;

/**
 * Parse a JSON string
 * @param json_string The JSON string to parse
 * @return Parsed JSON value or NULL on error
 */
JSONValue* json_parse(const char* json_string);

/**
 * Free a JSON value and all its children
 * @param value The JSON value to free
 */
void json_free(JSONValue* value);

/**
 * Get object value by key
 * @param object The JSON object
 * @param key The key to look up
 * @return The value or NULL if not found
 */
JSONValue* json_object_get(JSONValue* object, const char* key);

/**
 * Get array item by index
 * @param array The JSON array
 * @param index The array index
 * @return The value or NULL if out of bounds
 */
JSONValue* json_array_get(JSONValue* array, size_t index);

/**
 * Get string value
 * @param value The JSON value
 * @return The string or NULL if not a string
 */
const char* json_get_string(JSONValue* value);

/**
 * Get number value
 * @param value The JSON value
 * @return The number or 0 if not a number
 */
double json_get_number(JSONValue* value);

/**
 * Get boolean value
 * @param value The JSON value
 * @return The boolean or false if not a boolean
 */
bool json_get_bool(JSONValue* value);

#endif // JSON_PARSER_H