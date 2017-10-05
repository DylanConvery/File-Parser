#include <iostream>
#include <vector>
#include <string>
#include <fstream>

const int not_a_reading = -7777;
const int not_a_month = -1;

constexpr int implausible_min = -200;
constexpr int implausible_max = 200;

std::vector <std::string> month_input_table
{
	"jan", "feb", "mar", "apr", "may", "jun",
	"jul", "aug", "sep", "oct", "nov", "dec"
};

std::vector <std::string> month_print_table
{
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};

struct Day {
	std::vector<double> hour{ std::vector<double>(24,not_a_reading) };
};

struct Month {
	int month{ not_a_month };
	std::vector<Day> day{ 32 };
};

struct Year {
	int year;
	std::vector<Month> month{ 12 };
};

struct Reading {
	int day;
	int hour;
	double temperature;
};

std::istream &operator >> (std::istream &, Month &);
std::istream &operator >> (std::istream &, Year &);
std::istream &operator >> (std::istream &, Reading &);

void end_of_loop(std::istream &, char, const std::string &);
bool is_valid(const Reading &);
int month_to_int(std::string);
std::string int_to_month(int);

void fill(std::vector<Year> &, std::istream &);

void print_years(std::ostream &, const Year &);
void print_month(std::ostream &, const Month &, const int &);
void print_day(std::ostream &, const Day &, const int &, const int &, const int &);

std::istream &operator >> (std::istream &is, Month &m) {
	char ch = 0;
	if (is >> ch && ch != '{') {
		is.unget();
		is.clear(std::ios_base::failbit);
		return is;
	}

	std::string month_marker;
	std::string mm;

	is >> month_marker >> mm;
	if (!is || month_marker != "month") {
		throw std::runtime_error("bad start of month");
	}
	m.month = month_to_int(mm); // convert "whatever month e.g: feb" to its corrisponding month value starting from 0:11

	int duplicates = 0;
	int invalids = 0;

	for (Reading r; is >> r;) {
		if (is_valid(r)) { // check if the reading was valid
			/*/////////////////////////////////////////////////
			//                                               //
			//  if month day in slot [r.day]'s hour in slot  //
			//  [r.hour] != not_a_reading it's been written  //
			//  to before and is therefore a duplicate       //
			//                                               //
			/////////////////////////////////////////////////*/
			if (m.day[r.day].hour[r.hour] != not_a_reading) {
				++duplicates;
			}
			m.day[r.day].hour[r.hour] = r.temperature;
		}
		else {
			++invalids;
		}
	}
	if (invalids) { // if there's any invalids, throw an exception
		throw std::runtime_error("invalid readings from month " + invalids);
	}
	if (duplicates) { // if there's any duplicates, throw an exception
		throw std::runtime_error("duplicate readings in month " + duplicates);
	}
	end_of_loop(is, '}', "bad end of month");
	return is;
}

std::istream &operator >> (std::istream &is, Year &y) {
	char ch;
	is >> ch;
	if (ch != '{') {
		is.unget();
		is.clear(std::ios_base::failbit);
		return is;
	}

	std::string year_marker;
	int yy;

	is >> year_marker >> yy;
	if (!is || year_marker != "year") {
		throw std::runtime_error("bad start of year");
	}
	y.year = yy;

	while (true) {
		Month m; // we do this rather than a forloop because m would only be initialized once
		if (!(is >> m)) {
			break;
		}
		y.month[m.month] = m; // years month at [m.month](it's converted to an int with month_to_int()) = m
	}
	end_of_loop(is, '}', "bad end of year");
	return is;
}


std::istream &operator >> (std::istream &is, Reading &r) {
	char ch1;
	if (is >> ch1 && ch1 != '(') {
		is.unget();
		is.clear(std::ios_base::failbit);
		return is;
	}

	char ch2;
	int d, h;
	double t;

	is >> d >> h >> t >> ch2;
	if (!is || ch2 != ')') {
		throw std::runtime_error("bad reading");
	}
	r.day = d;
	r.hour = h;
	r.temperature = t;
	return is;
}

void end_of_loop(std::istream &is, char term, const std::string &message) {
	if (is.fail()) {
		is.clear();
		char c;
		if (is >> c && c == term) {
			return;
		}
		throw std::runtime_error(message);
	}
}

bool is_valid(const Reading &r) {
	if (r.day < 1 || r.day > 31) return false;
	if (r.hour < 0 || r.hour > 24) return false;
	if (r.temperature < implausible_min || r.temperature > implausible_max) return false;
	return true;
}

int month_to_int(std::string s) {
	for (int i = 0; i < 12; ++i) {
		if (month_input_table[i] == s) {
			return i;
		}
	}
	return -1;
}

std::string int_to_month(int i) {
	if (i < 0 || i >= 12) {
		throw std::runtime_error("bad month index");
	}
	return month_print_table[i];
}

void print_day(std::ostream &os, const Day &d, const int &the_day, const int &the_month, const int &the_year) {
	//Only prints when a day has a reading, otherwise ignores entries in input file
	for (int i = 0; i < 24; ++i) {
		if (d.hour[i] != not_a_reading) {
			os << "On the " << the_day;

			if (the_day % 10 == 1) { os << "st "; }
			else if (the_day % 10 == 2) { os << "nd "; }
			else if (the_day % 10 == 3) { os << "rd "; }
			else { os << "th "; }

			os << "of ";
			os << int_to_month(the_month) << " ";
			os << the_year << " ";
			os << "at " << i << ":00 ";
			os << "it was ";
			os << d.hour[i] << "\260\n";
		}
	}
}

void print_month(std::ostream &os, const Month &m, const int &year) {
	for (int i = 0; i < 32; ++i) {
		if (m.month != not_a_month) {
			print_day(os, m.day[i], i, m.month, year);
		}
	}
}

void print_years(std::ostream &os, const Year &y) {
	for (int i = 0; i < 12; ++i) {
		print_month(os, y.month[i], y.year);
	}
}

void fill(std::vector<Year> &years, std::istream &is) {
	while (true) {
		Year y;
		if (!(is >> y))break;
		years.push_back(y);
	}
}

int main() {
	try {
		std::cout << "Please enter input file name\n";
		std::string iname;
		std::cin >> iname;
		std::ifstream ifs{ iname };
		if (!ifs) {
			throw std::runtime_error("can't open input file " + iname);
		}

		ifs.exceptions(ifs.exceptions() | std::ios_base::badbit);

		std::cout << "Please enter an output file name\n";
		std::string oname;
		std::cin >> oname;
		std::ofstream ofs{ oname };
		if (!ofs) {
			throw std::runtime_error("can't open output file " + oname);
		}

		std::vector<Year> years;
		fill(years, ifs);
		std::cout << "read " << years.size() << " years of readings\n";

		for (Year &y : years) {
			print_years(ofs, y);
		}
	}
	catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
	}

	system("PAUSE");
	return 0;
}
