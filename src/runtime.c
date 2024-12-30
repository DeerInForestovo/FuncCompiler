#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include "runtime.h"

struct node_base* alloc_node() {
    struct node_base* new_node = malloc(sizeof(struct node_app));
    new_node->gc_next = NULL;
    new_node->gc_reachable = 0;
    assert(new_node != NULL);
    return new_node;
}

struct node_app* alloc_app(struct node_base* l, struct node_base* r) {
    struct node_app* node = (struct node_app*) alloc_node();
    node->base.tag = NODE_APP;
    node->left = l;
    node->right = r;
    return node;
}

struct node_num* alloc_num(int32_t n) {
    struct node_num* node = (struct node_num*) alloc_node();
    node->base.tag = NODE_NUM;
    node->value = n;
    return node;
}

struct node_global* alloc_global(void (*f)(struct gmachine*), int32_t a) {
    struct node_global* node = (struct node_global*) alloc_node();
    node->base.tag = NODE_GLOBAL;
    node->arity = a;
    node->function = f;
    return node;
}

struct node_ind* alloc_ind(struct node_base* n) {
    struct node_ind* node = (struct node_ind*) alloc_node();
    node->base.tag = NODE_IND;
    node->next = n;
    return node;
}

void free_node_direct(struct node_base* n) {
    if(n->tag == NODE_DATA) {
        free(((struct node_data*) n)->array);
    }
}

void gc_visit_node(struct node_base* n) {
    if(n->gc_reachable) return;
    n->gc_reachable = 1;

    if(n->tag == NODE_APP) {
        struct node_app* app = (struct node_app*) n;
        gc_visit_node(app->left);
        gc_visit_node(app->right);
    } if(n->tag == NODE_IND) {
        struct node_ind* ind = (struct node_ind*) n;
        gc_visit_node(ind->next);
    } if(n->tag == NODE_DATA) {
        struct node_data* data = (struct node_data*) n;
        struct node_base** to_visit = data->array;
        while(*to_visit) {
            gc_visit_node(*to_visit);
            to_visit++;
        }
    }
}

void stack_init(struct stack* s) {
    s->size = 4;
    s->count = 0;
    s->data = malloc(sizeof(*s->data) * s->size);
    assert(s->data != NULL);
}

void stack_free(struct stack* s) {
    free(s->data);
}

void stack_push(struct stack* s, struct node_base* n) {
    while(s->count >= s->size) {
        s->data = realloc(s->data, sizeof(*s->data) * (s->size *= 2));
        assert(s->data != NULL);
    }
    s->data[s->count++] = n;
}

struct node_base* stack_pop(struct stack* s) {
    assert(s->count > 0);
    return s->data[--s->count];
}

struct node_base* stack_peek(struct stack* s, size_t o) {
    assert(s->count > o);
    return s->data[s->count - o - 1];
}

void stack_popn(struct stack* s, size_t n) {
    assert(s->count >= n);
    s->count -= n;
}

void gmachine_init(struct gmachine* g) {
    stack_init(&g->stack);
    g->gc_nodes = NULL;
    g->gc_node_count = 0;
    g->gc_node_threshold = 128;
}

void gmachine_free(struct gmachine* g) {
    stack_free(&g->stack);
    struct node_base* to_free = g->gc_nodes;
    struct node_base* next;

    while(to_free) {
        next = to_free->gc_next;
        free_node_direct(to_free);
        free(to_free);
        to_free = next;
    }
}

void gmachine_slide(struct gmachine* g, size_t n) {
    assert(g->stack.count > n);
    g->stack.data[g->stack.count - n - 1] = g->stack.data[g->stack.count - 1];
    g->stack.count -= n;
}

void gmachine_update(struct gmachine* g, size_t o) {
    assert(g->stack.count > o + 1);
    struct node_ind* ind =
        (struct node_ind*) g->stack.data[g->stack.count - o - 2];
    ind->base.tag = NODE_IND;
    ind->next = g->stack.data[g->stack.count -= 1];
}

void gmachine_alloc(struct gmachine* g, size_t o) {
    while(o--) {
        stack_push(&g->stack,
                gmachine_track(g, (struct node_base*) alloc_ind(NULL)));
    }
}

