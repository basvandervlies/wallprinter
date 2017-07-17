#define GRBL_DEVICE

/*
 * buffer variables for input/output
*/
static char input[50];
static char output[70];

static char grbl_output[70];

static int input_size = 50;

static boolean DEBUG_MODE = false;
static boolean GRBL_MODE = false;
static boolean NEW_DATA = false;

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
 * 5 ---> gbrl_poll_delay_time not used
*/
char *parsed_values[5];

/*
 * variable used in functions
*/
unsigned int i,j;
unsigned int value;
float paint_time;
char *pch;

#ifdef GRBL_DEVICE
String GRBLdata;
#endif


void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

#ifdef GRBL_DEVICE
    Serial3.begin(115200);
    // Serial3.write("?");
#endif

    // set output Pins
    pinMode(black1_pin, OUTPUT);
    digitalWrite(black1_pin, LOW); 
  
}

boolean parse_line()
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
        sprintf(output, "Error input %s(%d<5)", input, i);
        Serial.println(output);
        return false;
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
/*
            sprintf(output, "grbl_poll_delay_time: %s\n", parsed_values[5]);
            Serial.print(output);
            Serial.println("");
*/

        }
    }
    return true;
}

void paint()
{
    for ( i=4; i<5; i++ )
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

#ifdef GRBL_DEVICE

void grbl_cmd()
{
    sprintf(output, "%s\n", parsed_values[0]); 
    Serial3.write(output);

    // time consuming
    //grbl_response();
}

boolean grbl_ready()
{
    
    delay(100);
    value = 100;
    for ( ;; )
    {
        Serial3.write("?\n");
        delay(value);
        value = value * 1.1;

        // grbl_response();
        if ( grbl_status_line() )
        {
            return true;
        }
    }
}

boolean grbl_status_line()
{
    j = 0;
    while (Serial3.available() > 0) 
    {
        i = Serial3.readBytesUntil('\n',grbl_output, sizeof(grbl_output));
        grbl_output[i] = '\0';
        if ( DEBUG_MODE )
        {
            Serial.println(j);
            Serial.println(grbl_output);
        }
        if ( strncmp(grbl_output,"<Idle", 5) == 0 )
        {
            return true;
        }
        j++;
    }
    return false;
}

void grbl_response()
{
    char rc;

    while (Serial3.available() > 0) 
    {
        rc = Serial3.read();
        if ( GRBL_MODE || DEBUG_MODE )
        {
            Serial.write(rc);
        }
    }
}

void grbl_write()
{
    if ( DEBUG_MODE )
    {
        Serial.println("write grbl device");
    }

    Serial3.write(input);
    Serial3.write("\n");

    grbl_response();
}

#else

void grbl_cmd()
{
    sprintf(output, "grbl cmd simulation mode: %s\n", parsed_values[0]); 
    Serial.write(output);
}

boolean grbl_ready()
{
    Serial.println("Grbl status request: simulation mode");
    return true;
}

void grbl_write()
{
    sprintf(output,"Grbl simulation mode direct write: %s", input);
    Serial.println(output);
}

#endif

void wall_printer_one_board()
{
    if ( parse_line() )
    {
        if ( DEBUG_MODE )
        {
            Serial.println("wall_printer_one_board");
        }

        sprintf(output, "%s\n", parsed_values[0]); 
        Serial3.write(output);

        value = atoi(parsed_values[4]);
        paint_time = value / 1000.0;

        strcpy(output, "M3 S1000\n");
        Serial3.write(output);

        sprintf(output, "G4 P%f\n", paint_time );
        if ( DEBUG_MODE )
        {
            Serial.println(output);
        }

        Serial3.write(output);

        strcpy(output, "M3 S0\n");
        Serial3.write(output);
        

        /*
         * Long black values must we give the grbl some time
        */
        if ( value > 201 ) {
            value = value - 200;
            delay(value);
        }

        grbl_ready();

        Serial.println("ready");
    }
}

void wall_printer()
{
    if ( parse_line() )
    {
        if ( DEBUG_MODE )
        {
            Serial.println("Control plotter device");
        }
        
        grbl_cmd();
        grbl_ready();
        paint();
        Serial.println("ready");
    }
}

void wall_printer_grbl_test_mode()
{
    strcpy(input, "G91 G1 F15000 X10,0,0,0,500,100");
    for  ( i=0 ; i < 2; i++ )
    {
        wall_printer_one_board();
    }
}

void wall_printer_test_mode()
{
    strcpy(input, "G91 G1 F15000 X10,0,0,0,1000,100");
    for  ( i=0 ; i < 4; i++ )
    {
        wall_printer();
    }
}

void process_data()
{
    if ( NEW_DATA )
    {
        if (DEBUG_MODE) 
        {
            sprintf(output,"Input data: %s", input);
            Serial.println(output);
        }

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
            grbl_ready();
            if ( ! GRBL_MODE )
            {
                Serial.println("ready");
            }
        }
        else if (input[0] == 'l')
        {
            Serial.println("GRBL mode  enabled");
            GRBL_MODE = true;
        }
        else if (input[0] == 'L')
        {
            Serial.println("GRBL mode  disabled");
            GRBL_MODE = false;
        }
        else if (input[0] == 't')
        {
            Serial.println("wall printer test mode");
            wall_printer_test_mode();
        }
        else if (input[0] == 'T')
        {
            Serial.println("wall printer grbl test mode");
            wall_printer_grbl_test_mode();
        }
        else 
        {
            if ( GRBL_MODE )
            {
                grbl_write();
            }
            else 
            {
                //wall_printer();
                wall_printer_one_board();
            }
        }

        // Clear/reset input data
        memset(parsed_values, 0, sizeof(parsed_values));
        NEW_DATA = false;
    }
}

void receive_data()
{
    char endMarker = '\n';
    char rc;
    static int index = 0;

    while ( (Serial.available() > 0) && (NEW_DATA == false) ) 
    {
        rc = Serial.read(); 
        if (rc != endMarker) 
        {
            input[index] = rc;
            index++;
            if (index >= input_size) 
            {
               index = input_size - 1;
            }
        }
        else 
        {
            input[index] = '\0'; // terminate the string
            index = 0;
            NEW_DATA = true;
        }
    }
}

void loop() 
{
    receive_data();
    process_data();
}
