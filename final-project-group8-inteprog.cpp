#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm> // For std::transform, std::find, std::remove_if, std::find_if
#include <limits>    // For std::numeric_limits
#include <map>       // For inventory allocation in events
#include <locale>    // For std::locale
#include <iomanip>   // For std::setw, std::left, std::right

// Add this line to use the std namespace
using namespace std;

// Forward declarations
class User;
class Admin;
class RegularUser;
class Event;
class Attendee;
class InventoryItem;
class System; // System is now a singleton

// --- Enums ---
enum class Role { ADMIN, REGULAR_USER, NONE };
enum class EventStatus { UPCOMING, ONGOING, COMPLETED, CANCELED };

// --- Helper Functions ---

// Function to convert string to lowercase
string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(),
                     [](unsigned char c){ return tolower(c); });
    return s;
}

// Function to get validated string input (ensures not empty)
string getStringInput(const string& prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
        if (!input.empty()) {
            return input;
        }
        cout << "Input cannot be empty. Please try again.\n";
    }
}

// Function to get validated integer input
int getIntInput(const string& prompt) {
    int input;
    while (true) {
        cout << prompt;
        cin >> input;
        if (cin.good()) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
            return input;
        }
        cout << "Invalid input. Please enter an integer.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Function to get validated positive integer input
int getPositiveIntInput(const string& prompt) {
    int input;
    while (true) {
        input = getIntInput(prompt);
        if (input > 0) {
            return input;
        }
        cout << "Input must be a positive integer. Please try again.\n";
    }
}

// Basic date validation (format-MM-DD)
bool isValidDate(const string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    try {
        int year = stoi(date.substr(0, 4));
        int month = stoi(date.substr(5, 2));
        int day = stoi(date.substr(8, 2));
        if (year < 1900 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        // Basic day check (not accounting for month/leap year complexities)
        if (day < 1 || day > 31) return false;
    } catch (const exception&) {
        return false;
    }
    return true;
}

// Basic time validation (format HH:MM - 24-hour)
bool isValidTime(const string& time) {
    if (time.length() != 5) return false;
    if (time[2] != ':') return false;
      try {
        int hour = stoi(time.substr(0, 2));
        int minute = stoi(time.substr(3, 2));
        if (hour < 0 || hour > 23) return false;
        if (minute < 0 || minute > 59) return false;
    } catch (const exception&) {
        return false;
    }
    return true;
}


// --- Class Definitions ---

// ** User Class (Abstract Base Class) **
class User {
protected:
    string username;
    string password;
    Role role;
    int userId;
    static int nextUserId;

public:
    User(string uname, string pwd, Role r);
    User(int id, string uname, string pwd, Role r);
    virtual ~User();

    string getUsername() const { return username; }
    string getPassword() const { return password; }
    Role getRole() const { return role; }
    int getUserId() const { return userId; }

    void setPassword(const string& newPassword);

    virtual void displayMenu(System& sys) = 0; // Pure virtual
    virtual void displayDetails() const = 0; // Pure virtual for displaying user-specific details
    virtual string toString() const;
    static User* fromString(const string& str); // Definition after Admin/RegularUser
    static void initNextId(int id) { if (id >= nextUserId) nextUserId = id + 1;}
};
int User::nextUserId = 1;


// ** Admin Class **
class Admin : public User {
public:
    Admin(string uname, string pwd);
    Admin(int id, string uname, string pwd);
    void displayMenu(System& sys) override;
    void displayDetails() const override; // Implementation for Admin
private:
    void adminUserManagementMenu(System& sys);
    void adminEventManagementMenu(System& sys);
    void adminAttendeeManagementMenu(System& sys);
    void adminInventoryManagementMenu(System& sys);
    void adminDataExportMenu(System& sys);
};

// ** RegularUser Class **
class RegularUser : public User {
public:
    RegularUser(string uname, string pwd);
    RegularUser(int id, string uname, string pwd);
    void displayMenu(System& sys) override;
    void displayDetails() const override; // Implementation for RegularUser
};


// ** Attendee Class **
class Attendee {
public:
    int attendeeId;
    string name;
    string contactInfo;
    int eventIdRegisteredFor;
    bool isCheckedIn;
    static int nextAttendeeId;

    Attendee(string n, string contact, int eventId);
    Attendee(int id, string n, string contact, int eventId, bool checkedInStatus);
    void checkIn();
    void displayDetails() const;
    string toString() const;
    static Attendee fromString(const string& str);
    static void initNextId(int id) { if (id >= nextAttendeeId) nextAttendeeId = id + 1;}
};
int Attendee::nextAttendeeId = 1;

// ** InventoryItem Class **
class InventoryItem {
public:
    int itemId;
    string name;
    int totalQuantity;
    int allocatedQuantity;
    string description;
    static int nextItemId;

    InventoryItem(string n, int qty, string desc);
    InventoryItem(int id, string n, int totalQty, int allocQty, string desc);
    int getAvailableQuantity() const;
    bool allocate(int quantityToAllocate);
    bool deallocate(int quantityToDeallocate);
    void setTotalQuantity(int newTotalQuantity);
    void displayDetails() const;
    string toString() const;
    static InventoryItem fromString(const string& str);
    static void initNextId(int id) { if (id >= nextItemId) nextItemId = id + 1;}
};
int InventoryItem::nextItemId = 1;

// ** Event Class **
class Event {
public:
    int eventId;
    string name;
    string date;
    string time;
    string location;
    string description;
    string category;
    EventStatus status;
    vector<int> attendeeIds;
    map<int, int> allocatedInventory;
    static int nextEventId;

    Event(string n, string d, string t, string loc, string desc, string cat);
    Event(int id, string n, string d, string t, string loc,
           string desc, string cat, EventStatus stat);
    void addAttendee(int attendeeId);
    void removeAttendee(int attendeeId);
    void allocateInventoryItem(int itemId, int quantity);
    int deallocateInventoryItem(int itemId, int quantityToDeallocate);
    string getStatusString() const;
    void displayDetails(const System& sys) const; // Definition after System
    string attendeesToString() const;
    string inventoryToString() const;
    string toString() const;
    static Event fromString(const string& str);
    static void initNextId(int id) { if (id >= nextEventId) nextEventId = id + 1;}
};
int Event::nextEventId = 1;


// --- Strategy Pattern: Exporting Data ---
// Abstract base class for export strategies
class IExportStrategy {
public:
    virtual ~IExportStrategy() = default;
    virtual void exportUsers(const vector<User*>& users, const string& filename) const = 0;
    virtual void exportEvents(const vector<Event>& events, const string& filename, const System& sys) const = 0;
    virtual void exportAttendees(const vector<Attendee>& attendees, const string& filename) const = 0;
    virtual void exportInventory(const vector<InventoryItem>& inventory, const string& filename) const = 0;
};

// Concrete strategy for text file export
class TextExportStrategy : public IExportStrategy {
public:
    void exportUsers(const vector<User*>& users, const string& filename) const override {
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "Error: Could not open " << filename << " for writing.\n";
            return;
        }
        for (const auto* user : users) {
            if (user) {
                outFile << user->toString() << endl;
            }
        }
        outFile.close();
        cout << "Users data exported to " << filename << endl;
    }

    void exportEvents(const vector<Event>& events, const string& filename, const System& sys) const override {
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "Error: Could not open " << filename << " for writing.\n";
            return;
        }
        for (const auto& event : events) {
            outFile << event.toString() << endl;
        }
        outFile.close();
        cout << "Events data exported to " << filename << endl;
    }

    void exportAttendees(const vector<Attendee>& attendees, const string& filename) const override {
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "Error: Could not open " << filename << " for writing.\n";
            return;
        }
        for (const auto& attendee : attendees) {
            outFile << attendee.toString() << endl;
        }
        outFile.close();
        cout << "Attendees data exported to " << filename << endl;
    }

    void exportInventory(const vector<InventoryItem>& inventory, const string& filename) const override {
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "Error: Could not open " << filename << " for writing.\n";
            return;
        }
        for (const auto& item : inventory) {
            outFile << item.toString() << endl;
        }
        outFile.close();
        cout << "Inventory data exported to " << filename << endl;
    }
};


// ** System Class (Singleton) **
class System {
private:
    // Singleton instance
    static System* instance;

