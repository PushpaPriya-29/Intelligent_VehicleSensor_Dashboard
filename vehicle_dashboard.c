#include "headers.h"
#include <ncurses.h>

// Global pointers
cumulative* vehicleshm;
alert_message* alertshm;
pid_t collision_pid;

// Color definitions
#define COLOR_TITLE 1
#define COLOR_NORMAL 2
#define COLOR_WARNING 3
#define COLOR_CRITICAL 4
#define COLOR_SUCCESS 5
#define COLOR_BUTTON 6
#define COLOR_SELECTED 7

void init_colors();
void screen1_welcome();
void screen2_input();
void screen3_main_dashboard();
void screen4_parking();
void display_sensor_boxes(int start_y);
void draw_categorized_alerts(int y, int x, int height, int width);
void draw_gps_box_with_alerts(int y, int x);
void draw_parking_alerts(int y, int x, int height, int width);
void clear_all_data();
int contains_keyword(const char* message, const char* keyword);

int main() {
    // Attach to sensor data shared memory
    key_t vehicle_key = 8108;
    int vehicle_shmid = shmget(vehicle_key, sizeof(cumulative), 0777 | IPC_CREAT);
    if (vehicle_shmid == -1) {
        perror("Error creating shared memory");
        return 1;
    }
    vehicleshm = (cumulative*)shmat(vehicle_shmid, NULL, 0);
    
    // Attach to alert shared memory
    key_t alert_key = 9999;
    int alert_shmid = shmget(alert_key, sizeof(alert_message), 0777 | IPC_CREAT);
    if (alert_shmid == -1) {
        perror("Error creating alert shared memory");
        return 1;
    }
    alertshm = (alert_message*)shmat(alert_shmid, NULL, 0);
    
    // Read collision PID from FIFO
    int fd = open("cuise_collision_fifo", O_RDONLY);
    if (fd < 0) {
        perror("fifo opening failed\n");
        return 1;
    }
    sleep(1);
    int l = read(fd, &collision_pid, sizeof(collision_pid));
    if (l < 0) {
        perror("fifo read failed\n");
        return 1;
    }
    close(fd);
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors()) {
        start_color();
        init_colors();
    }

    // Main loop
    while (1) {
        clear_all_data();
        screen1_welcome();
        screen2_input();
        screen3_main_dashboard();
    }

    endwin();
    shmdt(vehicleshm);
    shmdt(alertshm);
    return 0;
}

void init_colors() {
    init_pair(COLOR_TITLE, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_NORMAL, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_WARNING, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_CRITICAL, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_SUCCESS, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_BUTTON, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SELECTED, COLOR_BLACK, COLOR_CYAN);
}

// Helper function to check if message contains keyword
int contains_keyword(const char* message, const char* keyword) {
    return (strstr(message, keyword) != NULL);
}

// Clear all previous data at start
void clear_all_data() {
    clear_all_alerts(alertshm);
    
    vehicleshm->g.src.latitude = 0.0;
    vehicleshm->g.src.longitude = 0.0;
    vehicleshm->g.dest.latitude = 0.0;
    vehicleshm->g.dest.longitude = 0.0;
    vehicleshm->g.dist = 0.0;
    vehicleshm->g.reached_flag = 0;
    
    vehicleshm->c.relativeSpeed = 0.0;
    vehicleshm->c.distance = 0.0;
    vehicleshm->c.collisionflag = 0;
    
    vehicleshm->p.parking_flag = 0;
}

