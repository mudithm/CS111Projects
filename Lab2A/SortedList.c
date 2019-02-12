#ifndef SORTEDLIST_INCLUDED
#define SORTEDLIST_INCLUDED

#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "SortedList.h"


int opt_yield;
// Insert an element into a sorted list
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    // checks if the element to be added is empty
    if (element == NULL)
        return;

    // checks if the given list is empty
    if (list == NULL)
        return;

    // checks if the element is valid
    if (element->key == NULL)
    {
        fprintf(stderr, "Element has invalid key\n");
        exit(1);
    }

    // checks if the list is the head node
    if (list->key != NULL)
        return;

    if (list->next == NULL)
    {
        if (opt_yield & INSERT_YIELD)
            pthread_yield();
        list->next = element;
        return;
    }

    const char *k = element->key;
    SortedList_t *iter = list->next;

    while (iter->next != NULL && strcmp(iter->key, k) < 0)
        iter = iter->next;

    if (opt_yield & INSERT_YIELD)
        pthread_yield();

    // got to the end of the list
    if (iter->next == NULL)
    {
        iter->next = element;
        element->prev = iter;
        element->next = NULL;
        return;
    }

    // place element before iter
    element->next = iter;
    element->prev = iter->prev;
    element->prev->next = element;
    iter->prev = element;

}


// delete an element from the linked list
int SortedList_delete( SortedListElement_t *element)
{
    // Cannot delete empty element
    if (element == NULL)
        return 1;

    // Cannot delete the head of a list
    if (element->key == NULL)
        return 1;

    // check if the list is corrupted
    if ((element->prev != NULL || element->prev->next != element) || (element->next != NULL && element->next->prev != element))
        return 1;

    // opt_yield stuff
    if (opt_yield & DELETE_YIELD)
        pthread_yield();

    element->prev->next = element->next;
    // if not the last element in the list
    if (element->next != NULL)
    {
        element->next->prev = element->prev;
    }

    return 0;

}


// Find an element in a list
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    // list is empty or is not head
    if (list == NULL || list->key != NULL)
        return NULL;

    // first real element of the list
    SortedListElement_t *iter = list->next;
    while (iter != NULL)
    {
        // invalid elment in the list
        if (iter->key == NULL)
            return NULL;

        // if the key is found
        if (strcmp(iter->key, key) == 0)
            return iter;

        // opt_yield stuff
        if (opt_yield & LOOKUP_YIELD)
            pthread_yield();

        iter = iter->next;
    }

    // key not in the list
    return NULL;
}


// Find the length of a sorted list
int SortedList_length(SortedList_t *list)
{
    // list is empty or not head
    if (list == NULL || list->key != NULL)
        return 1;

    int length = 0;
    SortedList_t *iter = list->next;
    while (iter != NULL && iter != list)
    {
        length++;
        // yield stuff here
        if (opt_yield & LOOKUP_YIELD)
            pthread_yield();
        iter = iter->next;
    }

    return length;
}
/*

int main()
{
    SortedListElement_t n;
    SortedListElement_t newEl;
    SortedListElement_t newEl2;
    SortedListElement_t newEl3;
    SortedListElement_t newEl4;
    SortedListElement_t new;

    char* charArr[5] = {"hello", "there", "ululating", "western", "zz"};

    //head
    n.prev = NULL;
    n.next = &newEl;
    n.key = NULL;

    newEl.prev = &n;
    newEl.next = &newEl2;
    newEl.key = charArr[0];

    newEl2.prev = &newEl;
    newEl2.next = &newEl3;
    newEl2.key = charArr[1];

    newEl3.prev = &newEl2;
    newEl3.next = &newEl4;
    newEl3.key = charArr[2];

    newEl4.prev = &newEl3;
    newEl4.next = NULL;
    newEl4.key = charArr[3];

    new.key = charArr[4];

    SortedList_insert(&n, &new);

    printf("%s\n", SortedList_lookup(&n, "zz")->key);
    SortedList_t *iter = n.next;
    while (iter != NULL && iter->key != NULL)
    {
        printf("%s\n", iter->key);
        iter = iter->next;
    }

}
*/
#endif