    // Private constructor for Singleton
    System() : currentUser(nullptr), exportStrategy(new TextExportStrategy()) {}

    // Delete copy constructor and assignment operator to prevent copying
    System(const System&) = delete;
    System& operator=(const System&) = delete;

    IExportStrategy* exportStrategy; // Member for the chosen export strategy

public:
    // Public static method to get the single instance of System
    static System& getInstance() {
        if (instance == nullptr) {
            instance = new System();
        }
        return *instance;
    }

    // Public static method to clean up the singleton instance
    static void destroyInstance() {
        delete instance;
        instance = nullptr;
    }

    // Destructor to clean up resources (including export strategy)
    ~System();

    // Data members
    vector<User*> users;
    vector<Event> events;
    vector<InventoryItem> inventory;
    vector<Attendee> allAttendees;
    User* currentUser;

    // File names
    const string USERS_FILE = "users.txt";
    const string EVENTS_FILE = "events.txt";
    const string INVENTORY_FILE = "inventory.txt";
    const string ATTENDEES_FILE = "attendees.txt";

    // Data loading and saving methods (now use export strategy internally for saving)
    void loadData();
    void saveData(); // Calls individual save methods which use the strategy
    void loadUsers();
    void saveUsers(); // Uses exportStrategy
    void loadEvents();
    void saveEvents(); // Uses exportStrategy
    void loadInventory();
    void saveInventory(); // Uses exportStrategy
    void loadAttendees();
    void saveAttendees(); // Uses exportStrategy

    // User management
    bool usernameExists(const string& uname) const;
    void createUserAccount(const string& uname, const string& pwd, Role role);
    void publicRegisterNewUser();
    void deleteUserAccount(const string& uname);
    User* findUserByUsername(const string& uname);
    const User* findUserByUsername(const string& uname) const;
    void listAllUsers() const;

    // Authentication
    bool login();
    void logout();

    // Event management
    Event* findEventById(int eventId);
    const Event* findEventById(int eventId) const;
    void createEvent();
    void viewAllEvents(bool adminView = false) const;
    void searchEventsByNameOrDate() const;
    void editEventDetails();
    void deleteEvent();
    void updateEventStatus();

    // Attendee management
    Attendee* findAttendeeInMasterList(int attendeeId);
    const Attendee* findAttendeeInMasterList(int attendeeId) const;
    void registerAttendeeForEvent();
    void cancelOwnRegistration();
    void viewAttendeeListsPerEvent() const;
    void checkInAttendeeForEvent();
    void generateAttendanceReportForEvent() const;
    void exportAttendeeListForEventToFile() const; // Specific export for admin, can use strategy

    // Inventory management
    InventoryItem* findInventoryItemById(int itemId);
    const InventoryItem* findInventoryItemById(int itemId) const;
    InventoryItem* findInventoryItemByName(const string& name);
    const InventoryItem* findInventoryItemByName(const string& name) const;
    void addInventoryItem();
    void updateInventoryItemDetails();
    void viewAllInventoryItems() const;
    void trackInventoryAllocationToEvent();
    void generateFullInventoryReport() const;

    // Export methods using the Strategy pattern
    void exportAllEventsDataToFile() const;
    void exportAllAttendeesDataToFile() const;
    void exportAllInventoryDataToFile() const;
    void exportAllUsersDataToFile() const; // New export method

    // Application run
    void run();
    void updateCurrentLoggedInUserContactInfo();
    void seedInitialData(); // Made public as it's called from main
};

// Initialize the static instance pointer outside the class definition
System* System::instance = nullptr;


// --- User Class Method Definitions ---
User::User(string uname, string pwd, Role r)
    : username(std::move(uname)), password(std::move(pwd)), role(r) {
    userId = nextUserId++;
}
User::User(int id, string uname, string pwd, Role r)
    : userId(id), username(std::move(uname)), password(std::move(pwd)), role(r) {
    if (id >= nextUserId) {
        nextUserId = id + 1;
    }
}
User::~User() {}

void User::setPassword(const string& newPassword) {
    if (newPassword.length() < 6) {
        cout << "Password must be at least 6 characters long.\n";
        return;
    }
    password = newPassword;
    cout << "Password updated successfully.\n";
    // System::getInstance().saveUsers(); // Should save users when password changes
}

string User::toString() const {
    stringstream ss;
    ss << userId << "," << username << "," << password << "," << static_cast<int>(role);
    return ss.str();
}

// --- Admin Class Method Definitions ---
Admin::Admin(string uname, string pwd) : User(std::move(uname), std::move(pwd), Role::ADMIN) {}
Admin::Admin(int id, string uname, string pwd) : User(id, std::move(uname), std::move(pwd), Role::ADMIN) {}

void Admin::displayMenu(System& sys) {
    int choice;
    do {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. User Management\n";
        cout << "2. Event Management\n";
        cout << "3. Attendee Management\n";
        cout << "4. Inventory Management\n";
        cout << "5. Data Export Options\n";
        cout << "6. View My Profile\n";
        cout << "0. Logout\n";
        choice = getIntInput("Enter your choice: ");

        switch (choice) {
            case 1: adminUserManagementMenu(sys); break;
            case 2: adminEventManagementMenu(sys); break;
            case 3: adminAttendeeManagementMenu(sys); break;
            case 4: adminInventoryManagementMenu(sys); break;
            case 5: adminDataExportMenu(sys); break;
            case 6: sys.currentUser->displayDetails(); break; // Corrected call
            case 0: sys.logout(); break;
            default: cout << "Invalid choice. Please try again.\n"; break;
        }
    } while (choice != 0 && sys.currentUser != nullptr);
}

void Admin::displayDetails() const {
    cout << "Admin ID: " << userId
         << ", Username: " << username
         << ", Role: Admin" << endl;
}

void Admin::adminUserManagementMenu(System& sys) {
    int choice;
    do {
        cout << "\n--- Admin User Management ---\n";
        cout << "1. Create New User Account\n";
        cout << "2. Delete User Account\n";
        cout << "3. List All Users\n";
        cout << "0. Back to Admin Menu\n";
        choice = getIntInput("Enter your choice: ");

        switch (choice) {
            case 1: sys.publicRegisterNewUser(); break;
            case 2: {
                string uname = getStringInput("Enter username to delete: ");
                sys.deleteUserAccount(uname);
                break;
            }
            case 3: sys.listAllUsers(); break;
            case 0: break;
            default: cout << "Invalid choice. Please try again.\n"; break;
        }
    } while (choice != 0);
}

void Admin::adminEventManagementMenu(System& sys) {
    int choice;
    do {
        cout << "\n--- Admin Event Management ---\n";
        cout << "1. Create New Event\n";
        cout << "2. View All Events (Admin View)\n";
        cout << "3. Edit Event Details\n";
        cout << "4. Update Event Status\n";
        cout << "5. Delete Event\n";
        cout << "6. Track Inventory Allocation to Event\n";
        cout << "0. Back to Admin Menu\n";
        choice = getIntInput("Enter your choice: ");

        switch (choice) {
            case 1: sys.createEvent(); break;
            case 2: sys.viewAllEvents(true); break;
            case 3: sys.editEventDetails(); break;
            case 4: sys.updateEventStatus(); break;
            case 5: sys.deleteEvent(); break;
            case 6: sys.trackInventoryAllocationToEvent(); break;
            case 0: break;
            default: cout << "Invalid choice. Please try again.\n"; break;
        }
    } while (choice != 0);
}

void Admin::adminAttendeeManagementMenu(System& sys) {
    int choice;
    do {
        cout << "\n--- Admin Attendee Management ---\n";
        cout << "1. View Attendee Lists per Event\n";
        cout << "2. Check-in Attendee for Event\n";
        cout << "3. Generate Attendance Report for Event\n";
        cout << "4. Export Attendee List for Event to File\n";
        cout << "0. Back to Admin Menu\n";
        choice = getIntInput("Enter your choice: ");

        switch (choice) {
            case 1: sys.viewAttendeeListsPerEvent(); break;
            case 2: sys.checkInAttendeeForEvent(); break;
            case 3: sys.generateAttendanceReportForEvent(); break;
            case 4: sys.exportAttendeeListForEventToFile(); break;
            case 0: break;
            default: cout << "Invalid choice. Please try again.\n"; break;
        }
    } while (choice != 0);
}

