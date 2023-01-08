#include "search.h"

/*
* �ҵ��˷�����ȷ�±꣬���򷵻�-1
*/
int BinarySearch(int* array, int low, int high, int key) {
	int mid = 0;
	while (low < high) {
		mid = low + (high - low) / 2;
		if (array[mid] < key) low = mid + 1;
		else if (array[mid] == key) return mid;
		else high = mid;
	}
	return -1;
}

/*
* �ҵ��˷�����ȷ�±꣬���򷵻ص�һ��С�ڻ����key��Ԫ���±꣬(ͨ�����ص�һ������key��Ԫ���±꣬������Ԫ�ض�С��key���򷵻����һ��Ԫ�ص��±�)
*/
int BinarySearch_Range(int* array, int low, int high, int key) {
	int mid = 0;
	while (low < high) {
		mid = low + (high - low) / 2;
		if (array[mid] < key) low = mid + 1;
		else high = mid;
	}
	return low;
}


int BinarySearch_Custom(void* objArr, int objSize, int keyFieldOffset, int keyFieldSize, int low, int high, void* key, CmpFunc cmpFunc) {
	int mid = 0;
	while (low < high) {
		mid = low + (high - low) / 2;
		void* curObj = ObjArrAt(objArr, mid, objSize);
		void* curKey = GetFieldByFieldOffset(curObj, keyFieldOffset, void);
		int res = cmpFunc(curKey, key, keyFieldSize);
		if (res < 0) low = mid + 1;
		else if (res == 0) return mid;
		else high = mid;
	}
	return -1;
}

int BinarySearch_Range_Custom(void* objArr, int objSize, int keyFieldOffset, int keyFieldSize, int low, int high, void* key, CmpFunc cmpFunc) {
	int mid = 0;
	while (low < high) {
		mid = low + (high - low) / 2;
		void* curObj = ObjArrAt(objArr, mid, objSize);
		void* curKey = GetFieldByFieldOffset(curObj, keyFieldOffset, void);
		if (cmpFunc(curKey, key, keyFieldSize) < 0) low = mid + 1;
		else high = mid;
	}
	return low;
}
