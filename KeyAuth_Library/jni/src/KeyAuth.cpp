#include "KeyAuth.hpp"
#include "obfuscate.h"
#include <cstring>
#include <curl/curl.h>
#include <iomanip>
#include <sodium.h>
#include <sstream>
#include <sys/system_properties.h>
#include <vector>
#include <cstdlib>
#include <openssl/sha.h>
#include <stdio.h>

KeyAuthApp::KeyAuthApp(std::string name, std::string ownerid,
                       std::string version)
    : name(name), ownerid(ownerid), version(version) {
  this->url = OBFUSCATE("https://keyauth.win/api/1.3/");
}

size_t KeyAuthApp::WriteCallback(void *contents, size_t size, size_t nmemb,
                                 void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

bool KeyAuthApp::init(std::string hash) {
  if (initialized)
    return true;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("init");
  data[OBFUSCATE("ver")] = version;
  data[OBFUSCATE("hash")] = hash.empty() ? OBFUSCATE("null") : hash;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  if (response == OBFUSCATE("KeyAuth_Invalid"))
    return false;

  try {
    auto j = json::parse(response);
    if (j[OBFUSCATE("success")]) {
      sessionid = j[OBFUSCATE("sessionid")];
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
  data[OBFUSCATE("type")] = OBFUSCATE("login");
  data[OBFUSCATE("username")] = user;
  data[OBFUSCATE("pass")] = pass;
  data[OBFUSCATE("hwid")] = get_hwid();
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[OBFUSCATE("success")]) {
      load_user_data(j[OBFUSCATE("info")]);
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
  data[OBFUSCATE("type")] = OBFUSCATE("register");
  data[OBFUSCATE("username")] = user;
  data[OBFUSCATE("pass")] = pass;
  data[OBFUSCATE("key")] = key;
  data[OBFUSCATE("hwid")] = get_hwid();
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[OBFUSCATE("success")]) {
      load_user_data(j[OBFUSCATE("info")]);
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
  data[OBFUSCATE("type")] = OBFUSCATE("upgrade");
  data[OBFUSCATE("username")] = user;
  data[OBFUSCATE("key")] = key;
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[OBFUSCATE("success")];
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::license(std::string key) {
  if (!initialized)
    if (!init())
      return false;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("license");
  data[OBFUSCATE("key")] = key;
  data[OBFUSCATE("hwid")] = get_hwid();
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[OBFUSCATE("success")]) {
      load_user_data(j[OBFUSCATE("info")]);
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
  data[OBFUSCATE("type")] = OBFUSCATE("var");
  data[OBFUSCATE("varid")] = varid;
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[OBFUSCATE("success")]) {
      return j[OBFUSCATE("message")];
    }
  } catch (...) {
  }
  return "";
}

std::string KeyAuthApp::getvar(std::string varid) {
  if (!initialized)
    return "";

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("getvar");
  data[OBFUSCATE("var")] = varid;
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[OBFUSCATE("success")]) {
      return j[OBFUSCATE("response")];
    }
  } catch (...) {
  }
  return "";
}

bool KeyAuthApp::setvar(std::string varid, std::string vardata) {
  if (!initialized)
    return false;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("setvar");
  data[OBFUSCATE("var")] = varid;
  data[OBFUSCATE("data")] = vardata;
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[OBFUSCATE("success")];
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::ban() {
  if (!initialized)
    return false;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("ban");
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[OBFUSCATE("success")];
  } catch (...) {
  }
  return false;
}

std::string KeyAuthApp::webhook(std::string webid, std::string param,
                                std::string body, std::string conttype) {
  if (!initialized)
    return "";

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("webhook");
  data[OBFUSCATE("webid")] = webid;
  data[OBFUSCATE("params")] = param;
  data[OBFUSCATE("body")] = body;
  data[OBFUSCATE("conttype")] = conttype;
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[OBFUSCATE("success")]) {
      return j[OBFUSCATE("message")];
    }
  } catch (...) {
  }
  return "";
}

bool KeyAuthApp::check() {
  if (!initialized)
    return false;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("check");
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[OBFUSCATE("success")];
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::checkblacklist() {
  if (!initialized)
    return false;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("checkblacklist");
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[OBFUSCATE("success")];
  } catch (...) {
  }
  return false;
}

void KeyAuthApp::log(std::string message) {
  if (!initialized)
    return;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("log");
  data[OBFUSCATE("message")] = message;
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  req(data);
}

bool KeyAuthApp::change_username(std::string newname) {
  if (!initialized)
    return false;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("changeUsername");
  data[OBFUSCATE("newUsername")] = newname;
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[OBFUSCATE("success")];
  } catch (...) {
  }
  return false;
}

bool KeyAuthApp::logout() {
  if (!initialized)
    return false;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("logout");
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    return j[OBFUSCATE("success")];
  } catch (...) {
  }
  return false;
}

