#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>

using namespace std;

const int MAX_BOOKS = 100;
const int MAX_USERS = 50;
const int BORROW_TIME_MINUTES = 5; // 1 to 5 minutes

struct Book {
    int id;
    string title;
    string author;
    int quantity;
};

struct User {
    string username;
    string password;
    string role; // "admin" or "user"
};

struct BorrowRecord {
    int bookId;
    string username;
    time_t borrowTime;
    time_t returnTime;
    bool returned;
};

Book books[MAX_BOOKS];
User users[MAX_USERS];
BorrowRecord borrowRecords[MAX_BOOKS * 10];
int nextId = 1;
int bookCount = 0;
int userCount = 0;
int recordCount = 0;
User* currentUser = nullptr;

// Initialize some default users
void initializeUsers() {
    users[userCount++] = {"admin", "admin123", "admin"};
    users[userCount++] = {"user", "user123", "user"};
}

bool login() {
    string username, password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    for (int i = 0; i < userCount; ++i) {
        if (users[i].username == username && users[i].password == password) {
            currentUser = &users[i];
            cout << "Login successful! Welcome " << username << " (" << users[i].role << ")" << endl;
            return true;
        }
    }
    cout << "Invalid username or password!" << endl;
    return false;
}

void logout() {
    currentUser = nullptr;
    cout << "Logged out successfully!" << endl;
}

void addBook(string title, string author, int quantity) {
    if (currentUser->role != "admin") {
        cout << "Access denied! Only admins can add books." << endl;
        return;
    }
    
    if (bookCount >= MAX_BOOKS) {
        cout << "Library is full. Cannot add more books." << endl;
        return;
    }
    books[bookCount].id = nextId++;
    books[bookCount].title = title;
    books[bookCount].author = author;
    books[bookCount].quantity = quantity;
    bookCount++;
    cout << "Book added successfully!" << endl;
}

void removeBook(int bookId) {
    if (currentUser->role != "admin") {
        cout << "Access denied! Only admins can remove books." << endl;
        return;
    }
    
    for (int i = 0; i < bookCount; ++i) {
        if (books[i].id == bookId) {
            for (int j = i; j < bookCount - 1; ++j) {
                books[j] = books[j + 1];
            }
            bookCount--;
            cout << "Book removed successfully!" << endl;
            return;
        }
    }
    cout << "Book not found." << endl;
}

void displayBooks() {
    if (bookCount == 0) {
        cout << "No books in the library." << endl;
        return;
    }
    cout << "\n=== AVAILABLE BOOKS ===" << endl;
    for (int i = 0; i < bookCount; ++i) {
        cout << "ID: " << books[i].id
             << ", Title: " << books[i].title
             << ", Author: " << books[i].author
             << ", Available: " << books[i].quantity << endl;
    }
}

bool isBookBorrowedByUser(int bookId, string username) {
    for (int i = 0; i < recordCount; ++i) {
        if (borrowRecords[i].bookId == bookId && 
            borrowRecords[i].username == username && 
            !borrowRecords[i].returned) {
            return true;
        }
    }
    return false;
}

void showRemainingTime(int bookId, string username) {
    for (int i = 0; i < recordCount; ++i) {
        if (borrowRecords[i].bookId == bookId && 
            borrowRecords[i].username == username && 
            !borrowRecords[i].returned) {
            
            time_t now = time(0);
            time_t borrowTime = borrowRecords[i].borrowTime;
            double elapsedMinutes = difftime(now, borrowTime) / 60.0;
            double remainingMinutes = BORROW_TIME_MINUTES - elapsedMinutes;
            
            if (remainingMinutes > 0) {
                cout << "Time remaining: " << remainingMinutes << " minutes" << endl;
            } else {
                cout << "??  BOOK IS OVERDUE! Please return as soon as possible." << endl;
            }
            return;
        }
    }
}

