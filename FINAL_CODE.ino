//all the modes done except the auto mode along with the wifi part___wav to rgb function, time in min functions are added___complete manual input output and logic is done
#include <Keypad.h>
#include <TimeLib.h> 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define REDPIN 14        //GPIO pins on which RGB are connected
#define GREENPIN 12
#define BLUEPIN 13
#define D 5               //delay for rgb output
#define OLED_RESET     -1 // Reset pin # (-1 since sharing Node MCU reset pin)
#define SCREEN_ADDRESS 0x3c //Address of i2c display

const byte ROWS = 4;
const byte COLS = 3;
int flag;                   //flag for user userinput function
char Time[5];               //user time input
int Min;                    //time in min from the midnight
byte data_count = 0;        //index for Time[] array
char key;                   //main keypad input variable for selecting modes
int manual_hr, manual_min;
int rgb_manual[3];          //RGB values for manual output
double colour_wav_slope, colour_wav, colour_wav_manual;
byte rowPins[ROWS] = {10, 9, 16, 0};        //pins on which keypad is connected
byte colPins[COLS] = {2, 15, 3}; 
char keys[ROWS][COLS] = 
{
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );   //Object of keypad

int current_time() /* returns current time in terms of minutes*/
{
  return hour()*60 + minute();
}


void Wav_to_RGB (double wavelength, int arr[3])                            /*Takes wavelength, pointer to an array. Converts wavelength to RGB values and edits array to that. Source : Prof Dan Bruton, http://www.physics.sfasu.edu/astro/color/spectra.html */ 
{                                                                          /* Original code written in FORTRAN, last updated 20Feb 1996*/
    double gamma = 0.8;                                                    /*Input wavelength b/w 380 and 750nm, visible spectrum of light*/
    double R, G, B, attenuation;
    
    if (wavelength >= 380 && wavelength <= 440) 
    {
        attenuation = 0.3 + 0.7 * (wavelength - 380) / 60.0;
        R = pow(((-(wavelength - 440) / 60.0 ) * attenuation), gamma);
        G = 0.0;
        B =pow((1.0 * attenuation), gamma);
    }
    else if (wavelength >= 440 && wavelength <= 490)
    {
        R = 0.0;
        G = pow(((wavelength - 440) / 50.0 ), gamma);
        B = 1.0;
    }
    else if (wavelength >= 490 && wavelength <= 510)
    {
        R = 0.0;
        G = 1.0;
        B = pow((-(wavelength - 510) / 20.0 ), gamma);
    }
    else if (wavelength >= 510 && wavelength <= 580)
    {
        R = pow(((wavelength - 510) / 70.0 ), gamma);
        G = 1.0;
        B = 0.0;
    }
    else if (wavelength >= 580 && wavelength <= 645)
    {
        R = 1.0;
        G = pow((-(wavelength - 645) / 65.0 ), gamma);
        B = 0.0;
    }
    else if (wavelength >= 645 && wavelength <= 750)
    {
        attenuation = 0.3 + 0.7 * (750 - wavelength) / 105.0 ;
        R = pow((1.0 * attenuation), gamma);
        G = 0.0;
        B = 0.0;
    }
    else
    {
        R = 0.0;
        G = 0.0;
        B = 0.0;
    }
    
    int R1 = R * 255; /*Gives RGB values in range of 0-255*/
    if ( R1 > 255)  /*Checks if RGB value is greater than 255*/
    R1 = 255;
    int G1 = G * 255;
    if ( G1 > 255)
    G1 = 255;
    int b1 = B * 255; /*B1 is some pre-existing arduino object, used b1 instead*/
    if ( b1 > 255)
    b1 = 255;
    arr[0]=R1; arr[1]=G1; arr[2]=b1;
    //delay(1000);
}

void modesInterface_TextStyle()   //sets text style to size 1 and clears the existing display output along with setting cursor at (0,0)
{
    display.clearDisplay();
    display.display();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        
    display.setCursor(0,0);
}

void modesInterface()  /*displays the main modes interface, won't work without modesInterface_TextStyle both functions are seprate since it allows to print any other message than main interface*/
{
    display.setCursor(10,10);
    display.println(F("# to turnoff"));       
    display.setCursor(10,20);             
    display.println(F("1. Auto"));           
    display.setCursor(10,30);             
    display.println(F("4. Manual"));           
    display.setCursor(10,40);             
    display.println(F("7. Sleep"));           
    display.setCursor(10,50);             
    display.println(F("*. Work"));           
    display.display();
    display.display();
}

int charTOint(char c)  //converts the char time input from user stored in Time[] array to int form
{
    int integer = 0;
    integer = c - '0';
    return integer;
}  

