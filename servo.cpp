#include <iostream>
#include <wiringPi.h>
#include <softPwm.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <queue>
#define start 175
#define finish 194

using namespace std;

int pin = 29;

int turnToAngle(int angle); // mode = 0
int turnToPosition(int position); // mode = 1
int raspberrySetUp();

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

class FileHandler {
private:
	string filename;
	int mode;
public:
	FileHandler();
	string getFilename();
	int getMode();
	queue<string> commands;
};

int main() {
	int mode = 1; // Position mode

    if (!raspberrySetUp()) {
    	return -1;
    };

    FileHandler commandList();

	string repeatAnswer = "y";
    do {
        //moving
        ProgressBar Bar(commandList.getCommandCount());
        Bar.show();
        delay(1000);
		while(commandList.getCommandCount() != 0) {
			istringstream iss(commandList.getNextCommand());
			int lineValue = 0;
			if (!(iss >> lineValue)) {
				continue;
			}
			switch(commandList.getMode()) {
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
		cout << "Repeat procedure? (y/n) ";
        cin >> repeatAnswer;
    } while (repeatAnswer.compare("y") == 0);
    
	return 0;
}

int raspberrySetUp() {
	if (wiringPiSetup() == -1){
		cout << "ERROR: Unable too set up GPIO" << endl;
		return -1;
	};
	pinMode(pin,OUTPUT);
	digitalWrite(pin,LOW);
	if (softPwmCreate(pin,0,200) == -1){
		cout << "ERROR: Unable too set up GPIO" << endl;
		return -1;
	};
	return 0;
}

int turnToAngle(int angle) { // mode = 0
    if(angle >= 0 && angle <= 180) {
        float angleProcessed = start + float(angle * (finish - start)/ 180);
        cout << "Turning to angle: " << angle << endl;
        softPwmWrite(pin, angleProcessed);
        return 0;
    } else {
        cout << "ERROR: Unable to turn to angle: " << angle << " (Possible range: [0;180])" << endl;
        return 1;
    }
}
int turnToPosition(int position) { // mode = 1
    switch(position) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: break;
        default:
            cout << "ERROR: Unable to turn to position: " << position << " (Possible positions: 0, 1, 2, 3, 4)" << endl;
            return 1;
    }
    float angleProcessed = start + float(position * (finish - start)/ 4);
    cout << "Turning to position: " << position << " (angle: " << position * 45 << ")" << endl;
    softPwmWrite(pin, angleProcessed);
    return 0;
}
// Progress bar methods
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
// FileHandler methods
FileHandler::FileHandler() {
	do {
		this->filename = "servocmd.txt"; // Default filename

		ifstream inputSavedCommandFile("servoLastUsedFileName.txt");
		if(inputSavedCommandFile.is_open()) {
			inputSavedCommandFile >> this->filename;
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
				cin >> this->filename;
				break;
			}
		}
		//choosing mode, it have to be included in the first line of the command sheet
		ifstream inputFile(filename.c_str());
		string line;
		if (inputFile.is_open()) {
			getline(inputFile, line);
			if(line.compare("angle") == 0) {
				cout << "Running in 'Angle' mode" << endl;
				this->mode = 0;

			} else if(line.compare("position") == 0) {
				cout << "Running in 'Position' mode" << endl;
				this->mode = 1;
			} else {
				cout << "Error: No valid command file detected" << endl;
				inputFile.close();
                continue;
			}
		} else {
			cout << "Error: No valid command file detected" << endl;
			inputFile.close();
			continue;
		}
		// reading commands
        while(getline(inputFile, line)) {
            this->commands.push(line);
        }
        inputFile.close();
    	break;
	} while(true);
    
    //saving filename for future use
    ofstream outputSavedCommandFile("servoLastUsedFileName.txt");
    outputSavedCommandFile << this->filename;
    outputSavedCommandFile.close();
};
string FileHandler::getFilename() {
	return this->filename;
}
int FileHandler::getMode() {
	return this->mode;
}
int FileHandler::getCommandCount() {
	return this->commands.size();
}
int FileHandler::getNextCommand() {
	return this->commands.pop();
}