void borrowBook(int bookId) {
    // Check if user already has this book
    if (isBookBorrowedByUser(bookId, currentUser->username)) {
        cout << "You have already borrowed this book!" << endl;
        showRemainingTime(bookId, currentUser->username);
        return;
    }
    
    for (int i = 0; i < bookCount; ++i) {
        if (books[i].id == bookId) {
            if (books[i].quantity == 0) {
                cout << "All copies of the book are borrowed." << endl;
            } else {
                books[i].quantity--;
                
                // Add to borrow records
                borrowRecords[recordCount].bookId = bookId;
                borrowRecords[recordCount].username = currentUser->username;
                borrowRecords[recordCount].borrowTime = time(0);
                borrowRecords[recordCount].returned = false;
                recordCount++;
                
                cout << "\n=== BOOK ISSUED SUCCESSFULLY ===" << endl;
                cout << "Book: " << books[i].title << endl;
                cout << "Borrow Time: " << ctime(&borrowRecords[recordCount-1].borrowTime);
                cout << "Return Within: " << BORROW_TIME_MINUTES << " minutes" << endl;
                cout << "Remaining copies: " << books[i].quantity << endl;
                
                cout << "\n? COUNTDOWN STARTED! You have " << BORROW_TIME_MINUTES << " minutes." << endl;
            }
            return;
        }
    }
    cout << "Book not found." << endl;
}

void returnBook(int bookId) {
    for (int i = 0; i < bookCount; ++i) {
        if (books[i].id == bookId) {
            // Find borrow record
            for (int j = 0; j < recordCount; ++j) {
                if (borrowRecords[j].bookId == bookId && 
                    borrowRecords[j].username == currentUser->username && 
                    !borrowRecords[j].returned) {
                    
                    borrowRecords[j].returned = true;
                    borrowRecords[j].returnTime = time(0);
                    
                    books[i].quantity++;
                    
                    // Calculate actual borrow time
                    double borrowMinutes = difftime(borrowRecords[j].returnTime, borrowRecords[j].borrowTime) / 60.0;
                    
                    cout << "\n=== BOOK RETURNED SUCCESSFULLY ===" << endl;
                    cout << "Book: " << books[i].title << endl;
                    cout << "Total borrow time: " << borrowMinutes << " minutes" << endl;
                    cout << "Remaining copies: " << books[i].quantity << endl;
                    
                    if (borrowMinutes > BORROW_TIME_MINUTES) {
                        cout << "??  NOTE: Book was returned " << (borrowMinutes - BORROW_TIME_MINUTES) 
                             << " minutes late!" << endl;
                        cout << "But don't worry, you can still return books after deadline!" << endl;
                    } else {
                        cout << "? Great! You returned the book on time!" << endl;
                    }
                    return;
                }
            }
            cout << "No active borrow record found for this book." << endl;
            return;
        }
    }
    cout << "Book not found." << endl;
}

void showMyBooks() {
    cout << "\n=== MY BORROWED BOOKS ===" << endl;
    bool found = false;
    
    for (int i = 0; i < recordCount; ++i) {
        if (borrowRecords[i].username == currentUser->username && !borrowRecords[i].returned) {
            found = true;
            for (int j = 0; j < bookCount; ++j) {
                if (books[j].id == borrowRecords[i].bookId) {
                    cout << "ID: " << books[j].id << ", Title: " << books[j].title;
                    
                    // Show remaining time
                    time_t now = time(0);
                    double elapsedMinutes = difftime(now, borrowRecords[i].borrowTime) / 60.0;
                    double remainingMinutes = BORROW_TIME_MINUTES - elapsedMinutes;
                    
                    if (remainingMinutes > 0) {
                        cout << " - Time left: " << remainingMinutes << " minutes" << endl;
                    } else {
                        cout << " - ??  OVERDUE! (" << -remainingMinutes << " minutes late)" << endl;
                    }
                    break;
                }
            }
        }
    }
    
    if (!found) {
        cout << "No books currently borrowed." << endl;
    }
}

