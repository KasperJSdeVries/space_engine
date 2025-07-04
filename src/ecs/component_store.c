#include "ecs/component_store.h"
#include "containers/darray.h"
#include "core/assert.h"
#include "core/defines.h"
#include "ecs/entity.h"
#include "stdlib.h"

#include <string.h>
#include <unistd.h>

#define DEFAULT_ORDER 5
#define DEFAULT_COMPONENT_ARRAY_CAPACITY 8
#define COMPONENT_ARRAY_GROWTH_FACTOR 2

typedef struct node {
    darray(void *) pointers;
    darray(entity_id) keys;
    struct node *parent;
    struct node *next;
    b8 is_leaf;
} node;

static node *find_leaf(const ComponentStore *store, entity_id key) {
    if (store->root == NULL) {
        return NULL;
    }

    node *c = store->root;

    while (!c->is_leaf) {
        u32 next_index = 0;
        while (next_index < darray_length(c->keys)) {
            if (key >= c->keys[next_index]) {
                next_index++;
            } else {
                break;
            }
        }
        c = (node *)c->pointers[next_index];
    }

    return c;
}

u32 find_range(const ComponentStore *store,
               entity_id key_start,
               entity_id key_end,
               entity_id *returned_keys,
               void **returned_pointers) {
    node *n = find_leaf(store, key_start);
    if (n == NULL) {
        return 0;
    }

    u32 i;
    for (i = 0; i < darray_length(n->keys) && n->keys[i] < key_start; i++) {
    }
    if (i == darray_length(n->keys)) {
        return 0;
    }

    u32 num_found = 0;
    while (n != NULL) {
        for (; i < darray_length(n) && n->keys[i] <= key_end; i++) {
            returned_keys[num_found] = n->keys[i];
            returned_pointers[num_found] = n->pointers[i];
            num_found++;
        }

        n = n->pointers[store->order - 1];
        i = 0;
    }

    return num_found;
}

static node *make_node(void) {
    node *new_node = malloc(sizeof(node));
    new_node->keys = darray_new(entity_id);
    new_node->pointers = darray_new(void *);
    new_node->is_leaf = false;
    new_node->parent = NULL;
    new_node->next = NULL;
    return new_node;
}

static node *make_leaf(void) {
    node *leaf = make_node();
    leaf->is_leaf = true;
    return leaf;
}

static void node_destroy(node *node) {
    darray_destroy(node->keys);
    darray_destroy(node->pointers);
    free(node);
}

static u32 get_left_index(node *parent, node *left) {
    u32 left_index = 0;
    while (left_index <= darray_length(parent->keys) &&
           parent->pointers[left_index] != left) {
        left_index++;
    }
    return left_index;
}

static void insert_into_leaf(node *leaf, entity_id key, void *value) {
    u32 insertion_point = 0;
    while (insertion_point < darray_length(leaf->keys) &&
           leaf->keys[insertion_point] < key) {
        insertion_point++;
    }

    darray_insert_at(leaf->keys, insertion_point, key);
    darray_insert_at(leaf->pointers, insertion_point, value);
}

/**
 * @return NULL when nothing is found
 */
static void *find(const ComponentStore *store, entity_id key, node **leaf_out) {
    if (store->root == NULL) {
        if (leaf_out != NULL) {
            *leaf_out = NULL;
        }
        return NULL;
    }

    node *leaf = find_leaf(store, key);

    i32 found_index = -1;
    for (u32 i = 0; i < darray_length(leaf->keys); i++) {
        if (leaf->keys[i] == key) {
            found_index = i;
            break;
        }
    }

    if (leaf_out != NULL) {
        *leaf_out = leaf;
    }
    if (found_index == -1) {
        return NULL;
    } else {
        return leaf->pointers[found_index];
    }
}

static u32 cut(u32 length) {
    if (length % 2 == 0) {
        return length / 2;
    } else {
        return length / 2 + 1;
    }
}