void Admin::adminInventoryManagementMenu(System& sys) {
    int choice;
    do {
        cout << "\n--- Admin Inventory Management ---\n";
        cout << "1. Add New Inventory Item\n";
        cout << "2. Update Inventory Item Details (Name, Quantity)\n";
        cout << "3. View All Inventory Items\n";
        cout << "4. Generate Full Inventory Report\n";
        cout << "0. Back to Admin Menu\n";
        choice = getIntInput("Enter your choice: ");

        switch (choice) {
            case 1: sys.addInventoryItem(); break;
            case 2: sys.updateInventoryItemDetails(); break;
            case 3: sys.viewAllInventoryItems(); break;
            case 4: sys.generateFullInventoryReport(); break;
            case 0: break;
            default: cout << "Invalid choice. Please try again.\n"; break;
        }
    } while (choice != 0);
}

void Admin::adminDataExportMenu(System& sys) {
    int choice;
    do {
        cout << "\n--- Admin Data Export Menu ---\n";
        cout << "1. Export All Events Data\n";
        cout << "2. Export All Attendees Data\n";
        cout << "3. Export All Inventory Data\n";
        cout << "4. Export All Users Data\n";
        cout << "0. Back to Admin Menu\n";
        choice = getIntInput("Enter your choice: ");

        switch (choice) {
            case 1: sys.exportAllEventsDataToFile(); break;
            case 2: sys.exportAllAttendeesDataToFile(); break;
            case 3: sys.exportAllInventoryDataToFile(); break;
            case 4: sys.exportAllUsersDataToFile(); break;
            case 0: break;
            default: cout << "Invalid choice. Please try again.\n"; break;
        }
    } while (choice != 0);
}

// --- RegularUser Class Method Definitions ---
RegularUser::RegularUser(string uname, string pwd) : User(std::move(uname), std::move(pwd), Role::REGULAR_USER) {}
RegularUser::RegularUser(int id, string uname, string pwd) : User(id, std::move(uname), std::move(pwd), Role::REGULAR_USER) {}

void RegularUser::displayMenu(System& sys) {
    int choice;
    do {
        cout << "\n--- User Menu ---\n";
        cout << "1. View All Events\n";
        cout << "2. Search Events\n";
        cout << "3. Register for Event\n";
        cout << "4. Cancel My Registration\n";
        cout << "5. Update My Contact Info\n";
        cout << "6. View My Profile\n";
        cout << "0. Logout\n";
        choice = getIntInput("Enter your choice: ");

        switch (choice) {
            case 1: sys.viewAllEvents(); break;
            case 2: sys.searchEventsByNameOrDate(); break;
            case 3: sys.registerAttendeeForEvent(); break;
            case 4: sys.cancelOwnRegistration(); break;
            case 5: sys.updateCurrentLoggedInUserContactInfo(); break;
            case 6: sys.currentUser->displayDetails(); break; // Corrected call
            case 0: sys.logout(); break;
            default: cout << "Invalid choice. Please try again.\n"; break;
        }
    } while (choice != 0 && sys.currentUser != nullptr);
}

void RegularUser::displayDetails() const {
    cout << "User ID: " << userId
         << ", Username: " << username
         << ", Role: Regular User" << endl;
    // For regular users, contact info is typically stored in the Attendee class.
    // If you want to show the contact info here, you'd need a way to link User to Attendee.
    // E.g., add a method to System like System::getAttendeeForUser(User* user) const
    // and call it here. For now, it just shows basic user info.
}


// --- User Factory Method Definition (User::fromString) ---
User* User::fromString(const string& str) {
    stringstream ss(str);
    string segment;
    int id;
    string uname, pwd;
    Role role_val;

    if (str.empty() || count(str.begin(), str.end(), ',') < 3) {
        cerr << "Warning: Malformed user data line: '" << str << "'. Skipping.\n";
        return nullptr;
    }
    try {
        getline(ss, segment, ','); id = stoi(segment);
        getline(ss, uname, ',');
        getline(ss, pwd, ',');
        getline(ss, segment, ','); role_val = static_cast<Role>(stoi(segment));
    } catch (const exception& e) {
        cerr << "Warning: Invalid data format in user line '" << str << "': " << e.what() << ". Skipping.\n";
        return nullptr;
    }

    if (role_val == Role::ADMIN) {
        return new Admin(id, uname, pwd);
    } else if (role_val == Role::REGULAR_USER) {
        return new RegularUser(id, uname, pwd);
    }
    cerr << "Warning: Unknown role in user data line: '" << str << "'. Skipping.\n";
    return nullptr;
}


// --- Attendee Class Method Definitions ---
Attendee::Attendee(string n, string contact, int eventId)
    : name(std::move(n)), contactInfo(std::move(contact)), eventIdRegisteredFor(eventId), isCheckedIn(false) {
    attendeeId = nextAttendeeId++;
}
Attendee::Attendee(int id, string n, string contact, int eventId, bool checkedInStatus)
    : attendeeId(id), name(std::move(n)), contactInfo(std::move(contact)),
      eventIdRegisteredFor(eventId), isCheckedIn(checkedInStatus) {
    if (id >= nextAttendeeId) {
        nextAttendeeId = id + 1;
    }
}
void Attendee::checkIn() {
    if (!isCheckedIn) {
        isCheckedIn = true;
        cout << name << " checked in successfully for event ID " << eventIdRegisteredFor << ".\n";
    } else {
        cout << name << " is already checked in for event ID " << eventIdRegisteredFor << ".\n";
    }
}
void Attendee::displayDetails() const {
    cout << "Attendee ID: " << attendeeId
              << ", Name: " << name
              << ", Contact: " << contactInfo
              << ", Registered for Event ID: " << (eventIdRegisteredFor == 0 ? "N/A (Profile)" : to_string(eventIdRegisteredFor))
              << ", Checked-in: " << (isCheckedIn ? "Yes" : "No") << endl;
}
string Attendee::toString() const {
    stringstream ss;
    ss << attendeeId << "," << name << "," << contactInfo << "," << eventIdRegisteredFor << "," << (isCheckedIn ? "1" : "0");
    return ss.str();
}
Attendee Attendee::fromString(const string& str) {
    stringstream ss(str);
    string segment;
    int id, eventId;
    string name, contact;
    bool checkedIn;
    // Error handling for stoi for robustness in case of malformed data
    try {
        getline(ss, segment, ','); id = stoi(segment);
        getline(ss, name, ',');
        getline(ss, contact, ',');
        getline(ss, segment, ','); eventId = stoi(segment);
        getline(ss, segment, ','); checkedIn = (segment == "1");
    } catch (const exception& e) {
        cerr << "Warning: Malformed attendee data line: '" << str << "'. Defaulting values. Error: " << e.what() << "\n";
        return Attendee(0, "ERROR", "ERROR", 0, false); // Return a default/error attendee
    }
    return Attendee(id, name, contact, eventId, checkedIn);
}

