// InternshipAssignmentNo4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <thread>
#include <map>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

using namespace std;

enum EventType { Created, Deleted, Edited, Renamed };

class Event
{
    EventType type;
    string FileLocation;
    bool FileorFolder;
public:
    Event(EventType eventT, string loc, bool isFolder)
    {
        type = eventT;
        FileLocation = loc;
        FileorFolder = isFolder;
    }

    string ToString()
    {
        string result;

        if (type == EventType::Created)
            result += " Created ";
        if (type == EventType::Deleted)
            result += " Deleted ";
        if (type == EventType::Renamed)
            result += " Renamed ";
        if (type == EventType::Edited)
            result += " Edited ";
        
        
        result += FileLocation;
        
        
        if (FileorFolder)
            result += " Folder";
        else 
            result += " File";

        return result;
    }
};

class ElementInFolder
{
public:
    filesystem::file_time_type timestamp;
    bool isFolder;
    string path;


    ElementInFolder() {}
    ElementInFolder(filesystem::file_time_type t, bool isf, string p)
    {
        timestamp = t;
        isFolder = isf;
        path = p;
    }
};

map<string, ElementInFolder> FilesTable;
map<string, ElementInFolder> FilesTableTemp;




map<string, ElementInFolder> GetFolderInfo(string Path)
{
    map<string, ElementInFolder> result;

    for (const auto& entry : fs::directory_iterator(Path))
    {
        bool isFolder = fs::is_directory(entry.path());
        filesystem::file_time_type timestamp = filesystem::last_write_time(entry.path());
        
        ElementInFolder el(timestamp, isFolder, entry.path().generic_string());
        
        result[entry.path().generic_string()] = el;

    }
    return result;
}

//Event (enum, string, bool)
ElementInFolder FindMissingEntry(map<string, ElementInFolder> big, map<string, ElementInFolder> small)
{
    for (auto const& x : big)
    {
        string currentKey = x.first.c_str();
        if (!small.contains(currentKey))
        {
            return x.second;
        }
    }
}


bool CheckIfChanged(map<string, ElementInFolder> table1, map<string, ElementInFolder> table2)
{
    std::ofstream LogFile("logs.txt", std::ios_base::app);
    
    if (table1.size() != table2.size())
    {
        if (table1.size() > table2.size())
        {
            ElementInFolder missingEntry = FindMissingEntry(table1, table2);

            Event deletedEvent(EventType::Deleted, missingEntry.path, missingEntry.isFolder);

            cout << deletedEvent.ToString() << endl;
            LogFile << deletedEvent.ToString() << endl;
        }
            
        if (table1.size() < table2.size())
        {
            ElementInFolder missingEntry = FindMissingEntry(table2, table1);
            Event createdEvent(EventType::Created, missingEntry.path, missingEntry.isFolder);
            cout << createdEvent.ToString() << endl;
            LogFile << createdEvent.ToString() << endl;

        }
        return true; // created/deleted
    }
    for (auto const& x : table1)
    {
        string currentKey = x.first.c_str();
        
        if (table2.contains(currentKey))
        {
            if (table1[currentKey].timestamp != table2[currentKey].timestamp)
            {
                Event editedEvent(EventType::Edited, currentKey, table1[currentKey].isFolder);
                cout << editedEvent.ToString() << "\n";
                LogFile << editedEvent.ToString() << endl;
                return true; //edited by timestamp
            }
            
        }
        else
        {
            Event renamedEvent(EventType::Renamed, currentKey, table1[currentKey].isFolder);
            cout << renamedEvent.ToString() << "\n";
            LogFile << renamedEvent.ToString() << endl;
            return true; //renamed
        }
    }
    LogFile.close();
    return false;
}

int main(int argc, char** argv)
{
    string path = argv[1];
    cout << "The path is :" << path << endl;

    FilesTable = GetFolderInfo(path);
    

    while (true)
    {
        FilesTableTemp = GetFolderInfo(path);

        if (CheckIfChanged(FilesTable, FilesTableTemp))
        {
            //cout << "The Folder Has Been Changed\n";

        }
        else
        {
            //cout << "nothing happend\n";
        }
        FilesTable = FilesTableTemp;
        this_thread::sleep_for(std::chrono::seconds(1));
    }

}

