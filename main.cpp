#include <iostream>
#include<filesystem>
#include<string>
#include<vector>
#include <algorithm>
#include <fstream>
#include<ctime>
#include<chrono>
#include <map>
#include <regex>

using namespace std;
namespace fs = filesystem;
/*
 Wymagania dotyczące działania aplikacji:
1) po starcie szuka na dysku D: katalogu o nazwie 'logs',
2) po znalezieniu katalogu, aplikacja plik po pliku interpretuje pliki z logami w kolejności lastModified descending,
3) na koniec działania lub po każdym pliku aplikacja wylistowuje w konsoli:
- czas jaki upłynął na czytanie pliku,
- zakres logów w pliku (czyli różnica czasu miedzy pierwszym logiem w pliku a ostatnim),
- ilość logów pogrupowana wg severity (np. INFO, WARN, ERROR),
- stosunek ilości logów o severity ERROR lub wyższym do wszystkich logów,
- ilość unikalnych wystąpień bibliotek w logu (ta wartość w kwadratowych nawiasach zaraz po oznaczeniu severity, np:
[org.jboss.as.server]).

Dodatkowo zaproponuj strukturę tabeli bazodanowej, która mogłaby posłużyć do przechowywania logów znajdujących
się w pliku w bazie.
 */
struct LogEntry {
    string timestamp;
    string severity;
    string message;
    string library;
};

map<string,int> severityCount;

void ProcessLogEntry(const LogEntry& entry)
{
    cout<<"Timestamp: "<<entry.timestamp<<endl;
    severityCount[entry.severity]++;
    cout<<"Severity: "<<entry.severity<<endl;
    cout<<"Library: "<<entry.library<<endl;
    cout<<"Message: "<<entry.message<<endl;
    cout<<"--------------------------------------------------------"<<endl;

}
int main() {
    string logsDirectory = "D:/logs";
    vector<fs::path> logFiles;

    for (const auto &entry : fs::directory_iterator(logsDirectory)) {
        logFiles.push_back(entry.path());
    }

    sort(logFiles.begin(), logFiles.end(), [](const fs::path &a, const fs::path &b) {
        return fs::last_write_time(a) > fs::last_write_time(b);
    });

    for (const auto &logFile : logFiles) {
        cout << logFile << endl;
        cout << endl;

        chrono::steady_clock::time_point start_time = chrono::steady_clock::now();

        ifstream file(logFile);
        if (file.is_open()) {
            LogEntry currentEntry;
            string line;
            bool inEntry = false;

            while (getline(file, line)) {
                //cout << line << endl;
                if (!inEntry) {
                    //regex timestampRegex("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2},\\d{3}");
                    regex timestampRegex("[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[1-2][0-9]|3[0-1]) (2[0-3]|[01][0-9]):[0-5][0-9]:[0-5][0-9],[0-9]{3}");
                    if (regex_search(line, timestampRegex)) {
                        currentEntry.timestamp = line.substr(0, 23);
                        inEntry = true;
                    }
                } else {
                    if (line.empty()) {
                        ProcessLogEntry(currentEntry);
                        currentEntry = LogEntry();
                        inEntry = false;
                    } else if (line.find("]") != string::npos) {
                        size_t severityPos = line.find(" ",line.find(" ")+ 1);
                        size_t libraryPos = line.find("]", severityPos) + 1;
                        currentEntry.severity = line.substr(severityPos, libraryPos - severityPos);
                        currentEntry.library = line.substr(libraryPos);
                    } else {
                        currentEntry.message += line + '\n';
                    }
                }
            }
            if (!currentEntry.timestamp.empty()) {
                ProcessLogEntry(currentEntry);
            }
            file.close();
        } else {
            cout << "Nie można otworzyć pliku: " << logFile << endl;
        }
        chrono::steady_clock::time_point end_time = chrono::steady_clock::now();
        chrono::milliseconds elapsed_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

        cout << "Czas odczytu pliku: " << elapsed_time.count() << " ms" << endl;
        cout<<endl;
    }

        std::cout << "Statystyki:" << std::endl;
        for (const auto& severity : severityCount) {
            std::cout << severity.first << ": " << severity.second << " logow" << endl;
        }

    return 0;
}