// SCREEN 1: Welcome Screen
void screen1_welcome() {
    int selected = 0;
    int ch;
    
    while (1) {
        clear();
        
        attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        mvprintw(8, (COLS - 50) / 2, "+------------------------------------------------+");
        mvprintw(9, (COLS - 50) / 2, "|   INTELLIGENT VEHICLE SENSOR DASHBOARD        |");
        mvprintw(10, (COLS - 50) / 2, "+------------------------------------------------+");
        attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        
        int button_y = LINES / 2;
        int button_x = (COLS - 20) / 2;
        
        if (selected == 0) {
            attron(COLOR_PAIR(COLOR_SELECTED) | A_BOLD);
        } else {
            attron(COLOR_PAIR(COLOR_BUTTON));
        }
        mvprintw(button_y, button_x, "     [  START  ]     ");
        attroff(COLOR_PAIR(COLOR_SELECTED) | A_BOLD);
        attroff(COLOR_PAIR(COLOR_BUTTON));
        
        if (selected == 1) {
            attron(COLOR_PAIR(COLOR_SELECTED) | A_BOLD);
        } else {
            attron(COLOR_PAIR(COLOR_BUTTON));
        }
        mvprintw(button_y + 3, button_x, "     [  EXIT   ]     ");
        attroff(COLOR_PAIR(COLOR_SELECTED) | A_BOLD);
        attroff(COLOR_PAIR(COLOR_BUTTON));
        
        attron(COLOR_PAIR(COLOR_NORMAL));
        mvprintw(LINES - 3, (COLS - 45) / 2, "Use (v ^) arrows to navigate, ENTER to select");
        attroff(COLOR_PAIR(COLOR_NORMAL));
        
        refresh();
        ch = getch();
        
        if (ch == KEY_UP) selected = 0;
        else if (ch == KEY_DOWN) selected = 1;
        else if (ch == 10) {
            if (selected == 1) {
                endwin();
                system("make killall");
                exit(0);
            }
            return;
        }
    }
}

// SCREEN 2: Input destination
void screen2_input() {
    char lat_str[20] = "";
    char lon_str[20] = "";
    
    clear();
    curs_set(1);
    echo();
    
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(1, (COLS - 35) / 2, "==== SENSOR STATUS & DESTINATION ====");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    display_sensor_boxes(3);
    
    int input_y = 14;
    int input_x = (COLS - 60) / 2;
    
    attron(COLOR_PAIR(COLOR_TITLE));
    mvprintw(input_y, input_x, "+---------------------------------------------------------+");
    mvprintw(input_y + 1, input_x, "|              GPS DESTINATION INPUT                      |");
    mvprintw(input_y + 2, input_x, "+---------------------------------------------------------+");
    attroff(COLOR_PAIR(COLOR_TITLE));
    
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(input_y + 4, input_x + 2, "Enter Destination Latitude : ");
    mvprintw(input_y + 6, input_x + 2, "Enter Destination Longitude: ");
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    move(input_y + 4, input_x + 32);
    refresh();
    getnstr(lat_str, 15);
    
    move(input_y + 6, input_x + 32);
    refresh();
    getnstr(lon_str, 15);
    
    vehicleshm->g.dest.latitude = atof(lat_str);
    vehicleshm->g.dest.longitude = atof(lon_str);
    vehicleshm->g.reached_flag = 0;
    vehicleshm->c.relativeSpeed=0;
    vehicleshm->c.distance=0;
    
    noecho();
    curs_set(0);
    
    attron(COLOR_PAIR(COLOR_SUCCESS) | A_BOLD);
    mvprintw(input_y + 9, input_x + 2, " Destination Set! Starting navigation...");
    attroff(COLOR_PAIR(COLOR_SUCCESS) | A_BOLD);
    refresh();
    sleep(2);
}

