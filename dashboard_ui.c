#include "headers.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Function prototypes
void show_vehicledata();
void show_pressure();
void draw_button(int y, int x, char *label, int color_pair);
cumulative* vehicleshm;
int main() {
	key_t vehicle_key=8907;
	int vehicle_shmid=shmget(vehicle_key,sizeof(vehicle),0777|IPC_CREAT);
	if(vehicle_shmid==-1){
		perror("Error creatong shrd mem for vehicle\n");
		return 1;
	}
	vehicleshm=(vehicle*)shmat(vehicle_shmid,NULL,0);
	if(vehicleshm==(vehicle*)-1){
		perror("Error attaching vehicle shm mem\n");
		return 1;
	}
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    // Color pairs: foreground, background
    init_pair(1, COLOR_CYAN, COLOR_BLACK);    // Title
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Temperature
    //init_pair(3, COLOR_GREEN, COLOR_BLACK);   // Pressure
    init_pair(4, COLOR_WHITE, COLOR_BLUE);    // Buttons
    init_pair(5, COLOR_BLACK, COLOR_YELLOW);  // Highlight

    srand(time(0));

    int choice = 0;

    while (1) {
        clear();

        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(1, 18, "=== SENSOR MONITORING SYSTEM ===");
        attroff(COLOR_PAIR(1) | A_BOLD);

        // Draw buttons
        draw_button(4, 10, "1. TEMPERATURE SENSOR", 2);
        draw_button(7, 10, "2. PRESSURE SENSOR", 3);
        draw_button(10, 10, "3. EXIT", 4);

        mvprintw(13, 10, "Select an option (1-3): ");
        refresh();

        choice = getch();

        if (choice == '1') {
            show_vehicledata();
        } /*else if (choice == '2') {
            show_pressure();
        }*/ else if (choice == '3') {
            break;
        } else {
            attron(COLOR_PAIR(5));
            mvprintw(15, 10, "Invalid option! Press any key...");
            attroff(COLOR_PAIR(5));
            getch();
        }
    }

    endwin();
    return 0;
}

// ------------------------------------------------------
// Helper to draw colored buttons
// ------------------------------------------------------
void draw_button(int y, int x, char *label, int color_pair) {
    attron(COLOR_PAIR(color_pair) | A_BOLD);
    mvprintw(y, x,  "+----------------------------+");
    mvprintw(y + 1, x, "| %-26s |", label);
    mvprintw(y + 2, x,  "+----------------------------+");
    attroff(COLOR_PAIR(color_pair) | A_BOLD);
}

// ------------------------------------------------------
// Function to display Temperature Sensor screen
// ------------------------------------------------------
void show_vehicledata() {

    nodelay(stdscr, TRUE);
    float temp;
    int ch;
    box(stdscr, 0, 0);
	
    while (1) {
        clear();
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(2, 12, "  TEMPERATURE SENSOR DASHBOARD");
        attroff(COLOR_PAIR(2) | A_BOLD);

       // temp = 20 + rand() % 15 + (rand() % 100) / 100.0;
        mvprintw(5, 8, "+---------------------------------------+");
        mvprintw(6, 8, "| Tyre Pressure: %.2f °C          	|", vehicleshm->v.tyre_pressure);
        mvprintw(7, 8, "| Fuel: %.2f °C          		|", vehicleshm->v.fuel_level);
        mvprintw(8, 8, "| RPM: %d °C          			|", vehicleshm->v.engine_rpm);
       	mvprintw(9, 8, "| Battery: %d °C          		|", vehicleshm->v.battery);
       	mvprintw(10, 8, "| Engine Temperature: %.2f °C           |", vehicleshm->v.engine_temp);
        mvprintw(11, 8, "+---------------------------------------+");
	
	mvprintw(12, 8, "+--------------------------------------+");
        mvprintw(13, 8, "| Speed: %.2f °C          		|", vehicleshm->c.relativeSpeed);
        mvprintw(14, 8, "| Distance: %.2f °C          		|", vehicleshm->c.distance);
        mvprintw(15, 8, "| Brake status: %d °C          	|", vehicleshm->a.brake_status);
       	mvprintw(16, 8, "| Collision flag: %d °C          	|", vehicleshm->c.collisionflag);
       //	mvprintw(10, 8, "| Engine Temperature: %.2f °C           |", vehicleshm->v.engine_temp);
        mvprintw(17, 8, "+--------------------------------------+");
        mvprintw(19, 8, "Press 'b' to go back to the Main Menu");
        refresh();

        usleep(500000);
        ch = getch();
        if (ch == 'b' || ch == 'B') {
            nodelay(stdscr, FALSE);
            break;
        }
    }
}

// ------------------------------------------------------
// Function to display Pressure Sensor screen
// ------------------------------------------------------
/*void show_pressure() {
    nodelay(stdscr, TRUE);
    float pressure;
    int ch;
    box(stdscr, 0, 0);

    while (1) {
        clear();
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(2, 12, " PRESSURE SENSOR DASHBOARD");
        attroff(COLOR_PAIR(3) | A_BOLD);

        pressure = 900 + rand() % 200 + (rand() % 100) / 100.0;
        mvprintw(5, 8, "+--------------------------------------+");
        mvprintw(6, 8, "| Current Pressure: %.2f hPa            |", pressure);
        mvprintw(7, 8, "+--------------------------------------+");

        mvprintw(10, 8, "Press 'b' to go back to the Main Menu");
        refresh();

        usleep(500000);
        ch = getch();
        if (ch == 'b' || ch == 'B') {
            nodelay(stdscr, FALSE);
            break;
        }
    }
}*/

