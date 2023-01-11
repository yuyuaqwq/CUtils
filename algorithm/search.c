#include "search.h"

/*
* �ҵ��˷�����ȷ�±꣬���򷵻�-1
*/
int BinarySearch(int* array, int first, int last, int key) {
	int mid;
	while (first <= last) {
		mid = first + (last - first) / 2;
		if (array[mid] > key) {
			last = mid - 1;
		}
		else if (array[mid] < key) {
			first = mid + 1;
		}
		else {
			return mid;
		}
	}
	return -1;
}

/*
* �ҵ��˷�����ȷ�±꣬���򷵻ص�һ��С�ڻ����key��Ԫ���±꣬(ͨ�����ص�һ������key��Ԫ���±꣬������Ԫ�ض�С��key���򷵻����һ��Ԫ�ص��±�)
*/
int BinarySearch_Range(int* array, int first, int last, int key) {
	int mid = 0;
	while (first < last) {
		mid = first + (last - first) / 2;
		if (array[mid] < key) first = mid + 1;
		else last = mid;
	}
	return first;
}


int BinarySearch_Custom(void* objArr, int objSize, int keyFieldOffset, int keyFieldSize, int first, int last, void* key, CmpFunc cmpFunc) {
	int mid;
	while (first <= last) {
		mid = first + (last - first) / 2;
		void* curObj = ObjArrAt(objArr, mid, objSize);
		void* curKey = GetFieldByFieldOffset(curObj, keyFieldOffset, void);
		int res = cmpFunc(curKey, key, keyFieldSize);
		if (res > 0) {
			last = mid - 1;
		}
		else if (res < 0) {
			first = mid + 1;
		}
		else {
			return mid;
		}
	}
	return -1;
}

int BinarySearch_Range_Custom(void* objArr, int objSize, int keyFieldOffset, int keyFieldSize, int first, int last, void* key, CmpFunc cmpFunc) {
	int mid = 0;
	while (first < last) {
		mid = first + (last - first) / 2;
		void* curObj = ObjArrAt(objArr, mid, objSize);
		void* curKey = GetFieldByFieldOffset(curObj, keyFieldOffset, void);
		if (cmpFunc(curKey, key, keyFieldSize) < 0) first = mid + 1;
		else last = mid;
	}
	return first;
}
