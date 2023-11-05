#include<iostream>
#include<filesystem>
#include<string>
#include<vector>
#include<algorithm>
#include<fstream>
#include<chrono>
#include<map>
#include<regex>
#include<time.h>
#include<sstream>

using namespace std;
namespace fs = filesystem;

struct LogEntry {
    basic_string<char> timestamp;
    string severity;
    string message;
    string library;
};

map<string,int> severityCount;
map<string,int> libraryCount;

void DisplayLogEntry(const LogEntry& entry)
{
    cout<<"Timestamp: "<<entry.timestamp<<endl;
    cout<<"Severity: "<<entry.severity<<endl;
    cout<<"Library: ["<<entry.library<<"]"<<endl;
    cout<<"--------------------------------------------------------"<<endl;
}
void CalculateTimeDiffrence(const vector<LogEntry>& logs)
{
    tm tm1 = {0};
    istringstream ss1(logs.front().timestamp);

    ss1 >> tm1.tm_year;
    ss1.ignore();  // Ignorowanie znaku '-'
    ss1 >> tm1.tm_mon;
    ss1.ignore();
    ss1 >> tm1.tm_mday;
    ss1.ignore();
    ss1 >> tm1.tm_hour;
    ss1.ignore();
    ss1 >> tm1.tm_min;
    ss1.ignore();
    ss1 >> tm1.tm_sec;

    tm1.tm_year -= 1900;
    tm1.tm_mon -= 1;

    tm tm2 = {0};
    istringstream ss2(logs.back().timestamp);

    ss2 >> tm2.tm_year;
    ss2.ignore();
    ss2 >> tm2.tm_mon;
    ss2.ignore();
    ss2 >> tm2.tm_mday;
    ss2.ignore();
    ss2 >> tm2.tm_hour;
    ss2.ignore();
    ss2 >> tm2.tm_min;
    ss2.ignore();
    ss2 >> tm2.tm_sec;

    tm2.tm_year -= 1900;
    tm2.tm_mon -= 1;

    time_t time1 = mktime(&tm1);
    time_t time2 = mktime(&tm2);

    int timeDifference = difftime(time2, time1);
    timeDifference = abs(timeDifference);

    int days = timeDifference / (60 * 60 * 24);
    timeDifference -= days * 60 * 60 * 24;

    int hours = timeDifference / (60 * 60);
    timeDifference -= hours * 60 * 60;

    int minutes = timeDifference / 60;
    int seconds = timeDifference - minutes * 60;

    cout << "Roznica czasu miedzy pierwszym a ostatnim logiem: ";
    if (days > 0) {
        cout << days << " dni ";
    }
    if (hours > 0) {
        cout << hours << " godzin ";
    }
    if (minutes > 0) {
        cout << minutes << " minut ";
    }
    if (seconds > 0) {
        cout << seconds << " sekund";
    }
    cout << endl;
}
int main() {
    string logsDirectory = "D:/logs";
    if (fs::exists(logsDirectory) && fs::is_directory(logsDirectory)) {
        vector<fs::path> logFiles;
        vector<LogEntry> logs;
        for (const auto &entry: fs::directory_iterator(logsDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log")
                logFiles.push_back(entry.path());
        }
        sort(logFiles.begin(), logFiles.end(), [](const fs::path &a, const fs::path &b) {
            return fs::last_write_time(a) > fs::last_write_time(b);
        });
        if (logFiles.empty()) {
            cout << "Brak plikow z logami" << endl;
        } else {
            for (const auto &logFile: logFiles) {
                cout << logFile << endl;
                chrono::steady_clock::time_point start_time = chrono::steady_clock::now();
                logs.clear();
                ifstream file(logFile);
                if (file.is_open()) {
                    string line;
                    while (getline(file, line)) {
                        LogEntry currentEntry;
                        if (line.empty()) {
                            currentEntry = LogEntry();
                        } else {
                            regex timestampRegex("[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[1-2][0-9]|3[0-1]) (2[0-3]|[01][0-9]):[0-5][0-9]:[0-5][0-9],[0-9]{3}");
                            if (regex_search(line, timestampRegex)) {
                                currentEntry.timestamp = line.substr(0, 19);
                                if (line.find("[") != string::npos) {
                                    size_t severityPosB = line.find(" ", line.find(" ") + 1);
                                    size_t severityPosE = line.find(" ", severityPosB + 1);
                                    size_t libraryPosB = line.find("[", severityPosE) + 1;
                                    size_t libraryPosE = line.find("]", severityPosE);
                                    currentEntry.severity = line.substr(severityPosB + 1,severityPosE - severityPosB - 1);
                                    currentEntry.library = line.substr(libraryPosB, libraryPosE - libraryPosB);
                                    severityCount[currentEntry.severity]++;
                                    libraryCount[currentEntry.library]++;
                                    logs.push_back(currentEntry);
                                }
                            }
                        }
                    }
                    file.close();
                } else {
                    cout << "Nie można otworzyć pliku: " << logFile << endl;
                }
                chrono::steady_clock::time_point end_time = chrono::steady_clock::now();
                chrono::milliseconds elapsed_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
                cout << "--------------------------------------------------------" << endl;
                cout << "Czas odczytu pliku: " << elapsed_time.count() << " ms" << endl;
                cout << endl;
                CalculateTimeDiffrence(logs);
                int count = 0;
                int severityCountSize = 0;
                cout << "\nStatystyki logow:" << endl;
                for (const auto &severity: severityCount) {
                    cout << severity.first << ": " << severity.second << " logow" << endl;
                    if (severity.first == "ERROR" or severity.first == "FATAL")
                        count++;
                    severityCountSize += severity.second;
                }
                cout << "\nStosunek logow o severnity wyzszym badz rownym ERROR ";
                cout << fixed << setprecision(2) << float(count) / float(severityCountSize) * 100 << "%\n" << endl;
                cout << "Statystyki bibliotek:" << endl;
                for (const auto &library: libraryCount)
                    cout << library.first << ": " << library.second << endl;
                cout << "--------------------------------------------------------\n" << endl;
            }
        }
    }else
    {
        cout<<"Podany folder nie istnieje lub nie jest katalogiem"<<endl;
    }
    return 0;
}