// --- InventoryItem Class Method Definitions ---
InventoryItem::InventoryItem(string n, int qty, string desc)
    : name(std::move(n)), totalQuantity(qty), allocatedQuantity(0), description(std::move(desc)) {
    itemId = nextItemId++;
}
InventoryItem::InventoryItem(int id, string n, int totalQty, int allocQty, string desc)
    : itemId(id), name(std::move(n)), totalQuantity(totalQty), allocatedQuantity(allocQty), description(std::move(desc)) {
    if (id >= nextItemId) {
        nextItemId = id + 1;
    }
}
int InventoryItem::getAvailableQuantity() const { return totalQuantity - allocatedQuantity; }
bool InventoryItem::allocate(int quantityToAllocate) {
    if (quantityToAllocate <= 0) { cout << "Error: Allocation quantity must be positive.\n"; return false; }
    if (quantityToAllocate <= getAvailableQuantity()) {
        allocatedQuantity += quantityToAllocate;
        return true;
    }
    cout << "Error: Not enough '" << name << "' available. Available: " << getAvailableQuantity() << endl;
    return false;
}
bool InventoryItem::deallocate(int quantityToDeallocate) {
    if (quantityToDeallocate <= 0) { cout << "Error: Deallocation quantity must be positive.\n"; return false; }
    if (quantityToDeallocate <= allocatedQuantity) {
        allocatedQuantity -= quantityToDeallocate;
        return true;
    }
    cout << "Error: Cannot deallocate " << quantityToDeallocate << " of '" << name << "'. Allocated: " << allocatedQuantity << endl;
    return false;
}
void InventoryItem::setTotalQuantity(int newTotalQuantity) {
    if (newTotalQuantity < 0) { cout << "Error: Total quantity cannot be negative.\n"; return; }
    if (newTotalQuantity < allocatedQuantity) {
        cout << "Error: New total quantity (" << newTotalQuantity << ") cannot be less than allocated (" << allocatedQuantity << ").\n";
        return;
    }
    totalQuantity = newTotalQuantity;
    cout << "Total quantity for '" << name << "' updated to " << totalQuantity << ".\n";
}
void InventoryItem::displayDetails() const {
    cout << "Item ID: " << itemId << ", Name: " << name
              << ", Total: " << totalQuantity
              << ", Allocated: " << allocatedQuantity
              << ", Available: " << getAvailableQuantity()
              << ", Desc: " << description << endl;
}
string InventoryItem::toString() const {
    stringstream ss;
    ss << itemId << "," << name << "," << totalQuantity << "," << allocatedQuantity << "," << description;
    return ss.str();
}
InventoryItem InventoryItem::fromString(const string& str) {
    stringstream ss(str);
    string segment, name, desc;
    int id, totalQty, allocQty;
    try {
        getline(ss, segment, ','); id = stoi(segment);
        getline(ss, name, ',');
        getline(ss, segment, ','); totalQty = stoi(segment);
        getline(ss, segment, ','); allocQty = stoi(segment);
        getline(ss, desc); // Read the rest as description
    } catch (const exception& e) {
        cerr << "Warning: Malformed inventory data line: '" << str << "'. Defaulting values. Error: " << e.what() << "\n";
        return InventoryItem(0, "ERROR", 0, 0, "ERROR"); // Return a default/error item
    }
    return InventoryItem(id, name, totalQty, allocQty, desc);
}

// --- Event Class Method Definitions ---
Event::Event(string n, string d, string t, string loc, string desc, string cat)
    : name(std::move(n)), date(std::move(d)), time(std::move(t)), location(std::move(loc)),
      description(std::move(desc)), category(std::move(cat)), status(EventStatus::UPCOMING) {
    eventId = nextEventId++;
}
Event::Event(int id, string n, string d, string t, string loc,
      string desc, string cat, EventStatus stat)
    : eventId(id), name(std::move(n)), date(std::move(d)), time(std::move(t)), location(std::move(loc)),
      description(std::move(desc)), category(std::move(cat)), status(stat) {
    if (id >= nextEventId) {
        nextEventId = id + 1;
    }
}
void Event::addAttendee(int attId) {
    if (find(attendeeIds.begin(), attendeeIds.end(), attId) == attendeeIds.end()) {
        attendeeIds.push_back(attId);
    } else {
        cout << "Info: Attendee ID " << attId << " already registered for event '" << name << "'.\n";
    }
}
void Event::removeAttendee(int attId) {
    auto it = find(attendeeIds.begin(), attendeeIds.end(), attId);
    if (it != attendeeIds.end()) {
        attendeeIds.erase(it);
    }
}
void Event::allocateInventoryItem(int itmId, int quantity) {
    if (quantity > 0) allocatedInventory[itmId] += quantity;
}
int Event::deallocateInventoryItem(int itmId, int quantityToDeallocate) {
    if (quantityToDeallocate <= 0) return 0;
    auto it = allocatedInventory.find(itmId);
    if (it != allocatedInventory.end()) {
        int currentQty = it->second;
        int actualDeallocated = min(currentQty, quantityToDeallocate);
        it->second -= actualDeallocated;
        if (it->second <= 0) {
            allocatedInventory.erase(it);
        }
        return actualDeallocated;
    }
    return 0;
}
string Event::getStatusString() const {
    switch (status) {
        case EventStatus::UPCOMING: return "Upcoming";
        case EventStatus::ONGOING: return "Ongoing";
        case EventStatus::COMPLETED: return "Completed";
        case EventStatus::CANCELED: return "Canceled";
        default: return "Unknown";
    }
}
string Event::attendeesToString() const {
    stringstream ss;
    for (size_t i = 0; i < attendeeIds.size(); ++i) {
        ss << attendeeIds[i] << (i == attendeeIds.size() - 1 ? "" : ";");
    }
    return ss.str();
}
string Event::inventoryToString() const {
    stringstream ss;
    bool first = true;
    for (const auto& pair : allocatedInventory) {
        if (!first) ss << ";";
        ss << pair.first << ":" << pair.second;
        first = false;
    }
    return ss.str();
}
string Event::toString() const {
    stringstream ss;
    ss << eventId << "," << name << "," << date << "," << time << "," << location << ","
       << description << "," << category << "," << static_cast<int>(status) << ","
       << attendeesToString() << "," << inventoryToString();
    return ss.str();
}
Event Event::fromString(const string& str) {
    stringstream ss(str);
    string segment, name, date_str, time_str, loc, desc, cat, attendeesStr, inventoryStr;
    int id;
    EventStatus stat;
    try {
        getline(ss, segment, ','); id = stoi(segment);
        getline(ss, name, ',');
        getline(ss, date_str, ',');
        getline(ss, time_str, ',');
        getline(ss, loc, ',');
        getline(ss, desc, ',');
        getline(ss, cat, ',');
        getline(ss, segment, ','); stat = static_cast<EventStatus>(stoi(segment));
        // Read remaining segments for attendees and inventory. Use ss.peek() to check if more data exists.
        if (ss.peek() == ',') {
            getline(ss, segment, ','); // consume the comma
            if (ss.peek() != ',' && ss.good()) { // Check if next char is not comma, and stream is good
                getline(ss, attendeesStr, ',');
            } else {
                attendeesStr = "";
            }
        } else {
            attendeesStr = ""; // No comma after status, so no attendees data
        }

        if (ss.good() && ss.peek() != EOF) { // Check if there's more data for inventory
            getline(ss, inventoryStr); // Read the rest of the line
        } else {
            inventoryStr = "";
        }

    } catch (const exception& e) {
        cerr << "Warning: Malformed event data line: '" << str << "'. Defaulting values. Error: " << e.what() << "\n";
        return Event(0, "ERROR", "ERROR", "ERROR", "ERROR", "ERROR"); // Return a default/error event
    }

    Event event(id, name, date_str, time_str, loc, desc, cat, stat);

    if (!attendeesStr.empty()) {
        stringstream attSs(attendeesStr);
        string attIdStr;
        while (getline(attSs, attIdStr, ';')) {
            if(!attIdStr.empty()) event.attendeeIds.push_back(stoi(attIdStr));
        }
    }
    if (!inventoryStr.empty()) {
        stringstream invSs(inventoryStr);
        string itemStr;
        while (getline(invSs, itemStr, ';')) {
            if(!itemStr.empty()){
                size_t colonPos = itemStr.find(':');
                if (colonPos != string::npos) {
                    try {
                        event.allocatedInventory[stoi(itemStr.substr(0, colonPos))] = stoi(itemStr.substr(colonPos + 1));
                    } catch (const exception& e) {
                        cerr << "Warning: Malformed inventory item in event line: '" << itemStr << "'. Skipping. Error: " << e.what() << "\n";
                    }
                }
            }
        }
    }
    return event;
}

void Event::displayDetails(const System& sys) const {
    cout << "Event ID: " << eventId << "\n"
         << "Name: " << name << "\n"
         << "Date: " << date << "\n"
         << "Time: " << time << "\n"
         << "Location: " << location << "\n"
         << "Description: " << description << "\n"
         << "Category: " << category << "\n"
         << "Status: " << getStatusString() << "\n";

    cout << "  Attendees (" << attendeeIds.size() << "): ";
    if (attendeeIds.empty()) {
        cout << "None\n";
    } else {
        bool first = true;
        for (int attId : attendeeIds) {
            const Attendee* att = sys.findAttendeeInMasterList(attId);
            if (!first) cout << ", ";
            if (att) {
                cout << att->name << " (ID:" << att->attendeeId << (att->isCheckedIn ? " - Checked In" : "") << ")";
            } else {
                cout << "Unknown Attendee (ID:" << attId << ")";
            }
            first = false;
        }
        cout << "\n";
    }

    cout << "  Allocated Inventory: ";
    if (allocatedInventory.empty()) {
        cout << "None\n";
    } else {
        bool first = true;
        for (const auto& pair : allocatedInventory) {
            const InventoryItem* item = sys.findInventoryItemById(pair.first);
            if (!first) cout << ", ";
            if (item) {
                cout << item->name << " (" << pair.second << " units)";
            } else {
                cout << "Unknown Item (ID:" << pair.first << ") (" << pair.second << " units)";
            }
            first = false;
        }
        cout << "\n";
    }
}


