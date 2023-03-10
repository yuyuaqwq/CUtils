/*
https://blog.csdn.net/gongmin856/article/details/77101397
*/

#ifndef CUTILS_ALGORITHM_KMP_MATCH_H_
#define CUTILS_ALGORITHM_KMP_MATCH_H_

#include <CUtils/object.h>


#ifdef __cplusplus
extern "C" {
#endif

uint32_t Crc32(const void*buf, size_t size);
uint32_t Crc32Start();
uint32_t Crc32Continue(uint32_t crc, const void* buf, size_t size);
uint32_t Crc32End(uint32_t crc);

#ifdef __cplusplus
}
#endif

#endif // CUTILS_ALGORITHM_MERGE_SORT_H_