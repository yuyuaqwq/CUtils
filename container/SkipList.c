#include "SkipList.h"

const int SKIPLIST_P = (RAND_MAX / 2);
#define SKIPLIST_MAX_LEVEL 64

static int RandomLevel() {
	int level = 1;

	while (SKIPLIST_P <= rand() && level < SKIPLIST_MAX_LEVEL)
		level++;

	return level;
}

static SkipListEntry* SkipListCreateEntry(int level, void* obj) {
	SkipListEntry* entry = (SkipListEntry*)MemoryAlloc(sizeof(SkipListEntry) + level * sizeof(SkipListLevel));
	entry->obj = obj;
	return entry;
}

static forceinline SkipListEntry* SkipListFind_(SkipList* list, void* key, int* cmpRes, SkipListEntry** update) {
	SkipListEntry* cur = list->head;
	// ����㿪ʼ������ÿ��ѭ�����൱���½�һ������
	for (int i = list->level - 1; i >= 0; i--) {
		// ��ǰ������Ĳ���
		while (cur->upper[i].next) {
			void* nextKey = GetFieldByFieldOffset(cur->upper[i].next->obj, list->keyFieldOffset, void);
			*cmpRes = list->cmpFunc(nextKey, key, list->keyFieldSize);
			if (*cmpRes >= 0) {
				break;
			}
			cur = cur->upper[i].next;
		}
		
		if (update) {
			update[i] = cur;		// ��ǰ�ڵ�ò������������Ҫ ָ��ɾ����������һ���� / ָ���½ڵ�ͬ������
		}

		if (*cmpRes == 0) {
			// ��֧���ظ�key����ǰ�������ҵ��˾Ͳ�����Ҫ�½���
			break;
		}

	}
	return cur;
}

void SkipListInit(SkipList* list, int keyFieldOffset, int keyFieldSize, CmpFunc cmpFunc) {
	list->level = 1;
	list->head = SkipListCreateEntry(SKIPLIST_MAX_LEVEL, 0);

	for (int i = 0; i < SKIPLIST_MAX_LEVEL; i++) {
		list->head->upper[i].next = NULL;
	}

	list->keyFieldOffset = keyFieldOffset;
	list->keyFieldSize = keyFieldSize;

	if (cmpFunc == NULL) {
		cmpFunc = MemoryCmpR;
	}
	list->cmpFunc = cmpFunc;
}

void SkipListRelease(SkipList* list, bool deleteObj) {
	SkipListEntry* cur = SkipListFirst(list);
	while (cur) {
		if (deleteObj) {
			DeleteObject_(cur->obj);
		}
		SkipListEntry* next = SkipListNext(list, cur);
		DeleteObject_(cur);
		cur = next;
	}
}


void* SkipListFind(SkipList* list, void* key) {
	int cmpRes = 0;
	SkipListEntry* cur = SkipListFind_(list, key, &cmpRes, NULL);

	cur = cur->upper[0].next;		// �ճ�ѭ��ʱcur��δ����
	// ���ҳ�����Ҫô�����������㶼�Ҳ�����Ҫô��cur��key>=���ҵ�key

	if (cur && cmpRes == 0) {
		return cur->obj;
	}
	return NULL;
}

bool SkipListInsert(SkipList* list, void* obj) {
	SkipListEntry* update[SKIPLIST_MAX_LEVEL];		// ��Ӧÿһ����Ҫ���������Ľڵ㣬��Ϊ�½ڵ���ܻ��������

	void* key = GetFieldByFieldOffset(obj, list->keyFieldOffset, void);

	int cmpRes = 0;
	SkipListEntry* cur = SkipListFind_(list, key, &cmpRes, update);

	// cur��ʱ��nextҪôָ��NULL��Ҫô>=key
	if (cur->upper[0].next && cmpRes == 0) {
		// key����򲻲���
		return false;
	}

	// ׼����������߶�������Ľڵ�
	int level = RandomLevel();
	if (level > list->level) {
		// �½ڵ��������������Ľڵ㶼�ߣ��߳����Ĳ�����ͷ�ڵ�����������
		for (int i = list->level; i < level; i++) {
			update[i] = list->head;		// ͷ�ڵ�ò��������Ҫָ���½ڵ�ͬ������
		}
		list->level = level;
	}

	// �����ڵ�
	SkipListEntry* newEntry = SkipListCreateEntry(level, obj);
	for (int i = 0; i < level; i++) {
		// ���ӵ�ͬ������������
		newEntry->upper[i].next = update[i]->upper[i].next;
		update[i]->upper[i].next = newEntry;
	}

	return true;
}

void* SkipListDelete(SkipList* list, void* key) {
	SkipListEntry* update[SKIPLIST_MAX_LEVEL];		// ��Ӧÿһ����Ҫ���������Ľڵ㣬��Ϊ�½ڵ���ܻ�ɾ������

	int cmpRes = 0;
	SkipListEntry* cur = SkipListFind_(list, key, &cmpRes, update);
	cur = cur->upper[0].next;

	if (!cur || cmpRes) {
		// �Ҳ����ýڵ�
		return NULL;
	}

	// ��ײ�������ʼ���ϸ���
	for (int i = 0; i < list->level; i++) {
		if (update[i]->upper[i].next != cur) {		// �ò��������ָ��ɾ���Ľڵ㣬ֹͣ����
			break;
		}
		update[i]->upper[i].next = cur->upper[i].next;		// ɾ����������ָ��ɾ����������һ����
	}

	// ��ɾ�����������������������㣬��Ҫ����
	while (list->level > 1 && list->head->upper[list->level - 1].next == NULL) {
		list->level--;
	}

	void* obj = cur->obj;
	DeleteObject_(cur);
	return obj;
}


SkipListEntry* SkipListFirst(SkipList* list) {
	return list->head->upper[0].next;
}

SkipListEntry* SkipListNext(SkipList* list, SkipListEntry* cur) {
	return cur->upper[0].next;
}