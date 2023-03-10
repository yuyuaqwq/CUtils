/*
* @yuyuaqwq - 鱼鱼
* email:1454832774@qq.com
* project:https://github.com/yuyuaqwq/CUtils
* 请保留此声明
*/

#include <CUtils/container/list.h>

void ListHeadInit(ListHead* head) {
    head->prev = head;
    head->next = head;
}

void ListEntryInit(ListEntry* entry) {
    ListHeadInit(entry);
}

bool ListIsEmpty(ListHead* head) {
    return head->next == head && head->prev == head;
}

void ListInsertNext(ListEntry* head, ListEntry* entry) {
    ListEntry* old = head->next;
    head->next = entry;
    entry->prev = head;
    entry->next = old;
    old->prev = entry;
}

void ListInsertPrev(ListEntry* head, ListEntry* entry) {
    ListEntry* old = head->prev;
    head->prev = entry;
    entry->next = head;
    entry->prev = old;
    old->next = entry;
}

ListEntry* ListRemoveEntry(ListEntry* entry, bool init) {
    ListEntry* prev = entry->prev;
    ListEntry* next = entry->next;
    prev->next = next;
    next->prev = prev;
    if (init) {
        ListEntryInit(entry);
    }
    return entry;
}

ListEntry* ListRemoveNext(ListEntry* head) {
    return ListRemoveEntry(head->next, true);
}

ListEntry* ListRemovePrev(ListEntry* head) {
    return ListRemoveEntry(head->prev, true);
}

size_t ListGetEntryCount(ListHead* head) {
    size_t count = 0;
    ListEntry* cur = head->next;
    while (cur != head) {
        count++;
        cur = cur->next;
    }
    return count;
}


ListEntry* ListFirst(ListHead* head) {
    if (head->next == head) {
        return NULL;
    }
    return head->next;
}

ListEntry* ListPrev(ListHead* head, ListEntry* entry) {
    if (entry->prev == head) {
        return NULL;
    }
    return entry->prev;
}

ListEntry* ListNext(ListHead* head, ListEntry* entry) {
    if (entry->next == head) {
        return NULL;
    }
    return entry->next;
}