// --- System Method Definitions ---

// Destructor for the System Singleton
System::~System() {
    saveData(); // Make sure all data is saved on exit
    for (User* u : users) {
        delete u; // Clean up dynamically allocated User objects
    }
    users.clear();

    // Delete the export strategy
    if (exportStrategy != nullptr) {
        delete exportStrategy;
        exportStrategy = nullptr;
    }
}

void System::seedInitialData() {
    bool dataSeeded = false;
    if (users.empty()) {
        cout << "Info: No users found. Seeding initial accounts.\n";
        users.push_back(new Admin("admin", "adminpass"));
        cout << "Seeded Admin: admin (ID: " << users.back()->getUserId() << ")\n";
        users.push_back(new RegularUser("user1", "user1pass"));
        cout << "Seeded User: user1 (ID: " << users.back()->getUserId() << ")\n";
        users.push_back(new RegularUser("user2", "user2pass"));
        cout << "Seeded User: user2 (ID: " << users.back()->getUserId() << ")\n";
        dataSeeded = true;
    }
    if (events.empty()) {
        cout << "Info: No events found. Seeding initial events.\n";
        events.emplace_back("Tech Conference 2025", "2025-10-20", "09:00", "Grand Hall", "Annual tech conference", "Conference");
        cout << "Seeded Event: Tech Conference 2025 (ID: " << events.back().eventId << ")\n";
        events.emplace_back("Summer Music Festival", "2025-07-15", "14:00", "City Park", "Outdoor music event", "Social");
        cout << "Seeded Event: Summer Music Festival (ID: " << events.back().eventId << ")\n";
        dataSeeded = true;
    }
    if (inventory.empty()) {
        cout << "Info: No inventory found. Seeding initial items.\n";
        inventory.emplace_back("Projector", 5, "HD Projector");
        cout << "Seeded Inventory: Projector (ID: " << inventory.back().itemId << ")\n";
        inventory.emplace_back("Chairs", 100, "Standard chairs");
        cout << "Seeded Inventory: Chairs (ID: " << inventory.back().itemId << ")\n";
        dataSeeded = true;
    }
    if (dataSeeded) {
        cout << "Initial data seeded. Saving to files...\n";
        saveData(); // Use the new saveData which uses the strategy
    }
}

void System::loadData() {
    loadUsers(); loadEvents(); loadInventory(); loadAttendees();
    // Re-initialize next IDs based on loaded data to prevent ID collisions
    int maxId = 0; for(const auto* u : users) if(u && u->getUserId() > maxId) maxId = u->getUserId(); User::initNextId(maxId);
    maxId = 0; for(const auto& e : events) if(e.eventId > maxId) maxId = e.eventId; Event::initNextId(maxId);
    maxId = 0; for(const auto& i : inventory) if(i.itemId > maxId) maxId = i.itemId; InventoryItem::initNextId(maxId);
    maxId = 0; for(const auto& a : allAttendees) if(a.attendeeId > maxId) maxId = a.attendeeId; Attendee::initNextId(maxId);

    // After loading, ensure allocated quantities are consistent with events
    for (auto& item : inventory) {
        item.allocatedQuantity = 0; // Reset allocated quantities for re-calculation
    }
    for (const auto& event : events) {
        for (const auto& invPair : event.allocatedInventory) {
            InventoryItem* item = findInventoryItemById(invPair.first);
            if (item) {
                item->allocatedQuantity += invPair.second;
            }
        }
    }
}

void System::saveData() {
    saveUsers();
    saveEvents();
    saveInventory();
    saveAttendees();
}

void System::loadUsers() {
    ifstream inFile(USERS_FILE); if (!inFile) return;
    string line;
    while (getline(inFile, line)) {
        if (!line.empty()) {
            User* u = User::fromString(line);
            if(u) users.push_back(u);
        }
    }
    inFile.close();
}

void System::saveUsers() {
    if (exportStrategy) {
        exportStrategy->exportUsers(users, USERS_FILE);
    } else {
        cerr << "Error: No export strategy set for saving users.\n";
    }
}

void System::loadEvents() {
    ifstream inFile(EVENTS_FILE); if (!inFile) return;
    string line;
    while (getline(inFile, line)) {
        if (!line.empty()) events.push_back(Event::fromString(line));
    }
    inFile.close();
}

void System::saveEvents() {
    if (exportStrategy) {
        exportStrategy->exportEvents(events, EVENTS_FILE, *this);
    } else {
        cerr << "Error: No export strategy set for saving events.\n";
    }
}

void System::loadInventory() {
    ifstream inFile(INVENTORY_FILE); if (!inFile) return;
    string line;
    while (getline(inFile, line)) {
        if (!line.empty()) inventory.push_back(InventoryItem::fromString(line));
    }
    inFile.close();
}

void System::saveInventory() {
    if (exportStrategy) {
        exportStrategy->exportInventory(inventory, INVENTORY_FILE);
    } else {
        cerr << "Error: No export strategy set for saving inventory.\n";
    }
}

void System::loadAttendees() {
    ifstream inFile(ATTENDEES_FILE); if (!inFile) return;
    string line;
    while (getline(inFile, line)) {
        if (!line.empty()) allAttendees.push_back(Attendee::fromString(line));
    }
    inFile.close();
}

void System::saveAttendees() {
    if (exportStrategy) {
        exportStrategy->exportAttendees(allAttendees, ATTENDEES_FILE);
    } else {
        cerr << "Error: No export strategy set for saving attendees.\n";
    }
}

bool System::usernameExists(const string& uname) const {
    for (const auto* user : users) if (user && user->getUsername() == uname) return true;
    return false;
}
void System::createUserAccount(const string& uname, const string& pwd, Role role) {
    if (usernameExists(uname)) { cout << "Username already exists.\n"; return;}
    if (pwd.length() < 6) { cout << "Password too short.\n"; return; }
    if (role == Role::ADMIN) users.push_back(new Admin(uname, pwd));
    else if (role == Role::REGULAR_USER) users.push_back(new RegularUser(uname, pwd));
    else { cout << "Invalid role. Account not created.\n"; return; }
    cout << (role == Role::ADMIN ? "Admin" : "User") << " '" << uname << "' created (ID: " << users.back()->getUserId() << ").\n";
    saveUsers();
}
void System::publicRegisterNewUser() {
    cout << "\n--- Register New User ---\n";
    string uname = getStringInput("Username: ");
    string pwd = getStringInput("Password (min 6 chars): ");
    cout << "Account type: 1. Admin 2. Regular User\n";
    int rChoice = getIntInput("Choice (1-2): ");
    Role newRole;
    if (rChoice == 1) newRole = Role::ADMIN;
    else if (rChoice == 2) newRole = Role::REGULAR_USER;
    else newRole = Role::NONE; // Mark as invalid if choice is bad
    createUserAccount(uname, pwd, newRole);
}
void System::deleteUserAccount(const string& uname) {
    // Cannot delete currently logged in user
    if (currentUser && currentUser->getUsername() == uname) {
        cout << "Error: Cannot delete the currently logged-in user.\n";
        return;
    }

    auto it = remove_if(users.begin(), users.end(), [&](User* u) {
        if (u && u->getUsername() == uname) {
            delete u; // Deallocate the User object
            return true;
        }
        return false;
    });
    if (it != users.end()) {
        users.erase(it, users.end());
        cout << "User '" << uname << "' deleted.\n";
        saveUsers();
    }
    else { cout << "User '" << uname << "' not found.\n"; }
}
User* System::findUserByUsername(const string& uname) {
    for (auto* user : users) if (user && user->getUsername() == uname) return user; return nullptr;
}
const User* System::findUserByUsername(const string& uname) const {
    for (const auto* user : users) if (user && user->getUsername() == uname) return user; return nullptr;
}
void System::listAllUsers() const {
    cout << "\n--- All Users ---\n"; if (users.empty()) { cout << "No users.\n"; return; }
    for (const auto* user : users) if (user) user->displayDetails(); // Using polymorphic displayDetails
}
bool System::login() {
    cout << "\n--- Login ---\n";
    string uname = getStringInput("Username: "); string pwd = getStringInput("Password: ");
    for (User* u : users) if (u && u->getUsername() == uname && u->getPassword() == pwd) {
        currentUser = u; cout << "Login successful. Welcome, " << currentUser->getUsername() << "!\n"; return true;
    }
    cout << "Login failed. Invalid username or password.\n"; currentUser = nullptr; return false;
}
void System::logout() { if (currentUser) { cout << "Logging out " << currentUser->getUsername() << ".\n"; currentUser = nullptr; } }

