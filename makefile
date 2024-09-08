CFLAGS = -fno-omit-frame-pointer -g3 -std=c11 -o0
CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Werror=vla

EXEC = catforth
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
# HEADERS = catforth.h

$(EXEC): $(OBJECTS)
	$(CC) $^ -o $(EXEC)

.PHONY: clean check

clean:
	rm -f $(OBJECTS) $(EXEC)

check: $(SOURCES) # $(HEADERS)
	cppcheck --enable=all --std=c11 --suppress=missingIncludeSystem $^