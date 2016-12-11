String GRBLdata;
char input[30];
char output[30];

boolean DEBUG = false;

// digital output pins
//
const int black1_pin = 22;

// parsed input variables
//
char gcode[10];
int  cymk[4];


void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial3.begin(115200);
    Serial3.write("?");

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

    Serial3.write("G91 G0  X1");
    if (Serial3.available() > 0);
    {
        for ( i=0; i < 10; i++ )
        {
            GRBLdata = Serial3.readString();
            Serial.print("GBRL output: ");
            Serial.println(GRBLdata);
            delay(100);
        }
    }
}

void grbl_status()
{
    char c;
    
    if (Serial3.available() > 0);
    {
        c = Serial3.peek();
        if ( c == '<' )
        {
            GRBLdata = Serial3.readString();
            if ( GRBLdata.startsWith("<Idle,") )
            {
                Serial.print("GBRL output: ");
                Serial.println(GRBLdata);
            }
        }
    }
}

void loop() {

    grbl_status();

    // put your main code here, to run repeatedly
    if (Serial.available() > 0) {
    
        // read the incoming byte and convert it char array for easy processign
        //
        // InputData = Serial.readString();
        // InputData.toCharArray(input, 30);
        Serial.readBytes(input, 30);

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
        else 
        {
            if (DEBUG) 
            {
                sprintf(output,"Input data: %s", input);
                Serial.println(output);
            }
            if ( parse_line() )
            {
                Serial.println("Do processing");
                grbl_cmd();
                paint();
            }
        }
    }
}