// Admin only functions
void addUser() {
    if (currentUser->role != "admin") {
        cout << "Access denied! Only admins can add users." << endl;
        return;
    }
    
    if (userCount >= MAX_USERS) {
        cout << "Maximum user limit reached!" << endl;
        return;
    }
    
    string username, password, role;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;
    cout << "Enter role (admin/user): ";
    cin >> role;
    
    if (role != "admin" && role != "user") {
        cout << "Invalid role! Use 'admin' or 'user'." << endl;
        return;
    }
    
    users[userCount++] = {username, password, role};
    cout << "User added successfully!" << endl;
}

void showAdminMenu() {
    int choice;
    string title, author;
    int bookId, quantity;

    while (true) {
        cout << "\n=== ADMIN MENU ===" << endl;
        cout << "1. Add Book\n";
        cout << "2. Remove Book\n";
        cout << "3. Display Books\n";
        cout << "4. Add User\n";
        cout << "5. View All Borrow Records\n";
        cout << "6. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Enter book title: ";
                cin.ignore();
                getline(cin, title);
                cout << "Enter book author: ";
                getline(cin, author);
                cout << "Enter quantity: ";
                cin >> quantity;
                addBook(title, author, quantity);
                break;
            case 2:
                cout << "Enter book ID to remove: ";
                cin >> bookId;
                removeBook(bookId);
                break;
            case 3:
                displayBooks();
                break;
            case 4:
                addUser();
                break;
            case 5:
                cout << "\n=== ALL BORROW RECORDS ===" << endl;
                for (int i = 0; i < recordCount; ++i) {
                    cout << "User: " << borrowRecords[i].username 
                         << ", Book ID: " << borrowRecords[i].bookId
                         << ", Returned: " << (borrowRecords[i].returned ? "Yes" : "No");
                    if (!borrowRecords[i].returned) {
                        double elapsed = difftime(time(0), borrowRecords[i].borrowTime) / 60.0;
                        if (elapsed > BORROW_TIME_MINUTES) {
                            cout << " ?? OVERDUE";
                        }
                    }
                    cout << endl;
                }
                break;
            case 6:
                logout();
                return;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
        cout << "**********************************" << endl;
    }
}

void showUserMenu() {
    int choice;
    int bookId;

    while (true) {
        cout << "\n=== USER MENU ===" << endl;
        cout << "1. Display Available Books\n";
        cout << "2. Borrow Book (" << BORROW_TIME_MINUTES << " minutes limit)\n";
        cout << "3. Return Book (Can return even after deadline)\n";
        cout << "4. View My Borrowed Books\n";
        cout << "5. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                displayBooks();
                break;
            case 2:
                cout << "Reminder! Please Return The Book Within " << BORROW_TIME_MINUTES << " Minutes" << endl;
                cout << "Enter book ID to borrow: ";
                cin >> bookId;
                borrowBook(bookId);
                break;
            case 3:
                cout << "Enter book ID to return: ";
                cin >> bookId;
                returnBook(bookId);
                break;
            case 4:
                showMyBooks();
                break;
            case 5:
                logout();
                return;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
        cout << "**********************************" << endl;
    }
}

int main() {
    initializeUsers();
    
    while (true) {
        cout << "\n=== LIBRARY MANAGEMENT SYSTEM ===" << endl;
        cout << "=== " << BORROW_TIME_MINUTES << " MINUTES BORROW SYSTEM ===" << endl;
        cout << "=== YOU CAN RETURN BOOKS EVEN AFTER DEADLINE ===" << endl;
        cout << "1. Login" << endl;
        cout << "2. Exit" << endl;
        cout << "Enter your choice: ";
        
        int choice;
        cin >> choice;
        
        switch (choice) {
            case 1:
                if (login()) {
                    if (currentUser->role == "admin") {
                        showAdminMenu();
                    } else {
                        showUserMenu();
                    }
                }
                break;
            case 2:
                cout << "Thank you for using Library Management System!" << endl;
                return 0;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
}