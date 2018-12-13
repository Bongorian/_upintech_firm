#include "SPI.h"
#include "Adafruit_GFX_AS.h"      // Core graphics library, with extra fonts.
#include "Adafruit_ILI9341_STM.h" // STM32 DMA Hardware-specific library
#include <USBComposite.h>

char *manufacture = "bongorian";
char *product = "upintech";

#define TFT_CS PB4
#define TFT_DC PB9
#define TFT_RST PB8
#define POT_1_S1 PB12
#define POT_1_S2 PB13
#define POT_1_S3 PB14
#define POT_2_S1 PB15
#define POT_2_S2 PA8
#define POT_2_S3 PA9
#define POT_1_OUT PA0
#define POT_2_OUT PA1
#define RE_1_A PB10
#define RE_1_B PB11
#define RE_2_A PC14
#define RE_2_B PC15
#define RE_1_SW PB6
#define RE_2_SW PB7

#define JOYX PB1
#define JOYY PB0

#define JOY_SW PB5
#define LOADPIN PA15
#define DATAPIN PA4
#define CLOCKPIN PB3

unsigned int curpots[16];
unsigned int oldpots[16];
byte curKeys[40];
byte oldKeys[40];
unsigned int islongpressKeys[40];
unsigned int curjoystick[2];
unsigned int oldjoystick[2];
int encoder0PinANow = LOW;
int encoder0PinALast = LOW;
int encoder1PinANow = LOW;
int encoder1PinALast = LOW;
int mode;
int cursor = 0;
int cur_chou;
int old_chou;
int curoctave;
int oldoctave;
byte midikeyarray[40] = {200, 255, 202, 255, 203, 255, 201, 255,
                         255, 73, 75, 255, 78, 80, 82, 255,
                         72, 74, 76, 77, 79, 81, 83, 84,
                         255, 61, 63, 255, 66, 68, 70, 255,
                         60, 62, 64, 65, 67, 69, 71, 72};

byte midibassarray[40] = {200, 255, 202, 255, 203, 255, 201, 255,
                          43, 44, 45, 46, 47, 48, 49, 50,
                          38, 39, 40, 41, 42, 43, 44, 45,
                          33, 34, 35, 36, 37, 38, 39, 40,
                          28, 29, 30, 31, 32, 33, 34, 35};

Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST);
USBMIDI midi;
USBHID HID;
HIDKeyboard Keyboard(HID);

void setup()
{
    disableDebugPorts();
    setPins();
    USBComposite.setProductId(0x0075);
    USBComposite.setManufacturerString(manufacture);
    USBComposite.setProductString(product);
    tft.begin();
    tft.setRotation(1);
}

void loop(void)
{
    switch (mode)
    {
    case 0:
        HID.begin(HID_KEYBOARD);
        Keyboard.begin();
        mode0();
        HID.end();
        break;
    case 1:
        midi.begin();
        mode1();
        break;
    case 2:
        midi.begin();
        mode2();
        break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        mode = 1;
        break;
    }
}

void Setlogo()
{
}

void mode0() //mode USB HID
{

    char *title = "MODE0_USB_KEYBOARD";
    setTitle(32, 0, ILI9341_BLACK, ILI9341_WHITE, 2, title);
    while (mode == 0)
    {
        readKeys();
        readJoystick();
        setKeys();
        viewKeyboardinfos();
        readEnc1();
    }
}

void mode1()
{
    char *title = "MODE1_MIDI_KEYBOARD";
    setTitle(32, 0, ILI9341_RED, ILI9341_WHITE, 2, title);
    while (mode == 1)
    {
        readKeys();
        readJoystick();
        checkPots();
        setNote_Piano(curoctave);
        viewMidiPianoinfos();
        readEnc1();
    }
    AlloldNoteOff(curoctave, cur_chou);
}

void mode2()
{
    char *title = "MODE2_MIDI_BASS";
    setTitle(32, 0, ILI9341_GREEN, ILI9341_WHITE, 2, title);
    while (mode == 2)
    {
        readKeys();
        readJoystick();
        checkPots();
        setNote_Bass(curoctave);
        viewMidiBassinfos();
        readEnc1();
    }
    AlloldNoteOff(curoctave, cur_chou);
}

void setTitle(int x, int y, uint16_t backgroundcolor, uint16_t textcolor, uint8_t textsize, char *title)
{
    tft.fillScreen(backgroundcolor);
    tft.setCursor(x, y);
    tft.setTextColor(textcolor);
    tft.setTextSize(textsize);
    tft.println(title);
}

void setKeys()
{
    for (int i = 0; i < 40; i++)
    {
        if ((curKeys[i] == 1) && (islongpressKeys[i] == 0))
        {
            Keyboard.println("bongorian");
        }
        else if ((curKeys[i] == 1) && (islongpressKeys[i] != 0))
        {
        }
        else
        {
        }
    }
}

void viewKeyboardinfos()
{
}

