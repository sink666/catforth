/* 
    catforth

    word fields:
        - pointer to previous
        - length and flags
        - name
        - primitive ID or word definition

    we make a few assumptions: 
        no double cells. 
        no alternate number bases

*/

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
    interpreter pointer and a cache of the last one. ip is used by the address interpreter 
    for threading. as in, what the current command is, what the next one will be, etc
    'w' cache, just a scratch register
    'here' is the current address in memory
    'state' is the current state of the forth interpreter. compiling or interpreting
*/
cell ip;
cell last_ip;
cell w;
cell quit_addr;
cell here;
cell state;

/* 'high level' stuff for C-land. 1 = true, 0 = false */
int request_exit;
int error;

const char* firstinput = "BYE";

/* basic c-side operations; stack stuff, memory access */
void push(cell data)
{
    if(*sp >= DATA_STACK_CELLS)
    {
        printf("stack overflow!");
        error = 1;
        return 0;
    }

    top_cache = data;
    stack[(*sp)++] = data;
}

cell pop()
{
    if(*sp == 0)
    {
        printf("stack underflow!");
        error = 1;
        return 0;
    }
    
    top_cache = stack[(*sp) - CELL_SIZE];
    return stack[--(*sp)];
}

void rpush(cell data)
{
    if(*rp >= RETURN_STACK_CELLS)
    {
        printf("return stack overflow!");
        error = 1;
        return;
    }

    rack[(*rp)++] = data;
}

cell rpop()
{
    if(*rp == 0)
    {
        printf("return stack underflow!");
        error = 1;
        return 0;
    }

    return rack[--(*rp)];
}

cell mem_read(cell addr)
{
    if(addr > MEM_SIZE)
    {
        printf("internal error in mem_read: invalid address\n");
        error = 1;
        return 0;
    }

    return *((cell*)(memory + addr));
}

void mem_write(cell addr, cell val)
{
    if(addr > MEM_SIZE)
    {
        printf("internal error in mem_write: invalid address\n");
        error = 1;
        return;
    }

    *((cell*)(memory + addr)) = val;
}

/* dictionary related words  */
void dict_add_word()
{

}

void dict_add_builtin()
{

}

cell dict_find_word()
{

}

/* 
    'cfa'; the 'code field address'
    is it a primitive id or is it a 
*/
cell dict_get_cfa(cell addr)
{
    byte len = (memory[addr + CELL_SIZE] & MASK_NAMELENGTH) + 1;
    while((len & (CELL_SIZE - 1)) != 0) len++;
    return addr + CELL_SIZE + len;
}

/* primitives */
void prim_mem_read()
{
    push(mem_read(pop()));
}

void prim_mem_read_byte()
{
    push(memory[pop()]);
}

void prim_mem_write()
{
    cell address = pop();
    cell value = pop();
    mem_write(address, value);
}

void prim_mem_write_byte()
{
    cell address = pop();
    cell value = pop();
    memory[address] = value & 0xFF;
}

void prim_comma()
{
    push(here);
    prim_mem_write();
    here += CELL_SIZE;
}

void prim_commaByte()
{
    push(here);
    prim_mem_write_byte();
    here += sizeof(byte);
}

/* read from TIB into word buffer */
void prim_word()
{
    byte len = 0;
    char *line = (char*)word;
    int c;



    push(1);
    push(len);
}

void prim_find()
{
    cell len = pop();
    cell addr = pop();
    cell ret = dict_find_word(addr, len);
    push(ret);
}

/* the inner interpreter */
void prim_execute()
{

}

/* the outer interpreter */
void prim_interpret()
{

}

/* the main loop */
void prim_quit()
{

}

void prim_bye()
{
    request_exit = 1;
}

/* 
    our list of function pointers to built-ins aka primitives 
*/
void (*primitives[])(void) = {
    prim_mem_read,
    prim_mem_read_byte,
    prim_mem_write,
    prim_mem_write_byte,
    prim_comma,
    prim_commaByte,
    prim_execute,
    prim_interpret,
    prim_quit,
    prim_bye,
};

int main(void)
{
    *sp = stack; 
    *rp = rack;
    here = 0;
    state = 0;
    top_cache = 0;

    /* 
        manually add some builtins to the dictionary and then
        run the interpreter to see if it does the work
    */

   outer_interpreter();
   
   return 0;
}
