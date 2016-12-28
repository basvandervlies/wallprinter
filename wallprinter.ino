#define GBRL_DEVICE

/*
 * buffer variables for input/output
*/
static char input[50];
static char output[50];

static boolean DEBUG_MODE = false;
static boolean GBRL_MODE = false;

// digital output pins
//
const int black1_pin = 22;

/*
 * buffer that hold the parsed line
 * 0 ---> gcode
 * 1 ---> c
 * 2 ---> m
 * 3 ---> y
 * 4 ---> k
*/
char *parsed_values[5];

/*
 * variable used in functions
*/
unsigned int i;
unsigned int result;
unsigned int value;
char *pch;
char c;
String InputData;

#ifdef GBRL_DEVICE

String GBRLdata;
byte gbrl_output;

#else

static boolean gbrl_simulation=false;

#endif

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

#ifdef GBRL_DEVICE
    Serial3.begin(115200);
    // Serial3.write("?");
#endif

    // set output Pins
    pinMode(black1_pin, OUTPUT);
    digitalWrite(black1_pin, LOW); 
  
}

int parse_line()
{

    /*
     * Must us strtok. sscanf is buggy, delimiter is ,
    */
    i = 0;
    pch = strtok(input, ",");
    while (pch != NULL)
    { 
        parsed_values[i++] = pch;
        pch = strtok (NULL, ",");
    }


    if ( i != 5 ) 
    {
        sprintf(output, "Error input %s(%d)", input, i);
        Serial.println(output);
        return 0;
    }
    else 
    {
        if (DEBUG_MODE) 
        {
            sprintf(output, "gcode: %s\n", parsed_values[0]);
            Serial.println(output);

            Serial.println("color c m y k: ");
            for ( i=1; i<5; i++ )
            {
                
                sprintf(output, "%s ", parsed_values[i]);
                Serial.print(output);
            }
            Serial.println("");
        }
    }
    return 1;
}

void paint()
{
    for ( i=1; i<5; i++ )
    {
        value = atoi(parsed_values[i]);
        if (value > 0)
        {
            digitalWrite(black1_pin, HIGH); 
            if (DEBUG_MODE)
            {
                sprintf(output, "%d : %d\n", i, value);
                Serial.write(output);
            }
            delay(value);
            digitalWrite(black1_pin, LOW); 
        }
    }
    
}

#ifdef GBRL_DEVICE
void gbrl_cmd()
{
    sprintf(output, "%s\n", parsed_values[0]); 
    Serial3.write(output);

    // Must read the ok from the stack else gbrl_status infinite loop
    /*
    while (Serial3.available() > 0)
    {
        GBRLdata = Serial3.readString();
        if ( DEBUG_MODE )
        {
            Serial.println(GBRLdata);
        }
    }
    */
}

boolean gbrl_status()
{
    delay(50);
    for ( ;; )
    {
        Serial3.write("?\n");
        GBRLdata = Serial3.readString();
        if (DEBUG_MODE || GBRL_MODE)
        {
            Serial.print("GBRL output: ");
            Serial.println(GBRLdata);
            Serial.println("end GBRL output: ");
        }

        if ( GBRLdata.startsWith("<Idle,") )
        {
            return true;
        }
        delay(10);
    }
}
#endif

void serial_computer()
{

    if (Serial.available() > 0) {
    
        // read the incoming byte and convert it char array for easy processign
        //
        // InputData = Serial.readString();
        // InputData.toCharArray(input, 30);
        InputData = Serial.readStringUntil('\n');
        InputData.toCharArray(input, 50);

        if ( input[0] == 'd' )
        {
            DEBUG_MODE = true;
            Serial.println("DEBUG_MODE enabled");
        }
        else if ( input[0] == 'D' )
        {
            DEBUG_MODE = false;
            Serial.println("DEBUG_MODE disabled");
        }
        else if ( input[0] == '?' )
        {
#ifdef GBRL_DEVICE
            gbrl_status();
            if ( ! GBRL_MODE )
            {
                Serial.println("ready");
            }
#else
            Serial.println("gbrl status request: simulation mode");
            gbrl_simulation=true;
#endif
        }
        else if (input[0] == 'l')
        {
            Serial.println("GBRL mode  enabled");
            GBRL_MODE = true;
        }
        else if (input[0] == 'L')
        {
            Serial.println("GBRL mode  disabled");
            GBRL_MODE = false;
        }
        else 
        {
            if (DEBUG_MODE) 
            {
                sprintf(output,"Input data: %s", input);
                Serial.println(output);
                if (GBRL_MODE) 
                {
                    Serial.println("GBRL_MODE");
                }
            }
            if ( GBRL_MODE )
            {
#ifdef GBRL_DEVICE
                if ( DEBUG_MODE )
                {
                    Serial.println("write gbrl device");
                }
                Serial3.write(input);
                Serial3.write("\n");
#else
                sprintf(output,"Gbrl simulation mode direct write: %s", input);
                Serial.println(output);
                gbrl_simulation=true;
#endif
            }
            else 
            {
                i = parse_line();
                if ( i )
                {
                    if ( DEBUG_MODE )
                    {
                        Serial.println("Do processing");
                    }
#ifdef GBRL_DEVICE
                    gbrl_cmd();
                    gbrl_status();
#else
                    Serial.println("Gbrl command: simulation mode");
                    gbrl_simulation=true;
#endif
                    paint();
                    Serial.println("ready");
                }
                else 
                {
                    /*
                     * Bug in de code
                    GBRL_MODE=false;
                    DEBUG_MODE=false;
                    */
                    
                }
            }
        }

        // Clear/reset input data
        input[0] = '\0';
        memset(parsed_values, 0, sizeof(parsed_values));
    }

}

void serial_gbrl()
{
#ifdef GBRL_DEVICE
    if (Serial3.available() > 0) 
    {
        if ( GBRL_MODE )
        {
            gbrl_output = Serial3.read();
            Serial.write(gbrl_output);
        }
    }
#else
    if ( gbrl_simulation)  
    {
        if ( GBRL_MODE )
        {
                Serial.println("\tGbrl_simulation read");
        }
        else
        {
            Serial.println("Gbrl simulation: ready");
        }
        gbrl_simulation = false;
    }
#endif
}

void loop() 
{
    serial_computer();
    serial_gbrl();
}
