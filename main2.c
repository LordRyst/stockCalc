#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <float.h>

using namespace std;

const string files[] = {"VBMFX.csv","VGSIX.csv", "VGTSX.csv","VTSMX.csv"};
const int numFiles = 4;
const int daysSimulate = 365 * 4;

vector<string> lines[numFiles];
float* values[numFiles];
float* logs[numFiles];
float* diffs[numFiles];
int length[numFiles];
float* m[numFiles];
float* b[numFiles];
int maxLength;
int minLength;
float totalValues[numFiles];
float mixValues[numFiles];
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

void calcSlopes(int daysAgo) {
  for (int i = 0; i < numFiles; i++) {
    float xsum = 0;
    float ysum =0;
    float xysum = 0;
    float x2 = 0;
    float y2 = 0;
    int n = length[i] + 1 - daysAgo;
    for (int j = daysAgo; j < length[i]; j++) {
      float x = n - j;
      float y = logs[i][j];
      xsum += x;
      ysum += y;
      xysum += x * y;
      x2 += x * x;
      y2 += y * y;
    }
    m[i][daysAgo] = ((n * xysum) - (xsum * ysum)) /
      ((n * x2) - (xsum * xsum));
    b[i][daysAgo] = ((x2 * ysum) - (xsum * xysum)) /
      ((n * x2) - (xsum * xsum));
    //printf("y = %f x + %f\n", m[i], b[i]);
  }
}

void slopeDiff(int daysAgo) {
  for (int i = 0; i < numFiles; i++) {
    if (length[i] > daysAgo) {
      float diff = logs[i][daysAgo] - (m[i][daysAgo] * (length[i] - daysAgo) +b[i][daysAgo]);
      diffs[i][daysAgo] = diff;
      //printf ("Y: %f L: %f Diff: %f ", y, logs[i][daysAgo], diff);
    } else {
      diffs[i][daysAgo] = FLT_MAX;
      printf ("INFINITY ");
    }
  }
}

void calcDiffs() {
  maxLength = 0;
  for (int i = 0; i < numFiles; i++) {
    if (length[i] > maxLength) {
      maxLength = length[i];
    }
  }
  //printf("Max Length: %d\n", maxLength);
  for (int i = 0; i < numFiles; i++) {
    diffs[i] = new float[maxLength];
    m[i] = new float[maxLength];
    b[i] = new float[maxLength];
  }
  minLength = 9999999;
  for (int i = 0; i < numFiles; i++) {
    if (length[i] < minLength) {
      minLength = length[i];
    }
  }
  //printf("Min Length: %d\n", minLength);
  for (int i = 0; i < numFiles; i++) {
    printf("Files: %s\n", files[i].c_str());
    printf("  Value: %f\n", values[i][0]);
    printf("  Diff:  %f\n", diffs[i][0]);
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
    calcSlopes(j);
    slopeDiff(j);
    float minDiff = diffs[0][j];
    int minIndex = 0;
    for (int i = 1; i < numFiles; i++) {
      if (minDiff > diffs[i][j]) {
	minDiff = diffs[i][j];
	minIndex = i;
      }
    }
    mixValues[minIndex] += 100 / values[minIndex][j];
  }
  maxValue = 0;
  for (int i = 0; i < numFiles; i++) {
    float y = m[i] * (length[i] + 1) + b[i];
    maxValue += mixValues[i] * pow(10.0, y);
  }
  printf("Max   : %f\n", maxValue);
}

int main() {
  readFiles();
  findValues();
  calcLogs();
  calcSlopes(daysSimulate);
  calcDiffs();
  singleStocks();
  multiStocks();
}