Event* System::findEventById(int eventId) { for (auto& event : events) if (event.eventId == eventId) return &event; return nullptr; }
const Event* System::findEventById(int eventId) const { for (const auto& event : events) if (event.eventId == eventId) return &event; return nullptr; }

void System::createEvent() {
    cout << "\n--- Create Event ---\n";
    string name = getStringInput("Name: "); string date, time;
    while(true){ date = getStringInput("Date (YYYY-MM-DD): "); if(isValidDate(date)) break; cout << "Invalid date format. Please try again.\n"; }
    while(true){ time = getStringInput("Time (HH:MM): "); if(isValidTime(time)) break; cout << "Invalid time format. Please try again.\n"; }
    string loc = getStringInput("Location: "); string desc = getStringInput("Description: "); string cat = getStringInput("Category: ");
    events.emplace_back(name, date, time, loc, desc, cat);
    cout << "Event '" << name << "' created (ID: " << events.back().eventId << ").\n"; saveEvents();
}
void System::viewAllEvents(bool adminView) const {
    cout << "\n--- All Events ---\n"; if (events.empty()) { cout << "No events.\n"; return; }
    for (const auto& event : events) { event.displayDetails(*this); cout << "-------------------\n"; }
}
void System::searchEventsByNameOrDate() const {
    string searchTerm = toLower(getStringInput("Enter event name or date to search: "));
    bool found = false;
    cout << "\n--- Search Results ---\n";
    for (const auto& event : events) {
        if (toLower(event.name).find(searchTerm) != string::npos ||
            event.date.find(searchTerm) != string::npos) {
            event.displayDetails(*this);
            cout << "-------------------\n";
            found = true;
        }
    }
    if (!found) {
        cout << "No events found matching '" << searchTerm << "'.\n";
    }
}
void System::editEventDetails() {
    int eventId = getPositiveIntInput("Enter Event ID to edit: ");
    Event* event = findEventById(eventId);
    if (!event) {
        cout << "Event with ID " << eventId << " not found.\n";
        return;
    }

    cout << "\n--- Editing Event: " << event->name << " (ID: " << event->eventId << ") ---\n";
    cout << "1. Edit Name\n";
    cout << "2. Edit Date\n";
    cout << "3. Edit Time\n";
    cout << "4. Edit Location\n";
    cout << "5. Edit Description\n";
    cout << "6. Edit Category\n";
    cout << "7. Back\n";

    int choice = getIntInput("Enter your choice: ");
    string new_val;
    switch (choice) {
        case 1: new_val = getStringInput("Enter new name: "); event->name = new_val; break;
        case 2:
            while(true){ new_val = getStringInput("Enter new date (YYYY-MM-DD): "); if(isValidDate(new_val)) break; cout << "Invalid date format or value. Please try again.\n"; }
            event->date = new_val;
            break;
        case 3:
            while(true){ new_val = getStringInput("Enter new time (HH:MM): "); if(isValidTime(new_val)) break; cout << "Invalid time format or value. Please try again.\n"; }
            event->time = new_val;
            break;
        case 4: new_val = getStringInput("Enter new location: "); event->location = new_val; break;
        case 5: new_val = getStringInput("Enter new description: "); event->description = new_val; break;
        case 6: new_val = getStringInput("Enter new category: "); event->category = new_val; break;
        case 7: return;
        default: cout << "Invalid choice. No changes made.\n"; return;
    }
    cout << "Event details updated successfully.\n";
    saveEvents();
}
void System::deleteEvent() {
    int eventId = getPositiveIntInput("Enter Event ID to delete: ");
    auto it = remove_if(events.begin(), events.end(), [&](const Event& e) {
        return e.eventId == eventId;
    });

    if (it != events.end()) {
        // Deallocate any inventory allocated to this event
        for(const auto& pair : it->allocatedInventory) {
            InventoryItem* item = findInventoryItemById(pair.first);
            if(item) {
                item->deallocate(pair.second); // Return allocated items to general pool
            }
        }
        // Remove attendees registered for this event
        allAttendees.erase(remove_if(allAttendees.begin(), allAttendees.end(),
            [&](const Attendee& att){ return att.eventIdRegisteredFor == eventId; }), allAttendees.end());

        cout << "Event '" << it->name << "' (ID: " << it->eventId << ") and its associated registrations deleted.\n";
        events.erase(it, events.end());
        saveEvents();
        saveInventory(); // Save inventory changes
        saveAttendees(); // Save attendees changes
    } else {
        cout << "Event with ID " << eventId << " not found.\n";
    }
}
void System::updateEventStatus() {
    int eventId = getPositiveIntInput("Enter Event ID to update status: ");
    Event* event = findEventById(eventId);
    if (!event) {
        cout << "Event with ID " << eventId << " not found.\n";
        return;
    }
    cout << "Current status of '" << event->name << "': " << event->getStatusString() << endl;
    cout << "Select new status:\n";
    cout << "1. Upcoming\n";
    cout << "2. Ongoing\n";
    cout << "3. Completed\n";
    cout << "4. Canceled\n";
    int choice = getIntInput("Enter your choice: ");
    switch (choice) {
        case 1: event->status = EventStatus::UPCOMING; break;
        case 2: event->status = EventStatus::ONGOING; break;
        case 3: event->status = EventStatus::COMPLETED; break;
        case 4: event->status = EventStatus::CANCELED; break;
        default: cout << "Invalid status choice. Status not updated.\n"; return;
    }
    cout << "Status updated to " << event->getStatusString() << ".\n";
    saveEvents();
}

Attendee* System::findAttendeeInMasterList(int attendeeId) {
    for (auto& att : allAttendees) {
        if (att.attendeeId == attendeeId) return &att;
    }
    return nullptr;
}
const Attendee* System::findAttendeeInMasterList(int attendeeId) const {
    for (const auto& att : allAttendees) {
        if (att.attendeeId == attendeeId) return &att;
    }
    return nullptr;
}

void System::registerAttendeeForEvent() {
    if (currentUser == nullptr || currentUser->getRole() == Role::ADMIN) {
        cout << "Only regular users can register for events directly.\n";
        return;
    }

    int eventId = getPositiveIntInput("Enter Event ID to register for: ");
    Event* event = findEventById(eventId);
    if (!event) {
        cout << "Event with ID " << eventId << " not found.\n";
        return;
    }
    if (event->status == EventStatus::CANCELED || event->status == EventStatus::COMPLETED) {
        cout << "Cannot register for a " << event->getStatusString() << " event.\n";
        return;
    }

    // Check if the current user already has an attendee profile or create one
    // A simplified approach: assume a 1:1 mapping if a user wants to register
    // or create a new attendee record for the user if they don't have one associated
    // with this event or generically.
    string attendeeName = currentUser->getUsername(); // Default name from username
    string contact = getStringInput("Enter your contact info (email/phone): ");

    // Find if the user already has an attendee profile created for this event
    Attendee* existingAttendee = nullptr;
    for (auto& att : allAttendees) {
        if (toLower(att.name) == toLower(attendeeName) && att.eventIdRegisteredFor == eventId) {
            existingAttendee = &att;
            break;
        }
    }
     // Or, perhaps a user might have a generic attendee profile.
    if (!existingAttendee) {
        for (auto& att : allAttendees) {
            if (toLower(att.name) == toLower(attendeeName) && att.eventIdRegisteredFor == 0) { // Event ID 0 for generic profile
                existingAttendee = &att;
                existingAttendee->contactInfo = contact; // Update contact if changed
                break;
            }
        }
    }


    if (existingAttendee) {
        // If an attendee profile exists for the user and this event, just ensure they are registered
        event->addAttendee(existingAttendee->attendeeId);
        cout << "You are already associated with an attendee profile. Registration confirmed for event '" << event->name << "'.\n";
    } else {
        // Create a new attendee record for this specific registration
        Attendee newAttendee(attendeeName, contact, eventId);
        allAttendees.push_back(newAttendee);
        event->addAttendee(newAttendee.attendeeId);
        cout << "Registered as new attendee '" << newAttendee.name << "' (ID: " << newAttendee.attendeeId << ") for event '" << event->name << "'.\n";
    }
    saveEvents();
    saveAttendees();
}


