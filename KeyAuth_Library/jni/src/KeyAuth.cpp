#include "KeyAuth.hpp"
#include "oxorany_include.h"
#include <cstring>
#include <curl/curl.h>
#include <iomanip>
#include <sodium.h>
#include <sstream>
#include <sys/system_properties.h>
#include <vector>


KeyAuthApp::KeyAuthApp(std::string name, std::string ownerid,
                       std::string version)
    : name(name), ownerid(ownerid), version(version) {
  this->url = oxorany("https://keyauth.win/api/1.3/");
}

size_t KeyAuthApp::WriteCallback(void *contents, size_t size, size_t nmemb,
                                 void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

bool KeyAuthApp::init() {
  if (initialized)
    return true;

  json data;
  data[oxorany("type")] = oxorany("init");
  data[oxorany("ver")] = version;
  data[oxorany("hash")] = oxorany("null");
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  if (response == oxorany("KeyAuth_Invalid"))
    return false;

  try {
    auto j = json::parse(response);
    if (j[oxorany("success")]) {
      sessionid = j[oxorany("sessionid")];
      initialized = true;
      return true;
    }
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::login(std::string user, std::string pass) {
  if (!initialized)
    if (!init())
      return false;

  json data;
  data[oxorany("type")] = oxorany("login");
  data[oxorany("username")] = user;
  data[oxorany("pass")] = pass;
  data[oxorany("hwid")] = get_hwid();
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[oxorany("success")]) {
      load_user_data(j[oxorany("info")]);
      return true;
    }
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::register_user(std::string user, std::string pass,
                               std::string key) {
  if (!initialized)
    if (!init())
      return false;

  json data;
  data[oxorany("type")] = oxorany("register");
  data[oxorany("username")] = user;
  data[oxorany("pass")] = pass;
  data[oxorany("key")] = key;
  data[oxorany("hwid")] = get_hwid();
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[oxorany("success")]) {
      load_user_data(j[oxorany("info")]);
      return true;
    }
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::upgrade(std::string user, std::string key) {
  if (!initialized)
    if (!init())
      return false;

  json data;
  data[oxorany("type")] = oxorany("upgrade");
  data[oxorany("username")] = user;
  data[oxorany("key")] = key;
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[oxorany("success")];
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::license(std::string key) {
  if (!initialized)
    if (!init())
      return false;

  json data;
  data[oxorany("type")] = oxorany("license");
  data[oxorany("key")] = key;
  data[oxorany("hwid")] = get_hwid();
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[oxorany("success")]) {
      load_user_data(j[oxorany("info")]);
      return true;
    }
  } catch (...) {
  }
  return false;
}

std::string KeyAuthApp::var(std::string varid) {
  if (!initialized)
    return "";

  json data;
  data[oxorany("type")] = oxorany("var");
  data[oxorany("varid")] = varid;
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[oxorany("success")]) {
      return j[oxorany("message")];
    }
  } catch (...) {
  }
  return "";
}

std::string KeyAuthApp::getvar(std::string varid) {
  if (!initialized)
    return "";

  json data;
  data[oxorany("type")] = oxorany("getvar");
  data[oxorany("var")] = varid;
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[oxorany("success")]) {
      return j[oxorany("response")];
    }
  } catch (...) {
  }
  return "";
}

bool KeyAuthApp::setvar(std::string varid, std::string vardata) {
  if (!initialized)
    return false;

  json data;
  data[oxorany("type")] = oxorany("setvar");
  data[oxorany("var")] = varid;
  data[oxorany("data")] = vardata;
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[oxorany("success")];
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::ban() {
  if (!initialized)
    return false;

  json data;
  data[oxorany("type")] = oxorany("ban");
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[oxorany("success")];
  } catch (...) {
  }
  return false;
}

std::string KeyAuthApp::webhook(std::string webid, std::string param,
                                std::string body, std::string conttype) {
  if (!initialized)
    return "";

  json data;
  data[oxorany("type")] = oxorany("webhook");
  data[oxorany("webid")] = webid;
  data[oxorany("params")] = param;
  data[oxorany("body")] = body;
  data[oxorany("conttype")] = conttype;
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[oxorany("success")]) {
      return j[oxorany("message")];
    }
  } catch (...) {
  }
  return "";
}

