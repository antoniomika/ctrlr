// This #include statement was automatically added by the Particle IDE.
#include <IRremote.h>

int sendPin = A5;
int recvPin = D0;

IRrecv irrecv(recvPin);
IRsend irsend;

char string[256];
char *array[256];

char string2[256];
char *array2[256];

char charArr[256];

String eventName;
String dataString;
String stringer;
String variable;
String value;
String incomingString;

bool sendCode = false;
int delayAmount = 100;

void myHandler(const char *event, const char *data) {
    eventName = String(event);
    dataString = String(data);

    dataString.toCharArray(string, dataString.length() + 1);

    int i = 0;

    array[i] = strtok(string, ";");

    while(array[i] != NULL) {
        array[++i] = strtok(NULL, ";");
    }

    for (int j = 0; j < (sizeof(array)/sizeof(int)); ++j) {
        stringer = array[j];
        int index = stringer.indexOf(":");

        variable = stringer.substring(0, index);
        value = stringer.substring(index + 1, stringer.length());

        if (value == "executeFunction") {
            sendCode = true;
        }

        if (variable == "delay") {
            delayAmount = value.toInt();
        }

        if (variable == "codes") {
            value.toCharArray(string2, value.length());
            int i2 = 0;

            array2[i2] = strtok(string2, ",");

            while(array2[i2] != NULL) {
                array2[++i2] = strtok(NULL, ",");
            }

            for (int u = 0; u < (sizeof(array2)/sizeof(int)); ++u) {
                incomingString = array2[u];

                incomingString.toCharArray(charArr, incomingString.length() + 1);

                long sendVal = strtoul(charArr, NULL, 0);

                if (incomingString.length() < 8) {
                    irsend.sendSony(sendVal, 12);
                }

                delay(delayAmount);
            }

            irrecv.enableIRIn();
        }
    }

    for (int c = 0; c < 256; ++c) {
        string[c] = NULL;
        *array[c] = NULL;

        string2[c] = NULL;
        *array2[c] = NULL;

        charArr[c] = NULL;
    }
}

void setup() {
    Serial.begin(9600);

    Particle.subscribe("ctrlr", myHandler);

    irrecv.enableIRIn();

    pinMode(sendPin, OUTPUT);
    pinMode(recvPin, INPUT_PULLUP);
}

decode_results results;

void loop() {
    if (irrecv.decode(&results)) {
        Particle.publish("ctrlr", "subEvent:learning;code:" + String(results.value, HEX), 60, PRIVATE);
        irrecv.resume();
    }
}
