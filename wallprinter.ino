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
unsigned int value;
char *pch;

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
        sprintf(output, "Error in input line, expected 5 args got: %d", i);
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
    if (Serial3.available() > 0);
    {
            sprintf(output, "%s\n", parsed_values[0]); 
            Serial3.write(output);
    }
}

boolean gbrl_status()
{
    for ( ;; )
    {
        GBRLdata = Serial3.readString();
        if (DEBUG_MODE)
        {
            Serial.print("GBRL output: ");
            Serial.println(GBRLdata);
        }

        if ( GBRLdata.startsWith("<Idle,") )
        {
            Serial.write("ready\n");
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
        i = Serial.readBytes(input, sizeof(input)-1);
        input[i] = '\0';

        // sprintf(output,"Input data: %s(%d)", input, result);
        // Serial.println(output);
        // Serial.println(GBRL_MODE);
        // Serial.println(DEBUG_MODE);


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
            Serial3.write(input);
            if ( ! GBRL_MODE )
            {
                gbrl_status();
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
                Serial3.write(input);
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
                    Serial.println("Do processing");
#ifdef GBRL_DEVICE
                    gbrl_cmd();
#else
                    Serial.println("Gbrl command: simulation mode");
                    gbrl_simulation=true;
#endif
                    paint();
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
        else
        {
                gbrl_status();
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
