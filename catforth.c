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

int tib_pos = 0;
int tib_len = 0;

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

static int input_key_waiting(void)
{
    return tib_pos < tib_len ? -1 : 0;
}

/* 
    FUCKING SIDE EFFECT CENTRAL 
    this thing is dual purpose. it both reads from stdin -> TIB and from TIB -> word buffer
    refactor this stupid function
*/
static int input_get_key(void)
{
    int c;

    if(input_key_waiting())
        return tib[tib_pos++];

    tib_len = 0;

    while((c = fgetc(stdin)) != EOF)
    {
        if(tib_len == TIB_SIZE) break;
        tib[tib_len++] = c;
        if(c == '\n') break;
    }

    tib_pos = 1;
    return tib[0];
}

/* basic c-side operations; stack stuff, memory access */
static void push(cell data)
{
    if(*sp >= DATA_STACK_CELLS)
    {
        printf("stack overflow!");
        error = 1;
        return;
    }

    top_cache = data;
    stack[(*sp)++] = data;
}

static cell pop(void)
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

static void rpush(cell data)
{
    if(*rp >= RETURN_STACK_CELLS)
    {
        printf("return stack overflow!");
        error = 1;
        return;
    }

    rack[(*rp)++] = data;
}

static cell rpop(void)
{
    if(*rp == 0)
    {
        printf("return stack underflow!");
        error = 1;
        return 0;
    }

    return rack[--(*rp)];
}

static cell mem_read(cell addr)
{
    if(addr > MEM_SIZE)
    {
        printf("internal error in mem_read: invalid address\n");
        error = 1;
        return 0;
    }

    return *((cell*)(memory + addr));
}

static void mem_write(cell addr, cell val)
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
// void dict_add_word()
// {

// }

// void dict_add_builtin()
// {

// }

// cell dict_find_word()
// {

// }

/* 
    'cfa'; the 'code field address'
    is it a primitive id or is it a 
*/
static cell dict_get_cfa(cell addr)
{
    byte len = (memory[addr + CELL_SIZE] & MASK_NAMELENGTH) + 1;
    while((len & (CELL_SIZE - 1)) != 0) len++;
    return addr + CELL_SIZE + len;
}

/* primitives */
static void prim_mem_read(void)
{
    push(mem_read(pop()));
}

static void prim_mem_read_byte(void)
{
    push(memory[pop()]);
}

static void prim_mem_write(void)
{
    cell address = pop();
    cell value = pop();
    mem_write(address, value);
}

static void prim_mem_write_byte(void)
{
    cell address = pop();
    cell value = pop();
    memory[address] = value & 0xFF;
}

static void prim_comma(void)
{
    push(here);
    prim_mem_write();
    here += CELL_SIZE;
}

static void prim_commaByte(void)
{
    push(here);
    prim_mem_write_byte();
    here += sizeof(byte);
}

/* read from TIB into word buffer */
static void prim_word(void)
{
    byte len = 0;
    char *line = (char*)word;
    int c;

    while((c = input_get_key()) != EOF)
    {
        if(c == ' ') continue;
        if(c == '\n') continue;
        if(c != '\\') break;

        while((c = input_get_key()) != EOF)
        {
            if(c == '\n')
                break;
        }

    }

    while(c != ' ' && c != '\n' && c != EOF)
    {
        if(len >= (WORD_BUFFER_SIZE - 1))
            break;

        line[++len] = c;
        c = input_get_key();
    }

    line[0] = len;

    push(1);
    push(len);
}

static void prim_find(void)
{
    // printf("word buffer:\n");
    // for(int i = 0; i < (WORD_BUFFER_SIZE - 1); ++i)
    // {
    //     printf("index: %d | char: %c\n", i, word[i]);
    // }

    /* 
    cell len = pop();
    cell addr = pop();
    cell ret = dict_find_word(addr, len);
    push(ret); 
    */
}

/* the inner interpreter */
static void prim_execute(void)
{

}

/* the outer interpreter */
static void prim_interpret(void)
{

}

/* the main loop */
static void prim_quit(void)
{
    for (request_exit = 0; request_exit == 0;)
    {
        last_ip = ip = quit_addr;
        error = 0;
        
        /* 
            prim_word: 
                see input_get_key
            prim_find: 
                pop the stack to get the word length and address,
                call dict_find_word and push the result to the top of the stack
        */
        prim_word();
        prim_find();

        if (error)
            *sp = *rp = 1;
        else if(!input_key_waiting())
            printf("> OK\n");
        
    }
}

static void prim_bye(void)
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
    here = 0;
    state = 0;
    top_cache = 0;

    /* 
        manually add some builtins to the dictionary and then
        run the interpreter to see if it does the work
    */

    /* enter outer interpreter */
    prim_quit();
   
   return 0;
}