void gmachine_pack(struct gmachine* g, size_t n, int8_t t) {
    assert(g->stack.count >= n);

    struct node_base** data = malloc(sizeof(*data) * (n + 1));
    assert(data != NULL);
    memcpy(data, &g->stack.data[g->stack.count - n], n * sizeof(*data));
    data[n] = NULL;

    struct node_data* new_node = (struct node_data*) alloc_node();
    new_node->array = data;
    new_node->base.tag = NODE_DATA;
    new_node->tag = t;

    stack_popn(&g->stack, n);
    stack_push(&g->stack, gmachine_track(g, (struct node_base*) new_node));
}

void gmachine_split(struct gmachine* g, size_t n) {
    struct node_data* node = (struct node_data*) stack_pop(&g->stack);
    for(size_t i = 0; i < n; i++) {
        stack_push(&g->stack, node->array[i]);
    }
}

struct node_base* gmachine_track(struct gmachine* g, struct node_base* b) {
    g->gc_node_count++;
    b->gc_next = g->gc_nodes;
    g->gc_nodes = b;

    if(g->gc_node_count >= g->gc_node_threshold) {
        uint64_t nodes_before = g->gc_node_count;
        gc_visit_node(b);
        gmachine_gc(g);
        g->gc_node_threshold = g->gc_node_count * 2;
    }

    return b;
}

void gmachine_gc(struct gmachine* g) {
    for(size_t i = 0; i < g->stack.count; i++) {
        gc_visit_node(g->stack.data[i]);
    }

    struct node_base** head_ptr = &g->gc_nodes;
    while(*head_ptr) {
        if((*head_ptr)->gc_reachable) {
            (*head_ptr)->gc_reachable = 0;
            head_ptr = &(*head_ptr)->gc_next;
        } else {
            struct node_base* to_free = *head_ptr;
            *head_ptr = to_free->gc_next;
            free_node_direct(to_free);
            free(to_free);
            g->gc_node_count--;
        }
    }
}

void unwind(struct gmachine* g) {
    struct stack* s = &g->stack;

    while(1) {
        struct node_base* peek = stack_peek(s, 0);
        if(peek->tag == NODE_APP) {
            struct node_app* n = (struct node_app*) peek;
            stack_push(s, n->left);
        } else if(peek->tag == NODE_GLOBAL) {
            struct node_global* n = (struct node_global*) peek;
            assert(s->count > n->arity);

            for(size_t i = 1; i <= n->arity; i++) {
                s->data[s->count - i]
                    = ((struct node_app*) s->data[s->count - i - 1])->right;
            }

            n->function(g);
        } else if(peek->tag == NODE_IND) {
            struct node_ind* n = (struct node_ind*) peek;
            stack_pop(s);
            stack_push(s, n->next);
        } else {
            break;
        }
    }
}

extern void f_main(struct gmachine* s);

void print_node(struct node_base* n) {
    if(n->tag == NODE_APP) {
        struct node_app* app = (struct node_app*) n;
        print_node(app->left);
        putchar(' ');
        print_node(app->right);
    } else if(n->tag == NODE_DATA) {
        printf("(Packed)");
    } else if(n->tag == NODE_GLOBAL) {
        struct node_global* global = (struct node_global*) n;
        printf("(Global: %p)", global->function);
    } else if(n->tag == NODE_IND) {
        print_node(((struct node_ind*) n)->next);
    } else if(n->tag == NODE_NUM) {
        struct node_num* num = (struct node_num*) n;
        printf("%d", num->value);
    }
}

int main(int argc, char** argv) {
    struct gmachine gmachine;
    struct node_global* first_node = alloc_global(f_main, 0);
    struct node_base* result;

    gmachine_init(&gmachine);
    gmachine_track(&gmachine, (struct node_base*) first_node);
    stack_push(&gmachine.stack, (struct node_base*) first_node);
    unwind(&gmachine);
    result = stack_pop(&gmachine.stack);
    printf("Result: ");
    print_node(result);
    putchar('\n');
    gmachine_free(&gmachine);
}
