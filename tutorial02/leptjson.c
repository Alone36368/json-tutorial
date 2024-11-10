#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h> /*errno,ERANGE*/
#include <math.h> /*HUGE_VAL*/

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}
#if 0
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
#endif

/**
 * 合并前三个函数为lept_parse_literal()
 */
static int lept_parse_literal(lept_context* c,lept_value* v,const char* literal,lept_type type){
    size_t i;
    EXPECT(c,literal[0]);
    for(i=0;literal[i+1]!='\0';i++){
        if(c->json[i]!=literal[i+1])return LEPT_PARSE_INVALID_VALUE;
    }
    c->json+=i;
    v->type = type;
    return LEPT_PARSE_OK;
}



/**
 * 十进制转2进制使用strtod()函数
 */
#define ISDIGITAL(c) ((c)>='0'&&(c)<='9')
#define ISDIGITAL1TO9(c) ((c)>='1'&&(c)<='9')
static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;
    /* 负号 ... */
    if(*p=='-')p++;
    /* 整数 ... */
    if(*p=='0')p++;
    else{
        if(!ISDIGITAL1TO9(*p))return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGITAL(*p);p++);
    }
    /* 小数 ... */
    if(*p=='.'){
        p++;
        if(!ISDIGITAL(*p))return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGITAL(*p);p++);
    }
    /* 指数 ... */
    if(*p=='e'||*p=='E'){
        p++;
        if(*p=='+'||*p=='-')p++;
        if(!ISDIGITAL(*p))return LEPT_PARSE_INVALID_VALUE;
        for(p++;ISDIGITAL(*p);p++);
    }
    errno = 0;
    v->n = strtod(c->json, NULL);
    if(errno==ERANGE&&v->n==HUGE_VAL)return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v,"true",LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v,"false",LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v,"null",LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}


/**
 *   只有为number是才用n存储
 */
double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
