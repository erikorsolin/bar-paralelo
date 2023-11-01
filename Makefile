# Nome do executável
TARGET = program

# Opções do compilador
CC = gcc
CFLAGS = -Wall -Werror -lpthread

# Arquivo fonte
SOURCE = main.c

# Regra padrão
all: $(TARGET)

# Regra de compilação
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $^ -o $@

# Regra para limpeza
clean:
	rm -f $(TARGET)
