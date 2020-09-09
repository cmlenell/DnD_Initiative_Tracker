#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <time.h>
bool DEBUGMODE;
const int num_files = 4;
using std::cout;
using std::vector;
using std::string;

class Initiative
{
private:
	int initiative;
	int health;
	bool is_npc;
	string name;

public:
	Initiative() : initiative(0), health(0), is_npc(false), name ("0")
	{/* Default constructor */}
	Initiative(int init, int h, bool npc, string n)
	 					 : initiative(init), health(h), is_npc(npc), name(n)
	{/* Intentionally empty */}

	int roll_init(int modifier)	{
		int roll = (rand() % 20) + modifier + 1;
		return roll;
	}

	Initiative set_initiative(int init, int h, bool npc, string n)	{
		Initiative temp(init, h, npc, n);
		return temp;
	}

	int roll_health(int num_dice, int die_value, int modifier)	{
		int roll = 0;

		for (int j = 0; j < num_dice; j++) {
			roll += (rand() % die_value) + 1;
		}
		roll += modifier;

		return roll;
	}

	const int get_health() {
		return health;
	}

	const int get_init() {
		return initiative;
	}
	const string get_name() {
		return name;
	}

	const bool get_status() {
		return is_npc;
	}
};

class npc_page_number
{
private:
	string npc_name;
	string file_name;
	string page_number;

public:
	npc_page_number() : npc_name(""), file_name(""), page_number("")
	{/* Default constructor */}

	npc_page_number(string npc, string file, string page)
	 								: npc_name(npc), file_name(file), page_number(page)
	{/* Intentionally empty */}

	string get_npc_name() {
		return npc_name;
	}

	string get_file_name()	{
		return file_name;
	}

	string get_page_number()	{
		return page_number;
	}


};

void swap(Initiative &x, Initiative &y);
void sort(vector<Initiative> &Initiatives);
void alpha_sort(vector<Initiative> &Initiatives);
int roll_init(int modifier);
int roll_health(int num_dice, int die_value, int modifier);
void get_pcs(vector<Initiative> &Initiatives, int num_pcs);
void get_npcs(vector<Initiative> &Initiatives, vector<npc_page_number> &npc_page_numbers,
					vector<vector<string>> Stats_array);
void display_Initiatives(vector<Initiative> Initiatives,
					vector<npc_page_number> npc_page_numbers, int max_length);
void write_to_file(vector<Initiative> Initiatives,vector<npc_page_number> npc_page_numbers,int max_length);
string look_for_stats(string &line, vector<vector<string>> Stats_array);
//int binarySearch(vector<string> Stats_array, int l, int r, string line);

int main(int argc, char** argv)	{
	DEBUGMODE = false;
	int i;
	for(i = 0; i < argc; i++)
		if(strcmp(argv[i], "-d") == 0)
			DEBUGMODE = true;

	// Convert text files to a 2D vector of strings
	vector<vector<string>> Stats_array;
	vector<string> Stats;
	std::ifstream Stat_file;
	string line;
	string file_arr[num_files] = { "MMstats.txt", "GoSstats.txt", "Volostats.txt", "ToAstats.txt"};

	for (i = 0; i < num_files; i++) {
		Stat_file.open(file_arr[i]);

		while(!Stat_file.eof()) {
			getline(Stat_file, line);
			Stats.push_back(line);
		}

		if (DEBUGMODE) {
			cout << file_arr[i] << ":\n";
		for(int i = 0; i < Stats.size(); i++) {
			cout << Stats[i] << std::endl;
		}
	}

		Stats_array.push_back(Stats);
		Stats.clear();
		Stat_file.close();
	}

	vector<Initiative> Initiatives;	// Holds npc and pc names and initiatives
	vector<npc_page_number> npc_page_numbers;	// Holds file name and page number for each npc type
	int num_npcs;
	int num_pcs;
	int max_length;			// Length of longest name


	srand(time(NULL));	// Initialize random seed
	char answer;
	std::cin.sync();
	cout << "Write to file? (y/n): ";
	answer = std::cin.get();
	cout << "\nEnter number of pc's: ";
	std::cin >> num_pcs;
	std::cin.ignore(256, '\n');	// get rid of any extra characters
	if (num_pcs > 0)
		get_pcs(Initiatives, num_pcs);

	cout << "\nEnter number and names of npc's: \n";

	get_npcs(Initiatives, npc_page_numbers, Stats_array);

	sort(Initiatives);

	// Find length of longest character name (for formatting)
	int a = 0;
	for (i = 0; i < Initiatives.size(); i++) {
		if (Initiatives[i].get_name().length() > a) {
			a = Initiatives[i].get_name().length();
		}
	}
	max_length = a;


	display_Initiatives(Initiatives, npc_page_numbers, max_length);
	if (answer == 'y')
		write_to_file(Initiatives, npc_page_numbers, max_length);

	return 0;
}

