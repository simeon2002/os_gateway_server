#define _GNU_SOURCE


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"




/*
IMPORTANT INFO ABOUT *VOID POINTERS for myself!
 In C, the void * type is used for generic pointers,
 providing flexibility in handling different types of data.
 When list_node->element is assigned memory using malloc, it is assigned as a void * pointer.
 This means it's a pointer that can hold the address of any data type.
C doesn't require explicit typecasting in this scenario because the void * pointers don't hold
 any type-specific information. It's the responsibility of the programmer to properly interpret the data
 after it's been copied into the memory allocated for list_node->element based on the intended data type.
 */

struct dplist_node {
    dplist_node_t *prev, *next; // link to next els.
    void *element; // the date it holds.
};

struct dplist {
    dplist_node_t *head; // beginning of the double linked list.

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


// done, just testing.
dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
){
    dplist_t *list; // contains the head of the list as well the possible operations.
    list =(dplist_t*)malloc(sizeof(dplist_t));
    assert(list != NULL); // raising error if allocation != successful.
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

//done, just testing.
void dpl_free(dplist_t **list, bool free_element) {
    if (*list == NULL) return;
    dplist_node_t *current_node = (*list)->head;
    dplist_node_t *next_node;

    // clearing all the nodes.
    while (current_node != NULL) {
        next_node = current_node->next;
        // freeing the element if desired
        if (free_element == true && current_node->element != NULL)
        {
            (*list)->element_free(&current_node->element);
        }
        free(current_node);
        current_node = next_node;
    }
    // freeing the list itself
    free(*list);
    *list = NULL; // also setting pointer to NULL.
}

// done, just testing.
dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;
    list_node = malloc(sizeof(dplist_node_t));
    assert(list_node != NULL); // raising error if allocation != successful.
    if (insert_copy) {
        list_node->element = list->element_copy(element); // return is a pointer to copied el.
    }
    else list_node->element = element;
    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index); // gives me the node at this index.
        assert(ref_at_index != NULL); // checking condition, if not true --> error displayed.
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3 --> last element in the list.
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;
}

// done, just testing.
dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    dplist_node_t *prev_node, *next_node, *node_at_index;
    if (list == NULL) return NULL;
    else if(list->head == NULL) return list;
    else{
        node_at_index = dpl_get_reference_at_index(list, index);
        index = dpl_get_index_of_element(list, node_at_index->element);
        assert(node_at_index != NULL);
        //CORNER CASE --> INSERTING(it should be removing) ELEMENT AT INDEX 0!
        next_node = node_at_index->next;
        prev_node = node_at_index->prev;
        if (index == 0) { // if first element to be removed.
            if (next_node!= NULL) {
                next_node->prev = NULL;
                list->head = next_node;
            } else {
                list->head = NULL;
            }
        } else if(index == dpl_size(list) - 1) { // if last element to be removed
            prev_node->next = NULL;
        } else { // the generic case
            next_node->prev = prev_node;
            prev_node->next = next_node;
        }
        if (free_element) list->element_free(&(node_at_index->element));
        free(node_at_index);
    }
    return list;
}

// done, just testing.
int dpl_size(dplist_t *list) {
    int counter = 0;
    if (list == NULL) return -1;
    else if(list->head == NULL) return 0; // it's actually unnecessary
    else {
        dplist_node_t *temp = list->head;
        while(temp!= NULL) {
            temp = temp->next;
            counter++;
        }
    }
    return counter;
}

// done, just testing.
void *dpl_get_element_at_index(dplist_t *list, int index) {
    if (list == NULL || list->head == NULL) return 0;
    dplist_node_t *temp = dpl_get_reference_at_index(list, index);
    assert(temp!=NULL);
    return temp->element;
}

// done, just testing.
int dpl_get_index_of_element(dplist_t *list, void* element) {
    // case 1;
    if (list == NULL) return -1;
    // case 2;
    if (list->head == NULL) return -1;

    // case 3 and 4;
    int index = 0;
    dplist_node_t *temp = list->head;
    while(temp!= NULL) {
        if (list->element_compare(element, temp->element) == 0) {
            return index; // case 3, element exists.
        }
        temp = temp->next;
        index++;
    }

    // case 4, element doesn't exist;
    return -1;
}

// DONE, JUST TESTING.
dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    dplist_node_t *dummy = NULL;

    if (list == NULL) return NULL; //case 1: list empty or list doesn't exist.
    if (list->head == NULL) return NULL;
    else {
        // checking index errors:
        if (index > dpl_size(list) - 1) {
            index = dpl_size(list) - 1;
        } else if (index < 0) {
            index = 0;
        }
        // looping through DLL until index reached.
        dplist_node_t *temp = list->head;
        int i = 0;
        while( temp != NULL) { //OPMERKING FOR LOOP TOT EN MET DE INDEX ZELF KON HIER OOK WORDEN GEBRUIKT!
            if (i == index) {
                dummy = temp;
                break;
            }
            temp = temp->next;
            i++;
        }
    }
    return dummy;
}

// DOne, just testing.
void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL || list->head == NULL || reference == NULL) 
        return NULL;  // Case 1, 2, or 3: Invalid list, empty list, or null reference

    dplist_node_t *temp_node = list->head;
    while(temp_node != NULL) {
        if (temp_node == reference)
            return temp_node->element;  // Case 4: Reference node found, return its element
        temp_node = temp_node->next;
    }

    return NULL; // Case 5: Reference not found in the list
}