// Display 5 sensor boxes
void display_sensor_boxes(int start_y) {
    int box_width = 22;
    int spacing = 3;
    int total_width = 5 * box_width + 4 * spacing;
    int start_x = (COLS - total_width) / 2;
    
    // Box 1: FUEL
    int x = start_x;
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(start_y, x, "+---------------------+");
    mvprintw(start_y + 1, x, "|   FUEL LEVEL        |");
    mvprintw(start_y + 2, x, "|---------------------|");
    mvprintw(start_y + 3, x, "| %.1f%%              |", vehicleshm->v.fuel_level);
    mvprintw(start_y + 4, x, "| Status: %-3s        |", vehicleshm->v.fuel_flag ? "LOW" : "OK");
    mvprintw(start_y + 5, x, "+---------------------+");
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    // Box 2: TYRE
    x += box_width + spacing;
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(start_y, x, "+---------------------+");
    mvprintw(start_y + 1, x, "|  TYRE PRESSURE      |");
    mvprintw(start_y + 2, x, "|---------------------|");
    mvprintw(start_y + 3, x, "| %.1f PSI            |", vehicleshm->v.tyre_pressure);
    mvprintw(start_y + 4, x, "| Status: %-3s        |", vehicleshm->v.pressure_flag ? "LOW" : "OK");
    mvprintw(start_y + 5, x, "+---------------------+");
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    // Box 3: ENVIRONMENT
    x += box_width + spacing;
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(start_y, x, "+---------------------+");
    mvprintw(start_y + 1, x, "|   ENVIRONMENT       |");
    mvprintw(start_y + 2, x, "|---------------------|");
    mvprintw(start_y + 3, x, "| Temp: %.1f C       |", vehicleshm->e.temp);
    mvprintw(start_y + 4, x, "| Rain: %.1f%%       |", vehicleshm->e.rain);
    mvprintw(start_y + 5, x, "+---------------------+");
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    // Box 4: SPEED
    x += box_width + spacing;
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(start_y, x, "+---------------------+");
    mvprintw(start_y + 1, x, "|      SPEED          |");
    mvprintw(start_y + 2, x, "|---------------------|");
    mvprintw(start_y + 3, x, "| %.1f km/h ,Cruise:%d|", vehicleshm->c.relativeSpeed,vehicleshm->c.cruise_flag);
    mvprintw(start_y + 4, x, "| Dist: %.1fm        |", vehicleshm->c.distance);
    mvprintw(start_y + 5, x, "+---------------------+");
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    // Box 5: ENGINE
    x += box_width + spacing;
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(start_y, x, "+---------------------+");
    mvprintw(start_y + 1, x, "|   ENGINE HEALTH     |");
    mvprintw(start_y + 2, x, "|---------------------|");
    mvprintw(start_y + 3, x, "| RPM: %-4d          |", vehicleshm->v.engine_rpm);
    mvprintw(start_y + 4, x, "| Status: %-3s        |", vehicleshm->v.engine_flag ? "BAD" : "OK");
    mvprintw(start_y + 5, x, "+---------------------+");
    attroff(COLOR_PAIR(COLOR_NORMAL));
}

