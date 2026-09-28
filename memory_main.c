#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include<stddef.h>
size_t remaining_space = 0;

typedef struct block
{
        size_t size;
        int free;
        struct block *next;
}block_t;

block_t *head=0;

block_t *find_free_block(size_t size)
{
        block_t *current =head;
        while(current)
        {
                if(current->free==1&&current->size>=size)
                {
                        return current;
                }
                current=current->next;
        }
        return NULL;
}

void *my_malloc(size_t size)
{
        block_t *ptr;
        if(remaining_space >= size+sizeof(block_t)+1)
        {
                ptr=find_free_block(size);
                if(ptr!=0)
                {
                        if(ptr->size>=size+sizeof(block_t)+1)
                        {
                                block_t *new=(block_t *)((char *)(ptr+1)+size);
                                new->size=ptr->size-size-sizeof(block_t);
                                new->free=1;
                                new->next=ptr->next;
                                ptr->size=size;
                                ptr->next=new;
                        }
                        else    ptr->free=0;
                        remaining_space-=sizeof(block_t)+size;
                        return (void *)(ptr+1);
                }
        }
        ptr=mmap(NULL,4096,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
        if(ptr==MAP_FAILED) return 0;
        ptr->size=size;
        ptr->free=0;
        ptr->next = (block_t *)((char *)(ptr + 1) + size);
        ptr->next->size = 4096-sizeof(block_t)- size- sizeof(block_t);
        ptr->next->free=1;
        if(head==0) head=ptr;
        else
        {
                block_t *last=head;
                while(last->next)
                        last=last->next;
                last->next=ptr;
        }
        remaining_space-=sizeof(block_t)+size;
        return (void *)(ptr+1);
}

void merge(void)
{
        block_t *current = head;

        while (current && current->next)
        {
                block_t *next = current->next;

                if (current->free == 1 &&
                                next->free == 1 &&
                                (char *)(current + 1) + current->size == (char *)next)
                {
                        current->size += sizeof(block_t) + next->size;
                        current->next = next->next;
                }
                else
                {
                        current = current->next;
                }
        }
}

void my_free(void *p)
{
        if (p == NULL)
                return;

        block_t *ptr = (block_t *)p - 1;
        ptr->free=1;
        remaining_space+=sizeof(block_t)+ptr->size;
        merge();
}


void main()
{
        int *p=my_malloc(3*sizeof(int));
        p[0]=1,p[1]=2,p[2]=3;
        for(int i=0;i<3;i++)
        {
                printf("%d ",p[i]);
        }
        printf("\naddress: %u\n",p);
        //my_free(p);
        int *q=my_malloc(3*sizeof(int));
        q[0]=9,q[1]=8,q[2]=7;
        for(int i=0;i<3;i++)
        {
                printf("%d ",q[i]);
        }
        printf("\naddress: %u\n",q);
        my_free(p);
        int *r=my_malloc(3*sizeof(int));
        r[0]=11,r[1]=12,r[2]=13;
        for(int i=0;i<3;i++)
        {
                printf("%d ",r[i]);
        }
        printf("\naddress: %u\n",r);
        my_free(p);
}