// TEST DRIVEN DEVELOPMENT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fihjson.h"
//#include "jsonUtils.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;


#define EXPECT_EQ(equality, expect, actual, format, str) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: EXPECT: " format " BUT: " format " -- in case: %s\n",\
            		 __FILE__, __LINE__, expect, actual, str);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual, str) EXPECT_EQ((expect) == (actual), expect, actual, "%d", str)

#define EXPECT_EQ_LTE(expect, actual, str) EXPECT_EQ((expect) == (actual),\
					json_type_list[expect], json_type_list[actual], "%s", str)

#define EXPECT_EQ_DOUBLE(expect, actual, str) EXPECT_EQ((expect) == (actual),\
					expect, actual, "%f", str)

#define EXPECT_EQ_STR(expect, actual, length) \
EXPECT_EQ(strlen(expect) == length && memcmp(expect, actual, length) == 0, expect, actual, "%s", actual)

#define EXPECT_EQ_SIZE_T(expect, actual, str) EXPECT_EQ((expect) == (actual), \
(size_t)expect, (size_t)actual, "%zu", str)

/*__________________DECLARATION_AND_MACRO_________________*/

static void test_parse_literal(json_type expect, const char* test_case)
{
    json_value v;
    v.type = F_NULL;
    json_parse(&v, test_case);
    EXPECT_EQ_LTE(expect, json_get_type(&v), test_case);
    free_json_value(&v);
}

static void test_parse_number(double expect_number, const char* test_case)
{
    json_value v;
    v.type = F_NULL;
    json_parse(&v, test_case);
    
    EXPECT_EQ_DOUBLE(expect_number, json_get_number(&v), test_case);
    free_json_value(&v);
}

static void test_string(char* expect_str, const char* test_case)
{
    json_value v;
    v.type = F_NULL;
    json_parse(&v, test_case);
    
    EXPECT_EQ_STR(expect_str, json_get_string(&v), json_get_string_length(&v));
    free_json_value(&v);
}

static void test_parse_single_array(const char* json, size_t len)
{
    json_value v;
    v.type = F_NULL;
    EXPECT_EQ_INT(F_PARSE_OK, json_parse(&v, json), json);
    EXPECT_EQ_LTE(F_ARRAY, json_get_type(&v), json);
    EXPECT_EQ_SIZE_T(len, json_get_array_size(&v), json);
    free_json_value(&v);
}

static void test_error_case(parse_code expect, const char* test_case)
{
	json_value v;
    v.type = F_NULL;
	EXPECT_EQ_INT(expect, json_parse(&v, test_case), test_case);
    free_json_value(&v);
}

static void test_round_trip(const char* test_case)
{
    json_value v;
    v.type = F_NULL;
    char* json;
    size_t len;

}

/* ... */

