#define SERIAL3


static char input[50];
static char output[50];

static boolean DEBUG_MODE = false;
static boolean GBRL_MODE = false;

// digital output pins
//
const int black1_pin = 22;

// parsed input variables
//
static char gcode[10];
static int  cymk[4];

int i;
int result;

#ifdef SERIAL3

String GBRLdata;
byte gbrl_output;

#else

static boolean gbrl_simulation=false;

#endif

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

#ifdef SERIAL3
    Serial3.begin(115200);
    // Serial3.write("?");
#endif

    // set output Pins
    pinMode(black1_pin, OUTPUT);
    digitalWrite(black1_pin, LOW); 
  
}

int parse_line()
{

    //Serial.println(GBRL_MODE);
    //Serial.println(DEBUG_MODE);
    result = sscanf(input,"%[^,],%d,%d,%d,%d", gcode, &cymk[0], &cymk[1], &cymk[2], &cymk[3]);
    //Serial.println(GBRL_MODE);
    //Serial.println(DEBUG_MODE);

    if ( result != 5 ) 
    {
        sprintf(output, "Error in input line, expected 5 args got: %d", result);
        Serial.println(output);
        return 0;
    }
    else 
    {
        if (DEBUG_MODE) 
        {
            sprintf(output, "gcode: %s\n", gcode);
            Serial.println(output);

            Serial.println("color c y m k: ");
            for ( i=0; i<4; i++ )
            {
                
                sprintf(output, "%d ", cymk[i]);
                Serial.print(output);
            }
            Serial.println("");
        }
    }
    return 1;
}

void paint()
{
    for ( i=0; i<4; i++ )
    {
        if (cymk[i] > 0)
        {
            digitalWrite(black1_pin, HIGH); 
            if (DEBUG_MODE)
            {
                sprintf(output, "%d : %d\n", i, cymk[i]);
                Serial.write(output);
            }
            delay(cymk[i]);
            digitalWrite(black1_pin, LOW); 
        }
    }
    
}

#ifdef SERIAL3
void gbrl_cmd()
{
    if (Serial3.available() > 0);
    {
            sprintf(gcode, "%s\n", gcode); 
            Serial3.write(gcode);
    }
}

boolean gbrl_status()
{
    for ( ;; )
    {
        GBRLdata = Serial3.readString();
        GBRLdata = "<Idle";
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
        result = Serial.readBytes(input, sizeof(input)-1);
        input[result] = '\0';

        // sprintf(output,"Input data: %s(%d)", input, result);
        // Serial.println(output);
        //Serial.println(GBRL_MODE);
        //Serial.println(DEBUG_MODE);


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
#ifdef SERIAL3
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
#ifdef SERIAL3
                Serial3.write(input);
#else
                sprintf(output,"Gbrl simulation mode direct write: %s", input);
                Serial.println(output);
                gbrl_simulation=true;
#endif
            }
            else 
            {
                result = parse_line();
                if ( result )
                {
                    Serial.println("Do processing");
#ifdef SERIAL3
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
                    */
                    GBRL_MODE=false;
                    DEBUG_MODE=false;
                    
                }
            }
        }

        // Clear/reset input data
        input[0] = '\0';
    }

}

void serial_gbrl()
{
#ifdef SERIAL3
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
