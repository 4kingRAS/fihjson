#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include "fihjson.h"
//#include "jsonUtils.h"
//
//https://github.com/miloyip/json-tutorial/

char* json_type_list[7] = { "null", "false", "true", "number",
	"string", "array", "object" };

#define JSON_INIT(v) do { (v)->type = F_NULL; } while(0)
#define PUT_CHAR(ch, c) do { *(char*)context_push(c, sizeof(char)) = (ch); } while(0)

static int json_parse_value(json_context* c, json_value* value);
static void* context_push(json_context* c, size_t size);
static void* context_pop(json_context* c, size_t size);

static void json_parse_whitespace(json_context* c);
static int json_parse_literal(json_context* c, json_value* v, json_type expect);
static int json_parse_number(json_context* c, json_value* v);
static int json_parse_string(json_context* c, json_value* v);

/*__________________DECLARATION_AND_MACRO___________________*/


/*
	parameter:  value - json type, json - json string
	return:     status enum
*/
int json_parse(json_value* value, const char* json) 
{
	json_context c; // For parse
	int ret;
	assert(value != NULL);

	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;

	JSON_INIT(value); //initialize, the parser will reassign
	json_parse_whitespace(&c); // parse before context
	ret = json_parse_value(&c, value);
	if (ret == F_PARSE_OK)
	{
		json_parse_whitespace(&c); // parse after context
		if (c.json[0] != '\0')
		{
			//multiple root exist
			ret = F_PARSE_ERROR_MULTIPLE_ROOT;
		}
	}
	assert(c.top == 0);
	free(c.stack);			// free stack
	return ret;
}

/*
	skip whitespace
*/
static void json_parse_whitespace(json_context* c)
{
	const char *p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
	{
		p++;
	}
	c->json = p;
}

/*
	deal with null, true, false
	return:     status enum
*/
static int json_parse_literal(json_context* c, json_value* v, json_type expect)
{
	assert(c != NULL && expect >= 0 && v != NULL);
	char* expect_name = json_type_list[expect];
	//char initial = expect_name[0];
	int len = (int)strlen(expect_name);

	for (int i = 0; i < len; i++)
	{
		if (c->json[i] != expect_name[i])
		{
			return F_PARSE_ERROR_VALUE;
		}
	}

	c->json += len;
	v->type = expect; //assign expect value return 0
	return F_PARSE_OK;
}

/*
	json number formation:
	數字 number = [ "-" ] int [ factor ] [ exp ]
	整數 int = "0" / digit1-9 *digit
	小數 frac = "." 1*digit
	科學計數法 exp = ("e" / "E") ["-" / "+"] 1*digit
*/
static int json_parse_number(json_context* c, json_value* v)
{
    assert(c->json != NULL);
	const char* p = c->json;
	/* validate formation */
	if (*p == '-') { p++; } /* skip - */
	if (*p == '0') 			/* skip 0 */
	{
		p++;
	}
	else
	{
		if (*p > '9' || *p < '1')
		{
			return F_PARSE_ERROR_VALUE;
		}
		for (p++; isdigit(*p); p++); /* skip integer */
	}
	if (*p == '.') 
	{
		p++;
		if (!isdigit(*p)) { return F_PARSE_ERROR_VALUE; }
		for (p++; isdigit(*p); p++); /* skip decimal */
	}
	if (*p == 'e' || *p == 'E')
	{
		p++;
		if (*p == '+' || *p == '-') { p++; }
		if (!isdigit(*p)) { return F_PARSE_ERROR_VALUE; }
		for (p++; isdigit(*p); p++); /* skip decimal */
	}
	
	errno = 0;
	v->num = strtod(c->json, NULL);
	if (errno == ERANGE && (v->num == HUGE_VAL || v->num == -HUGE_VAL)) 
	{
		return F_PARSE_ERROR_NUMBER_OUT_OF_RANGE;
	}

	v->type = F_NUMBER;
	c->json = p; // check ok
	return F_PARSE_OK;

}

