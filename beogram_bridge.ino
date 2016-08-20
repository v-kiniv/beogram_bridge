#include <IRremote.h>

/* Bridge output -> receiver IR port input.
   As defined in IRremote library, output PWM pin = 3. */
#define RECEIVER_INPUT_PIN TIMER_PWM_PIN
#define IR_RECEIVER_PIN 6
#define LED_PIN 13

//#define DEBUG_MODE // uncomment to print receiverd remote codes to terminal
#define DEBUG_SERIAL_SPEED 9600

#define BEO_SERIAL_SPEED 320
#define BEO_CMD_DELAY 50  // Delay between next command send/receive
#define BEO_TONEARM_DELAY 1500
#define RECEIVER_VOL_TRIM 21
#define RECEIVER_CMD_DELAY 70
#define RECEIVER_CODE_SIZE 32

// Bang&Olufsen Beogram 5000 codes
typedef enum {
  BEO_PLAY = 0x2A,   // or 0x35, resend code X times for X repeats
  BEO_PAUSE = 0x36,  // or ox26
  BEO_STOP = 0x16,
  BEO_TONEARM_UP = 0x46,
  BEO_TONEARM_DOWN = 0x1E,
  BEO_SHUTDOWN = 0x2E,
} beo_code_t;

// Yamaha R-N500 receiver remote codes
typedef enum {
  Y_CODE_REPEAT = 0x00,
  Y_SOURCE_OFF = 0x9E610679,
  Y_SOURCE_PHONO = 0x5EA12857,
  Y_SOURCE_NET = 0xFE80FC83,
  Y_PLAY = 0xFE801669,
  Y_PAUSE = 0xFE80E699,
  Y_STOP = 0xFE8096E9,
  Y_VOL_UP = 0x5EA15827,
  Y_VOL_DOWN = 0x5EA1D8A7,
  Y_DISK_SKIP = 0x9E61F20D,
} nec_code_t;

IRrecv m_ir_recv(IR_RECEIVER_PIN);
IRsend m_ir_send;
bool m_bridge_state = false;
bool m_led_state = false;

void setup() {
#ifdef DEBUG_MODE
  Serial.begin(DEBUG_SERIAL_SPEED);
#else
  Serial.begin(BEO_SERIAL_SPEED, SERIAL_7N1);
#endif

  m_ir_recv.enableIRIn();  // Start receiver

  pinMode(LED_PIN, OUTPUT);
}

void bridge_enabled(bool s) {
  m_bridge_state = s;
  led_enabled(s);
  if (!s) {
    on_beo_shutdown();
  }
}

void on_beo_play() {
  receiver_send(Y_SOURCE_PHONO);

  if (!m_bridge_state) {
    bridge_enabled(true);

    // Increase receiver volume
    for (int i = 0; i < RECEIVER_VOL_TRIM; i++) {
      receiver_send(Y_VOL_UP);
      delay(RECEIVER_CMD_DELAY);
    }
  }
}

void on_beo_shutdown() {
  // Decrease receiver volume
  for (int i = 0; i <= RECEIVER_VOL_TRIM; i++) {
    receiver_send(Y_VOL_DOWN);
    delay(RECEIVER_CMD_DELAY);
  }

  // Play Spotify
  receiver_send(Y_SOURCE_NET);
  delay(RECEIVER_CMD_DELAY);
  receiver_send(Y_PLAY);
}

void beo_send(int code) {
  // Code must be sent twice
  Serial.write(code);
  delay(BEO_CMD_DELAY);
  Serial.write(code);

  blink(200, 3);
}

void beo_receive(int code) {
  static int prev_code = 0;

  // All commands from turntable repeats twice
  if (code != prev_code) {
    prev_code = code;
    delay(BEO_CMD_DELAY);
    return;
  }

  switch (code) {
    case BEO_TONEARM_DOWN:
      on_beo_play();
      break;
    case BEO_SHUTDOWN:
      on_beo_shutdown();
      break;
  }
  prev_code = 0;
}

void receiver_send(long int code) {
  m_ir_send.sendNEC(code, RECEIVER_CODE_SIZE);
}

void remote_receive(decode_results *code) {
#ifdef DEBUG_MODE
  Serial.println(code->value, HEX);
#endif

  switch (code->value) {
    case Y_SOURCE_OFF:
      bridge_enabled(!m_bridge_state);
      if (!m_bridge_state) {
        beo_send(BEO_STOP);
      }
      break;
    case Y_STOP:
      if (m_bridge_state) {
        beo_send(BEO_STOP);
      }
      break;
    case Y_PAUSE:
      if (m_bridge_state) {
        beo_send(BEO_PAUSE);
      }
      break;
    case Y_PLAY:
      if (m_bridge_state) {
        beo_send(BEO_PLAY);
      }
      break;
    case Y_DISK_SKIP:
      if (m_bridge_state) {
        beo_send(BEO_PAUSE);
        delay(BEO_TONEARM_DELAY);  // wait while tonearm end his move
        beo_send(BEO_PLAY);
      }
      break;

    default:
      break;
  }
}

void led_enabled(bool state) {
  digitalWrite(LED_PIN, state);
  m_led_state = state;
}

void blink(int delay_ms, int repeat) {
  for (int i = repeat; i > 0; i--) {
    led_enabled(!m_led_state);
    delay(delay_ms / repeat / 2);
    led_enabled(!m_led_state);
    delay(delay_ms / repeat / 2);
  }
}

void loop() {
  decode_results remote_code;

  while (Serial.available()) {
    beo_receive(Serial.read());
    m_ir_recv.enableIRIn();
  }

  if (m_ir_recv.decode(&remote_code)) {
    remote_receive(&remote_code);

    m_ir_recv.resume();  // Receive next value
    m_ir_recv.enableIRIn();
  }
}
