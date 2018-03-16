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

int sendCode = 0;
int delayAmount = 50;

void sendData(int type, long sendVal) {
    Serial.println("");
    Serial.println(type, HEX);
    Serial.println(sendVal, HEX);
    Serial.println("");
    switch(type) {
        case JVC:
        case SANYO:
        case MITSUBISHI:
        case UNKNOWN:
            break;
        case RC5:
            irsend.sendRC5(sendVal, 12);
            break;
        case RC6:
            irsend.sendRC6(sendVal, 12);
            break;
        case NEC:
            irsend.sendNEC(sendVal, 12);
            break;
        case SONY:
            irsend.sendSony(sendVal, 12);
            break;
        case PANASONIC:
            irsend.sendPanasonic(12, sendVal);
            break;
        case DISH:
            irsend.sendDISH(sendVal, 12);
            break;
        case SHARP:
            irsend.sendSharp(sendVal, 12);
            break;
    }
}

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

        if (variable == "codeType") {
            sendCode = value.toInt();
        }

        if (variable == "delay") {
            delayAmount = value.toInt();
        }

        if (variable == "codes") {
            value.toCharArray(string2, value.length() + 1);
            int i2 = 0;

            array2[i2] = strtok(string2, ",");

            while(array2[i2] != NULL) {
                array2[++i2] = strtok(NULL, ",");
            }

            for (int u = 0; u < (sizeof(array2)/sizeof(int)); ++u) {
                incomingString = array2[u];

                incomingString.toCharArray(charArr, incomingString.length() + 1);

                long sendVal = strtoul(charArr, NULL, 0);

                if (incomingString.length() < 8 && sendVal != 0) {
                    sendData(sendCode, sendVal);
                    delay(delayAmount);
                }
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
        Particle.publish("ctrlr", "subEvent:learning;codeType:" + String(results.decode_type) + ";code:" + String(results.value, HEX), 60, PRIVATE);
        irrecv.resume();
    }
}