/*
	json string:
	ws " char* " ws    
*/
static int json_parse_string_raw(json_context* c, char** str, size_t* len)
{
	size_t head = c->top;
	const char* p;
	assert(*c->json == '\"');
	c->json++;
	p = c->json;

	for (;;)
	{
		char ch = *p++;
		switch (ch)
		{
			case '\"':
				*len = c->top - head;
				*str = context_pop(c, *len);
				c->json = p;
				return F_PARSE_OK;
			case '\\':
				switch (*p++)
				{
					case '\"': PUT_CHAR('\"', c); break;
					case '\\': PUT_CHAR('\\', c); break;
					case '/':  PUT_CHAR('/', c);  break;
					case 'b':  PUT_CHAR('\b', c); break;
					case 'n':  PUT_CHAR('\n', c); break;
					case 'r':  PUT_CHAR('\r', c); break;
					case 't':  PUT_CHAR('\t', c); break;	
					default:
						c->top = head;
						return F_PARSE_ERROR_ESCAPE;
				}
				break;
			case '\0':
				c->top = head;
				return F_PARSE_ERROR_MISS_QUOTATION;
			default: // for Normal char, put ch to stack
				PUT_CHAR(ch, c);
		}
	}
}

static int json_parse_string(json_context* c, json_value* v)
{
	int ret;
	char* s;
	size_t len;
	if ((ret = json_parse_string_raw(c, &s, &len)) == F_PARSE_OK)
	{
		json_set_string(v, s, len);
	}
	return ret;
}

static int json_parse_array(json_context* c, json_value* v)
{
	size_t size = 0;
	int ret;
	assert(*c->json == '[');
	c->json++; // skip [
	json_parse_whitespace(c);// skip ws

	if (*c->json == ']')
	{
		c->json++;
		v->type = F_ARRAY;
		v->arrlen = 0;
		v->arr = NULL;
		return F_PARSE_OK;
	}
	for (;;) // array loop
	{
		json_value e; // tmp element
		e.type = F_NULL;
		/* recursive descent parser  a->b->a->b */
		/* if parse failed, break and free the stack*/
		if ((ret = json_parse_value(c, &e)) != F_PARSE_OK) { break; }

		//parse ok, push to stack, rm ws
		memcpy(context_push(c, sizeof(json_value)), &e, sizeof(json_value)); 
		json_parse_whitespace(c);
		size++;
		if (*c->json == ',') 
		{
			c->json++; 
			json_parse_whitespace(c);
		} else {
			if (*c->json == ']')
			{
				c->json++;
				v->type = F_ARRAY;
				v->arrlen = size;
				size *= sizeof(json_value);
				/* array end, pop to memory */
				memcpy(v->arr = (json_value*)malloc(size), context_pop(c, size), size);
				return F_PARSE_OK;
			} else {
				ret = F_PARSE_ERROR_MISS_COMMA_OR_BKT; 
				break;
			}
		}			
	}
	for (size_t i = 0; i < size; i++)
	{
		free_json_value((json_value*)context_pop(c, sizeof(json_value)));
	}
	return ret;
}

static int json_parse_object(json_context* c, json_value* v)
{
	size_t size;
	json_object o;
	int ret;
	assert(*c->json == '{');
	c->json++;
	json_parse_whitespace(c);   //skip { ws
	if (*c->json == '}') {      //empty object
		c->json++;
		v->type = F_OBJECT;
		v->obj = 0;
		v->olen = 0;
		return F_PARSE_OK;
	}
	o.k = NULL;
	size = 0; // initialize
	for (;;)
	{
	    char* str;
		o.v.type = F_NULL; // init
		/* parse key not string */
		if (*c->json != '"')
		{
		    ret = F_PARSE_ERROR_MISS_KEY;
            break;
		}
		if ((ret = json_parse_string_raw(c, &str, &o.klen)) != F_PARSE_OK) { break; }
		memcpy(o.k = (char*)malloc(o.klen + 1), str, o.klen);
		o.k[o.klen] = '\0';         // 1 - save key

		json_parse_whitespace(c);
		if (*c->json == ':')        //parse colon
		{
		    c->json++;
		    json_parse_whitespace(c);
			if ((ret = json_parse_value(c, &o.v)) != F_PARSE_OK) { break; }
			memcpy(context_push(c, sizeof(json_object)), &o, sizeof(json_object));
			size++;
			o.k = NULL;
		} else {
			ret = F_PARSE_ERROR_MISS_COLON;
            break;
        }
		json_parse_whitespace(c);
		if (*c->json == ',')
		{
		    c->json++;
		    json_parse_whitespace(c);
        } else {
            if (*c->json == '}') {
                c->json++;
                // save objects to a array
                size_t arr_size = sizeof(json_object) * size;
                v->type = F_OBJECT;
                v->olen = size;
                memcpy(v->obj = (json_object *) malloc(arr_size),
                       context_pop(c, arr_size), arr_size);
                return F_PARSE_OK;
            } else {
                ret = F_PARSE_ERROR_MISS_COMMA_OR_CBKT;
                break;
            }
        }
	}
	free(o.k);
    for (int i = 0; i < size; i++) {
        json_object* tmp = (json_object*)context_pop(c, sizeof(json_object));
        free(tmp->k);
        free_json_value(&tmp->v);
    }
    v->type = F_NULL;
	
	return ret;
}

