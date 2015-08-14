#include <iostream>
#include <wiringPi.h>
#include <softPwm.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#define pin 29
#define start 175
#define finish 194

using namespace std;

string filename = "servocmd.txt";
int mode = 1;

int turnToAngle(int angle);
int turnToPosition(int position);

class ProgressBar {
private:
    string bar;
    int totalNumber;
    int currentNumber;
    int valid;
    int failed;
public:
    ProgressBar(int max);
    void addValid();
    void addNotValid();
    void show();
    void report();
};

ProgressBar::ProgressBar(int max) {
    this->bar = "[";
    this->totalNumber = max;
    this->currentNumber = 0;
    this->valid = 0;
    this->failed = 0;
};
void ProgressBar::addValid() {
    currentNumber++;
    valid++;
    bar += "+";
};
void ProgressBar::addNotValid() {
    currentNumber++;
    failed++;
    bar += "?";
};
void ProgressBar::show() {
    string barEnd = "";
    for(int i = 0; i < totalNumber - currentNumber; i++) barEnd += " ";
    cout << bar << barEnd << "] (" << currentNumber << "/" << totalNumber << ")" << endl << endl;
    if (currentNumber == totalNumber) {
        delay(500);
        cout << "PROCESS REPORT:\nProcess finished with\n" << "> "<< valid << " commands completed\n> " << failed << " commands failed to complete" << "\n> " << totalNumber << " total commands" << endl << endl;
    }
};

int main() {
    //setting up
	if (wiringPiSetup() == -1){
		cout << "ERROR: Unable too set up GPIO" << endl;
		return 1;
	};
	pinMode(pin,OUTPUT);
	digitalWrite(pin,LOW);
	if (softPwmCreate(pin,0,200) == -1){
		cout << "ERROR: Unable too set up GPIO" << endl;
		return 1;
	};
    
    //inputing file with commands
	string repeatAnswer = "y";
    do {
        int commandCount = 0;
		do {
            //choosing file
			ifstream inputSavedCommandFile("servoLastUsedFileName.txt");
			if(inputSavedCommandFile.is_open()) {
				inputSavedCommandFile >> filename;
				inputSavedCommandFile.close();
			}
			string continueAnswer = "y";
			for(;;) {
				cout << "Continue with last used command file '" << filename << "'? (y/n) ";
				cin >> continueAnswer;
				if(continueAnswer == "y") {
					break;
				} else if(continueAnswer == "n") {
					cout << "Please, enter command filename: ";
					cin >> filename;
					break;
				}
			}
            
			//choosing mode, it have to be included in the first line of the command sheet
			ifstream inputFile(filename.c_str());
			string line;
			getline(inputFile, line);
			if(line.compare("angle") == 0) {
				cout << "Running in 'Angle' mode" << endl;
				mode = 0;
			} else if(line.compare("position") == 0) {
				cout << "Running in 'Position' mode" << endl;
				mode = 1;
			} else {
				cout << "Error: No valid command file detected" << endl;
                continue;
			}
            //counting commands
            commandCount = 0;
            while(getline(inputFile, line)) {
                commandCount++;
            }
            inputFile.close();
            break;
		} while(true);
        
        //saving filename for future use
        ofstream outputSavedCommandFile("servoLastUsedFileName.txt");
        outputSavedCommandFile << filename;
        outputSavedCommandFile.close();
        
        //moving
		ifstream inputFileMove(filename.c_str());
		string line;
		getline(inputFileMove, line);
        ProgressBar Bar(commandCount);
        Bar.show();
        delay(1000);
		while(getline(inputFileMove, line)) {
			istringstream iss(line);
			int lineValue = 0;
			if (!(iss >> lineValue)) {
				continue;
			}
			switch(mode) {
				case 0:
                    if (turnToAngle(lineValue) == 0) {
                        Bar.addValid();
                    } else {
                        Bar.addNotValid();
                    }
					break;
				case 1:
                    if (turnToPosition(lineValue) == 0) {
                        Bar.addValid();
                    } else {
                        Bar.addNotValid();
                    }
					break;
			}
            //resetting progress bar
            Bar.show();
			delay(1000);
		}
        inputFileMove.close();
		cout << "Repeat procedure? (y/n) ";
        cin >> repeatAnswer;
    } while (repeatAnswer.compare("y") == 0);
    
	return 0;
}

int turnToAngle(int angle) {
    if(angle >= 0 && angle <= 180) {
        float angleProcessed = start + float(angle * (finish - start)/ 180);
        cout << "Turning to angle: " << angle << endl;
        softPwmWrite(pin, angleProcessed);
        return 0;
    } else {
        cout << "ERROR: Unable to turn to angle: " << angle << " (Posiible range: [0;180])" << endl;
        return 1;
    }
}
int turnToPosition(int position) {
    switch(position) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: break;
        default:
            cout << "ERROR: Unable to turn to position: " << position << " (Posiible positions: 0, 1, 2, 3, 4)" << endl;
            return 1;
    }
    float angleProcessed = start + float(position * (finish - start)/ 4);
    cout << "Turning to position: " << position << " (angle: " << position * 45 << ")" << endl;
    softPwmWrite(pin, angleProcessed);
    return 0;
}
