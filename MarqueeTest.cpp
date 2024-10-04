#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <conio.h>

using namespace std;

// Function to set cursor position
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Function to clear the previous position of the marquee
void clearMarqueeArea(int prevX, int prevY, int textLength) {
    gotoxy(prevX, prevY);
    cout << string(textLength, ' '); // Clear the line at the previous position
}

// Function to move the marquee within the screen
void marqueeMovement(atomic<bool>& running, int screenWidth, int screenHeight, string& userInput) {
    std::string text = "Hi, this is a marquee";
    int x = 0, y = 0; // Start at the top left of the screen
    int dx = 1, dy = 1; // Movement direction

    // Hide the cursor for a cleaner effect
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // Hide the cursor
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    int prevX = x, prevY = y; // Track the previous position

    while (running) {
        // Clear the previous position of the marquee
        clearMarqueeArea(prevX, prevY, text.length());

        // Print marquee text at the new position
        gotoxy(x, y);
        std::cout << text;

        // Print the command input at the bottom of the screen, preserving the current user input
        gotoxy(0, screenHeight);
        std::cout << "Command: " << userInput;

        // Sleep for a short while to control speed
        Sleep(100);

        // Update previous position to current before the movement
        prevX = x;
        prevY = y;

        // Update position
        x += dx;
        y += dy;

        // Bounce text at edges of the screen area (leaving space for command input)
        if (x + text.length() >= screenWidth || x <= 0) dx = -dx;
        if (y >= screenHeight - 1 || y <= 0) dy = -dy;
    }
}

// Function to capture user input while preserving the current screen
void captureUserInput(string& userInput, atomic<bool>& running, int screenWidth, int screenHeight) {
    char ch;
    while (running) {
        ch = _getch(); // Get character without waiting for Enter
        if (ch == '\r') { // Enter key
            if (userInput == "exit") {
                running = false; // Exit the program if "exit" is typed
                break;
            }
            else {
                // Handle invalid commands
                gotoxy(0, screenHeight + 1); // Move below the command prompt area
                cout << "Invalid command";
                Sleep(1000); // Pause for a second to display the message

                // Clear the invalid command message
                gotoxy(0, screenHeight + 1);
                cout << string(screenWidth, ' '); // Clear the line
            }

            // Clear the command input area after pressing Enter
            userInput.clear(); // Clear the user input string

            // Move to command line and clear
            gotoxy(9, screenHeight); // Move after "Command: "
            cout << string(screenWidth - 9, ' '); // Clear previous input
        }
        else if (ch == '\b' && !userInput.empty()) { // Backspace
            userInput.pop_back(); // Remove last character
        }
        else if (ch >= 32 && ch <= 126) { // Printable characters
            userInput += ch; // Add character to input

            // Print the updated input after each character
            gotoxy(9, screenHeight); // Move cursor after "Command: "
            cout << userInput << string(screenWidth - 9 - userInput.size(), ' '); // Update input and clear extra chars
        }
    }
}


int main() {
    atomic<bool> running(true);
    string userInput;

    // Define the screen dimensions
    int screenWidth = 80; // Adjust according to your console width
    int screenHeight = 20; // Height for the marquee area, excluding the bottom for the command prompt

    // Launch the marquee movement in a separate thread
    thread marqueeThread(marqueeMovement, ref(running), screenWidth, screenHeight, ref(userInput));

    // Capture user input in the main thread
    captureUserInput(userInput, running, screenWidth,screenHeight);

    // Wait for the marquee thread to finish
    if (marqueeThread.joinable()) {
        marqueeThread.join();
    }

    return 0;
}
