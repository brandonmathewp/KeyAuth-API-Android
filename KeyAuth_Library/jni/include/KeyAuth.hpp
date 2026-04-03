#pragma once
#include "Json/json.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <jni.h>

using json = nlohmann::json;

class __attribute__((visibility("default"))) KeyAuthApp {
public:
  struct user_data_class {
    std::string username, ip, hwid, expires, createdate, lastlogin,
        subscription;
  };

  struct application_data_class {
    std::string numUsers, numKeys, app_ver, customer_panel, onlineUsers;
  };

  std::string name, ownerid, version, url;
  std::string sessionid;
  bool initialized = false;
  user_data_class user_data;
  application_data_class app_data;

  KeyAuthApp(std::string name, std::string ownerid, std::string version);

  bool init(std::string hash = "");
  bool login(std::string user, std::string pass);
  bool register_user(std::string user, std::string pass, std::string key);
  bool upgrade(std::string user, std::string key);
  bool license(std::string key);
  bool fetchStats();
  std::string var(std::string varid);
  std::string getvar(std::string varid);
  bool setvar(std::string varid, std::string vardata);
  bool ban();
  std::string webhook(std::string webid, std::string param,
                      std::string body = "", std::string conttype = "");
  bool check();
  bool checkblacklist();
  void log(std::string message);
  bool change_username(std::string newname);
  bool logout();
  std::vector<uint8_t> download(std::string fileid);

private:
  static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                              void *userp);
  std::string get_hwid();
  std::string req(json data);
  void load_user_data(json data);
  void load_app_data(json data);
};
