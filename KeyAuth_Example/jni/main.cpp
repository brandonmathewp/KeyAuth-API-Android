#include "include/KeyAuth.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <fstream>

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

  // --- Blacklist Test ---
  std::cout << "[*] Checking blacklist status..." << std::endl;
  if (app.checkblacklist()) {
    std::cout << "[!] Device is blacklisted! Access denied." << std::endl;
    return 1;
  }
  std::cout << "[+] Device is not blacklisted." << std::endl;

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
      break; 
    } else {
      std::cout << "[!] Action failed!" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  }
  
  app.fetchStats();

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
  
  if (app.check()) {
    std::cout << "\n[+] Session validation check: PASSED" << std::endl;
  } else {
    std::cout << "\n[+] Session validation check: FAILED" << std::endl;
  }

  // --- Download File Test ---
  std::cout << "\n--- Testing File Download ---" << std::endl;
  std::string file_id = "xxxxxxxxxx"; // download file id here
  std::cout << "[*] Downloading file ID: " << file_id << "..." << std::endl;
  
  std::vector<uint8_t> file_bytes = app.download(file_id);
  
  if (!file_bytes.empty()) {
    std::cout << "[+] Download successful! Received " << file_bytes.size() << " bytes." << std::endl;
    std::ofstream out("downloaded_test.png", std::ios::binary);
    out.write(reinterpret_cast<const char*>(file_bytes.data()), file_bytes.size());
    out.close();
    std::cout << "[+] File saved as 'downloaded_test.png'" << std::endl;
  } else {
    std::cout << "[!] Download failed. Verify the File ID exists on the dashboard." << std::endl;
  }

  // Final logs and checks
  std::cout << "\n--- Finalizing ---" << std::endl;
  app.log("Terminal session test completed successfully.");

  std::cout << "\nExiting in 5 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  return 0;
}