static void insert_into_new_root(ComponentStore *store,
                                 node *left,
                                 entity_id key,
                                 node *right) {
    node *root = make_node();
    darray_push(root->keys, key);
    darray_push(root->pointers, left);
    darray_push(root->pointers, right);
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    store->root = root;
}

static void insert_into_node(node *n,
                             u32 left_index,
                             entity_id key,
                             node *right) {
    darray_insert_at(n->keys, left_index, key);
    darray_insert_at(n->pointers, left_index + 1, right);
}

static void insert_into_parent(ComponentStore *store,
                               node *left,
                               entity_id key,
                               node *right);

static void insert_into_node_after_splitting(ComponentStore *store,
                                             node *old_node,
                                             u32 left_index,
                                             entity_id key,
                                             node *right) {
    node *temp_pointers[store->order + 1];
    entity_id temp_keys[store->order];

    for (u32 i = 0, j = 0; i < darray_length(old_node->keys) + 1; i++, j++) {
        if (j == left_index + 1)
            j++;
        temp_pointers[j] = old_node->pointers[i];
    }
    for (u32 i = 0, j = 0; i < darray_length(old_node->keys); i++, j++) {
        if (j == left_index)
            j++;
        temp_keys[j] = old_node->keys[i];
    }

    temp_pointers[left_index + 1] = right;
    temp_keys[left_index] = key;

    u32 split = cut(store->order);
    node *new_node = make_node();

    darray_clear(old_node->keys);
    darray_clear(old_node->pointers);
    for (u32 i = 0; i < split - 1; i++) {
        darray_push(old_node->pointers, temp_pointers[i]);
        darray_push(old_node->keys, temp_keys[i]);
    }
    darray_push(old_node->pointers, temp_pointers[split]);

    entity_id k_prime = temp_keys[split - 1];
    for (u32 i = split + 1; i < store->order; i++) {
        darray_push(new_node->pointers, temp_pointers[i]);
        darray_push(new_node->keys, temp_keys[i]);
    }
    darray_push(new_node->pointers, temp_pointers[store->order]);

    new_node->parent = old_node->parent;
    for (u32 i = 0; i <= darray_length(new_node->keys); i++) {
        node *child = new_node->pointers[i];
        child->parent = new_node;
    }

    insert_into_parent(store, old_node, k_prime, new_node);
}

static void insert_into_parent(ComponentStore *store,
                               node *left,
                               entity_id key,
                               node *right) {
    node *parent = left->parent;

    if (parent == NULL) {
        insert_into_new_root(store, left, key, right);
        return;
    }

    u32 left_index = get_left_index(parent, left);

    if (darray_length(parent->keys) < store->order - 1) {
        insert_into_node(parent, left_index, key, right);
        return;
    }

    insert_into_node_after_splitting(store, parent, left_index, key, right);
}

static void insert_into_leaf_after_splitting(ComponentStore *store,
                                             node *leaf,
                                             entity_id key,
                                             void *value) {
    u32 insertion_index = 0;
    while (insertion_index < store->order - 1 &&
           leaf->keys[insertion_index] < key) {
        insertion_index++;
    }

    entity_id temp_keys[store->order];
    void *temp_pointers[store->order];

    for (u32 i = 0, j = 0; i < darray_length(leaf->keys); i++, j++) {
        if (j == insertion_index) {
            j++;
        }
        temp_keys[j] = leaf->keys[i];
        temp_pointers[j] = leaf->pointers[i];
    }

    temp_keys[insertion_index] = key;
    temp_pointers[insertion_index] = value;

    darray_clear(leaf->keys);
    darray_clear(leaf->pointers);

    u32 split = cut(store->order - 1);

    for (u32 i = 0; i < split; i++) {
        darray_push(leaf->pointers, temp_pointers[i]);
        darray_push(leaf->keys, temp_keys[i]);
    }

    node *new_leaf = make_leaf();

    for (u32 i = split; i < store->order; i++) {
        darray_push(new_leaf->pointers, temp_pointers[i]);
        darray_push(new_leaf->keys, temp_keys[i]);
    }

    new_leaf->next = leaf->next;
    leaf->next = new_leaf;

    new_leaf->parent = leaf->parent;
    entity_id new_key = new_leaf->keys[0];

    insert_into_parent(store, leaf, new_key, new_leaf);
}