static void test_parse_cases()
{
#if 0
	/* null  */
	test_parse_literal(F_NULL, "  null");
    test_parse_literal(F_NULL, "  null  	");
    test_error_case(F_PARSE_ERROR_VALUE, "  nu11 	");
    test_error_case(F_PARSE_ERROR_EMPTY, "");
    test_error_case(F_PARSE_ERROR_EMPTY, "  		");
    test_error_case(F_PARSE_ERROR_MULTIPLE_ROOT, "null  null ");

    /* true  */
    test_parse_literal(F_TRUE, "  true  	");

    /* false  */
    test_parse_literal(F_FALSE, " false  	");

    /* number */
    test_parse_number(0.0,  "0");
    test_parse_number(0.0, "-0");
    test_parse_number(0.0, "-0.0");
    test_parse_number(1.0, "1");
    test_parse_number(-1.0, "-1");
    test_parse_number(1.5, "1.5");
    test_parse_number(-3.1416, "-3.1416");
    test_parse_number(1E10, "1E10");
    test_parse_number(1e10, "1e10");
    test_parse_number(1E+10, "1E+10");
    test_parse_number(-1e10, "-1e10");
    test_parse_number(1.234E+10, "1.234E+10");
    test_parse_number(1.234E-10, "1.234E-10");
    test_parse_number(0, "+0");
    test_error_case(F_PARSE_ERROR_VALUE, "+1");
    test_error_case(F_PARSE_ERROR_VALUE, ".123");
    test_error_case(F_PARSE_ERROR_VALUE, "1.");
    test_error_case(F_PARSE_ERROR_VALUE, "INF");
    test_error_case(F_PARSE_ERROR_VALUE, "inf");
    test_error_case(F_PARSE_ERROR_VALUE, "NAN");
    //test_parse_number(0.0, "1e-10000");

    /* string */
    test_string("12321312-0=-';l;as", "\"12321312-0=-';l;as\"");
    test_string("aaaa", "\"aaaa\"");
    test_string("", "\"\"");
    test_error_case(F_PARSE_ERROR_ESCAPE, "\"\\v\"");
    test_error_case(F_PARSE_ERROR_ESCAPE, "\"\\'\"");
    test_error_case(F_PARSE_ERROR_ESCAPE, "\"\\0\"");
    test_error_case(F_PARSE_ERROR_ESCAPE, "\"\\x12\"");
    test_error_case(F_PARSE_ERROR_ESCAPE, "\"\x01\"");
    test_error_case(F_PARSE_ERROR_ESCAPE, "\"\x1F\"");

    test_error_case(F_PARSE_ERROR_MISS_QUOTATION, "\"AAAAA  XXX");
    test_error_case(F_PARSE_ERROR_MISS_QUOTATION, "AAA\"AAAAA  XXX");
    test_error_case(F_PARSE_ERROR_MISS_QUOTATION, "   \"");
    test_error_case(F_PARSE_ERROR_MISS_QUOTATION, "\"\"\"");

    test_parse_single_array("[  ]", 0);
#endif
    //[ null , false , true , 123 , "abc" ]
    //[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]
    test_parse_single_array("[  ]", 0);
    test_parse_single_array("[ 21]", 1);
    test_parse_single_array("[ 2 ]", 1);

#if 1
    test_error_case(F_PARSE_ERROR_MISS_KEY, "{:1,");
    test_error_case(F_PARSE_ERROR_MISS_KEY, "{1:1,");
    test_error_case(F_PARSE_ERROR_MISS_KEY, "{true:1,");
    test_error_case(F_PARSE_ERROR_MISS_KEY, "{false:1,");
    test_error_case(F_PARSE_ERROR_MISS_KEY, "{null:1,");
    test_error_case(F_PARSE_ERROR_MISS_KEY, "{[]:1,");
    test_error_case(F_PARSE_ERROR_MISS_KEY, "{{}:1,");

    test_error_case(F_PARSE_ERROR_MISS_COLON, "{\"a\"}");
    test_error_case(F_PARSE_ERROR_MISS_COLON, "{\"a\",\"b\"}");

    test_error_case(F_PARSE_ERROR_MISS_COMMA_OR_CBKT, "{\"a\":1");
    test_error_case(F_PARSE_ERROR_MISS_COMMA_OR_CBKT, "{\"a\":1]");
    test_error_case(F_PARSE_ERROR_MISS_COMMA_OR_CBKT, "{\"a\":1 \"b\"");
    test_error_case(F_PARSE_ERROR_MISS_COMMA_OR_CBKT, "{\"a\":{}");
#endif


}

void print_json(json_value* v) {
    switch (v->type) {
        case F_OBJECT:
            printf("{\n");
            for (int i = 0; i < v->olen; i++) {
                printf("%s : ", v->obj[i].k);
                print_json(&v->obj[i].v);
            }
            printf("}\n");
            break;
        case F_STRING:
            printf("%s\n", v->s);
            break;
        case F_NUMBER:
            printf("%f\n", v->num);
            break;
        case F_NULL:
        case F_TRUE:
        case F_FALSE:
            printf("%s\n", json_type_list[v->type]);
            break;
        default:
            printf("error\n");
    }
}

json_value generate_json()
{

}

int main()
{
#if 0
    test_parse_cases();

    test_error_case(F_PARSE_OK, "{\"AAAA\":{\"XXX\":889}}");
#endif

    //char* json = "{\"car\" :{\n  \"id\":\"213\", \"weight\": 289.89 , \"owner\":{\"ooo\":\"111\"}  \n}\n}";
    FILE* fp = fopen("../json.txt", "r");
    if(!fp) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

    int c; // note: int, not char, required to handle EOF
    char s[256];
    size_t i = 0;

    while ((c = fgetc(fp)) != EOF) { // standard C I/O file reading loop
        s[i] = (char)c;
        i++;
    }
    s[i] = '\0';

    if (ferror(fp))
        puts("I/O error when reading");
    fclose(fp);

    printf("%s\n\n", s);

    json_value v;
    json_parse(&v, s);

    print_json(&v);

    //printf("%d/%d (%3.2f%%) PASSED \n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}


