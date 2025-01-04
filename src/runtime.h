#pragma once
#include <stdlib.h>

struct gmachine;

enum node_tag {
    NODE_APP,
    NODE_NUM,
    NODE_FLOAT,
    NODE_GLOBAL,
    NODE_IND,
    NODE_DATA
};

struct node_base {
    enum node_tag tag;
    int8_t gc_reachable;
    struct node_base* gc_next;
};

struct node_app {
    struct node_base base;
    struct node_base* left;
    struct node_base* right;
};

struct node_num {
    struct node_base base;
    int32_t value;
};

struct node_float {
    struct node_base base;
    float value;
};

struct node_global {
    struct node_base base;
    int32_t arity;
    void (*function)(struct gmachine*);
};

struct node_ind {
    struct node_base base;
    struct node_base* next;
};

struct node_data {
    struct node_base base;
    int8_t tag;
    struct node_base** array;
};

struct node_base* alloc_node();
struct node_app* alloc_app(struct node_base* l, struct node_base* r);
struct node_num* alloc_num(int32_t n);
struct node_float* alloc_float(float n);
struct node_global* alloc_global(void (*f)(struct gmachine*), int32_t a);
struct node_ind* alloc_ind(struct node_base* n);
void free_node_direct(struct node_base*);
void gc_visit_node(struct node_base*);

struct stack {
    size_t size;
    size_t count;
    struct node_base** data;
};

void stack_init(struct stack* s);
void stack_free(struct stack* s);
void stack_push(struct stack* s, struct node_base* n);
struct node_base* stack_pop(struct stack* s);
struct node_base* stack_peek(struct stack* s, size_t o);
void stack_popn(struct stack* s, size_t n);

struct gmachine {
    struct stack stack;
    struct node_base* gc_nodes;
    int64_t gc_node_count;
    int64_t gc_node_threshold;
    int8_t gc_enabled;
};

void gmachine_init(struct gmachine* g);
void gmachine_free(struct gmachine* g);
void gmachine_slide(struct gmachine* g, size_t n);
void gmachine_update(struct gmachine* g, size_t o);
void gmachine_alloc(struct gmachine* g, size_t o);
void gmachine_pack(struct gmachine* g, size_t n, int8_t t);
void gmachine_split(struct gmachine* g, size_t n);
void gmachine_enablegc(struct gmachine* g);
void gmachine_disablegc(struct gmachine* g);
struct node_base* gmachine_track(struct gmachine* g, struct node_base* b);
void gmachine_gc(struct gmachine* g);
