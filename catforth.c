/* catforth */

#include <stdio.h>
#include <stdint.h>

#define byte uint8_t
#define cell uint32_t
#define CELL_SIZE sizeof(cell)
#define MEM_SIZE 4096
#define DATA_STACK_CELLS 64
#define RETURN_STACK_CELLS 64
#define TIB_SIZE 96
#define WORD_BUFFER_SIZE 32

#define FLAG_IMMEDIATE 0x80
#define FLAG_HIDDEN 0x40
#define MASK_NAMELENGTH 0x1F

/* memory, terminal input buffer, word buffer, stacks and their pointers */
byte memory[MEM_SIZE];
byte tib[TIB_SIZE];
byte word[WORD_BUFFER_SIZE];

cell stack[DATA_STACK_CELLS] = {0};
cell* sp = stack; 
cell top_cache = 0;
cell rack[RETURN_STACK_CELLS] = {0};
cell* rp = rack;

/* 
    interpreter pointer and a cache of the last one
    'w' cache, just a scratch register
*/
cell ip;
cell last_ip;
cell w;

/* 'high level' stuff for C-land. 1 = true, 0 = false */
int request_exit;
int error;

int main(void)
{
    return 0;
}
