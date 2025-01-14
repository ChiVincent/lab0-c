#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head == NULL)
        return NULL;

    INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    // When l is NULL, it should not be freed.
    if (l == NULL)
        return;

    struct list_head *pos, *tmp;

    list_for_each_safe (pos, tmp, l) {
        list_del(pos);
        q_release_element(list_entry(pos, element_t, list));
    }

    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    // When head is NULL or given string is invalid, it should not be inserted.
    if (head == NULL || s == NULL)
        return false;

    // Allocate space for new node.
    element_t *e = malloc(sizeof(element_t));
    if (e == NULL)
        return false;

    // Copy string to new node.
    int str_len = strlen(s);
    e->value = malloc(str_len + 1);
    if (e->value == NULL) {
        free(e);
        return false;
    }

    memcpy(e->value, s, str_len);
    e->value[str_len] = '\0';

    // Insert new node at head.
    list_add(&e->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    // When head is NULL or given string is invalid, it should not be inserted.
    if (head == NULL || s == NULL)
        return false;

    element_t *e = malloc(sizeof(element_t));
    if (e == NULL)
        return false;

    // Copy string to new node.
    int str_len = strlen(s);
    e->value = malloc(str_len + 1);
    if (e->value == NULL) {
        free(e);
        return false;
    }

    memcpy(e->value, s, str_len);
    e->value[str_len] = '\0';

    // Insert new node at tail.
    list_add_tail(&e->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_first_entry(head, element_t, list);
    list_del(&e->list);

    if (sp) {
        memcpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return e;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_last_entry(head, element_t, list);
    list_del(&e->list);

    if (sp) {
        memcpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return e;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int size = 0;
    struct list_head *pos;
    list_for_each (pos, head)
        size++;

    return size;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *slow = head->next, *fast = head->next->next;

    for (; fast != head && fast->next != head;
         fast = fast->next->next, slow = slow->next)
        ;

    struct list_head *mid = slow;
    list_del(mid);
    q_release_element(list_entry(mid, element_t, list));

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    if (list_empty(head))
        return true;

    // create garbage colloector
    struct list_head *gc = q_new();
    if (!gc)
        return false;

    bool should_delete = false;
    struct list_head *pos, *next;
    list_for_each_safe (pos, next, head) {
        if (next == head)
            break;
        element_t *e = list_entry(pos, element_t, list),
                  *n = list_entry(next, element_t, list);

        if (strcmp(e->value, n->value) == 0) {
            should_delete = true;
            list_move(pos, gc);
        } else if (should_delete) {
            should_delete = false;
            list_move(pos, gc);
        }
    }

    q_free(gc);

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    for (struct list_head **indirect = &head->next;
         *indirect != head && (*indirect)->next != head;
         indirect = &(*indirect)->next->next) {
        list_move(*indirect, (*indirect)->next);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    for (struct list_head *pos = head->next; pos != head; pos = pos->prev) {
        struct list_head *tmp = pos->prev;
        pos->prev = pos->next;
        pos->next = tmp;
    }

    struct list_head *last = head->next;
    head->next = head->prev;
    head->prev = last;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
struct list_head *merge(struct list_head *L1, struct list_head *L2)
{
    struct list_head *head = NULL, **ptr = &head, **cur;
    for (cur = NULL; L1 && L2; *cur = (*cur)->next) {
        element_t *l = list_entry(L1, element_t, list),
                  *r = list_entry(L2, element_t, list);
        cur = (strcmp(l->value, r->value) <= 0) ? &L1 : &L2;
        *ptr = *cur;
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((uintptr_t) L1 | (uintptr_t) L2);
    return head;
}

struct list_head *merge_sort(struct list_head *head)
{
    if (!head->next)
        return head;

    struct list_head *slow = head, *fast = head->next;
    for (; fast && fast->next; fast = fast->next->next, slow = slow->next)
        ;

    struct list_head *mid = slow->next;
    slow->next = NULL;

    return merge(merge_sort(head), merge_sort(mid));
}

void q_sort(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    head->prev->next = NULL;
    head->next = merge_sort(head->next);

    struct list_head *ptr = head;
    for (; ptr->next; ptr = ptr->next)
        ptr->next->prev = ptr;
    ptr->next = head;
    head->prev = ptr;
}
