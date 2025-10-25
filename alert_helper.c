#include "headers.h"

// Add alert to shared memory
void add_alert(alert_message* alertshm, const char* msg, Severity sev) {
    // Find first inactive slot or replace oldest
    for (int i = 0; i < MAX_ALERTS; i++) {
        if (!alertshm->alerts[i].active) {
            alertshm->alerts[i].active = 1;
            alertshm->alerts[i].severity = sev;
            strncpy(alertshm->alerts[i].message, msg, MAX_ALERT_MSG - 1);
            alertshm->alerts[i].message[MAX_ALERT_MSG - 1] = '\0';
            return;
        }
    }
    
    // If all slots full, replace first one
    alertshm->alerts[0].active = 1;
    alertshm->alerts[0].severity = sev;
    strncpy(alertshm->alerts[0].message, msg, MAX_ALERT_MSG - 1);
    alertshm->alerts[0].message[MAX_ALERT_MSG - 1] = '\0';
}

// Clear all alerts
void clear_all_alerts(alert_message* alertshm) {
    for (int i = 0; i < MAX_ALERTS; i++) {
        alertshm->alerts[i].active = 0;
        alertshm->alerts[i].message[0] = '\0';
    }
    alertshm->alert_count = 0;
}

// Clear specific alert by message prefix
void clear_alert_by_prefix(alert_message* alertshm, const char* prefix) {
    for (int i = 0; i < MAX_ALERTS; i++) {
        if (alertshm->alerts[i].active && strstr(alertshm->alerts[i].message, prefix) != NULL) {
            alertshm->alerts[i].active = 0;
        }
    }
}
