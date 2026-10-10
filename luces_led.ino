/* ============================================================================
   CONTROL DE 5 LUCES LED POR USB (Web Serial)  --  pines 2 a 6
   ----------------------------------------------------------------------------
   Se controla desde la pagina luces_led.html (Chrome/Edge) con Web Serial.
   Al conectar, el navegador abre el puerto a 115200 y envia texto por lineas:

     LED:<n>:<0|1>     enciende/apaga un LED  (n = 1..5)
     LEDS:xxxxx        estado de los 5 a la vez, ej. LEDS:10101
     ?                 pide el estado actual

   El Arduino responde (cada cambio y al pedirlo) con una linea JSON:

     {"leds":[0,0,0,0,0]}

   MONTAJE (LED a cada pin -> resistencia 220-330 ohm -> GND)
     LED 1 -> pin 2      LED 2 -> pin 3      LED 3 -> pin 4
     LED 4 -> pin 5      LED 5 -> pin 6
   ========================================================================== */

const uint8_t NUM_LEDS = 5;
const uint8_t PINES[NUM_LEDS] = {2, 3, 4, 5, 6};   // LED 1..5

bool estado[NUM_LEDS] = {false, false, false, false, false};

void aplicaSalidas() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    digitalWrite(PINES[i], estado[i] ? HIGH : LOW);
  }
}

void enviaEstado() {
  Serial.print(F("{\"leds\":["));
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    Serial.print(estado[i] ? 1 : 0);
    if (i < NUM_LEDS - 1) Serial.print(',');
  }
  Serial.println(F("]}"));
}

void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pinMode(PINES[i], OUTPUT);
    digitalWrite(PINES[i], LOW);
  }

  delay(400);
  Serial.println(F("=== Luces LED listas (pines 2-6) ==="));
  Serial.println(F("Comandos: LED:<1-5>:<0|1>  |  LEDS:xxxxx  |  ?"));
  enviaEstado();
}

void ejecutaComando(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;
  cmd.toUpperCase();

  if (cmd == "?") {
    enviaEstado();
    return;
  }

  if (cmd.startsWith("LED:")) {
    int p = cmd.indexOf(':', 4);           // segundo ':'
    if (p < 0) return;
    int n = cmd.substring(4, p).toInt();   // 1..5
    int v = cmd.substring(p + 1).toInt();  // 0 / 1
    if (n >= 1 && n <= NUM_LEDS) {
      estado[n - 1] = (v != 0);
      aplicaSalidas();
      enviaEstado();
    }
    return;
  }

  if (cmd.startsWith("LEDS:")) {
    String datos = cmd.substring(5);
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      if (i < datos.length()) {
        char c = datos.charAt(i);
        estado[i] = (c == '1');
      }
    }
    aplicaSalidas();
    enviaEstado();
    return;
  }
}

void loop() {
  static String buf;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (buf.length()) { ejecutaComando(buf); buf = ""; }
    } else if (buf.length() < 32) {
      buf += c;
    }
  }
}
