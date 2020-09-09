#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <time.h>
using namespace std;

string Parse_String(string line);

int main() {
  std::ifstream Stat_file;
  std::ofstream csv_file;

  string line;

  Stat_file.open("GoSstats.txt");
  csv_file.open("GoSstats.csv");

  while(!Stat_file.eof()) {
    getline(Stat_file, line);
    cout << line << endl;
    csv_file << line << endl;
  }

  Stat_file.close();
  csv_file.close();
}

string Parse_String(string line) {

  vector<char> tempInt;
  string Name;
  string init_modifier;
  string health_num_dice;
  string health_die_value;
  string health_modifier;
  string page_number;
  bool l, m, n, p, q;
  int k, j, z;


  l = true;
  m = true;
  n = true;
  p = true;
  q = true;
  k = 0;
  j = 0;
  z = 0;

  for (auto it = line.begin(); it != line.end(); ++it) {
    if ((isalpha(*it) ) && l)	{ // If initiative was already rolled (!l) then dont try to get a name
      while (it != line.end() && *it != '.')	{ // First letter to the first period will be the mob's name
        Name.push_back(*it);
        it++;
      }
    }
    else if (isdigit(*it)) {
      if (l) { // Init modifier
        if(*(it-1) == '-') {// Has a negative init mod
          init_modifier.push_back('-');
          init_modifier.push_back(*it);
        }
        else
          init_modifier.push_back(*it);
        l = false;
      }
      else if (m) { // num of health dice
          while (it != line.end() && isdigit(*it)) {
            tempInt.push_back(*it);
            it++;
          }
          string tempString(tempInt.begin(), tempInt.end());
          health_num_dice = tempString;
          m = false;
          tempInt.clear();
      }
      else if (n) { // health dice value
        while (it != line.end() && isdigit(*it)) {
          tempInt.push_back(*it);
          it++;
        }
        string tempString(tempInt.begin(), tempInt.end());
        health_die_value = tempString;
        n = false;
        tempInt.clear();
      }
      else if (p) { // Health mod
        while (it != line.end() && isdigit(*it)) {
          tempInt.push_back(*it);
          it++;
        }
        string tempString(tempInt.begin(), tempInt.end());
        health_modifier = tempString;
        p = false;
        tempInt.clear();
      }
      else if (q) {
        while (it != line.end() && isdigit(*it)) {
          tempInt.push_back(*it);
          it++;
        }
        string tempString(tempInt.begin(), tempInt.end());
        page_number = tempString;
        q = false;
        tempInt.clear();
      }
    }
  }
  Name.push_back(',');
  init_modifier.push_back(',');
  health_num_dice.push_back(',');
  health_die_value.push_back(',');
  health_modifier.push_back(',');
  string new_line = Name + init_modifier + health_num_dice +
            health_die_value + health_modifier + page_number;
  //cout << new_line << endl;
  return new_line;
}