bool KeyAuthApp::check() {
  if (!initialized)
    return false;

  json data;
  data[oxorany("type")] = oxorany("check");
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[oxorany("success")];
  } catch (...) {
  }
  return false;
}

void KeyAuthApp::log(std::string message) {
  if (!initialized)
    return;

  json data;
  data[oxorany("type")] = oxorany("log");
  data[oxorany("message")] = message;
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  req(data);
}

bool KeyAuthApp::change_username(std::string newname) {
  if (!initialized)
    return false;

  json data;
  data[oxorany("type")] = oxorany("changeUsername");
  data[oxorany("newUsername")] = newname;
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[oxorany("success")];
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::logout() {
  if (!initialized)
    return false;

  json data;
  data[oxorany("type")] = oxorany("logout");
  data[oxorany("sessionid")] = sessionid;
  data[oxorany("name")] = name;
  data[oxorany("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[oxorany("success")];
  } catch (...) {
  }
  return false;
}

std::string KeyAuthApp::get_hwid() {
  const char *props[] = {oxorany("ro.product.manufacturer"),
                         oxorany("ro.product.model"),
                         oxorany("ro.board.platform"),
                         oxorany("ro.product.cpu.abi"),
                         oxorany("ro.serialno"),
                         oxorany("ro.build.fingerprint")};

  std::string raw_id = "";
  char buffer[PROP_VALUE_MAX];

  for (const char *prop : props) {
    memset(buffer, 0, sizeof(buffer));
    __system_property_get(prop, buffer);
    raw_id += std::string(buffer);
    raw_id += oxorany("|");
  }

  if (raw_id.length() < 10) {
    raw_id = oxorany("unknown_device_fingerprint");
  }

  unsigned char hash[crypto_hash_sha256_BYTES];
  crypto_hash_sha256(hash, (const unsigned char *)raw_id.c_str(),
                     raw_id.length());

  std::stringstream ss;
  for (int i = 0; i < crypto_hash_sha256_BYTES; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }

  return ss.str();
}

std::string KeyAuthApp::req(json data) {
  CURL *curl = curl_easy_init();
  std::string response_string;
  if (curl) {
    std::string post_str = "";
    for (auto &el : data.items()) {
      if (!post_str.empty())
        post_str += "&";

      char *escaped_key = curl_easy_escape(curl, el.key().c_str(), 0);
      std::string val_str;
      if (el.value().is_string())
        val_str = el.value().get<std::string>();
      else
        val_str = el.value().dump();

      char *escaped_val = curl_easy_escape(curl, val_str.c_str(), 0);

      post_str += std::string(escaped_key) + "=" + std::string(escaped_val);

      curl_free(escaped_key);
      curl_free(escaped_val);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return response_string;
}

void KeyAuthApp::load_user_data(json data) {
  user_data.username = data[oxorany("username")];
  user_data.ip = data[oxorany("ip")];
  user_data.hwid = data[oxorany("hwid")].is_null()
                       ? oxorany("N/A")
                       : data[oxorany("hwid")].get<std::string>();
  user_data.createdate = data[oxorany("createdate")];
  user_data.lastlogin = data[oxorany("lastlogin")];
  if (data[oxorany("subscriptions")].is_array() &&
      !data[oxorany("subscriptions")].empty()) {
    user_data.expires = data[oxorany("subscriptions")][0][oxorany("expiry")];
    user_data.subscription =
        data[oxorany("subscriptions")][0][oxorany("subscription")];
  }
}

void KeyAuthApp::load_app_data(json data) {
  app_data.numUsers = data[oxorany("numUsers")];
  app_data.numKeys = data[oxorany("numKeys")];
  app_data.app_ver = data[oxorany("version")];
  app_data.customer_panel = data[oxorany("customerPanelLink")];
  app_data.onlineUsers = data[oxorany("numOnlineUsers")];
}
