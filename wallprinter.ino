String GRBLdata;
char input[50];
byte grbl_output;
char output[50];

int DEBUG = 0;
int GBRL_MODE = 0;

// digital output pins
//
const int black1_pin = 22;

// parsed input variables
//
char gcode[10];
int  cymk[4];



int i;
int result;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial3.begin(115200);
    // Serial3.write("?");

    // set output Pins
    pinMode(black1_pin, OUTPUT);
    digitalWrite(black1_pin, LOW); 
  
}

int parse_line()
{

    result = sscanf(input,"%[^,],%d,%d,%d,%d", gcode, &cymk[0], &cymk[1], &cymk[2], &cymk[3]);

    if ( result != 5 ) 
    {
        sprintf(output, "Error in input line, expected 5 args got: %d", result);
        Serial.println(output);
        return 0;
    }
    else 
    {
        if (DEBUG) 
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
    int i;

    for ( i=0; i<4; i++ )
    {
        if (cymk[i] > 0)
        {
            digitalWrite(black1_pin, HIGH); 
            if (DEBUG)
            {
                sprintf(output, "%d : %d", i, cymk[i]);
                Serial.println(output);
            }
            delay(cymk[i]);
            digitalWrite(black1_pin, LOW); 
        }
    }
    
}

boolean grbl_cmd()
{
    int i; 

    if (Serial3.available() > 0);
    {
            sprintf(gcode, "%s\n", gcode); 
            Serial3.write(gcode);
    }
}

boolean grbl_status()
{
    char c;
    
    for ( ;; )
    {
        GRBLdata = Serial3.readString();
        if (DEBUG)
        {
            Serial.print("GBRL output: ");
            Serial.println(GRBLdata);
        }

        if ( GRBLdata.startsWith("<Idle,") )
        {
            Serial.write("ready\n");
            return true;
        }
        delay(10);
    }
}

void serial_monitor()
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
        //Serial.println(DEBUG);


        if ( input[0] == 'd' )
        {
            DEBUG=1;
            Serial.println("DEBUG enabled");
        }
        else if ( input[0] == 'D' )
        {
            DEBUG=0;
            Serial.println("DEBUG disabled");
        }
        else if ( input[0] == '?' )
        {
            Serial3.write(input);
            if ( ! GBRL_MODE )
            {
                grbl_status();
            }
        }
        else if (input[0] == 'l')
        {
            Serial.println("GRBL mode  enabled");
            GBRL_MODE = 1;
        }
        else if (input[0] == 'L')
        {
            Serial.println("GRBL mode  disabled");
            GBRL_MODE = 0;
        }
        else 
        {
            if (DEBUG) 
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
                Serial3.write(input);
            }
            else 
            {
                result = parse_line();
                if ( result )
                {
                    Serial.println("Do processing");
                    grbl_cmd();
                    paint();
                }
                else 
                {
                    GBRL_MODE=0;
                    DEBUG=0;
                    
                }
            }
        }

        // Clear/reset input data
        input[0] = '\0';
    }

}

void serial_gbrl()
{
    if (Serial3.available() > 0) 
    {
        if ( GBRL_MODE )
        {
            grbl_output = Serial3.read();
            Serial.write(grbl_output);
        }
        else
        {
                grbl_status();
        }
    }
}

void loop() 
{
    serial_monitor();
    serial_gbrl();
}
