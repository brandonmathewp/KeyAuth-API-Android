#include "include/KeyAuth.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main() {
  std::string name = "YourAppName";
  std::string ownerid = "YourOwnerID";
  std::string version = "1.0";

  KeyAuthApp app(name, ownerid, version);

  std::cout << "[*] Initializing session..." << std::endl;
  if (!app.init()) {
    std::cout << "[!] Init Failed! Check your Name/OwnerID/Version."
              << std::endl;
    return 1;
  }
  std::cout << "[+] Init Success!" << std::endl;

  while (true) {
    std::cout << "\n1. Login" << std::endl;
    std::cout << "2. Register" << std::endl;
    std::cout << "3. Upgrade" << std::endl;
    std::cout << "4. License Key Only" << std::endl;
    std::cout << "5. Exit" << std::endl;

    std::cout << "\nSelect Option: ";
    int option;
    std::cin >> option;

    std::string user, pass, key;
    bool success = false;

    switch (option) {
    case 1:
      std::cout << "Provide username: ";
      std::cin >> user;
      std::cout << "Provide password: ";
      std::cin >> pass;
      success = app.login(user, pass);
      break;
    case 2:
      std::cout << "Provide username: ";
      std::cin >> user;
      std::cout << "Provide password: ";
      std::cin >> pass;
      std::cout << "Provide license: ";
      std::cin >> key;
      success = app.register_user(user, pass, key);
      break;
    case 3:
      std::cout << "Provide username: ";
      std::cin >> user;
      std::cout << "Provide license: ";
      std::cin >> key;
      success = app.upgrade(user, key);
      break;
    case 4:
      std::cout << "Enter your license: ";
      std::cin >> key;
      success = app.license(key);
      break;
    case 5:
      return 0;
    default:
      std::cout << "Invalid option!" << std::endl;
      continue;
    }

    if (success) {
      std::cout << "[+] Action successful!" << std::endl;
      break; // Break menu and show data
    } else {
      std::cout << "[!] Action failed!" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  }

  // Display Application Data
  std::cout << "\n--- Application Data ---" << std::endl;
  std::cout << "App Version: " << app.app_data.app_ver << std::endl;
  std::cout << "Customer Panel Link: " << app.app_data.customer_panel
            << std::endl;
  std::cout << "Number of Keys: " << app.app_data.numKeys << std::endl;
  std::cout << "Number of Users: " << app.app_data.numUsers << std::endl;
  std::cout << "Online Users: " << app.app_data.onlineUsers << std::endl;

  // Display User Data
  std::cout << "\n--- User Data ---" << std::endl;
  std::cout << "Username: " << app.user_data.username << std::endl;
  std::cout << "IP address: " << app.user_data.ip << std::endl;
  std::cout << "Hardware-Id: " << app.user_data.hwid << std::endl;
  std::cout << "Subscription: " << app.user_data.subscription << std::endl;
  std::cout << "Created at: " << app.user_data.createdate << std::endl;
  std::cout << "Last login at: " << app.user_data.lastlogin << std::endl;
  std::cout << "Expires at: " << app.user_data.expires << std::endl;

  // Test other features
  std::cout << "\n--- Testing other functions ---" << std::endl;
  std::cout << "[*] Testing Log function..." << std::endl;
  app.log("Test log message from C++");

  std::cout << "[*] Testing Check function..." << std::endl;
  if (app.check())
    std::cout << "[+] Session is valid" << std::endl;

  std::cout << "\nExiting in 5 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  return 0;
}
