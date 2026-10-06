/* vector.c —— 你要实现的地方 */

#include "vector.h"
#include<stdlib.h>

int vector_init(vector *v, size_t capacity) {
    if(capacity==0)
    {
         v->cap=NULL;
         v->data=NULL;
         v->end=NULL;
        return 0;
    }
    if(capacity>(SIZE_MAX/sizeof(int)))
    {
         v->cap=NULL;
         v->data=NULL;
         v->end=NULL;
         return -1;
    }
    v->data=calloc(capacity,sizeof(int));
    if(v->data==NULL)
    {
         v->cap=NULL;
         v->data=NULL;
         v->end=NULL;
        return -1;
    }
    v->cap=v->data+capacity;
    v->end=v->data;
    return 0;
}

void vector_destroy(vector *v) 
{
    free(v->data);
    v->cap=NULL;
    v->data=NULL;
    v->end=NULL;
}

size_t size(const vector *v) {

    return v->end-v->data;
}

size_t capacity(const vector *v) {
    return v->cap-v->data;
}

int empty(const vector *v) {
    size_t s=size(v);
    if(s==0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int get(const vector *v, size_t index, int *out) {
    if(index>=size(v))
    {
        return -1;
    }
    else
    {
        *out=*(v->data+index);
         return 0;
}
    }
    
/* 把下标 index 处的元素改成 value
 * index >= size() 时返回 -1 且不修改任何元素，成功返回 0
 * 时间复杂度：O(1)
 */

int set(vector *v, size_t index, int value) {
    if(index>=size(v))
    {
        return -1;
    }
    else
    {
        *(v->data+index)=value;
        return 0;

    }
    
}


/* 读取首元素，写入 *out
 * empty(v) 时返回 -1 且不写入 *out，成功返回 0
 * 时间复杂度：O(1)
 */

int front(const vector *v, int *out) {
    if(empty(v))
    {
        return -1;
    }
    else
    {
        *out=*(v->data);
        return 0;

    }  
}

/* 读取末元素，写入 *out
 * empty(v) 时返回 -1 且不写入 *out，成功返回 0
 * 时间复杂度：O(1)
 */

int back(const vector *v, int *out) {
    if(empty(v))
    {
        return -1;
    }
    else
    {
        *out=*(v->end-1);
        return 0;

    }
}

/* 在尾部追加一个元素，size() 加一，容量不够时自动扩容
 * 容量足够时容量不变；容量为 0 时增至 1，否则严格增至原容量的 2 倍
 * 两倍容量超过 SIZE_MAX / sizeof(int) 或分配失败时返回 -1，不修改 vector
 * 成功返回 0
 * 时间复杂度：不扩容时 O(1)，扩容时 O(size())
 */

int push_back(vector *v, int value) {
    size_t s=size(v)+1;
    if(s>capacity(v))
    {
        int *data1;
        if(capacity(v)==0)
        {
            size_t old_size=size(v);
            data1=calloc(1,sizeof(int));
            if(data1==NULL)
           {
              return -1;
           }
        free(v->data);
        v->data=data1;
        v->end=data1+old_size;
        v->cap=data1+1;
             
        }
        else
        {
            size_t old_capacity=capacity(v);
            size_t old_size1=size(v);     
            if(2*old_capacity>SIZE_MAX/sizeof(int))
           {
              return -1;
           }
           data1=calloc(2*capacity(v),sizeof(int));
           if(data1==NULL)
           {
              return -1;
           }         
           int *p=v->data;
           int *p1=data1;
           for(size_t i=0;i<old_size1;i++)
           {
               *p1=*p;
               p++;
               p1++;
           }
        free(v->data);
        v->data=data1;
        v->end=data1+old_size1;
        v->cap=data1+2*old_capacity;
       

        }
        
    }
        
        *(v->end)=value;
        v->end++;
        return 0;
}


/* 删除尾部元素并写入 *out，size() 减一，容量不变
 * empty(v) 时返回 -1，不写入 *out 且不修改 vector；成功返回 0
 * 时间复杂度：O(1)
 */

int pop_back(vector *v, int *out) {
    if(empty(v))
    {
        return -1;
    }
    else
    {
        *out=*(v->end-1);
        v->end--;
        return 0;
    
    }
}

/* 扩容到至少能装 capacity 个元素，size() 和已有元素保持不变；容量只增不减
 * capacity 最大为 SIZE_MAX / sizeof(int)
 * 超过上限或扩容失败时返回 -1，容量和元素都不变
 * 成功（含 capacity 不大于当前容量、无需扩容）时返回 0
 * 时间复杂度：O(size())，不需要扩容时 O(1)
 */

int reserve(vector *v, size_t new_capacity) {
    if(new_capacity>SIZE_MAX/sizeof(int))
    {
        return -1;
    }
    if(new_capacity>capacity(v))
    {
        size_t old_size=size(v);
        int *data1;
        
        data1=calloc(new_capacity,sizeof(int));
        if(data1==NULL)
        {
            return -1;
        }
        int *p=v->data;
        int *p1=data1;
        for(size_t i=0;i<old_size;i++)
        {
             *p1=*p;
             p++;
             p1++;
        }
        free(v->data);
        v->data=data1;
        v->end=data1+old_size;
        v->cap=data1+new_capacity;
        
    }
    return 0;
}
/* 收缩容量，使 capacity() == size()，元素和 size() 不变
 * size() 为 0 时释放内存并把三个指针置为 NULL，返回 0
 * 失败时返回 -1，容量和元素都不变；成功返回 0
 * 时间复杂度：O(size())
 */
int shrink_to_fit(vector *v) {
    if(size(v)==0)
    {
        vector_destroy(v);
        return 0;
    }
    size_t old_size=size(v);
    int *data1;
        
    data1=calloc(size(v),sizeof(int));
    if(data1==NULL)
    {
        return -1;

    }
    int *p=v->data;
    int *p1=data1;
    for(size_t i=0;i<old_size;i++)
    {
        *p1=*p;
         p++;
         p1++;
    }
        free(v->data);
        v->data=data1;
        v->end=data1+old_size;
        v->cap=data1+old_size;
       
    
    return 0;
}
/* 清空所有元素：size() 变成 0，容量和缓冲区保持不变
 * 时间复杂度：O(1)
 */

void clear(vector *v) 
{
    v->end=v->data;
}
