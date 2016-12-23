String GRBLdata;
char input[50];
byte grbl_output;
char output[50];

boolean DEBUG = false;
boolean RAW_MODE = false;

// digital output pins
//
const int black1_pin = 22;

// parsed input variables
//
char gcode[10];
int  cymk[4];


void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial3.begin(115200);
    // Serial3.write("?");

    // set output Pins
    pinMode(black1_pin, OUTPUT);
    digitalWrite(black1_pin, LOW); 
  
}

boolean parse_line()
{
    int result;
    int i;

    result = sscanf(input,"%[^,],%d,%d,%d,%d", gcode, &cymk[0], &cymk[1], &cymk[2], &cymk[3]);

    if ( result != 5 ) 
    {
        sprintf(output, "Error in input line, expected 5 args got: %d", result);
        Serial.println(output);
        return false;
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
    return true;
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

    grbl_status();
    if (Serial3.available() > 0);
    {
            sprintf(gcode, "%s\n", gcode); 
            Serial3.write(gcode);
            GRBLdata = Serial3.readString();
            Serial.print("GBRL cmd output: ");
            Serial.println(GRBLdata);


            grbl_status();
    }
}

boolean grbl_status()
{
    char c;
    

    if (Serial3.available() > 0);
    {

        for ( ;; )
        {
            Serial3.write("?");
            GRBLdata = Serial3.readString();
            if (DEBUG)
            {
                Serial.print("GBRL output: ");
                Serial.println(GRBLdata);
            }

            if ( GRBLdata.startsWith("<Idle,") )
            {
                return true;
            }
            delay(10);
        }
            /*
        c = Serial3.peek();
        if ( c == '<' )
        {
            GRBLdata = Serial3.readString();
            Serial.print("GBRL output: ");
            Serial.println(GRBLdata);
            if ( GRBLdata.startsWith("<Idle,") )
            {
                Serial.print("GBRL output: ");
                Serial.println(GRBLdata);
            }
        }
            */
    }
}

void loop() {


    // put your main code here, to run repeatedly
    if (Serial.available() > 0) {
    
        // read the incoming byte and convert it char array for easy processign
        //
        // InputData = Serial.readString();
        // InputData.toCharArray(input, 30);
        Serial.readBytes(input, sizeof(input));

        if ( input[0] == 'd' )
        {
            DEBUG=true;
            Serial.println("DEBUG enabled");
        }
        else if ( input[0] == 'D' )
        {
            DEBUG=false;
            Serial.println("DEBUG disabled");
        }
        else if ( input[0] == '?' )
        {
            grbl_status();
        }
        else if ( input[0] == 't' )
        {
            Serial3.write("G91 G1 F500 x100 y100\n");
        }
        else if ( input[0] == 's' )
        {
            Serial3.write("??");
        }
        else if ( input[0] == 'l' )
        {
            RAW_MODE = true;
        }
        else if ( input[0] == 'L' )
        {
            RAW_MODE = false;
        }
        else 
        {
            if (DEBUG) 
            {
                sprintf(output,"Input data: %s", input);
                Serial.println(output);
                if (RAW_MODE) 
                {
                    Serial.println("RAW_MODE");
                }
            }
            if ( RAW_MODE )
            {
                Serial3.write(input);
            }
            else if ( parse_line() )
            {
                Serial.println("Do processing");
                grbl_cmd();
                paint();
            }
        }
    }

    if (Serial3.available() > 0) 
    {
        grbl_output = Serial3.read();
        Serial.write(grbl_output);
    
    }
}
