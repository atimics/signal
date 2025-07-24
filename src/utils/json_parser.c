/**
 * @file json_parser.c
 * @brief Lightweight JSON parser implementation
 */

#include "json_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

// Forward declarations
static JSONValue* parse_value(JSONContext* ctx);
static JSONValue* parse_object(JSONContext* ctx);
static JSONValue* parse_array(JSONContext* ctx);
static JSONValue* parse_string(JSONContext* ctx);
static JSONValue* parse_number(JSONContext* ctx);
static JSONValue* parse_literal(JSONContext* ctx);
static void skip_whitespace(JSONContext* ctx);
static bool consume_char(JSONContext* ctx, char c);
static void set_error(JSONContext* ctx, const char* format, ...);

// Implementation

JSONValue* json_parse(const char* json_string) {
    if (!json_string) return NULL;
    
    JSONContext ctx = {
        .input = json_string,
        .position = 0,
        .length = strlen(json_string),
        .error_message = {0}
    };
    
    skip_whitespace(&ctx);
    JSONValue* result = parse_value(&ctx);
    
    if (result) {
        skip_whitespace(&ctx);
        if (ctx.position < ctx.length) {
            set_error(&ctx, "Unexpected character after JSON value");
            json_free(result);
            return NULL;
        }
    }
    
    return result;
}

void json_free(JSONValue* value) {
    if (!value) return;
    
    switch (value->type) {
        case JSON_STRING:
            free(value->data.string);
            break;
            
        case JSON_ARRAY:
            for (size_t i = 0; i < value->data.array.count; i++) {
                json_free(value->data.array.items[i]);
            }
            free(value->data.array.items);
            break;
            
        case JSON_OBJECT:
            for (size_t i = 0; i < value->data.object.count; i++) {
                free(value->data.object.keys[i]);
                json_free(value->data.object.values[i]);
            }
            free(value->data.object.keys);
            free(value->data.object.values);
            break;
            
        default:
            break;
    }
    
    free(value);
}

JSONValue* json_object_get(JSONValue* object, const char* key) {
    if (!object || object->type != JSON_OBJECT || !key) return NULL;
    
    for (size_t i = 0; i < object->data.object.count; i++) {
        if (strcmp(object->data.object.keys[i], key) == 0) {
            return object->data.object.values[i];
        }
    }
    
    return NULL;
}

JSONValue* json_array_get(JSONValue* array, size_t index) {
    if (!array || array->type != JSON_ARRAY) return NULL;
    if (index >= array->data.array.count) return NULL;
    
    return array->data.array.items[index];
}

const char* json_get_string(JSONValue* value) {
    if (!value || value->type != JSON_STRING) return NULL;
    return value->data.string;
}

double json_get_number(JSONValue* value) {
    if (!value || value->type != JSON_NUMBER) return 0;
    return value->data.number;
}

bool json_get_bool(JSONValue* value) {
    if (!value || value->type != JSON_BOOL) return false;
    return value->data.boolean;
}

// Parser implementation

static void skip_whitespace(JSONContext* ctx) {
    while (ctx->position < ctx->length && isspace((unsigned char)ctx->input[ctx->position])) {
        ctx->position++;
    }
}

static bool consume_char(JSONContext* ctx, char c) {
    skip_whitespace(ctx);
    if (ctx->position < ctx->length && ctx->input[ctx->position] == c) {
        ctx->position++;
        return true;
    }
    return false;
}

static void set_error(JSONContext* ctx, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(ctx->error_message, sizeof(ctx->error_message), format, args);
    va_end(args);
}

static JSONValue* parse_value(JSONContext* ctx) {
    skip_whitespace(ctx);
    
    if (ctx->position >= ctx->length) {
        set_error(ctx, "Unexpected end of input");
        return NULL;
    }
    
    char c = ctx->input[ctx->position];
    
    if (c == '{') return parse_object(ctx);
    if (c == '[') return parse_array(ctx);
    if (c == '"') return parse_string(ctx);
    if (c == '-' || isdigit((unsigned char)c)) return parse_number(ctx);
    if (c == 't' || c == 'f' || c == 'n') return parse_literal(ctx);
    
    set_error(ctx, "Unexpected character: %c", c);
    return NULL;
}