void entervalidtime()  //displays the error message when invalid time input is given inside the manual input interface
{
    display.setTextSize(1);             
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20,50);
    display.println(F("Enter Valid Time"));
    display.display();
    delay(1000);
    data_count = 0;
    usertimeinput();
}

void usertimeinput()
{        
    int a, b, c, d;                       
    char keytime = '\0'; 
    for (data_count=0; data_count <= 3; data_count++)   //loop to get time input from user
    {
        while (keytime == '\0')    //won't proceed ahead unless a input from keypad is given within 4 second for each input, if while was not present user won't get any time to give input
        {
            keytime = keypad.getKey();
        }

        Time[data_count] = keytime;   //stores time input in array

        if(Time[0]> '2' || Time[data_count] == '*' || Time[2]> '5')   //conditions for invalid input
        {
           entervalidtime();
        }

        a = charTOint(Time[0]);         //converts the char input from the keypad to int form
        b = charTOint(Time[1]);
        c = charTOint(Time[2]);
        d = charTOint(Time[3]);
        manual_hr = a*10+b;
        manual_min = c*10+d;

        display.setTextSize(2);             // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(data_count*20+20,27);    //cursor position according to index of array
        display.print(Time[data_count]);
        display.setCursor(50,27);
        display.println(':');
        display.display();
        display.setTextSize(1);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);  
        display.setCursor(0,10);             
        display.println(F("# to re-enter"));
        display.display(); 
    
        keytime = '\0';      //again set to null since it will allow user to give next digit input in the next loop
    }
        
    delay(500);              //if complete input is given by user displays time set message
    display.clearDisplay();
    modesInterface_TextStyle();
    modesInterface();
    display.setCursor(20,0);
    display.println(F("Time set!"));
    display.display();
    data_count=0;              //if user wants to input values again the Time array index is reset her
    display.clearDisplay();
    
    setTime(manual_hr, manual_min, 0, 0, 0, 0);    //sets clock according to input time     
    Min = current_time();                          //time in minutes
    
    if ( Min <= 780 && Min >= 360 )                //gives wavelength according to the time of day the sunrise(6am) and sunset(7pm) time for manual input are perdefined
    {
        colour_wav_manual = 750 - 0.7971*(Min-360);
    }
    else if ( Min >= 780 && Min <= 1140 )
    {
        colour_wav_manual = 750 - 0.7971*(1140-Min);
    }
    else
    {
        colour_wav_manual = 750;
    }

    Wav_to_RGB(colour_wav_manual, rgb_manual);   //calls the wav to rgb function to get rgb data according to the wavelength
    String s = "Manual";
    modergb(s);                                  //give required output on the display and to the rgb pins
    flag = 1;
}

void automode()
{
    modesInterface_TextStyle();
    display.println(F("Selected mode Auto"));        
    modesInterface();
}


void modergb(String c)               //displays the interface in a particular mode along with giving appropriate analog output for RGB input string is the name of mode
{
    int r,g,b;                       //RGB output values
    modesInterface_TextStyle();
    display.print(F("Current mode:")); 
    display.println(c);
    modesInterface();
    if(c == "Sleep")
    {
          r = 255, g = 112, b = 0;
    }
     if(c == "Work")
    {
          r = 255, g = 244, b = 248;
    }
     if(c == "Manual")
    {
          r = rgb_manual[0], g = rgb_manual[1], b = rgb_manual[2];
    }
     if(c == "Turnoff")
    {
          r = 0, g = 0, b = 0;       
    }
    analogWrite(REDPIN, r);
    analogWrite(GREENPIN, g);
    analogWrite(BLUEPIN, b);
    delay(D);
}

void setup() 
{
    pinMode(REDPIN, OUTPUT);
    pinMode(GREENPIN, OUTPUT);
    pinMode(BLUEPIN, OUTPUT);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c))    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }

    modesInterface_TextStyle();               //main screen
    display.setCursor(0,0);                   //coordinates of point from which printing on display starts total pixels -> (128,64)    
    display.println(F("DIY TEAM 7"));         //prints any variable or text
    display.display();                        //prints all the things in buffer
    modesInterface(); 
}

void loop()
{

    Serial.begin(9600);
    key = keypad.getKey();        //takes input from keypad
    
    if (key)                      //won't enter the if unless a key is pressed
    {
        if (key == '7')
        {
            String s = "Sleep";
            modergb(s);
        }
        if (key == '*')
        {
            String s = "Work";
            modergb(s);
        }
        if (key == '1')
        {
            automode();
        }
        if (key == '4')
        {
            modesInterface_TextStyle();        
            display.setCursor(0,0);
            display.println(F("Enter time in HH : MM"));
            display.display();
            usertimeinput();
            while(flag == 1);           //won't proceed unless complete time input is give manually by user or invalid input is given
        }        
    }
}