void System::cancelOwnRegistration() {
    if (currentUser == nullptr || currentUser->getRole() == Role::ADMIN) {
        cout << "Only regular users can cancel their own registrations.\n";
        return;
    }

    int eventId = getPositiveIntInput("Enter Event ID to cancel registration for: ");
    Event* event = findEventById(eventId);
    if (!event) {
        cout << "Event with ID " << eventId << " not found.\n";
        return;
    }

    // Find the attendee ID corresponding to the current user and this event
    int attendeeIdToCancel = -1;
    for (const auto& att : allAttendees) {
        if (toLower(att.name) == toLower(currentUser->getUsername()) && att.eventIdRegisteredFor == eventId) {
            attendeeIdToCancel = att.attendeeId;
            break;
        }
    }

    if (attendeeIdToCancel != -1) {
        event->removeAttendee(attendeeIdToCancel); // Remove from event's list
        // Remove from master attendees list
        allAttendees.erase(remove_if(allAttendees.begin(), allAttendees.end(),
            [&](const Attendee& att){ return att.attendeeId == attendeeIdToCancel; }), allAttendees.end());
        cout << "Your registration for event '" << event->name << "' has been canceled.\n";
        saveEvents();
        saveAttendees();
    } else {
        cout << "You are not registered for event '" << event->name << "'.\n";
    }
}

void System::viewAttendeeListsPerEvent() const {
    cout << "\n--- Attendee Lists Per Event ---\n";
    if (events.empty()) {
        cout << "No events available to view attendee lists.\n";
        return;
    }
    for (const auto& event : events) {
        cout << "Event: " << event.name << " (ID: " << event.eventId << ")\n";
        if (event.attendeeIds.empty()) {
            cout << "  No attendees registered.\n";
        } else {
            for (int attId : event.attendeeIds) {
                const Attendee* att = findAttendeeInMasterList(attId);
                if (att) {
                    cout << "    - " << att->name << " (ID: " << att->attendeeId << ", Contact: " << att->contactInfo << ", Checked-in: " << (att->isCheckedIn ? "Yes" : "No") << ")\n";
                } else {
                    cout << "    - Unknown Attendee (ID: " << attId << ")\n";
                }
            }
        }
        cout << "---------------------------------\n";
    }
}
void System::checkInAttendeeForEvent() {
    int eventId = getPositiveIntInput("Enter Event ID: ");
    Event* event = findEventById(eventId);
    if (!event) {
        cout << "Event with ID " << eventId << " not found.\n";
        return;
    }
    int attendeeId = getPositiveIntInput("Enter Attendee ID to check-in: ");
    Attendee* attendee = findAttendeeInMasterList(attendeeId);

    if (attendee && attendee->eventIdRegisteredFor == eventId) {
        attendee->checkIn();
        saveAttendees();
    } else {
        cout << "Attendee ID " << attendeeId << " not found or not registered for event ID " << eventId << ".\n";
    }
}
void System::generateAttendanceReportForEvent() const {
    int eventId = getPositiveIntInput("Enter Event ID for attendance report: ");
    const Event* event = findEventById(eventId);
    if (!event) {
        cout << "Event with ID " << eventId << " not found.\n";
        return;
    }

    cout << "\n--- Attendance Report for Event: " << event->name << " (ID: " << event->eventId << ") ---\n";
    if (event->attendeeIds.empty()) {
        cout << "No attendees registered for this event.\n";
        return;
    }

    int checkedInCount = 0;
    cout << "Registered Attendees:\n";
    for (int attId : event->attendeeIds) {
        const Attendee* att = findAttendeeInMasterList(attId);
        if (att) {
            cout << "  - Name: " << att->name << ", Contact: " << att->contactInfo << ", Checked-in: " << (att->isCheckedIn ? "Yes" : "No") << "\n";
            if (att->isCheckedIn) {
                checkedInCount++;
            }
        } else {
            cout << "  - Unknown Attendee (ID: " << attId << ")\n";
        }
    }
    cout << "--------------------------------------\n";
    cout << "Total Registered: " << event->attendeeIds.size() << "\n";
    cout << "Total Checked-in: " << checkedInCount << "\n";
    cout << "Attendance Percentage: " << (event->attendeeIds.empty() ? 0.0 : (static_cast<double>(checkedInCount) / event->attendeeIds.size()) * 100.0) << "%\n";
}
void System::exportAttendeeListForEventToFile() const {
    int eventId = getPositiveIntInput("Enter Event ID to export attendee list: ");
    const Event* event = findEventById(eventId);
    if (!event) {
        cout << "Event with ID " << eventId << " not found.\n";
        return;
    }

    string filename = "attendees_event_" + to_string(eventId) + ".txt";
    ofstream outFile(filename);
    if (!outFile) {
        cerr << "Error: Could not open " << filename << " for writing.\n";
        return;
    }

    outFile << "Attendee List for Event: " << event->name << " (ID: " << event->eventId << ")\n";
    outFile << "Date: " << event->date << " Time: " << event->time << "\n";
    outFile << "---------------------------------------------------------\n";
    if (event->attendeeIds.empty()) {
        outFile << "No attendees registered for this event.\n";
    } else {
        outFile << "ID,Name,ContactInfo,CheckedInStatus\n";
        for (int attId : event->attendeeIds) {
            const Attendee* att = findAttendeeInMasterList(attId);
            if (att) {
                outFile << att->attendeeId << "," << att->name << "," << att->contactInfo << "," << (att->isCheckedIn ? "Checked In" : "Not Checked In") << "\n";
            }
        }
    }
    outFile.close();
    cout << "Attendee list for event '" << event->name << "' exported to " << filename << endl;
}