// Display categorized alerts in separate lines
void draw_categorized_alerts(int y, int x, int height, int width) {
    attron(COLOR_PAIR(COLOR_WARNING));
    mvprintw(y, x, "+");
    for (int i = 1; i < width - 1; i++) mvaddch(y, x + i, '-');
    mvaddch(y, x + width - 1, '+');
    
    mvprintw(y + 1, x, "| SYSTEM ALERTS");
    for (int i = 15; i < width - 1; i++) mvaddch(y + 1, x + i, ' ');
    mvaddch(y + 1, x + width - 1, '|');
    
    mvprintw(y + 2, x, "+");
    for (int i = 1; i < width - 1; i++) mvaddch(y + 2, x + i, '-');
    mvaddch(y + 2, x + width - 1, '+');
    
    for (int i = 3; i < height - 1; i++) {
        mvprintw(y + i, x, "|");
        for (int j = 1; j < width - 1; j++) mvaddch(y + i, x + j, ' ');
        mvaddch(y + i, x + width - 1, '|');
    }
    
    mvprintw(y + height - 1, x, "+");
    for (int i = 1; i < width - 1; i++) mvaddch(y + height - 1, x + i, '-');
    mvaddch(y + height - 1, x + width - 1, '+');
    attroff(COLOR_PAIR(COLOR_WARNING));
    
    int line = 3;
    int alert_count = 0;
    
    // VEHICLE HEALTH section
    int vehicle_health_found = 0;
    for (int i = 0; i < MAX_ALERTS && line < height - 2; i++) {
        if (alertshm->alerts[i].active) {
            if (contains_keyword(alertshm->alerts[i].message, "FUEL") ||
                contains_keyword(alertshm->alerts[i].message, "TYRE") ||
                contains_keyword(alertshm->alerts[i].message, "ENGINE")) {
                if (!vehicle_health_found) {
                    attron(COLOR_PAIR(COLOR_TITLE));
                    mvprintw(y + line++, x + 2, "VEHICLE HEALTH:");
                    attroff(COLOR_PAIR(COLOR_TITLE));
                    vehicle_health_found = 1;
                }
                int color = (alertshm->alerts[i].severity == CRITICAL) ? COLOR_CRITICAL : COLOR_WARNING;
                attron(COLOR_PAIR(color) | A_BOLD);
                mvprintw(y + line++, x + 4, "%s", alertshm->alerts[i].message);
                attroff(COLOR_PAIR(color) | A_BOLD);
                alert_count++;
            }
        }
    }
    
    // SAFETY section
    int safety_found = 0;
    for (int i = 0; i < MAX_ALERTS && line < height - 2; i++) {
        if (alertshm->alerts[i].active) {
            if (contains_keyword(alertshm->alerts[i].message, "COLLISION") ||
                contains_keyword(alertshm->alerts[i].message, "BRAKE") ||
                contains_keyword(alertshm->alerts[i].message, "LANE")) {
                if (!safety_found) {
                    attron(COLOR_PAIR(COLOR_TITLE));
                    mvprintw(y + line++, x + 2, "SAFETY:");
                    attroff(COLOR_PAIR(COLOR_TITLE));
                    safety_found = 1;
                }
                int color = (alertshm->alerts[i].severity == CRITICAL) ? COLOR_CRITICAL : COLOR_WARNING;
                attron(COLOR_PAIR(color) | A_BOLD);
                mvprintw(y + line++, x + 4, "%s", alertshm->alerts[i].message);
                attroff(COLOR_PAIR(color) | A_BOLD);
                alert_count++;
            }
        }
    }
    
    // DRIVER section
    int driver_found = 0;
    for (int i = 0; i < MAX_ALERTS && line < height - 2; i++) {
        if (alertshm->alerts[i].active) {
            if (contains_keyword(alertshm->alerts[i].message, "DROWSINESS") ||
                contains_keyword(alertshm->alerts[i].message, "BLIND SPOT")) {
                if (!driver_found) {
                    attron(COLOR_PAIR(COLOR_TITLE));
                    mvprintw(y + line++, x + 2, "DRIVER:");
                    attroff(COLOR_PAIR(COLOR_TITLE));
                    driver_found = 1;
                }
                int color = (alertshm->alerts[i].severity == CRITICAL) ? COLOR_CRITICAL : COLOR_WARNING;
                attron(COLOR_PAIR(color) | A_BOLD);
                mvprintw(y + line++, x + 4, "%s", alertshm->alerts[i].message);
                attroff(COLOR_PAIR(color) | A_BOLD);
                alert_count++;
            }
        }
    }
    
    // ENVIRONMENT section
    int env_found = 0;
    for (int i = 0; i < MAX_ALERTS && line < height - 2; i++) {
        if (alertshm->alerts[i].active) {
            if (contains_keyword(alertshm->alerts[i].message, "RAIN") ||
                contains_keyword(alertshm->alerts[i].message, "FOG") ||
                contains_keyword(alertshm->alerts[i].message, "Temperature")) {
                if (!env_found) {
                    attron(COLOR_PAIR(COLOR_TITLE));
                    mvprintw(y + line++, x + 2, "ENVIRONMENT:");
                    attroff(COLOR_PAIR(COLOR_TITLE));
                    env_found = 1;
                }
                int color = (alertshm->alerts[i].severity == CRITICAL) ? COLOR_CRITICAL : COLOR_WARNING;
                attron(COLOR_PAIR(color) | A_BOLD);
                mvprintw(y + line++, x + 4, "%s", alertshm->alerts[i].message);
                attroff(COLOR_PAIR(color) | A_BOLD);
                alert_count++;
            }
        }
    }
    
    if (alert_count == 0) {
        attron(COLOR_PAIR(COLOR_SUCCESS));
        mvprintw(y + line, x + 2, " All systems normal");
        attroff(COLOR_PAIR(COLOR_SUCCESS));
    }
}

