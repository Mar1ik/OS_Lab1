CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
LDFLAGS = -lcrypto -lpthread

# Dirs
SHELL_DIR = shell
CPU_DIR = cpu-calc-md5
EMA_DIR = ema-replace-int

# Binaries
SHELL_BIN = $(SHELL_DIR)/shell
CPU_BIN = $(CPU_DIR)/cpu-calc-md5
CPU_BIN_OPT = $(CPU_DIR)/cpu-calc-md5-opt
CPU_BIN_MT = $(CPU_DIR)/cpu-calc-md5-mt
EMA_BIN = $(EMA_DIR)/ema-replace-int
EMA_GEN = $(EMA_DIR)/ema-gen-data

.PHONY: all clean shell cpu ema test

all: shell cpu ema

shell: $(SHELL_BIN)

$(SHELL_BIN): $(SHELL_DIR)/shell.c
	$(CC) $(CFLAGS) -o $@ $<

cpu: $(CPU_BIN) $(CPU_BIN_OPT) $(CPU_BIN_MT)

$(CPU_BIN): $(CPU_DIR)/cpu-calc-md5.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(CPU_BIN_OPT): $(CPU_DIR)/cpu-calc-md5.c
	$(CC) $(CFLAGS) -O3 -o $@ $< $(LDFLAGS)

$(CPU_BIN_MT): $(CPU_DIR)/cpu-calc-md5-mt.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

ema: $(EMA_BIN) $(EMA_GEN)

$(EMA_BIN): $(EMA_DIR)/ema-replace-int.c
	$(CC) $(CFLAGS) -o $@ $<

$(EMA_GEN): $(EMA_DIR)/ema-gen-data.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(SHELL_BIN) $(CPU_BIN) $(CPU_BIN_OPT) $(CPU_BIN_MT) $(EMA_BIN) $(EMA_GEN)
	rm -f *.bin *.dat

test: all
	@echo "=== Тестирование Shell ==="
	echo "echo test1 ; echo test2 ; echo test3" | $(SHELL_BIN)
	@echo ""
	@echo "=== Тестирование CPU нагрузчика ==="
	$(CPU_BIN) 10
	@echo ""
	@echo "=== Тестирование EMA нагрузчика ==="
	$(EMA_GEN) test.bin 1 42
	$(EMA_BIN) test.bin 1 42 99 1