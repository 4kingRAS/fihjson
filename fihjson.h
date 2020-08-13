#ifndef _FIHJSON_H_
#define _FIHJSON_H_

#include <stddef.h>

/*  JSON 6 types ,  @ECMA-404 */
/*
    https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf
	JSON-text Format = whitespace value whitespace
	whitespace = *(%x20 / %x09 / %x0A / %x0D) 
              	   space / tab  /  LF  /  CR
	value = null / false / true 
       	   "null" "false" "true"

 */
typedef enum 
{ 
	F_NULL, F_FALSE, F_TRUE, F_NUMBER,
 	F_STRING, F_ARRAY, F_OBJECT
}json_type;

typedef enum 
{
	F_PARSE_OK = 0,
	F_STRINGIFY_OK = 1,
	F_PARSE_ERROR_VALUE = 2,
	F_PARSE_ERROR_MULTIPLE_ROOT = 4,
	F_PARSE_ERROR_EMPTY = 6,
	F_PARSE_ERROR_NUMBER_OUT_OF_RANGE = 8,
	F_PARSE_ERROR_MISS_QUOTATION = 10,
	F_PARSE_ERROR_ESCAPE = 12,
	F_PARSE_ERROR_MISS_COMMA_OR_BKT = 14,
	F_PARSE_ERROR_MISS_KEY = 16,
	F_PARSE_ERROR_MISS_COLON = 18,
	F_PARSE_ERROR_MISS_COMMA_OR_CBKT = 20
}parse_code;

typedef struct json_value json_value;
typedef struct json_object json_object;

struct json_value
{
	json_type type;
	/* number */
	double num;

	/* for string */
	char* s;
	size_t slen;
	
	/* for array */
	json_value* arr;
	size_t arrlen;

	/* for object*/
	json_object* obj; 
	size_t olen;
};

struct json_object 
{
	size_t klen;
	char* k;
	json_value v;
};

typedef struct 
{
	const char* json;
	char* stack;
	size_t size, top;	// dynamic stack
}json_context;

#ifndef F_JSON_CONTEXT_STACK_SIZE
#define F_JSON_CONTEXT_STACK_SIZE 256
#endif

extern char* json_type_list[]; 

void json_set_boolean(json_value* v, const int b);
void json_set_number(json_value* v, const double num);
void json_set_string(json_value* v, const char* str, size_t len);

json_type json_get_type(const json_value* value);

int json_get_boolean(const json_value* value);
double json_get_number(const json_value* value);
const char* json_get_string(const json_value* value);
size_t json_get_string_length(const json_value* value);
size_t json_get_array_size(const json_value* v);
json_value* json_get_array_element(const json_value* v, size_t index);
size_t json_get_object_size(const json_value* v);
const char* json_get_object_key(const json_value* v, size_t index);
size_t json_get_object_key_length(const json_value* v, size_t index);
json_value* json_get_object_value(const json_value* v, size_t index);


int json_parse(json_value* value, const char* json);
int json_stringify(const json_value* v, char** json, size_t* length);
int json_stringify_string(json_context* c, const char* str, size_t len);
//int json_parse_literal(json_context* c, json_value* v, char* expect);

void free_json_value(json_value* v);

#endif //FIHJSON_H