// Display GPS box with GPS-specific alerts
void draw_gps_box_with_alerts(int y, int x) {
    attron(COLOR_PAIR(COLOR_TITLE));
    mvprintw(y, x, "+----------------------------------------------+");
    mvprintw(y + 1, x, "|         GPS NAVIGATION                       |");
    mvprintw(y + 2, x, "+----------------------------------------------+");
    attroff(COLOR_PAIR(COLOR_TITLE));
    
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(y + 3, x, "| Current: %.2f, %.2f                   |", 
             vehicleshm->g.src.latitude, vehicleshm->g.src.longitude);
    mvprintw(y + 4, x, "| Destination: %.2f, %.2f               |", 
             vehicleshm->g.dest.latitude, vehicleshm->g.dest.longitude);
    mvprintw(y + 5, x, "| Distance: %.2f km                         |", vehicleshm->g.dist);
    mvprintw(y + 6, x, "+----------------------------------------------+");
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    // Display GPS alerts below the box
    int gps_line = y + 7;
    for (int i = 0; i < MAX_ALERTS; i++) {
        if (alertshm->alerts[i].active) {
            if (contains_keyword(alertshm->alerts[i].message, "Approaching") ||
                contains_keyword(alertshm->alerts[i].message, "DESTINATION")) {
                attron(COLOR_PAIR(COLOR_SUCCESS) | A_BOLD);
                mvprintw(gps_line++, x, "%s", alertshm->alerts[i].message);
                attroff(COLOR_PAIR(COLOR_SUCCESS) | A_BOLD);
            }
        }
    }
}

// SCREEN 3: Main Dashboard
void screen3_main_dashboard() {
    nodelay(stdscr, TRUE);
    int ch;
    
    while (1) {
        clear();
        
        attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        mvprintw(0, (COLS - 20) / 2, "==== MAIN DASHBOARD ====");
        attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        
        display_sensor_boxes(2);
        draw_categorized_alerts(11, 2, 10, COLS - 52);
        draw_gps_box_with_alerts(11, COLS - 48);
        
        int button_y = 20;
        int button_x = COLS - 48;
        attron(COLOR_PAIR(COLOR_BUTTON));
        mvprintw(button_y, button_x, " [C] Enable Cruise Control ");
        attroff(COLOR_PAIR(COLOR_BUTTON));
        
        attron(COLOR_PAIR(COLOR_NORMAL));
        mvprintw(LINES - 1, 2, "Press 'C' for Cruise Control");
        attroff(COLOR_PAIR(COLOR_NORMAL));
        
        refresh();
        
        if (vehicleshm->g.dist <= 0.5) {
            vehicleshm->p.parking_flag = 0;
            system("./parking_run.sh");
            screen4_parking();
            nodelay(stdscr, FALSE);
            return;
        }
        
        ch = getch();
        if (ch == 'c' || ch == 'C') {
            kill(collision_pid, SIGUSR1);
        }
        
        sleep(1);
    }
}