ComponentStore component_store_new(const char *component_name,
                                   u64 component_size) {
    return (ComponentStore){
        .component_name = component_name,
        .order = DEFAULT_ORDER,
        .component_array =
            calloc(DEFAULT_COMPONENT_ARRAY_CAPACITY, component_size),
        .component_size = component_size,
        .component_capacity = DEFAULT_COMPONENT_ARRAY_CAPACITY,
        .component_count = 0,
        .free_slots = darray_new(u64),
    };
}

static void tree_destroy(node *n) {
    if (!n->is_leaf) {
        for (u32 i = 0; i < darray_length(n->pointers); i++) {
            tree_destroy(n->pointers[i]);
        }
    }

    node_destroy(n);
}

void component_store_destroy(ComponentStore *store) {
    if (store->root != NULL) {
        tree_destroy(store->root);
    }
    free(store->component_array);
    darray_destroy(store->free_slots);
}

static void start_new_tree(ComponentStore *store, entity_id key, void *value) {
    node *root = make_leaf();

    darray_push(root->keys, key);
    darray_push(root->pointers, value);

    store->root = root;
}

void component_store_insert(ComponentStore *store,
                            entity_id key,
                            const void *value_ptr) {
    void *component_pointer = find(store, key, NULL);
    if (component_pointer != NULL) {
        memcpy(component_pointer, value_ptr, store->component_size);
        return;
    }

    u64 component_index;
    if (darray_length(store->free_slots) > 0) {
        darray_pop(store->free_slots, &component_index);
    } else {
        if (store->component_count >= store->component_capacity) {
            store->component_capacity *= COMPONENT_ARRAY_GROWTH_FACTOR;
            store->component_array =
                realloc(store->component_array,
                        store->component_capacity * store->component_size);
        }
        component_index = store->component_count;
        store->component_count++;
    }
    memcpy((u8 *)store->component_array +
               component_index * store->component_size,
           value_ptr,
           store->component_size);

    component_pointer = (void *)((u64)store->component_array +
                                 component_index * store->component_size);

    if (store->root == NULL) {
        start_new_tree(store, key, component_pointer);
        return;
    }

    node *leaf = find_leaf(store, key);

    if (darray_length(leaf->keys) < store->order - 1) {
        insert_into_leaf(leaf, key, component_pointer);
        return;
    }

    insert_into_leaf_after_splitting(store, leaf, key, component_pointer);
}

static void remove_entry_from_node(node *n, entity_id key) {
    u32 index = 0;
    while (n->keys[index] != key) {
        index++;
    }

    darray_remove_at_sorted(n->keys, index);
    darray_remove_at_sorted(n->pointers, index);
}

static void adjust_root(ComponentStore *store) {
    if (darray_length(store->root->keys) > 0) {
        return;
    }

    node *new_root;
    if (!store->root->is_leaf) {
        new_root = store->root->pointers[0];
        new_root->parent = NULL;
    } else {
        new_root = NULL;
    }

    node_destroy(store->root);

    store->root = new_root;
}

static i32 get_neighbor_index(node *n) {
    for (i32 i = 0; (u32)i <= darray_length(n->parent->keys); i++) {
        if (n->parent->pointers[i] == n) {
            return i - 1;
        }
    }

    ASSERT_UNREACHABLE();
}

static void delete_entry(ComponentStore *store, node *n, entity_id key);

