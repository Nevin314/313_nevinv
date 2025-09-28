/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Nevin Valayathil 
	UIN: 634001916
	Date: 09/23/2025
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <cstddef>
#include <iterator>
#include <unistd.h>

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
    int64_t max_buffer = MAX_MESSAGE;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
            case 'm':
                max_buffer = atoi(optarg); 
                break;
		}
	}

    pid_t pid = fork(); 

    if (pid == -1) {
        perror("fork failed"); 
    } else if (pid == 0) {
        // child 

        string buf_size = to_string(max_buffer);
        char* args[] = {(char*) "./server", (char*) buf_size.data(), NULL};
        execvp(args[0], args);
        perror("exec failed.");
    }

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE); 
    if (p > 0 && t > 0 && e > 0) {
        datamsg dmsg(p, t, e);

        char buf[MAX_MESSAGE]; 
        double reply = 0.0;

        memcpy(buf, &dmsg, sizeof(datamsg)); 
        chan.cwrite(buf, sizeof(datamsg)); 
        chan.cread(&reply, sizeof(double)); 
        cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;


        MESSAGE_TYPE m = QUIT_MSG;
        chan.cwrite(&m, sizeof(MESSAGE_TYPE));
    } else if (p > 0) {
        // top 1000? 
        ofstream xfile("./received/x1.csv");

        if (!xfile.is_open()) {
            cout << "File didn't open." << '\n';
        } else {
            datamsg dmsg(p, 0.00, 0);
            for (double time = 0.000; time < 1000 * 0.004; time+=0.004) {
                xfile << time;
                for (int ecg = 1; ecg < 3; ecg++) {
                    dmsg.ecgno = ecg; 
                    dmsg.seconds = time;
                    
                    char buf[MAX_MESSAGE]; 
                    double val = 0.0; 

                    memcpy(buf, &dmsg, sizeof(datamsg)); 
                    chan.cwrite(buf, sizeof(datamsg)); 
                    chan.cread(&val, sizeof(double));
                    xfile << ',' << val;
                }
                xfile << '\n';
            }

            xfile.close();
        }
        MESSAGE_TYPE m = QUIT_MSG;
        chan.cwrite(&m, sizeof(MESSAGE_TYPE));
    }
    if (filename != "") {
             
        filemsg fm(0, 0);
        
        int len = sizeof(filemsg) + (filename.size() + 1); 
        char* buf2 = new char[len];  // how big the buffer has to be 
        memcpy(buf2, &fm, sizeof(filemsg)); 
        strcpy(buf2 + sizeof(filemsg), filename.c_str());
        // buf2 = fm and filename?
        chan.cwrite(buf2, len); // ask for file length?

        int64_t filesize = 0;
        chan.cread(&filesize, sizeof(int64_t));
         
        string path = "./received/" + filename;
        ofstream res(path);
        char* buf3 = new char[max_buffer];
        
        for (int i = 0; i < ceil((double) filesize / max_buffer); i++) {
            //filemsg fmsg(i * max_buffer, min(max_buffer, (int) filesize - (i * max_buffer))); // wil this conversion work? 

            // request a buffer of bytes 
            fm.offset = i * max_buffer; 
            fm.length = min(max_buffer, filesize - fm.offset);
            
            memcpy(buf2, &fm, sizeof(filemsg)); 
            strcpy(buf2 + sizeof(filemsg), filename.c_str());

            chan.cwrite(buf2, len);

            // read the response into file 

            chan.cread(buf3, min(max_buffer, filesize - fm.offset));
            res.write(buf3, min(max_buffer, filesize - fm.offset));
        }

        MESSAGE_TYPE m = QUIT_MSG;
        chan.cwrite(&m, sizeof(MESSAGE_TYPE));
    }



    
    /*
    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	
	// example data point request
    char buf[MAX_MESSAGE]; // 256
    datamsg x(1, 0.0, 1);
	
	memcpy(buf, &x, sizeof(datamsg));
	chan.cwrite(buf, sizeof(datamsg)); // question
	double reply;
	chan.cread(&reply, sizeof(double)); //answer
	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	
    // sending a non-sense message, you need to change this
	filemsg fm(0, 0);
	string fname = "teslkansdlkjflasjdf.dat";
	
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // I want the file length;

	delete[] buf2;
	
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));
    */
}