void get_pcs(vector<Initiative> &Initiatives, int num_pcs) {
	string line;
	vector<char> tempInt;
	vector<char> tempChar;
	int tempInit;
	bool l;
	bool m;
	int k;
	int j;
	int z;

	for (int i = 0; i < num_pcs; i++)	{
		cout << "Enter character #" << i + 1 << "'s roll: ";

		std::getline(std::cin, line);
		l = true;

		for (string::iterator it = line.begin(); it != line.end(); ++it)	{

			if (isalpha(*it) && l)	{// Don't try to get a name after we got initiative
					tempChar.push_back(*it);
			}

			else if (isdigit(*it)) {
				if (l)	{
					tempInt.push_back(*it);

					if (tempInt.size() == 2)	{// Initiative can only be max two digits
						string tempString(tempInt.begin(), tempInt.end());
						tempInit = std::stoi(tempString);
						l = false;
					}
				}
			}
		}

		if (l)	{// If initiative was not a two digit number this will fire
			string tempString(tempInt.begin(), tempInt.end());
			tempInit = std::stoi(tempString);
		}

		string tempString(tempChar.begin(), tempChar.end());
		Initiative temp(tempInit, 0, false, tempString);
		Initiatives.push_back(temp);
		tempInt.clear();
		tempChar.clear();
	}

}

