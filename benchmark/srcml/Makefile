.PHONY: all

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.xml)

all:$(OBJS)

%.xml:%.c
	src2srcml --position $< | xmllint --format - > $@
#	src2srcml $< | xmllint --format - > $@

clean:
	rm -rf $(OBJS)