// Display only parking alerts
void draw_parking_alerts(int y, int x, int height, int width) {
    attron(COLOR_PAIR(COLOR_WARNING));
    mvprintw(y, x, "+");
    for (int i = 1; i < width - 1; i++) mvaddch(y, x + i, '-');
    mvaddch(y, x + width - 1, '+');
    
    mvprintw(y + 1, x, "| PARKING INSTRUCTIONS");
    for (int i = 22; i < width - 1; i++) mvaddch(y + 1, x + i, ' ');
    mvaddch(y + 1, x + width - 1, '|');
    
    mvprintw(y + 2, x, "+");
    for (int i = 1; i < width - 1; i++) mvaddch(y + 2, x + i, '-');
    mvaddch(y + 2, x + width - 1, '+');
    
    for (int i = 3; i < height - 1; i++) {
        mvprintw(y + i, x, "|");
        for (int j = 1; j < width - 1; j++) mvaddch(y + i, x + j, ' ');
        mvaddch(y + i, x + width - 1, '|');
    }
    
    mvprintw(y + height - 1, x, "+");
    for (int i = 1; i < width - 1; i++) mvaddch(y + height - 1, x + i, '-');
    mvaddch(y + height - 1, x + width - 1, '+');
    attroff(COLOR_PAIR(COLOR_WARNING));
    
    // Display ONLY parking alerts
    int line = 3;
    for (int i = 0; i < MAX_ALERTS && line < height - 1; i++) {
        if (alertshm->alerts[i].active) {
            if (contains_keyword(alertshm->alerts[i].message, "STOP") ||
                contains_keyword(alertshm->alerts[i].message, "Caution") ||
                contains_keyword(alertshm->alerts[i].message, "Warning") ||
                contains_keyword(alertshm->alerts[i].message, "Safe") ||
                contains_keyword(alertshm->alerts[i].message, "PARKED")) {
                int color = (alertshm->alerts[i].severity == CRITICAL) ? COLOR_CRITICAL : 
                           (alertshm->alerts[i].severity == WARNING) ? COLOR_WARNING : COLOR_SUCCESS;
                attron(COLOR_PAIR(color) | A_BOLD);
                mvprintw(y + line++, x + 2, "%s", alertshm->alerts[i].message);
                attroff(COLOR_PAIR(color) | A_BOLD);
            }
        }
    }
}

// SCREEN 4: Parking
void screen4_parking() {
    nodelay(stdscr, TRUE);
    
    while (vehicleshm->p.parking_flag == 0) {
        clear();
        
        attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        mvprintw(0, (COLS - 25) / 2, "==== PARKING ASSIST ====");
        attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        
        int viz_y = 5;
        int viz_x = (COLS - 70) / 2;
        
        attron(COLOR_PAIR(COLOR_NORMAL));
        mvprintw(viz_y, viz_x, "+--------------------------------------------------------------------+");
        mvprintw(viz_y + 1, viz_x, "|                     PARKING SENSORS                                |");
        mvprintw(viz_y + 2, viz_x, "|--------------------------------------------------------------------|");
        mvprintw(viz_y + 3, viz_x, "|  Front: %.1f cm  |  Rear: %.1f cm                               |", 
                 vehicleshm->p.front_distance, vehicleshm->p.rear_distance);
        mvprintw(viz_y + 4, viz_x, "+--------------------------------------------------------------------+");
        attroff(COLOR_PAIR(COLOR_NORMAL));
        
        int alert_y = viz_y + 7;
        draw_parking_alerts(alert_y, viz_x, 8, 70);
        
        refresh();
        sleep(1);
    }
    
    clear();
    attron(COLOR_PAIR(COLOR_SUCCESS) | A_BOLD);
    mvprintw(LINES / 2, (COLS - 40) / 2, " VEHICLE PARKED SUCCESSFULLY!");
    mvprintw(LINES / 2 + 2, (COLS - 40) / 2, "Press any key to return...");
    attroff(COLOR_PAIR(COLOR_SUCCESS) | A_BOLD);
    refresh();
    nodelay(stdscr, FALSE);
    getch();
}

