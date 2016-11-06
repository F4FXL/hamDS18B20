/*
 * Standalone temperature logger
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "hamds18b20.h"

#define SEQFILE "/tmp/aprsTemperSequence"

int main(int argc, char **argv)
{
    unsigned char optChar;
    BOOL label = FALSE;
    BOOL unit = FALSE;
    BOOL eq = FALSE;
    BOOL telem = FALSE;
    BOOL status = FALSE;
    BOOL dstar = FALSE;
    char * callSign = NULL;
    char * fileName = NULL;

    while((optChar = getopt (argc, argv, "luetsdc:f:")) != 255)
    {
        switch(optChar)
        {
        case 'l':
            label = TRUE;
            break;
        case 'u' :
            unit = TRUE;
            break;
        case 'e':
            eq = TRUE;
            break;
        case 't':
            telem = TRUE;
            break;
        case 's':
            status = TRUE;
            break;
        case 'c':
            callSign = optarg;
            break;
        case 'f':
	    fileName = optarg;
            break;
        case 'd':
            dstar = TRUE;
            break;
        default:
            return 1;
        }
    }

    if(callSign == NULL && (label || unit || eq))//no callsign specified, exit
        return 1;

    if(label)
        print_labels(callSign);
    if(unit)
        print_units(callSign);
    if(eq)
        print_equation(callSign);
    if(telem)
        print_telemetry(fileName);
    if(status)
        print_status(fileName);
    if(dstar)
        print_text(fileName);

    return 0;
}

void print_labels(char * callSign)
{
    printf(":%-9s:PARM.PA Temp\n", callSign);
    fflush(stdout);
}

void print_units(char * callSign)
{
    printf(":%-9s:UNIT.deg.C\n", callSign);
    fflush(stdout);
}

void print_equation(char * callSign)
{
    printf(":%-9s:EQNS.0,0.16016,-40,0,0,0,0,0,0,0,0,0,0,0,0\n", callSign);
    fflush(stdout);
}

void print_status(char * fileName)
{
    float temp;
    if(!read_temp(&temp, fileName))
    {
        printf(">Temperature: %.1f C\n", temp);
        fflush(stdout);
    }
}

void print_text(char * fileName)
{
    float temp;
    if(!read_temp(&temp, fileName))
    {
        printf("Temp.: %.1f C\n", temp);
        fflush(stdout);
    }
}

void print_telemetry(char * fileName)
{
    int seq;
    float temp;

    if(!read_temp(&temp, fileName))
    {
        seq = get_telemetry_sequence();

        temp += 40;
        temp /= 0.16016;

        printf("T#%03d,%.0f,000,000,000,000,00000000\n", seq, temp);//since aprs specs says data is unsigned ad 40 to it, the 40 will substracted by listeners using EQNS
        fflush(stdout);
    }
}

int get_telemetry_sequence(void)
{
    int seq = 0;
    FILE * fileds = fopen(SEQFILE, "rb");
    if(fileds)
    {
        fread(&seq, sizeof(int), 1, fileds);
        fclose(fileds);
    }

    seq++;

    if(seq > 999)
        seq = 1;

    fileds = fopen(SEQFILE, "wb");
    if(fileds)
    {
        fwrite(&seq, sizeof(int), 1, fileds);
        fclose(fileds);
    }

    return seq;
}

int read_temp(float * temp, char * fileName)
{
    char * buffer = NULL;
    long length;
    int result = 1;
    FILE* fileHandle = fopen(fileName, "rb");

    if(fileHandle != NULL)
    {
        // Read complete file until the end
        fseek (fileHandle, 0, SEEK_END);
        length = ftell (fileHandle) + 1;//add one so we can add teminating '\0' later on
        fseek (fileHandle, 0, SEEK_SET);//seek back to beginning of file
        buffer = malloc (length);
        if (buffer)
        {
            fread (buffer, 1, length - 1, fileHandle);

            buffer[length - 1] = '\0';

            char * valueStr = strstr(buffer, "t=");
            if(valueStr != NULL)
            {
                valueStr += 2;//skip t=
                *temp = atof(valueStr) / 1000.0; 
                result = 0;
            }

            free(buffer);
        }
        fclose(fileHandle);
    }

    if(*temp >= 85.0)//this means the temp sensor did not sample first temp, say it is invalid
        result = 1;

    return result;
}
