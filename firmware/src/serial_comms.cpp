#include <Arduino.h>
#include "serial_comms.h"
#include "motors.h"
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

    if (strcmp(cmd, "SET_VEL") == 0)   cmd_set_vel(args);
    else if (strcmp(cmd, "STOP") == 0) cmd_stop();
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

    // Convert v (m/s) and omega (deg/s) to left/right PWM
    float omega_ms = omega * (TRACK_WIDTH_M / 2.0f) / 57.2958f;  // deg/s → m/s contribution
    int16_t left  = constrain((int16_t)((v + omega_ms) * VEL_TO_PWM_SCALE), -PWM_MAX, PWM_MAX);
    int16_t right = constrain((int16_t)((v - omega_ms) * VEL_TO_PWM_SCALE), -PWM_MAX, PWM_MAX);
    motors_set(left, right);
}

static void cmd_stop() {
    motors_stop();
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