static void coalesce_nodes(ComponentStore *store,
                           node *n,
                           node *neigbor,
                           i32 neighbor_index,
                           entity_id k_prime) {
    if (neighbor_index == -1) {
        node *tmp = n;
        n = neigbor;
        neigbor = tmp;
    }

    if (!n->is_leaf) {
        darray_push(neigbor->keys, k_prime);

        for (u32 i = 0; i < darray_length(n->keys); i++) {
            darray_push(neigbor->keys, n->keys[i]);
            darray_push(neigbor->pointers, n->pointers[i]);
        }
        darray_push(neigbor->pointers,
                    n->pointers[darray_length(n->pointers) - 1]);

        for (u32 i = 0; i < darray_length(neigbor->pointers); i++) {
            node *tmp = (node *)neigbor->pointers[i];
            tmp->parent = neigbor;
        }
    } else {
        for (u32 i = 0; i < darray_length(n->keys); i++) {
            darray_push(neigbor->keys, n->keys[i]);
            darray_push(neigbor->pointers, n->pointers[i]);
        }
        neigbor->next = n->next;
    }

    delete_entry(store, n->parent, k_prime);
    node_destroy(n);
}

static void redistribute_nodes(node *n,
                               node *neighbor,
                               i32 neighbor_index,
                               u32 k_prime_index,
                               entity_id k_prime) {
    if (neighbor_index != -1) {
        void *neighbor_pointer;
        darray_pop(neighbor->pointers, &neighbor_pointer);
        darray_insert_at(n->pointers, 0, neighbor_pointer);

        entity_id neighbor_key;
        darray_pop(neighbor->keys, &neighbor_key);

        if (!n->is_leaf) {
            node *tmp = (node *)n->pointers[0];
            tmp->parent = n;
            darray_insert_at(n->keys, 0, k_prime);
            n->parent->keys[k_prime_index] = neighbor_key;
        } else {
            darray_insert_at(n->keys, 0, neighbor_key);
            n->parent->keys[k_prime_index] = n->keys[0];
        }
    } else {
        entity_id neighbor_key;
        darray_pop_front(neighbor->keys, &neighbor_key);

        void *neighbor_pointer;
        darray_pop_front(neighbor->pointers, &neighbor_pointer);

        if (n->is_leaf) {
            darray_push(n->keys, neighbor_key);
            darray_push(n->pointers, neighbor_pointer);
            n->parent->keys[k_prime_index] = neighbor->keys[0];
        } else {
            darray_push(n->keys, k_prime);
            darray_push(n->pointers, neighbor_pointer);
            ((node *)(neighbor_pointer))->parent = n;
            n->parent->keys[k_prime_index] = neighbor_key;
        }
    }
}

static void delete_entry(ComponentStore *store, node *n, entity_id key) {
    remove_entry_from_node(n, key);

    if (n == store->root) {
        adjust_root(store);
        return;
    }

    u32 min_keys = n->is_leaf ? cut(store->order - 1) : cut(store->order) - 1;

    if (darray_length(n->keys) >= min_keys) {
        return;
    }

    i32 neighbor_index = get_neighbor_index(n);
    u32 k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    entity_id k_prime = n->parent->keys[k_prime_index];
    node *neigbor = neighbor_index == -1 ? n->parent->pointers[1]
                                         : n->parent->pointers[neighbor_index];

    u32 capacity = n->is_leaf ? store->order : store->order - 1;

    if (darray_length(neigbor->keys) + darray_length(n->keys) < capacity) {
        coalesce_nodes(store, n, neigbor, neighbor_index, k_prime);
    } else {
        redistribute_nodes(n, neigbor, neighbor_index, k_prime_index, k_prime);
    }
}

void component_store_remove(ComponentStore *store, entity_id key) {
    node *leaf;
    void *found_component = find(store, key, &leaf);

    if (found_component == NULL) {
        return;
    }

    delete_entry(store, leaf, key);

    u64 slot = ((u64)found_component - (u64)store->component_array) /
               store->component_size;
    darray_push(store->free_slots, slot);
}

void *component_store_find(const ComponentStore *store, entity_id key) {
    return find(store, key, NULL);
}
