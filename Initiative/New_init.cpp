#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <time.h>
#include <map>
#include <algorithm>

bool DEBUGMODE;
const int num_files = 4;
using std::cout;
using std::vector;
using std::string;

// holds initiative count, health (if npc), if entity is an npc, and entity's name
class Initiative {
private:
	int initiative;
	int health;
	bool is_npc;
	string name;

public:
	Initiative() : initiative(0), health(0), is_npc(false), name("")
	{/* Default constructor */}
	Initiative(int init, int h, bool npc, string n)
	 					 	 : initiative(init), health(h), is_npc(npc),  name(n)
	{/* Intentionally empty */}

	int get_health() const { return health; }

	int get_init() const { return initiative; }

	string get_name() const { return name; }

	bool get_status() const { return is_npc; }

};

// holds strings of npc's name, the book it is found in, and what page of that book
class npc_page_number {
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

	string get_npc_name() const { return npc_name; }

	string get_file_name()	const { return file_name; }

	string get_page_number()	const { return page_number; }
};

void sort(vector<Initiative> &Initiatives);
void alpha_sort(vector<Initiative> &Initiatives);
string trim(const string &line);
int roll_init(int modifier);
int roll_health(int num_dice, int die_value, int modifier);
void get_pcs(vector<Initiative> &Initiatives);
void get_npcs(vector<Initiative> &Initiatives, vector<npc_page_number> &npc_page_numbers,
					const vector<std::map<string,string>> &Stats_map_vec);
void display_Initiatives(vector<Initiative> &Initiatives,
					const vector<npc_page_number> &npc_page_numbers, int max_length);
void write_to_file(vector<Initiative> Initiatives,
					const vector<npc_page_number> &npc_page_numbers, int max_length);
string look_for_stats(string &line, const vector<std::map<string,string>> &Stats_map_vec);

int main(int argc, char** argv)	{
	DEBUGMODE = false;
	int i;
	for(i = 0; i < argc; i++)
		if(strcmp(argv[i], "-d") == 0)
			DEBUGMODE = true;

	// Convert text files to a 2D vector of strings
	vector<string> Stats;	// only used for debugging
	std::ifstream Stat_file;
	string line;
	string file_arr[num_files] = { "MMstats.csv", "GoSstats.csv", "Volostats.csv", "ToAstats.csv"};

	// vector of map<string, string>'s. Each element comes from a different file
	// the first string is the mob's name (in lower case), the second string is the whole stat line
	vector<std::map<string, string>> Stats_map_vec;
	std::map<string, string> Stats_map;
	int found;	// index of first ',' in file line

	for (i = 0; i < num_files; i++) {
		Stat_file.open(file_arr[i]);

		while(!Stat_file.eof()) {
			getline(Stat_file, line);
			Stats.push_back(line);

			found = line.find_first_of(",");
			string name_string(line, 0, found);
			// transform name into lower case and store it as the key
			std::transform(name_string.begin(), name_string.end(), name_string.begin(),
											[](char c) { return std::tolower(c);});
			Stats_map.insert(std::pair<string, string>(name_string, line));
		}

		if (DEBUGMODE) {
			cout << file_arr[i] << ":\n";
			for(const string& str : Stats) {
				cout << str << std::endl;
			}
		}

		Stats.clear();
		Stats_map_vec.push_back(Stats_map);
		Stats_map.clear();
		Stat_file.close();
	}

	vector<Initiative> Initiatives;	// Holds npc and pc names and initiatives
	vector<npc_page_number> npc_page_numbers;	// Holds file name and page number for each npc type
	int num_npcs;
	int num_pcs;
	int max_length;			// Length of longest name
	char answer;

	srand(time(NULL));	// Initialize random seed


	cout << "Write to file? (y/n): ";
	answer = std::cin.get();
	std::cin.sync();
	std::cin.ignore(256, '\n');	// get rid of any extra characters
	cout << std::endl;

	get_pcs(Initiatives);

	cout << "\nEnter number and names of npc's: \n";

	get_npcs(Initiatives, npc_page_numbers, Stats_map_vec);

	sort(Initiatives);

	// Find length of longest character name (for formatting)
	max_length = 0;
	for (const Initiative& Init : Initiatives) {
		if (Init.get_name().length() > max_length) {
			max_length = Init.get_name().length();
		}
	}


	if (answer == 'y')
		write_to_file(Initiatives, npc_page_numbers, max_length);

		display_Initiatives(Initiatives, npc_page_numbers, max_length);


	return 0;
}

