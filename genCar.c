//
// Created by 4king on 2020/8/13.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fihjson.h"

typedef struct commandValues commandValues;
typedef struct script script;
typedef struct carProfile carProfile;
typedef struct {
    char *k;
    char *v;
}pair;

struct commandValues {
    char* commandData;
    carProfile *carProf;
};

struct carProfile {
    char *carId;
    char *carBrand;
    char *carName;
    char *createTimestamp;
};

/*__________________DECLARATION_AND_MACRO_________________*/
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

json_value* generate_json() {
    return NULL;
}

void create_info_json(char* timestamp) {
    /*
    info_json json;
    json.cmd.k = "command";
    json.cmd.v = "onlineRemoteControl";
    json.cmdVal->k = "commandValues";
    json.cmdVal->commandData.k = "commandData";
    json.cmdVal->commandData.v = "RemoteCMD";
    json.cmdVal->carProf->k = "carProfile";
    json.cmdVal->carProf->carId.k = "carId";
    json.cmdVal->carProf->carId.v = "456";
    json.cmdVal->carProf->carBrand.k = "carBrand";
    json.cmdVal->carProf->carBrand.v = "BMW";
    json.cmdVal->carProf->carName.k = "carName";
    json.cmdVal->carProf->carName.v = "123";
    json.cmdVal->carProf->createTimestamp.k = "createTimestamp";
    json.cmdVal->carProf->createTimestamp.v = "20200202";
    json.timestamp.k = "createTimestamp";
    json.timestamp.v = "20200201";
    json.s = NULL;
    */
}

char* loadStringFromFile(const char *path) {
    FILE* fpr = fopen(path, "r");
    if(!fpr) {
        perror("File opening failed");
        return NULL;
    }

    int c; // note: int, not char, required to handle EOF
    char *s = (char*)malloc(256);
    size_t i = 0;

    while ((c = fgetc(fpr)) != EOF) { // standard C I/O file reading loop
        s[i] = (char)c;
        i++;
    }
    s[i] = '\0';

    if (ferror(fpr)) {
        puts("I/O error when reading");
        return NULL;
    }
    fclose(fpr);
    return s;
}

int main() {
    //char* json = "{\"car\" :{\n  \"id\":\"213\", \"weight\": 289.89 , \"owner\":{\"ooo\":\"111\"}  \n}\n}";

#if 0
    char* s = loadStringFromFile("../json.txt");
    printf("%s\n\n", s);

    json_value v;
    json_parse(&v, s);
    print_json(&v);

    char* json;
    size_t len;
    json_stringify(&v, &json, &len);

    FILE* fpw = fopen("../a.txt", "w");
    if(!fpw) {
        perror("File create failed");
        return EXIT_FAILURE;
    }
    fprintf(fpw, "%s", json);
#endif

    json_value v;
    v.add = add_json;
    init_json_value(&v);
    v.add(&v, F_OBJECT, "AAA", add_json());


    //printf("%d/%d (%3.2f%%) PASSED \n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}
