#include <Arduino.h>
#include "serial_comms.h"
#include "motors.h"
#include "encoders.h"
#include "heading.h"
#include "config/params.h"

#define RX_BUF_SIZE 64

static char     s_buf[RX_BUF_SIZE];
static uint8_t  s_len = 0;

static void handle_line(char *line);
static void cmd_set_vel(char *args);
static void cmd_stop();
static void cmd_set_param(char *args);

void serial_init() {
    Serial.begin(115200);
}

void serial_update() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            s_buf[s_len] = '\0';
            if (s_len > 0) handle_line(s_buf);
            s_len = 0;
        } else if (c != '\r' && s_len < RX_BUF_SIZE - 1) {
            s_buf[s_len++] = c;
        }
    }
}

static void handle_line(char *line) {
    char *space = strchr(line, ' ');
    char *args  = space ? space + 1 : NULL;
    char *cmd   = strtok(line, " ");

    if (!cmd) return;

    if (strcmp(cmd, "SET_VEL") == 0)        cmd_set_vel(args);
    else if (strcmp(cmd, "STOP") == 0)      cmd_stop();
    else if (strcmp(cmd, "SET_PARAM") == 0) cmd_set_param(args);
    else {
        Serial.print(F("ERR UNKNOWN unknown command: "));
        Serial.println(cmd);
    }
}

static void cmd_set_vel(char *args) {
    if (!args) { Serial.println(F("ERR SET_VEL missing arguments")); return; }

    char *token1 = strtok(args, " ");
    char *token2 = token1 ? strtok(NULL, " ") : NULL;
    if (!token1) { Serial.println(F("ERR SET_VEL missing arguments")); return; }

    float v     = atof(token1);
    float omega = token2 ? atof(token2) : 0.0f;

    // Always route through heading module — it holds heading for omega=0 (straight)
    // and tracks a continuously advancing target for omega≠0 (curves), both with IMU feedback.
    heading_set_velocity(v, omega);
}

static void cmd_stop() {
    heading_set_velocity(0.0f, 0.0f);
    Serial.println(F("OK STOP"));
}

static void cmd_set_param(char *args) {
    if (!args) { Serial.println(F("ERR SET_PARAM missing arguments")); return; }

    char *name = strtok(args, " ");
    if (!name) { Serial.println(F("ERR SET_PARAM missing arguments")); return; }
    // Params will be added here as new phases introduce tunable values
    Serial.print(F("ERR SET_PARAM unknown param: "));
    Serial.println(name);
}
