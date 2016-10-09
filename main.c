#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <float.h>

using namespace std;

const string files[] = {"VTI.csv", "VXUS.csv", "VBMFX.csv", "VGSIX.csv", "VGTSX.csv", "VTSMX.csv"};
const int numFiles = 6;
const int daysSimulate = 365 * 3;

vector<string> lines[numFiles];
float* values[numFiles];
float* logs[numFiles];
float* lineVal[numFiles];
int length[numFiles];
int minLength;
float totalValues[numFiles];
float mixValues[numFiles];
float mixSpent[numFiles];
float maxValue;

void readFiles() {
  ifstream csv;
  for (int file = 0; file < numFiles; file++) {
    csv.open(files[file].c_str());
    string line;
    if (csv.is_open()) {
      getline(csv, line);
      while (getline(csv, line) ) {
	lines[file].push_back(line);
	//printf("%s\n", line.c_str());
      }
      csv.close();
    } else {
      printf("File %s would not open.\n", files[file].c_str());
    }
    length[file] = lines[file].size();
  }
}

void findValues() {
  for (int i = 0; i < numFiles; i++){
    values[i] = new float[length[i]];
    for (int j = 0; j < length[i]; j++) {
      int lastComma = lines[i][j].find_last_of(',');
      if (lastComma != string::npos) {
	string value = lines[i][j].substr(lastComma + 1, string::npos);
	values[i][j] = atof(value.c_str());
	//printf("%f ", values[i][j]);
      }
    }
  }
}

void calcLogs() {
  for (int i = 0; i < numFiles; i++) {
    logs[i] = new float[length[i]];
    for (int j = 0; j < length[i]; j++) {
      logs[i][j] = log10(values[i][j]);
      //printf("%f ", logs[i][j]);
    }
  }
}

void calcLineVals(int daysAgo) {
  for (int i = 0; i < numFiles; i++) {
    float xsum = 0;
    float ysum = 0;
    float xysum = 0;
    float x2 = 0;
    float y2 = 0;
    int n = length[i] - daysAgo;
    for (int j = daysAgo; j < length[i]; j++) {
      float x = length[i] - j;
      float y = logs[i][j];
      xsum += x;
      ysum += y;
      xysum += x * y;
      x2 += pow(x, 2);
      y2 += pow(y, 2);
    }
    float m = ((n * xysum) - (xsum * ysum)) /
      ((n * x2) - (xsum * xsum));
    float b = ((x2 * ysum) - (xsum * xysum)) /
      ((n * x2) - (xsum * xsum));
    float logEst = m * n + b;
    lineVal[i][daysAgo] = pow(10, logEst);
    //printf("Est%d: %f\n", i + 1, lineVal[i][daysAgo]);
  }
}

void calcDiffs() {
  for (int i = 0; i < numFiles; i++) {
    lineVal[i] = new float[length[i]];
  }
  for (int j = 0; j < daysSimulate; j++) {
    calcLineVals(j);
  }
  for (int i = 0; i < numFiles; i++) {
    printf("Files: %s\n", files[i].c_str());
    printf("  Value: %f\n", values[i][0]);
    printf("  EstVal:  %f\n", lineVal[i][0]);
  }
}

void singleStocks() {
  printf("MoneyIn: %f\n",100.0 * daysSimulate);
  for (int i = 0 ; i < numFiles; i++) {
    totalValues[i] = 0;
    mixValues[i] = 0;
    for (int j = 0; j < daysSimulate; j++) {
      totalValues[i] += 100.0 / values[i][j];
    }    
    totalValues[i] *= values[i][0];
    printf("Total%d: %f\n", i + 1, totalValues[i]);
  }
}

void multiStocks() {
  for (int j = 0; j < daysSimulate; j++) {
    float mult = 1 / values[0][j];
    
    float min = mult * lineVal[0][j];
    int minIndex = 0;
    for (int i = 1; i < numFiles; i++) {
      mult = 1 / values[i][j];
      float est = mult * lineVal[i][j];
      if (est < min) {
	min = est;
	minIndex = i;
      }
    }
    mixValues[minIndex] += 100 / values[minIndex][j];
    mixSpent[minIndex] += 100;
  }
  maxValue = 0;
  for (int i = 0; i < numFiles; i++) {
    float value = mixValues[i] * values[i][0];
    maxValue += value;
    printf("  Stock%d: $%f\n", i + 1, value);
    printf("  Spent  : $%f\n", mixSpent[i]);
  }
  printf("Max   : %f\n", maxValue);
}

int main() {
  readFiles();
  findValues();
  calcLogs();
  calcDiffs();
  singleStocks();
  multiStocks();
  float rate = 0.000428443;
  float value = 100 * (pow(1 + rate, 1095)-1) / rate;
  printf("Value: %f", value);
}