static JSONValue* parse_object(JSONContext* ctx) {
    if (!consume_char(ctx, '{')) {
        set_error(ctx, "Expected '{'");
        return NULL;
    }
    
    JSONValue* obj = calloc(1, sizeof(JSONValue));
    obj->type = JSON_OBJECT;
    
    size_t capacity = 8;
    obj->data.object.keys = calloc(capacity, sizeof(char*));
    obj->data.object.values = calloc(capacity, sizeof(JSONValue*));
    
    while (!consume_char(ctx, '}')) {
        // Parse key
        JSONValue* key_val = parse_string(ctx);
        if (!key_val) {
            json_free(obj);
            return NULL;
        }
        
        char* key = key_val->data.string;
        key_val->data.string = NULL;
        json_free(key_val);
        
        if (!consume_char(ctx, ':')) {
            free(key);
            json_free(obj);
            set_error(ctx, "Expected ':' after object key");
            return NULL;
        }
        
        // Parse value
        JSONValue* value = parse_value(ctx);
        if (!value) {
            free(key);
            json_free(obj);
            return NULL;
        }
        
        // Add to object
        if (obj->data.object.count >= capacity) {
            capacity *= 2;
            obj->data.object.keys = realloc(obj->data.object.keys, capacity * sizeof(char*));
            obj->data.object.values = realloc(obj->data.object.values, capacity * sizeof(JSONValue*));
        }
        
        obj->data.object.keys[obj->data.object.count] = key;
        obj->data.object.values[obj->data.object.count] = value;
        obj->data.object.count++;
        
        // Check for comma or end
        skip_whitespace(ctx);
        if (ctx->position < ctx->length && ctx->input[ctx->position] == ',') {
            ctx->position++;
        } else if (ctx->position < ctx->length && ctx->input[ctx->position] == '}') {
            // End of object
        } else {
            json_free(obj);
            set_error(ctx, "Expected ',' or '}' in object");
            return NULL;
        }
    }
    
    return obj;
}

static JSONValue* parse_array(JSONContext* ctx) {
    if (!consume_char(ctx, '[')) {
        set_error(ctx, "Expected '['");
        return NULL;
    }
    
    JSONValue* arr = calloc(1, sizeof(JSONValue));
    arr->type = JSON_ARRAY;
    
    size_t capacity = 8;
    arr->data.array.items = calloc(capacity, sizeof(JSONValue*));
    
    while (!consume_char(ctx, ']')) {
        JSONValue* item = parse_value(ctx);
        if (!item) {
            json_free(arr);
            return NULL;
        }
        
        // Add to array
        if (arr->data.array.count >= capacity) {
            capacity *= 2;
            arr->data.array.items = realloc(arr->data.array.items, capacity * sizeof(JSONValue*));
        }
        
        arr->data.array.items[arr->data.array.count++] = item;
        
        // Check for comma or end
        skip_whitespace(ctx);
        if (ctx->position < ctx->length && ctx->input[ctx->position] == ',') {
            ctx->position++;
        } else if (ctx->position < ctx->length && ctx->input[ctx->position] == ']') {
            // End of array
        } else {
            json_free(arr);
            set_error(ctx, "Expected ',' or ']' in array");
            return NULL;
        }
    }
    
    return arr;
}

