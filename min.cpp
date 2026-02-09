#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <limits>
#include <ctime>
#include <cstring>  
using namespace std;

// Base Class: Appliance (Abstract)
class Appliance {
protected:
    string name;
    float powerRating; 
    float hoursUsed;    
public:
    Appliance(const string& n, float w) : name(n), powerRating(w), hoursUsed(0.0f) {}
    virtual ~Appliance() {}

    void setUsage(float hours) { hoursUsed = hours; }
    string getName() const { return name; }
    float getHours() const { return hoursUsed; }
    float getPower() const { return powerRating; }

    virtual float calculateEnergy() const = 0;  
    virtual float calculateCost(float ratePerUnit) const {
        return calculateEnergy() * ratePerUnit;
    }

    virtual void display() const {
        cout << left << setw(20) << name
             << setw(10) << powerRating
             << setw(10) << hoursUsed
             << setw(12) << fixed << setprecision(2) << calculateEnergy()
             << endl;
    }

    virtual string getType() const = 0;
};

// Derived Classes
class Light : public Appliance {
public:
    Light(const string& n, float w) : Appliance(n, w) {}
    float calculateEnergy() const override { return (powerRating * hoursUsed) / 1000.0f; }
    string getType() const override { return "Light"; }
};

class Fan : public Appliance {
public:
    Fan(const string& n, float w) : Appliance(n, w) {}
    float calculateEnergy() const override { return (powerRating * hoursUsed) / 1000.0f; }
    string getType() const override { return "Fan"; }
};

class AirConditioner : public Appliance {
public:
    AirConditioner(const string& n, float w) : Appliance(n, w) {}
    float calculateEnergy() const override {
        return (powerRating * hoursUsed * 1.2f) / 1000.0f;
    }
    string getType() const override { return "AC"; }
};

class Fridge : public Appliance {
public:
    Fridge(const string& n, float w) : Appliance(n, w) {}
    float calculateEnergy() const override {
        return (powerRating * hoursUsed * 0.6f) / 1000.0f;
    }
    string getType() const override { return "Fridge"; }
};

 
static bool iequals(const string& a, const string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
    return true;
}