InventoryItem* System::findInventoryItemById(int itemId) {
    for (auto& item : inventory) if (item.itemId == itemId) return &item; return nullptr;
}
const InventoryItem* System::findInventoryItemById(int itemId) const {
    for (const auto& item : inventory) if (item.itemId == itemId) return &item; return nullptr;
}
InventoryItem* System::findInventoryItemByName(const string& name) {
    for (auto& item : inventory) if (toLower(item.name) == toLower(name)) return &item; return nullptr;
}
const InventoryItem* System::findInventoryItemByName(const string& name) const {
    for (const auto& item : inventory) if (toLower(item.name) == toLower(name)) return &item; return nullptr;
}
void System::addInventoryItem() {
    cout << "\n--- Add New Inventory Item ---\n";
    string name = getStringInput("Item Name: ");
    int quantity = getPositiveIntInput("Total Quantity: ");
    string desc = getStringInput("Description: ");
    inventory.emplace_back(name, quantity, desc);
    cout << "Inventory item '" << name << "' added (ID: " << inventory.back().itemId << ").\n";
    saveInventory();
}
void System::updateInventoryItemDetails() {
    int itemId = getPositiveIntInput("Enter Item ID to update: ");
    InventoryItem* item = findInventoryItemById(itemId);
    if (!item) {
        cout << "Inventory item with ID " << itemId << " not found.\n";
        return;
    }
    cout << "\n--- Updating Inventory Item: " << item->name << " (ID: " << item->itemId << ") ---\n";
    cout << "1. Update Name\n";
    cout << "2. Update Total Quantity\n";
    cout << "3. Update Description\n";
    cout << "4. Back\n";
    int choice = getIntInput("Enter your choice: ");
    string new_str_val;
    int new_int_val;
    switch (choice) {
        case 1: new_str_val = getStringInput("Enter new name: "); item->name = new_str_val; break;
        case 2: new_int_val = getPositiveIntInput("Enter new total quantity: "); item->setTotalQuantity(new_int_val); break;
        case 3: new_str_val = getStringInput("Enter new description: "); item->description = new_str_val; break;
        case 4: return;
        default: cout << "Invalid choice. No changes made.\n"; return;
    }
    cout << "Inventory item updated successfully.\n";
    saveInventory();
}
void System::viewAllInventoryItems() const {
    cout << "\n--- All Inventory Items ---\n";
    if (inventory.empty()) { cout << "No inventory items.\n"; return; }
    for (const auto& item : inventory) item.displayDetails();
}
void System::trackInventoryAllocationToEvent() {
    int eventId = getPositiveIntInput("Enter Event ID: ");
    Event* event = findEventById(eventId);
    if (!event) {
        cout << "Event with ID " << eventId << " not found.\n";
        return;
    }
    cout << "\n--- Managing Inventory for Event: " << event->name << " (ID: " << event->eventId << ") ---\n";
    cout << "1. Allocate Item to Event\n";
    cout << "2. Deallocate Item from Event\n";
    cout << "0. Back\n";
    int choice = getIntInput("Enter your choice: ");

    if (choice == 1) {
        int itemId = getPositiveIntInput("Enter Inventory Item ID to allocate: ");
        InventoryItem* item = findInventoryItemById(itemId);
        if (!item) {
            cout << "Inventory item with ID " << itemId << " not found.\n";
            return;
        }
        cout << "Available quantity of '" << item->name << "': " << item->getAvailableQuantity() << endl;
        int quantity = getPositiveIntInput("Enter quantity to allocate: ");
        if (item->allocate(quantity)) {
            event->allocateInventoryItem(item->itemId, quantity);
            cout << quantity << " of '" << item->name << "' allocated to event '" << event->name << "'.\n";
            saveInventory();
            saveEvents();
        }
    } else if (choice == 2) {
        int itemId = getPositiveIntInput("Enter Inventory Item ID to deallocate: ");
        InventoryItem* item = findInventoryItemById(itemId);
        if (!item) {
            cout << "Inventory item with ID " << itemId << " not found.\n";
            return;
        }
        auto it = event->allocatedInventory.find(itemId);
        if (it == event->allocatedInventory.end()) {
            cout << "'" << item->name << "' was not allocated to event '" << event->name << "'.\n";
            return;
        }
        cout << "Currently allocated to this event: " << it->second << " of '" << item->name << "'.\n";
        int quantity = getPositiveIntInput("Enter quantity to deallocate: ");
        int actualDeallocated = event->deallocateInventoryItem(item->itemId, quantity);
        if (actualDeallocated > 0) {
            item->deallocate(actualDeallocated);
            cout << actualDeallocated << " of '" << item->name << "' deallocated from event '" << event->name << "'.\n";
            saveInventory();
            saveEvents();
        } else {
            cout << "No quantity deallocated.\n";
        }
    }
}
void System::generateFullInventoryReport() const {
    cout << "\n--- Full Inventory Report ---\n";
    if (inventory.empty()) {
        cout << "No inventory items to report.\n";
        return;
    }

    int totalItems = 0;
    int totalAllocated = 0;
    int totalAvailable = 0;

    cout << "Item ID | Name              | Total | Allocated | Available | Description\n";
    cout << "-----------------------------------------------------------------------\n";
    for (const auto& item : inventory) {
        cout << left << setw(8) << item.itemId
             << "| " << left << setw(17) << item.name
             << "| " << right << setw(5) << item.totalQuantity
             << " | " << right << setw(9) << item.allocatedQuantity
             << " | " << right << setw(9) << item.getAvailableQuantity()
             << " | " << item.description << "\n";
        totalItems += item.totalQuantity;
        totalAllocated += item.allocatedQuantity;
        totalAvailable += item.getAvailableQuantity();
    }
    cout << "-----------------------------------------------------------------------\n";
    cout << "Overall Totals: Total: " << totalItems
         << ", Allocated: " << totalAllocated
         << ", Available: " << totalAvailable << "\n";

    cout << "\nAllocation per Event:\n";
    bool anyEventAllocated = false;
    for (const auto& event : events) {
        bool eventHasAllocations = false;
        stringstream ss;
        ss << "  Event: " << event.name << " (ID: " << event.eventId << ")\n";
        for (const auto& pair : event.allocatedInventory) {
            const InventoryItem* item = findInventoryItemById(pair.first);
            if (item && pair.second > 0) {
                ss << "    - " << item->name << ": " << pair.second << " units\n";
                eventHasAllocations = true;
                anyEventAllocated = true;
            }
        }
        if (eventHasAllocations) {
            cout << ss.str();
        }
    }
    if (!anyEventAllocated) {
        cout << "  No inventory currently allocated to any event.\n";
    }
    cout << "-----------------------------------------------------------------------\n";
}

void System::exportAllEventsDataToFile() const {
    if (exportStrategy) {
        exportStrategy->exportEvents(events, "events_export.txt", *this);
    } else {
        cerr << "Error: No export strategy set.\n";
    }
}

void System::exportAllAttendeesDataToFile() const {
    if (exportStrategy) {
        exportStrategy->exportAttendees(allAttendees, "attendees_export.txt");
    } else {
        cerr << "Error: No export strategy set.\n";
    }
}

void System::exportAllInventoryDataToFile() const {
    if (exportStrategy) {
        exportStrategy->exportInventory(inventory, "inventory_export.txt");
    } else {
        cerr << "Error: No export strategy set.\n";
    }
}

void System::exportAllUsersDataToFile() const {
    if (exportStrategy) {
        exportStrategy->exportUsers(users, "users_export.txt");
    } else {
        cerr << "Error: No export strategy set.\n";
    }
}

void System::run() {
    cout << "Welcome to the Event Management System!\n";
    int choice;
    do {
        if (currentUser == nullptr) {
            cout << "\n--- Main Menu ---\n";
            cout << "1. Login\n";
            cout << "2. Register New User\n";
            cout << "0. Exit\n";
            choice = getIntInput("Enter your choice: ");

            switch (choice) {
                case 1:
                    if (login()) {
                        // Display user-specific menu after successful login
                        currentUser->displayMenu(*this);
                    }
                    break;
                case 2:
                    publicRegisterNewUser();
                    break;
                case 0:
                    cout << "Exiting Event Management System. Goodbye!\n";
                    break;
                default:
                    cout << "Invalid choice. Please try again.\n";
                    break;
            }
        } else {
            // This case should ideally not be reached if displayMenu handles returning to main menu
            // by setting currentUser to nullptr on logout. This acts as a fallback.
            currentUser->displayMenu(*this);
        }
    } while (choice != 0 || currentUser != nullptr);
}

void System::updateCurrentLoggedInUserContactInfo() {
    if (currentUser == nullptr || currentUser->getRole() == Role::ADMIN) {
        cout << "This option is for regular users to update their attendee contact info.\n";
        return;
    }

    string newContact = getStringInput("Enter new contact information (email/phone): ");

    // Find the attendee profile associated with the current user.
    // Assuming a regular user primarily has one main attendee profile (eventIdRegisteredFor = 0 or similar generic).
    // Or, prompt them to choose which attendee profile to update if they have multiple.
    Attendee* userAttendeeProfile = nullptr;
    for (auto& att : allAttendees) {
        if (toLower(att.name) == toLower(currentUser->getUsername()) && att.eventIdRegisteredFor == 0) { // Generic profile
            userAttendeeProfile = &att;
            break;
        }
        // If not found as generic, maybe they only have event-specific ones.
        // For simplicity, let's update all associated with their name.
        if (toLower(att.name) == toLower(currentUser->getUsername())) {
             att.contactInfo = newContact;
        }
    }

    if (userAttendeeProfile) {
        userAttendeeProfile->contactInfo = newContact;
        cout << "Your primary attendee contact information has been updated.\n";
    } else {
        // If no generic attendee profile, offer to create one or update their first registered one.
        cout << "No generic attendee profile found for you. Creating one with new contact info.\n";
        allAttendees.emplace_back(currentUser->getUsername(), newContact, 0); // Event ID 0 for generic
    }
    saveAttendees();
}


// --- Main Function ---
int main() {
    // Get the singleton instance of System
    System& eventSystem = System::getInstance();

    eventSystem.loadData();
    eventSystem.seedInitialData(); // Now public

    eventSystem.run(); // Start the main application loop

    // Clean up the singleton instance before exiting
    System::destroyInstance();

    return 0;
}