static JSONValue* parse_string(JSONContext* ctx) {
    if (!consume_char(ctx, '"')) {
        set_error(ctx, "Expected '\"'");
        return NULL;
    }
    
    size_t start = ctx->position;
    size_t length = 0;
    
    while (ctx->position < ctx->length && ctx->input[ctx->position] != '"') {
        if (ctx->input[ctx->position] == '\\') {
            ctx->position++; // Skip escape character
            if (ctx->position >= ctx->length) {
                set_error(ctx, "Unexpected end of string");
                return NULL;
            }
        }
        ctx->position++;
        length++;
    }
    
    if (!consume_char(ctx, '"')) {
        set_error(ctx, "Unterminated string");
        return NULL;
    }
    
    // Allocate and copy string
    char* str = malloc(length + 1);
    size_t write_pos = 0;
    
    for (size_t read_pos = start; read_pos < start + length; read_pos++) {
        if (ctx->input[read_pos] == '\\' && read_pos + 1 < start + length) {
            read_pos++;
            switch (ctx->input[read_pos]) {
                case '"': str[write_pos++] = '"'; break;
                case '\\': str[write_pos++] = '\\'; break;
                case '/': str[write_pos++] = '/'; break;
                case 'b': str[write_pos++] = '\b'; break;
                case 'f': str[write_pos++] = '\f'; break;
                case 'n': str[write_pos++] = '\n'; break;
                case 'r': str[write_pos++] = '\r'; break;
                case 't': str[write_pos++] = '\t'; break;
                default: str[write_pos++] = ctx->input[read_pos]; break;
            }
        } else {
            str[write_pos++] = ctx->input[read_pos];
        }
    }
    str[write_pos] = '\0';
    
    JSONValue* val = calloc(1, sizeof(JSONValue));
    val->type = JSON_STRING;
    val->data.string = str;
    
    return val;
}

static JSONValue* parse_number(JSONContext* ctx) {
    size_t start = ctx->position;
    
    // Optional negative sign
    if (ctx->input[ctx->position] == '-') {
        ctx->position++;
    }
    
    // Integer part
    if (ctx->position >= ctx->length || !isdigit((unsigned char)ctx->input[ctx->position])) {
        set_error(ctx, "Invalid number");
        return NULL;
    }
    
    while (ctx->position < ctx->length && isdigit((unsigned char)ctx->input[ctx->position])) {
        ctx->position++;
    }
    
    // Fractional part
    if (ctx->position < ctx->length && ctx->input[ctx->position] == '.') {
        ctx->position++;
        if (ctx->position >= ctx->length || !isdigit((unsigned char)ctx->input[ctx->position])) {
            set_error(ctx, "Invalid number");
            return NULL;
        }
        while (ctx->position < ctx->length && isdigit((unsigned char)ctx->input[ctx->position])) {
            ctx->position++;
        }
    }
    
    // Exponent part
    if (ctx->position < ctx->length && (ctx->input[ctx->position] == 'e' || ctx->input[ctx->position] == 'E')) {
        ctx->position++;
        if (ctx->position < ctx->length && (ctx->input[ctx->position] == '+' || ctx->input[ctx->position] == '-')) {
            ctx->position++;
        }
        if (ctx->position >= ctx->length || !isdigit((unsigned char)ctx->input[ctx->position])) {
            set_error(ctx, "Invalid number");
            return NULL;
        }
        while (ctx->position < ctx->length && isdigit((unsigned char)ctx->input[ctx->position])) {
            ctx->position++;
        }
    }
    
    // Parse the number
    size_t length = ctx->position - start;
    char* num_str = malloc(length + 1);
    memcpy(num_str, ctx->input + start, length);
    num_str[length] = '\0';
    
    double num = strtod(num_str, NULL);
    free(num_str);
    
    JSONValue* val = calloc(1, sizeof(JSONValue));
    val->type = JSON_NUMBER;
    val->data.number = num;
    
    return val;
}

static JSONValue* parse_literal(JSONContext* ctx) {
    if (ctx->position + 4 <= ctx->length && strncmp(ctx->input + ctx->position, "true", 4) == 0) {
        ctx->position += 4;
        JSONValue* val = calloc(1, sizeof(JSONValue));
        val->type = JSON_BOOL;
        val->data.boolean = true;
        return val;
    }
    
    if (ctx->position + 5 <= ctx->length && strncmp(ctx->input + ctx->position, "false", 5) == 0) {
        ctx->position += 5;
        JSONValue* val = calloc(1, sizeof(JSONValue));
        val->type = JSON_BOOL;
        val->data.boolean = false;
        return val;
    }
    
    if (ctx->position + 4 <= ctx->length && strncmp(ctx->input + ctx->position, "null", 4) == 0) {
        ctx->position += 4;
        JSONValue* val = calloc(1, sizeof(JSONValue));
        val->type = JSON_NULL;
        return val;
    }
    
    set_error(ctx, "Invalid literal");
    return NULL;
}