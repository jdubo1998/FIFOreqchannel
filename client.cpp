/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"
#include <getopt.h>

using namespace std;

int buffercapacity = MAX_MESSAGE;
char* buffer = NULL;

printUsage() {
    printf("Usage: client -p <patient> -t <time> -e <ecg>\n");
    printf("Usage: client -m <buffercapacity>\n");
    printf("Usage: client -f <filename>\n");
    printf("Usage: client -c <channelname>\n");
    exit(2);
}

void getDataPt(FIFORequestChannel* chan, int patient, double time, int ecg) {  
    datamsg d (patient, time, ecg);
    chan->cwrite (&d, sizeof(datamsg));
    double resp = 0;
    
    chan->cread((char*) &resp, sizeof (double));
    
    cout << resp << endl;
}

/* uses a FIFORequestChannel to gather data from a file.
 * Then the file  */
void getFile(FIFORequestChannel* chan, const char* filename) {
    string filepath = "received/" + string(filename);
    ofstream ofile;
    int requests;
    buffer = (char*) malloc(sizeof(filemsg)+strlen(filename)+1); // initialize buffer package
    char data[buffercapacity];

    __int64_t filesize = 0;

    ofile.open(filepath); // opens the file

    /* set buffer to filemsg(0,0) + fileName */
    *(filemsg*) buffer = filemsg(0,0);
    strcpy((char*)buffer + sizeof(filemsg), filename);

    /* returns the fileSize */
    chan->cwrite(buffer, sizeof(filemsg)+strlen(filename)+1);
    chan->cread((char*) &filesize, sizeof(__int64_t));

    requests = ceil(filesize/buffercapacity); // calculates the number of requests needed

    /* iterates through each request */
    for (int i = 0; i <= requests; i++) {
       int bufcap = buffercapacity;

       /* checks if on the last package */
       if (i == requests) {
           bufcap = filesize-(requests*buffercapacity); // calculate last package size, which will be less than buffercapacity.
       }

       *(filemsg*) buffer = filemsg(i*buffercapacity, bufcap); // creates the filemsg to send to the server

       /* gets the data in a char* for each request */
       chan->cwrite(buffer, sizeof(filemsg)+strlen(filename)+1);
       chan->cread(data , bufcap);

       ofile.write(data, bufcap); // writes the data from the package to the output file
    }

    ofile.close();
    delete filename, buffer;
}

int main(int argc, char *argv[]){
    int n = 100;    // default number of requests per "patient"
    int p = 15;		// number of patients
    srand(time_t(NULL));
    
    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
    
    char option;
    
    int patient = -1, ecg = -1;
    double time = -1.0;
    
    int cap = -1;
    char* filename;
    
    bool file = false, datapt = false, channel = false;
    
    while ((option = getopt(argc, argv, "p:e:m:f:ct:")) != -1) {
        switch (option) {
            case 'p':
                datapt = true;
                
                if (atoi(optarg) > 0 && atoi(optarg) <= p) { 
                    patient = atoi(optarg);
                } else {
                    printUsage();
                }
                break;
            case 't':
                datapt = true;
                
                if (atof(optarg) >= 0) {
                    time = atof(optarg);
                } else {
                    printUsage();
                }
                break;
            case 'e':
                datapt = true;
                
                if (atoi(optarg) == 1 || atoi(optarg) == 2) {
                    ecg = atoi(optarg);
                } else {
                    printUsage();
                }
                break;
            case 'm':
                if (atoi(optarg) > 0) {
                    cap = atoi(optarg);
                } else {
                    printUsage();
                }
                break;
            case 'f':
                file = true;
                
                if (strlen(optarg) > 0) {
                    filename = new char[strlen(optarg)];
                    strcpy(filename, optarg);
                } else {
                    printUsage();
                }
                break;
            case 'c':
                channel = true;
                break;
            case '?':
            default:
                printUsage();
                break;
        }
    }
    
    char newchannel[buffercapacity];
    
    if (patient != -1 && time > -1 && ecg != -1) {
        for (int i = 0; i < 15000; i++) {
            
        }
        getDataPt(&chan, patient, time, ecg);
    } else if (datapt) {
        printUsage();
    }
    
    if (file) {
        getFile(&chan, filename);
    }
        
    if (channel) {
        filename = new char[5];
        strcpy(filename, "1.csv");
        
        MESSAGE_TYPE m = NEWCHANNEL_MSG;
        chan.cwrite(&m, sizeof(MESSAGE_TYPE));
        chan.cread(newchannel, buffercapacity);
        
        FIFORequestChannel chan1 (string(newchannel), FIFORequestChannel::CLIENT_SIDE);
        getDataPt(&chan1, 1, 0.0, 1);
        getFile(&chan1, filename);
        
        m = QUIT_MSG;
        chan1.cwrite (&m, sizeof (MESSAGE_TYPE));
    }
    
    // closes the channel
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));
}