void viewMidiPianoinfos()
{
    viewPodinfo(ILI9341_GREEN, ILI9341_WHITE, ILI9341_RED);
    tft.setCursor(32, 160);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(2);
    tft.print("OCTAVE:");
    tft.print(curoctave);
    tft.setCursor(32, 184);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);
    tft.print("SHIFT:");
    tft.print(cur_chou);
    tft.setCursor(32, 208);
    tft.setTextColor(ILI9341_BLUE);
    tft.setTextSize(2);
    tft.print("ANALOG:");
    tft.print("x:");
    tft.print(curjoystick[0]);
    tft.print("y:");
    tft.print(curjoystick[1]);
}

void viewMidiBassinfos()
{
    viewPodinfo(ILI9341_RED, ILI9341_WHITE, ILI9341_GREEN);
    tft.setCursor(32, 160);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print("OCTAVE:");
    tft.print(curoctave);
    tft.setCursor(32, 184);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);
    tft.print("SHIFT:");
    tft.print(cur_chou);
    tft.setCursor(32, 208);
    tft.setTextColor(ILI9341_BLUE);
    tft.setTextSize(2);
    tft.print("ANALOG:");
    tft.print("x:");
    tft.print(curjoystick[0]);
    tft.print("y:");
    tft.print(curjoystick[1]);
}

void viewPodinfo(uint16_t textcol1, uint16_t textcol2, uint16_t background)
{
    tft.fillRect(0, 16, 320, 224, background);
    tft.setTextSize(1);
    for (int i = 0; i < 16; i++)
    {
        if (i < 8)
        {
            tft.setCursor(((i + 1) * 32), 32);
            tft.setTextColor(textcol1);
            tft.print("P");
            tft.print(i + 1);
            tft.setCursor(((i + 1) * 32), 64);
            tft.setTextColor(textcol2);
            tft.print(curpots[i]);
        }
        else
        {
            tft.setCursor(((i - 7) * 32), 96);
            tft.setTextColor(textcol1);
            tft.print("P");
            tft.print(i + 1);
            tft.setCursor(((i - 7) * 32), 128);
            tft.setTextColor(textcol2);
            tft.print(curpots[i]);
        }
    }
}

void setNote_Piano(int octave)
{
    int shift = octave * 12;
    isPianoactive(shift + cur_chou);
}

void setNote_Bass(int octave)
{
    int shift = octave * 12;
    isBassactive(shift + cur_chou);
}

void isPianoactive(int shift)
{
    for (int i = 0; i < 40; i++)
    {
        if ((curKeys[i] == 1) && (islongpressKeys[i] == 0))
        {
            if (midikeyarray[i] == 200)
            {
                oldoctave = curoctave;
                curoctave++;
            }
            else if (midikeyarray[i] == 201)
            {
                oldoctave = curoctave;
                curoctave--;
            }
            if (midikeyarray[i] == 202)
            {
                old_chou = cur_chou;
                cur_chou++;
            }
            else if (midikeyarray[i] == 203)
            {
                old_chou = cur_chou;
                cur_chou--;
            }
            else if (midikeyarray[i] < 128)
            {
                midi.sendNoteOn(0, midikeyarray[i] + shift, 127);
            }
        }
        else if ((curKeys[i] == 1) && (islongpressKeys[i] != 0))
        {
        }
        else
        {
            if (midikeyarray[i] < 128)
            {
                if ((i == 16) || (i == 39))
                {
                    if ((curKeys[16] == 0) && (curKeys[39] == 0))
                    {
                        midi.sendNoteOff(0, midikeyarray[i] + shift, 127);
                    }
                }
                else
                {
                    midi.sendNoteOff(0, midikeyarray[i] + shift, 127);
                }
            }
        }
    }
}

void isBassactive(int shift)
{
    for (int i = 0; i < 40; i++)
    {
        if ((curKeys[i] == 1) && (islongpressKeys[i] == 0))
        {
            if (midibassarray[i] == 200)
            {
                oldoctave = curoctave;
                curoctave++;
            }
            else if (midibassarray[i] == 201)
            {
                oldoctave = curoctave;
                curoctave--;
            }
            if (midibassarray[i] == 202)
            {
                old_chou = cur_chou;
                cur_chou++;
            }
            else if (midibassarray[i] == 203)
            {
                old_chou = cur_chou;
                cur_chou--;
            }
            else if (midibassarray[i] < 128)
            {
                midi.sendNoteOn(0, midibassarray[i] + shift, 127);
            }
        }
        else if ((curKeys[i] == 1) && (islongpressKeys[i] != 0))
        {
        }
        else
        {
            if (midibassarray[i] < 128)
            {
                if (i > 20)
                {
                    if ((curKeys[i] == 0) && (curKeys[i - 13] == 0))
                    {
                        midi.sendNoteOff(0, midibassarray[i] + shift, 127);
                    }
                }
                else
                {
                    midi.sendNoteOff(0, midibassarray[i] + shift, 127);
                }
            }
        }
    }
}