void get_npcs(vector<Initiative> &Initiatives, vector<npc_page_number> &npc_page_numbers,
							vector<vector<string>> Stats_array)
{
	string line;
	vector<char> tempInt;
	vector<char> tempChar;
	int tempInit;
	int init_modifier;
	int health_num_dice;
	int health_die_value;
	int health_modifier;
	int num_npcs;
	string file_name;
	bool l, m, n , p, q;		// Keep track of what parts of stats have been entered
	int k, j, z;


	while (std::cin >> num_npcs) {
		std::cin >> std::ws;	// get rid of preceding spaces
		std::getline(std::cin, line);

		// get rid of succeeding spaces
		string::reverse_iterator rit;
		for(rit = line.rbegin(); isspace(*rit); ++rit);
		if(*rit != line.back()) {	// check if rit actually moved
			if (DEBUGMODE) {
				cout << "Trimming end of string...\n";
			}
			line.erase(rit.base(), line.end());
		}

		for( int i = 0; i < num_npcs; i++)	{

		if (DEBUGMODE)
			cout << num_npcs << " " << line << " -- size is " << line.size() << std::endl;
		// Only need to look through files and get stats first time through
		if (i == 0) {
			string page_number("");
			if((file_name = look_for_stats(line, Stats_array)) == "") { // Stat block not found -- Manually enter stats
				cout << line << " not found -- enter stat line: ";
				string stats;
				std::getline(std::cin, stats);
				line = line + "." + stats + "/n";
			}
			line.push_back('#');

			if(DEBUGMODE)
				cout <<  line << std::endl;

		l = true;
		m = true;
		n = true;
		p = true;
		q = true;
		k = 0;
		j = 0;
		z = 0;
			for (string::iterator it = line.begin(); it != line.end() && *it != '#'; ++it) {
			if (DEBUGMODE)
				cout << std::endl << j << " looking at " << *it << std::endl;

			if ((isalpha(*it) ) && l)	{ // If initiative was already rolled (!l) then dont try to get a name
				while (it != line.end() && *it != '.')	{ // First letter to the first period will be the mob's name
					tempChar.push_back(*it);
					if (DEBUGMODE)
						cout << " pushed " << tempChar[k] << std::endl;
					k++;
					it++;
				}
			}

			else if (isdigit(*it))	{
				if (l)	{// Initiative modifier -- will only take one digit
					init_modifier = std::atoi(&*it);
					if(*(it-1) == '-')	// Has a negative initiative modifier
						init_modifier *= -1;
					tempInit = roll_init(init_modifier);
					if (DEBUGMODE)
						cout << "\nInit modifier is " << init_modifier << "\nInit roll is " << tempInit << std::endl;
					l = false;
				}
				else if (m)	{ // Number of health dice to be rolled
					while (it != line.end() && isdigit(*it))	{
						if (DEBUGMODE)
							cout << std::endl << j << " looking inside m at " << *it << std::endl;

						tempInt.push_back(*it);

						if (DEBUGMODE)
							cout << "pushed " << tempInt[z];
						z++;
						j++;
						it++;
					}
					string tempString(tempInt.begin(), tempInt.end());
					health_num_dice = std::stoi(tempString);
					if (DEBUGMODE)
						cout << "\nNumber of health dice is " << health_num_dice << std::endl;
					m = false;
					z = 0;
					tempInt.clear();;
				}
				else if (n)	// Health dice value
				{
					// stop when a non digit is found or reached the end of the line
					while (it != line.end() && isdigit(*it))	{
						if (DEBUGMODE)
							cout << std::endl << j << " looking inside n at " << *it << std::endl;

						tempInt.push_back(*it);

						if (DEBUGMODE)
							cout << "pushed " << tempInt[z];

						z++;
						j++;
						it++;
					}
					string tempString(tempInt.begin(), tempInt.end());
					health_die_value = std::stoi(tempString);
					if (DEBUGMODE)
						cout << "\nValue of health dice is " << health_die_value << std::endl;
					n = false;
					z = 0;
					tempInt.clear();
				}
				else if (p) {// Health modifier
					// stop when a non digit is found or reached the end of the line
					while (it != line.end() && isdigit(*it))	{
						if (DEBUGMODE)
							cout << std::endl << j << " looking inside p at " << *it << std::endl;
						tempInt.push_back(*it);

						if (DEBUGMODE)
							cout << "pushed " << tempInt[z];
						z++;
						j++;
						it++;
					}
					string tempString(tempInt.begin(), tempInt.end());
					health_modifier = std::stoi(tempString);
					if (DEBUGMODE)
						cout << "\nHealth modifier is " << health_modifier << std::endl;
					p = false;
					z = 0;
					tempInt.clear();
				}
				else if (q) {
					while (it != line.end() && isdigit(*it))	{
						if (DEBUGMODE)
							cout << std::endl << j << " looking inside q at " << *it << std::endl;
						tempInt.push_back(*it);

						if (DEBUGMODE)
							cout << "pushed " << tempInt[z];
						z++;
						j++;
						it++;
					}

					if (tempInt.size() == 0) // No page number ws given
						page_number = "";
					else {
						string tempString(tempInt.begin(), tempInt.end());
						page_number = tempString;
					}
					if (DEBUGMODE)
						cout << "\nPage number is " << page_number << std::endl;
					q = false;
					z = 0;
					tempInt.clear();
				}
			}
			j++;
			}
			string tempString(tempChar.begin(), tempChar.end());
			if (DEBUGMODE) {
				cout << "Putting " << tempString << " " << file_name << " " << page_number << " into npc_page_number\n";
			}
			npc_page_number temp(tempString, file_name, page_number);
			npc_page_numbers.push_back(temp);
		}

		tempInit = roll_init(init_modifier);
		if (DEBUGMODE)
			cout << "\nInit modifier is " << init_modifier << "\nInit roll is " << tempInit << std::endl;
		if (DEBUGMODE)
			cout << std::endl << health_num_dice << " " << health_die_value << " " << health_modifier << std::endl;
		int tempHealth = roll_health(health_num_dice, health_die_value, health_modifier);

		string tempString(tempChar.begin(), tempChar.end());
		if (DEBUGMODE)
			cout << tempString << std::endl;

		if (num_npcs > 1)	// if more than one of same type, append number to end
			tempString = tempString + std::to_string(i+1);

		if (DEBUGMODE) {
			cout << "\nPutting ";
			for (int o = 0; o < tempChar.size(); o++)	{
				cout << tempChar[o];
			}
			cout << " into " << tempString << std::endl;
		}

		Initiative temp(tempInit, tempHealth, true, tempString);
		Initiatives.push_back(temp);

		}
		tempInt.clear();
		tempChar.clear();

	}
}