/*
*	GETTER AND SETTER________________________
*/
void json_set_boolean(json_value* v, int b)
{
	assert(v != NULL && b > 0 && b < 2);
	free_json_value(v);
	v->type = b ? F_TRUE : F_FALSE;
}

void json_set_number(json_value* v, const double num)
{
	assert(v != NULL);
	free_json_value(v);
	v->type = F_NUMBER;
	v->num = num;
}

void json_set_string(json_value* v, const char* str, size_t len)
{
	assert(v != NULL && (str != NULL || len == 0));
	free_json_value(v);
	v->s = (char*)malloc(len + 1); // for end with \0
	memcpy(v->s, str, len);
	v->s[len] = '\0';
	v->slen = len;
	v->type = F_STRING;
}

/* getter part */
json_type json_get_type(const json_value* value)
{
	assert(value != NULL);
	return value->type;
}

int json_get_boolean(const json_value* value)
{
	assert(value != NULL &&
			(value->type ==  F_TRUE || value->type == F_FALSE));
	return (int)value->type - 1;
}

double json_get_number(const json_value* value)
{
	assert(value != NULL && value->type == F_NUMBER);
	return value->num;
}

const char* json_get_string(const json_value* value)
{
	assert(value != NULL && value->type == F_STRING);
	return value->s;
}

size_t json_get_string_length(const json_value* value)
{
	assert(value != NULL && value->type == F_STRING);
	return value->slen;
}

size_t json_get_array_size(const json_value* v)
{
	assert(v != NULL && v->type == F_ARRAY);
	return v->arrlen;
}

json_value* json_get_array_element(const json_value* v, size_t index)
{
	assert(v != NULL && v->type == F_ARRAY);
	assert(index < v->arrlen);
	return &(v->arr[index]);
}

size_t json_get_object_size(const json_value* v)
{
    assert(v != NULL && v->type == F_OBJECT);
    return v->olen;
}
const char* json_get_object_key(const json_value* v, size_t index)
{
    assert(v != NULL && v->type == F_OBJECT);
    assert(index < v->olen && index >= 0);
    return v->obj[index].k;
}
size_t json_get_object_key_length(const json_value* v, size_t index)
{
    assert(v != NULL && v->type == F_OBJECT);
    assert(index < v->olen && index >= 0);
    return v->obj[index].klen;
}
json_value* json_get_object_value(const json_value* v, size_t index)
{
    assert(v != NULL && v->type == F_OBJECT);
    assert(index < v->olen && index >= 0);
    return &v->obj[index].v;
}

/*__________________Memory Opt_____________________*/
/* FREE MEMORY*/
void free_json_value(json_value* v)
{
	assert(v != NULL);
	switch (v->type)
	{
		case F_STRING:
			free(v->s);
			break;
		case F_ARRAY:
			for (size_t i = 0; i < v->arrlen; i++)
			{
				free_json_value(&v->arr[i]); // get element
			}
			//finally free arr
			free(v->arr);
			break;
        case F_OBJECT:
            for (size_t i = 0; i < v->olen; i++)
            {
                free(v->obj[i].k);
                free_json_value(&v->obj[i].v);
            }
            free(v->obj);
            break;
		default: break;
	}
	v->type = F_NULL;
}

/*
 	json parser main branch
*/
static int json_parse_value(json_context* c, json_value* v) 
{
    /**/
    v->obj = NULL;
    v->olen = 0;
    v->type = F_NULL;
    v->arr = NULL;
    v->arrlen = 0;
    v->s = NULL;
    v->slen = 0;
    v->num = 0;
	switch (c->json[0])
	{
	    case '{'    : return json_parse_object(c, v);
		case '['	: return json_parse_array(c, v);
		case 'n'    : return json_parse_literal(c, v, F_NULL);
		case 'f'    : return json_parse_literal(c, v, F_FALSE);
		case 't'    : return json_parse_literal(c, v, F_TRUE);
		default		: return json_parse_number(c, v);
		case '"'	: return json_parse_string(c, v);
		case '\0'   : return F_PARSE_ERROR_EMPTY;
	}
}

/* return top */
static void* context_push(json_context* c, size_t size) {
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0) { c->size = F_JSON_CONTEXT_STACK_SIZE; }
        while (c->top + size >= c->size)
		{
            c->size += c->size;  /* c->size * 1.5 */
		}
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* context_pop(json_context* c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