std::vector<uint8_t> KeyAuthApp::download(std::string fileid) {
  std::vector<uint8_t> ret;
  if (!initialized)
    return ret;

  json data;
  data[OBFUSCATE("type")] = OBFUSCATE("file");
  data[OBFUSCATE("fileid")] = fileid;
  data[OBFUSCATE("sessionid")] = sessionid;
  data[OBFUSCATE("name")] = name;
  data[OBFUSCATE("ownerid")] = ownerid;

  std::string response = req(data);
  try {
    auto j = json::parse(response);
    if (j[OBFUSCATE("success")]) {
      std::string contents = j[OBFUSCATE("contents")];
      for (size_t i = 0; i < contents.length(); i += 2) {
        std::string byteString = contents.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
        ret.push_back(byte);
      }
    }
  } catch (...) {
  }
  return ret;
}

std::string KeyAuthApp::get_hwid() {
  const char *props[] = {OBFUSCATE("ro.product.manufacturer"),
                         OBFUSCATE("ro.product.model"),
                         OBFUSCATE("ro.board.platform"),
                         OBFUSCATE("ro.product.cpu.abi"),
                         OBFUSCATE("ro.serialno"),
                         OBFUSCATE("ro.build.fingerprint")};

  std::string raw_id = "";
  char buffer[PROP_VALUE_MAX];

  for (const char *prop : props) {
    memset(buffer, 0, sizeof(buffer));
    __system_property_get(prop, buffer);
    raw_id += std::string(buffer);
    raw_id += OBFUSCATE("|");
  }

  if (raw_id.length() < 10) {
    raw_id = OBFUSCATE("unknown_device_fingerprint");
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

std::string KeyAuthApp::get_hash(JNIEnv *env) {
  jclass activityThreadClass = env->FindClass(OBFUSCATE("android/app/ActivityThread"));
  jmethodID currentApplicationMethod = env->GetStaticMethodID(activityThreadClass, OBFUSCATE("currentApplication"), OBFUSCATE("()Landroid/app/Application;"));
  jobject context = env->CallStaticObjectMethod(activityThreadClass, currentApplicationMethod);

  if (context == nullptr) {
      return OBFUSCATE("Error: Could not obtain application context.");
  }

  jclass contextClass = env->GetObjectClass(context);
  jmethodID getPackageCodePathMethod = env->GetMethodID(contextClass, OBFUSCATE("getPackageCodePath"), OBFUSCATE("()Ljava/lang/String;"));
  jstring apkPath = (jstring) env->CallObjectMethod(context, getPackageCodePathMethod);

  if (apkPath == nullptr) {
      return OBFUSCATE("Error: Could not obtain APK path.");
  }

  const char *apkPathStr = env->GetStringUTFChars(apkPath, nullptr);

  FILE *file = fopen(apkPathStr, OBFUSCATE("rb"));
  if (!file) {
      env->ReleaseStringUTFChars(apkPath, apkPathStr);
      return OBFUSCATE("Error: Could not open APK file via C standard I/O.");
  }

  SHA256_CTX sha256;
  SHA256_Init(&sha256);

  const int bufferSize = 65536; 
  unsigned char buffer[bufferSize];
  size_t bytesRead;

  while ((bytesRead = fread(buffer, 1, bufferSize, file)) > 0) {
      SHA256_Update(&sha256, buffer, bytesRead);
  }

  if (ferror(file)) {
      fclose(file);
      env->ReleaseStringUTFChars(apkPath, apkPathStr);
      return OBFUSCATE("Error: Failed while reading APK stream.");
  }

  fclose(file);
  env->ReleaseStringUTFChars(apkPath, apkPathStr);

  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_Final(hash, &sha256);

  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
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
  user_data.username = data[OBFUSCATE("username")];
  user_data.ip = data[OBFUSCATE("ip")];
  user_data.hwid = data[OBFUSCATE("hwid")].is_null()
                       ? OBFUSCATE("N/A")
                       : data[OBFUSCATE("hwid")].get<std::string>();
  user_data.createdate = data[OBFUSCATE("createdate")];
  user_data.lastlogin = data[OBFUSCATE("lastlogin")];
  if (data[OBFUSCATE("subscriptions")].is_array() &&
      !data[OBFUSCATE("subscriptions")].empty()) {
    user_data.expires = data[OBFUSCATE("subscriptions")][0][OBFUSCATE("expiry")];
    user_data.subscription =
        data[OBFUSCATE("subscriptions")][0][OBFUSCATE("subscription")];
  }
}

void KeyAuthApp::load_app_data(json data) {
  app_data.numUsers = data[OBFUSCATE("numUsers")];
  app_data.numKeys = data[OBFUSCATE("numKeys")];
  app_data.app_ver = data[OBFUSCATE("version")];
  app_data.customer_panel = data[OBFUSCATE("customerPanelLink")];
  app_data.onlineUsers = data[OBFUSCATE("numOnlineUsers")];
}