void get_pcs(vector<Initiative> &Initiatives) {
	string init, name, line;
	int tempInit, i = 0;

	while(1) {
		cout << "Enter character #" << i + 1 << "'s roll: ";
		std::getline(std::cin, line);

		// All characters before a digit will be the name (trimming whitespace later)
		auto iter = line.begin();
		for (; iter != line.end() && !(isdigit(*iter)); ++iter) {
			name.push_back(*iter);
		}
		// Roll is all digits after the name
		for (; iter != line.end(); ++iter) {
			if(isdigit(*iter))
				init.push_back(*iter);
		}
		if (init.size() == 0 || name.size() == 0) return; // no name or initiative was entered

		name = trim(name);
		tempInit = std::stoi(init);
		Initiative temp(tempInit, 0, false, name);
		Initiatives.push_back(temp);

		init.clear();
		name.clear();
		i++;
	}
}

void get_npcs(vector<Initiative> &Initiatives, vector<npc_page_number> &npc_page_numbers,
							const vector<std::map<string,string>> &Stats_map_vec)
{
	string line;
	int num_npcs;
	string file_name;

	// loop as long as the user enters num_pcs then the type of mob (i.e. 2 zombie)
	while (std::cin >> num_npcs) {
		std::getline(std::cin, line);
		line = trim(line);

		vector<string> line_vec;

		for (int i = 0; i < num_npcs; i++) {
			if (i == 0) { // only check for first type of that npc
				string page_number("");
				if((file_name = look_for_stats(line, Stats_map_vec)) == "") {
					cout << line << " not found -- enter stat line: (init,num_health_dice,dice_val,health_mod)\n";
					string stats;
					std::getline(std::cin, stats);
					line = line + "," + stats;
				}
				if (DEBUGMODE)
					cout << line << std::endl;

			std::stringstream stream (line);
			string data = "";

			// line_vec[0] => name 						// line_vec[1] => init modifier
			// line_vec[2] => num health dice // line_vec[3] => value of health dice
			// line_vec[4] => health mod 			// line_vec[5] => page number *NOT ALWAYS PRESENT*
			while (std::getline(stream, data, ',' )) {
					line_vec.push_back(data);
				}
			}

			if (DEBUGMODE)
				cout << "Trying to initailize mob " << i << std::endl;
			string tempName = line_vec[0];
			if (num_npcs > 1) { // append mob number if more than one of them
				tempName += std::to_string(i+1);
			}
			int tempInit = roll_init(std::stoi(line_vec[1]));
			int tempHealth = roll_health(std::stoi(line_vec[2]), std::stoi(line_vec[3]), std::stoi(line_vec[4]));
			Initiative temp_init(tempInit, tempHealth, true, tempName);
			Initiatives.push_back(temp_init);

			// Check if page number is present
			// only add one page_number per type of mob
			if (line_vec.size() == 6 && i == 0) {
				npc_page_number temp_page(line_vec[0], file_name, line_vec[5]);
				npc_page_numbers.push_back(temp_page);
			}
		}	// end for
	}	// end while
}