string look_for_stats(string &line, vector<vector<string>> Stats_array)
{
	/*	*Something wrong with string comparisons*
	int index;

	for(int i = 0; i << num_files; i++)	{
		index = binarySearch(Stats_array[i], 0, Stats_array[i].size() - 1, line);
		if (index =! -1);{	// line is in one of the files.
			line = Stats_array[i][index];
			return true;
		}
	}

	return false;
	*/

		string::iterator line_iter;
		string::iterator Stat_iter;
		int i;
		int j;
		for (i = 0; i < Stats_array.size(); i++)	{

		int size = Stats_array[i].size();

		for(j = 0; j < size; j++)	{
			for(line_iter = line.begin(), Stat_iter = Stats_array[i][j].begin();
				line_iter != line.end()  && 	// Go to end of line entered or until spces
					(*line_iter == *Stat_iter || *line_iter + 32 == *Stat_iter || *line_iter - 32 == *Stat_iter);	// compare chars case-insensitive
				++line_iter, ++Stat_iter)	{
				if (DEBUGMODE)
					cout << *line_iter << " " << *Stat_iter << std::endl;
			}

			// if we reached the end of the string entered then
			// replace string entered with file line and return true
			if (line_iter == line.end()) {
				if(DEBUGMODE)
					cout << "Found the stat block\n";
				line = Stats_array[i][j];
				if (i == 0)
					return "Monster Manual";
				else if (i == 1)
					return "Ghosts of Saltmarsh";
				else if (i == 2)
					return "Volo's Guide to Monsters";
				else
					return "Tomb of Annihilation";
			}
		}
	}
	return "";

}

int binarySearch(vector<string> Stats_array, int l, int r, string line)
{
	if (r >= l)	{
			int mid = l + (r - l) / 2;
			string::iterator iter;
			vector<char> tempChar;
			while (*iter != '.')	{ // First letter to the first period will be the character's name
					tempChar.push_back(*iter);
					iter++;
				}
			string tempString(tempChar.begin(), tempChar.end());
			if (tempString == line)
				return mid;

			if (tempString > line)
				return binarySearch(Stats_array, l, mid - 1, line);

			return binarySearch(Stats_array, mid + 1, r, line);
		}
	return -1;
}

void display_Initiatives(vector<Initiative> Initiatives,vector<npc_page_number> npc_page_numbers, int max_length)
{
	sort(Initiatives);
	cout << "\nCombat order is: \n";
	for (int i = Initiatives.size() - 1; i >= 0; i--)	{
		if (DEBUGMODE)
			cout << "looking at " << Initiatives[i].get_name() << std::endl;
		cout << std::setfill('.') << std::setw(max_length + 3) << std::left			// Format with charcter name and roll seperated by periods
			<< Initiatives[i].get_name() << Initiatives[i].get_init() << std::endl;
	}

	alpha_sort(Initiatives);
	cout << "\n\n";
	cout << std::setfill('.') << std::setw(max_length + 3) << std::left << "NPCs" << "HP\n";
	// Display npc healths
	for (int i = 0; i < Initiatives.size(); i++)	{
		if (DEBUGMODE)
			cout << "looking at " << Initiatives[i].get_name() << std::endl;
		if (Initiatives[i].get_status())	{
			cout << std::setfill('.') << std::setw(max_length + 3) << std::left
				<< Initiatives[i].get_name() << Initiatives[i].get_health() << std::endl;
		}
	}

	cout << "\n\n";
	cout << std::setfill('.') << std::setw(max_length + 3) << std::left << "NPC" << "Page number\n";
	for (int i = 0; i < npc_page_numbers.size(); i++)	{
		if(npc_page_numbers[i].get_page_number() != ""){	// Only display page number if there is one
		cout << std::setfill('.') << std::setw(max_length + 3) << std::left			// Format with charcter name and roll seperated by periods
				<< npc_page_numbers[i].get_npc_name() << npc_page_numbers[i].get_file_name()
				<< " pg." << npc_page_numbers[i].get_page_number() << std::endl;
		}
	}
}