bool octaveChange()
{
    if (oldoctave == curoctave)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void AlloldNoteOff(int octave, int chou)
{
    int oldshift = octave * 12;
    for (int i = 0; i < 40; i++)
    {
        if (midikeyarray[i] < 128)
        {
            midi.sendNoteOn(0, midikeyarray[i] + oldshift + chou, 0);
        }
    }
}

void setPins()
{
    pinMode(POT_1_S1, OUTPUT);
    pinMode(POT_1_S2, OUTPUT);
    pinMode(POT_1_S3, OUTPUT);
    pinMode(POT_2_S1, OUTPUT);
    pinMode(POT_2_S2, OUTPUT);
    pinMode(POT_2_S3, OUTPUT);
    pinMode(POT_1_OUT, INPUT);
    pinMode(POT_2_OUT, INPUT);
    pinMode(RE_1_A, INPUT);
    pinMode(RE_1_B, INPUT);
    pinMode(RE_2_A, INPUT);
    pinMode(RE_2_B, INPUT);
    pinMode(JOYX, INPUT_PULLUP);
    pinMode(JOYY, INPUT_PULLUP);
    pinMode(JOY_SW, INPUT_PULLUP);
    pinMode(RE_1_SW, INPUT_PULLUP);
    pinMode(RE_2_SW, INPUT_PULLUP);
    pinMode(LOADPIN, OUTPUT);
    pinMode(CLOCKPIN, OUTPUT);
    pinMode(DATAPIN, INPUT);
    digitalWrite(CLOCKPIN, LOW);
    digitalWrite(LOADPIN, HIGH);
}

void readEnc1()
{
    if (!digitalRead(RE_1_SW))
    {
        encoder0PinANow = digitalRead(RE_1_A);
        if ((encoder0PinALast == LOW) && (encoder0PinANow == HIGH))
        {
            if (digitalRead(RE_1_B) == LOW)
            {
                mode--;
            }
            else
            {
                mode++;
            }
            if (mode == 8)
            {
                mode = 0;
            }
        }
        encoder0PinALast = encoder0PinANow;
    }
}

void readEnc2(byte x)
{
    encoder1PinANow = digitalRead(RE_2_A);
    if ((encoder1PinALast == LOW) && (encoder1PinANow == HIGH))
    {
        if (digitalRead(RE_2_B) == LOW)
        {
            cursor--;
        }
        else
        {
            cursor++;
        }
        if (cursor == x)
        {
            mode = 0;
        }
    }
    encoder1PinALast = encoder1PinANow;
}

void readJoystick()
{
    oldjoystick[0] = curjoystick[0];
    oldjoystick[1] = curjoystick[1];
    curjoystick[0] = analogRead(JOYX);
    curjoystick[1] = analogRead(JOYY);
}

void checkPots()
{
    for (int i = 0; i < 16; i++)
    {
        oldpots[i] = curpots[i];
        curpots[i] = readPots(i) / 32;
    }
}

unsigned int readPots(byte potnumber)
{
    unsigned int pv;
    if (potnumber < 8)
    {
        digitalWrite(POT_1_S1, bitRead(potnumber, 0));
        digitalWrite(POT_1_S2, bitRead(potnumber, 1));
        digitalWrite(POT_1_S3, bitRead(potnumber, 2));
        pv = analogRead(POT_1_OUT);
    }
    else
    {
        digitalWrite(POT_2_S1, bitRead(potnumber, 0));
        digitalWrite(POT_2_S2, bitRead(potnumber, 1));
        digitalWrite(POT_2_S3, bitRead(potnumber, 2));
        pv = analogRead(POT_2_OUT);
    }
    return pv;
}

void readKeys()
{
    digitalWrite(LOADPIN, LOW);
    digitalWrite(LOADPIN, HIGH);
    for (int i = 0; i < 40; i++)
    {
        oldKeys[39 - i] = curKeys[39 - i];
        curKeys[39 - i] = !digitalRead(DATAPIN);
        digitalWrite(CLOCKPIN, HIGH);
        digitalWrite(CLOCKPIN, LOW);
        if ((oldKeys[39 - i] == 1) && (curKeys[39 - i] == 1))
        {
            islongpressKeys[39 - i]++;
        }
        else
        {
            islongpressKeys[39 - i] = 0;
        }
    }
}

void displayKeys()
{
    Serial.print("Pin States:\r\n");

    for (int i = 0; i < 40; i++)
    {
        Serial.print("  Pin-");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(curKeys[i]);
        Serial.print("\r\n");
    }

    Serial.print("\r\n");
}

void displayPots()
{
    Serial.print("pot States:\r\n");

    for (int i = 0; i < 16; i++)
    {
        Serial.print("  Pot-");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(curpots[i]);
        Serial.print("\r\n");
    }

    Serial.print("\r\n");
}
