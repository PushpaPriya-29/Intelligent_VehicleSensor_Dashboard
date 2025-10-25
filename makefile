# Makefile for Intelligent Vehicle Sensor Dashboard

CC = gcc
CFLAGS = -Wall -g
LIBS_PTHREAD = -lpthread
LIBS_NCURSES = -lncurses
LIBS_MATH = -lm

# Source files
ALERT_HELPER = alert_helper.c

# Executables
TARGETS = alert collision drivermonitor environmental gpsloc parking dashboard cruise_fifo

# Default target
all: fifo $(TARGETS)
	@echo "=========================================="
	@echo "All programs compiled successfully!"
	@echo "=========================================="
	@echo "Run 'make start' to start all processes"

# Create FIFO
fifo: cruise_fifo.c
	$(CC) $(CFLAGS) cruise_fifo.c -o cruise_fifo
	./cruise_fifo 2>/dev/null || true

# Individual targets with alert_helper
alert: vehicle_health.c alert_helper.c headers.h
	$(CC) $(CFLAGS) vehicle_health.c alert_helper.c -o alert 

collision: collision.c alert_helper.c headers.h
	$(CC) $(CFLAGS) collision.c alert_helper.c -o collision 

drivermonitor: drivermonitor.c alert_helper.c headers.h
	$(CC) $(CFLAGS) drivermonitor.c alert_helper.c -o drivermonitor 

environmental: environmental_sensors.c alert_helper.c headers.h
	$(CC) $(CFLAGS) environmental_sensors.c alert_helper.c -o environmental 

gpsloc: gpsloc.c alert_helper.c headers.h
	$(CC) $(CFLAGS) gpsloc.c alert_helper.c -o gpsloc  $(LIBS_MATH)

parking: parking.c alert_helper.c headers.h
	$(CC) $(CFLAGS) parking.c alert_helper.c -o parking 

#cruise: cruise.c headers.h
	#$(CC) $(CFLAGS) cruise.c -o cruise

dashboard: vehicle_dashboard.c alert_helper.c headers.h
	$(CC) $(CFLAGS) vehicle_dashboard.c alert_helper.c -o dashboard $(LIBS_NCURSES) $(LIBS_MATH)

# Start all processes
start: all
	@echo "Starting all sensor processes..."
	@./alert &
	@./collision &
	@./drivermonitor &
	@./environmental &
	@./gpsloc &
	@./parking &
	@sleep 2
	@echo "Starting dashboard..."
	@./dashboard

# Clean target
clean:
	rm -f $(TARGETS)
	rm -f cuise_collision_fifo
	@echo "Cleaned all executables and FIFO"

# Clean shared memory
cleanshm:
	@echo "Removing shared memory segments..."
	@ipcs -m | grep 8108 | awk '{print $$2}' | xargs -I {} ipcrm -m {} 2>/dev/null || true
	@ipcs -m | grep 9999 | awk '{print $$2}' | xargs -I {} ipcrm -m {} 2>/dev/null || true
	@echo "Shared memory cleaned"

# Kill all running processes
killall:
	@echo "Killing all running processes..."
	@killall alert collision drivermonitor environmental gpsloc parking dashboard 2>/dev/null || true
	@echo "All processes terminated"

# Full clean (executables + shared memory + processes)
fullclean: killall clean cleanshm
	@echo "Full cleanup complete"

# Help target
help:
	@echo "Available targets:"
	@echo "  make          - Compile all programs"
	@echo "  make start    - Compile and start all processes + dashboard"
	@echo "  make clean    - Remove executables and FIFO"
	@echo "  make cleanshm - Remove shared memory segments"
	@echo "  make killall  - Kill all running processes"
	@echo "  make fullclean- Full cleanup (all of the above)"
	@echo "  make help     - Show this help message"

.PHONY: all clean cleanshm killall fullclean start help fifo