void write_to_file(vector<Initiative> Initiatives,vector<npc_page_number> npc_page_numbers, int max_length)
{
	std::ofstream out;

	out.open("Init");

	// Put initiative order into text file
	out << "Combat order is: \n";
	for (int i = Initiatives.size() - 1; i >= 0; i--)	{
		if (DEBUGMODE)
			out << "looking at " << Initiatives[i].get_name() << std::endl;
		out << std::setfill('.') << std::setw(max_length + 3) << std::left			// Format with charcter name and roll seperated by periods
			<< Initiatives[i].get_name() << Initiatives[i].get_init() << std::endl;
	}

	alpha_sort(Initiatives);
	out << "\n\n";
	out << std::setfill('.') << std::setw(max_length + 3) << std::left << "NPCs" << "HP\n";
	// Display npc healths
	for (int i = 0; i < Initiatives.size(); i++)	{
		if (DEBUGMODE)
			out << "looking at " << Initiatives[i].get_name() << std::endl;
		if (Initiatives[i].get_status())	{
			out << std::setfill('.') << std::setw(max_length + 3) << std::left
				<< Initiatives[i].get_name() << Initiatives[i].get_health() << std::endl;
		}
	}

	out << "\n\n";
	out << std::setfill('.') << std::setw(max_length + 3) << std::left << "NPC" << "Page number\n";

	for (int i = 0; i < npc_page_numbers.size(); i++)	{
		if(npc_page_numbers[i].get_page_number() != ""){	// Only display page number if there is one
			out << std::setfill('.') << std::setw(max_length + 3) << std::left			// Format with charcter name and roll seperated by periods
				<< npc_page_numbers[i].get_npc_name() << npc_page_numbers[i].get_file_name()
				<< " pg." << npc_page_numbers[i].get_page_number() << std::endl;
		}
	}

	out.close();
}

void swap(Initiative &x, Initiative &y)
{
	Initiative temp = x;
	x = y;
	y = temp;
}

void sort(vector<Initiative> &Initiatives)
{
	int n = Initiatives.size();
	int i;
	int j;
	bool swapped;

	for (i = 0; i < n - 1; i++) {
		swapped = false;
		for (j = 0; j < n - i - 1; j++)
			if (Initiatives[j].get_init() > Initiatives[j + 1].get_init()) {
				swap(Initiatives[j], Initiatives[j + 1]);
				swapped = true;
			}
		if (swapped == false)
			break;
	}

	// If a pc and npc are tied, put the pc first in the initiative list
	for (i = 0; i < n - 1; i++)	{
		if (Initiatives[i].get_init() == Initiatives[i + 1].get_init()
				&& (!Initiatives[i].get_status() && Initiatives[i + 1].get_status())) {
			swap(Initiatives[i], Initiatives[i + 1]);
		}
	}
}

int roll_init(int modifier) {
	int roll = (rand() % 20) + modifier + 1;
	return roll;
}

int roll_health(int num_dice, int die_value, int modifier) {
	int roll = 0;
	for (int j = 0; j < num_dice; j++) {
		roll += (rand() % die_value) + 1;
	}
	roll += modifier;
	return roll;
}

void alpha_sort(vector<Initiative> &Initiatives) {

	int n = Initiatives.size();
	int i;
	int j;
	bool swapped;
	for (i = 0; i < n; i++){
		swapped = false;
		for (j = 0; j < n - 1; j++)	{
			std::string temp1 = Initiatives[j].get_name();
			std::string temp2 = Initiatives[j+1].get_name();

			if (temp1 > temp2) {
				swap(Initiatives[j], Initiatives[j+1]);
				swapped = true;
			}
		}
		if (swapped == false)
			break;
	}
}