string look_for_stats(string &line, const vector<std::map<string,string>> &Stats_map_vec)
{
	string new_name = line;	// holds lower case version of line
	std::transform(new_name.begin(), new_name.end(), new_name.begin(),
									[](char c) { return std::tolower(c);});
	if(DEBUGMODE)
		cout << "new_name: " << new_name << std::endl;

	for(int i = 0; i < Stats_map_vec.size(); i++) {
		auto stat = Stats_map_vec[i].find(new_name);
		if (stat != Stats_map_vec[i].end()) { // name was found, get what book it's from
			if(DEBUGMODE)
				cout << "Found the stat block: " << stat->second << std::endl;
			line = stat->second;
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
	return "";	// return empty string if mob name was not found
}

void display_Initiatives(vector<Initiative> &Initiatives,
							const vector<npc_page_number>& npc_page_numbers, int max_length)
{
	cout << "\nCombat order is: \n";
	for (const Initiative& Init : Initiatives) {
		if (DEBUGMODE)
			cout << "looking at " << Init.get_name() << std::endl;
		cout << std::setfill('.') << std::setw(max_length + 3) << std::left			// Format with charcter name and roll seperated by periods
			<< Init.get_name() << Init.get_init() << std::endl;
	}

	alpha_sort(Initiatives);
	cout << "\n\n";
	cout << std::setfill('.') << std::setw(max_length + 3) << std::left << "NPCs" << "HP\n";
	// Display npc healths
	for (const Initiative& Init : Initiatives) {
		if (DEBUGMODE)
			cout << "looking at " << Init.get_name() << std::endl;
		if (Init.get_status() )	{
			cout << std::setfill('.') << std::setw(max_length + 3) << std::left
				<< Init.get_name() << Init.get_health() << std::endl;
		}
	}

	cout << "\n\n";
	cout << std::setfill('.') << std::setw(max_length + 3) << std::left << "NPC" << "Page number\n";
	for (const npc_page_number& npc : npc_page_numbers) {
		if(npc.get_page_number() != "") {	// Only display page number if there is one
			cout << std::setfill('.') << std::setw(max_length + 3) << std::left			// Format with charcter name and roll seperated by periods
				<< npc.get_npc_name() << npc.get_file_name()
				<< " pg." << npc.get_page_number() << std::endl;
		}
	}
}

void write_to_file(vector<Initiative> Initiatives,
				const vector<npc_page_number>& npc_page_numbers, int max_length)
{
	std::ofstream out;

	out.open("Init.txt");

	// Put initiative order into text file
	out << "Combat order is: \n";
	for (const Initiative& Init : Initiatives) {
		if (DEBUGMODE)
			out << "looking at " << Init.get_name() << std::endl;
		out << std::setfill('.') << std::setw(max_length + 3) << std::left			// Format with charcter name and roll seperated by periods
			<< Init.get_name() << Init.get_init() << std::endl;
	}

	alpha_sort(Initiatives);
	out << "\n\n";
	out << std::setfill('.') << std::setw(max_length + 3) << std::left << "NPCs" << "HP\n";
	// Display npc healths
	for (const Initiative& Init : Initiatives) {
		if (DEBUGMODE)
			out << "looking at " << Init.get_name() << std::endl;
		if (Init.get_status())	{
			out << std::setfill('.') << std::setw(max_length + 3) << std::left
				<< Init.get_name() << Init.get_health() << std::endl;
		}
	}

	out << "\n\n";
	out << std::setfill('.') << std::setw(max_length + 3) << std::left << "NPC" << "Page number\n";

	for (const npc_page_number& npc : npc_page_numbers) {
		if(npc.get_page_number() != ""){	// Only display page number if there is one
			out << std::setfill('.') << std::setw(max_length + 3) << std::left			// Format with charcter name and roll seperated by periods
				<< npc.get_npc_name() << npc.get_file_name() << " pg." << npc.get_page_number() << std::endl;
		}
	}

	out.close();
}

// get rid of spaces around a string
string trim(const string& line) {
		size_t first = line.find_first_not_of(' ');
    if (first == string::npos) { // no spaces found
        return line;
    }
    size_t last = line.find_last_not_of(' ');
    return line.substr(first, (last - first + 1));
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

void sort(vector<Initiative> &Initiatives) {
	std::sort(Initiatives.begin(), Initiatives.end(),
						[](const Initiative& a, const Initiative& b) {
								if (a.get_init() > b.get_init())
									return true;
								else if (a.get_init() < b.get_init())
									return false;
								else { // inits are equal => put players first
									if (!a.get_status() && b.get_status())
										return true;
									}
								return false; });
}

void alpha_sort(vector<Initiative> &Initiatives) {
	std::sort(Initiatives.begin(), Initiatives.end(),
						[](const Initiative& a, const Initiative& b) {
								if (a.get_name() > b.get_name())
									return false;
								else
									return true; });
}