// Manager Class: EnergyTracker
class EnergyTracker {
private:
    vector<Appliance*> appliances;
    float ratePerUnit; // ₹ per kWh
    const string dataFile = "appliances.txt";
    const string logFile = "operations.txt";

    
    void logOperation(const string& msg) const {
        ofstream ofs(logFile, ios::app);
        if (!ofs) return;
        time_t now = time(nullptr);

        tm localTm;
        memset(&localTm, 0, sizeof(localTm));
        tm* tmptr = std::localtime(&now);  
        if (tmptr) {
            localTm = *tmptr;  
        }

        char buf[64];
        if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &localTm)) {
            ofs << buf << " - " << msg << '\n';
        } else {
            ofs << "unknown-time - " << msg << '\n';
        }
    }

    void saveToFile() const {
        ofstream ofs(dataFile);
        if (!ofs) {
            cerr << "Error: could not open file for saving: " << dataFile << endl;
            return;
        }
        
        for (auto* app : appliances) {
            ofs << app->getType() << '|' 
                << app->getName() << '|' 
                << app->getPower() << '|' 
                << app->getHours() << '\n';
        }
    }

    void loadFromFile() {
        ifstream ifs(dataFile);
        if (!ifs) return;   
        string type, name, wattsStr, hoursStr;
        string line;
        while (getline(ifs, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            if (!getline(ss, type, '|')) continue;
            if (!getline(ss, name, '|')) continue;
            if (!getline(ss, wattsStr, '|')) continue;
            if (!getline(ss, hoursStr)) continue;

            float watts = 0.0f, hours = 0.0f;
            try {
                watts = stof(wattsStr);
                hours = stof(hoursStr);
            } catch (...) {
                continue;
            }

            Appliance* app = nullptr;
            if (iequals(type, "Light")) app = new Light(name, watts);
            else if (iequals(type, "Fan")) app = new Fan(name, watts);
            else if (iequals(type, "AC") || iequals(type, "AirConditioner")) app = new AirConditioner(name, watts);
            else if (iequals(type, "Fridge")) app = new Fridge(name, watts);

            if (app) {
                app->setUsage(hours);
                appliances.push_back(app);
            }
        }
    }

public:
    EnergyTracker(float rate = 8.0f) : ratePerUnit(rate) {
        loadFromFile();
    }

    void addAppliance() {
        string type, name;
        float watt;

        cout << "\nEnter appliance type (Light/Fan/AC/Fridge): ";
        cin >> type;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter appliance name: ";
        getline(cin, name);
        cout << "Enter power rating (in watts): ";
        if (!(cin >> watt)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid watt input. Aborting add.\n";
            return;
        }

        Appliance* newApp = nullptr;
        if (iequals(type, "Light")) newApp = new Light(name, watt);
        else if (iequals(type, "Fan")) newApp = new Fan(name, watt);
        else if (iequals(type, "AC") || iequals(type, "AirConditioner")) newApp = new AirConditioner(name, watt);
        else if (iequals(type, "Fridge")) newApp = new Fridge(name, watt);

        if (!newApp) {
            cout << "Invalid appliance type!\n";
            return;
        }

        appliances.push_back(newApp);
        saveToFile();
         
        {
            ostringstream oss;
            oss << "Added appliance: " << newApp->getType() << " | " << newApp->getName()
                << " | " << newApp->getPower() << " W";
            logOperation(oss.str());
        }
        cout << "Appliance added successfully!\n";
    }

    void recordUsage() {
        string name;
        float hours;
        cout << "\nEnter appliance name to record usage: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, name);

        for (auto* app : appliances) {
            if (app->getName() == name) {
                cout << "Enter usage hours for " << name << ": ";
                if (!(cin >> hours)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid hours input. Aborting.\n";
                    return;
                }
                app->setUsage(hours);
                saveToFile();
                 
                {
                    ostringstream oss;
                    oss << "Recorded usage: " << app->getName() << " -> " << hours << " hours";
                    logOperation(oss.str());
                }
                cout << "Usage recorded!\n";
                return;
            }
        }
        cout << "Appliance not found!\n";
    }

    void showAll() const {
        cout << "\n--- Appliance Summary ---\n";
        cout << left << setw(20) << "Name"
             << setw(10) << "Watts"
             << setw(10) << "Hours"
             << setw(12) << "Energy(kWh)"
             << endl;
        cout << "------------------------------------------------\n";

        for (auto* app : appliances)
            app->display();
    }

    void generateBill() const {
        float totalEnergy = 0.0f;
        float totalCost = 0.0f;

        cout << "\n--- Energy Usage & Bill ---\n";

         
        ostringstream billLog;
        billLog << "Generated bill details:\n";

        for (auto* app : appliances) {
            float e = app->calculateEnergy();
            float c = app->calculateCost(ratePerUnit);
            cout << left << setw(20) << app->getName()
                 << " -> " << fixed << setprecision(2) << e << " kWh, Cost: ₹" << c << endl;
            totalEnergy += e;
            totalCost += c;

            billLog << "  " << app->getName() << " : " << fixed << setprecision(2) << e << " kWh, Cost ₹" << c << '\n';
        }

        cout << "\n---------------------------------\n";
        cout << "Total Appliances: " << appliances.size() << endl;
        cout << "Total Energy: " << totalEnergy << " kWh" << endl;
        cout << "Estimated Bill: ₹" << totalCost << endl;

        billLog << "TOTAL Energy: " << fixed << setprecision(2) << totalEnergy << " kWh, TOTAL Cost ₹" << totalCost;
        logOperation(billLog.str());
    }

    ~EnergyTracker() {
        for (auto* app : appliances)
            delete app;
        appliances.clear();
    }
};

// Main Function (Menu-driven)
int main() {
    EnergyTracker tracker;
    int choice;

    do {
        cout << "\n========== Smart Home Energy Tracker ==========\n";
        cout << "1. Add Appliance\n";
        cout << "2. Record Usage\n";
        cout << "3. Show All Appliances\n";
        cout << "4. Generate Bill\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }

        switch (choice) {
        case 1: tracker.addAppliance(); break;
        case 2: tracker.recordUsage(); break;
        case 3: tracker.showAll(); break;
        case 4: tracker.generateBill(); break;
        case 5: cout << "Exiting... Goodbye!\n"; break;
        default: cout << "Invalid choice! Try again.\n"; break;
        }
    } while (choice != 5);

    return 0;
}
