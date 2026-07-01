#include "GConf.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <thread>

#include "Conf.h"
#include "config/LanguageConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "Language.h"
#include "pkgs/Network.h"
#include "ui/Interface.h"
#include "ui/ui_utils.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

GConf &GConf::get()
{
  static GConf instance;
  return instance;
}

void GConf::login(bool force)
{
  if (!force && !token.empty() &&
      !ui().ask("An account is already logged in. Do you want to change the account ?"))
    throw ZCException(ZCE_ABORTED, "Interrupted");

  Network &net = Network::get();

  auto json_resp =
    nlohmann::json::parse(net.post(DEVICE_CODE_URL, "client_id=" CLIENT_ID "&scope=public_repo"));

  if (json_resp.contains("error"))
  {
    throw ZCException(
      ZCE_NETWORK_ERROR, "Failed to initiate login: " + json_resp["error_description"].get<string>()
    );
  }

  const string device_code      = json_resp["device_code"];
  const string user_code        = json_resp["user_code"];
  const string verification_uri = json_resp["verification_uri"];

  int interval = json_resp["interval"];

  ui().new_line();
  ui().info("===============================================================");
  ui().info("1. Open your browser and go to: " U_BLUE + verification_uri + RESET);
  ui().info("2. Enter the following code:    " B_WHITE + user_code + RESET);
  ui().info("===============================================================");
  ui().new_line();

  ui().info("Waiting for authorization (press Ctrl+C to abort)...");
  ui().flush_screen();

  const std::string token_payload = "client_id=" CLIENT_ID "&device_code=" + device_code +
                                    "&grant_type=urn:ietf:params:oauth:grant-type:device_code";

  while (true)
  {
    this_thread::sleep_for(std::chrono::seconds(interval));

    auto poll_json = nlohmann::json::parse(net.post(TOKEN_URL, token_payload));

    if (poll_json.contains("error"))
    {
      std::string err = poll_json["error"];

      if (err == "authorization_pending")
        ui().flush_screen();
      else if (err == "slow_down")
        interval += 5;
      else if (err == "expired_token")
        throw ZCException(ZCE_AUTHENTICATION_ERROR, "The code has expired. Please run `zc login` again.");
      else
        throw ZCException(
          ZCE_NETWORK_ERROR, "Authorization failed: " + poll_json["error_description"].get<string>()
        );
    }
    else if (poll_json.contains("access_token"))
    {
      ui().new_line();
      token     = poll_json["access_token"];
      modified_ = true;

      ui().success("Successfully authenticated with GitHub!");
      ui().success("Your credentials have been securely saved to " + file_.string());
      break;
    }
  }
}

void GConf::logout()
{
  if (token.empty())
  {
    ui().info("Already logged out.");
  }
  else
  {
    token     = "";
    modified_ = true;
    ui().success("Successfully logged out.");
  }
}

void GConf::edit_config(const bool force)
{
  if (!force && fs::exists(file_) &&
      !ui().ask("A configuration already exists. Do you want to override it ?"))
    throw ZCException(ZCE_ABORTED, "Aborted.");

#define ASK_BOOL_FIELDS(name, deflt, question) name = ui().ask(question, name);
  GCONF_BOOL_FIELDS(ASK_BOOL_FIELDS)
#undef ASK_BOOL_FIELDS

#define ASK_STR_FIELDS(name, deflt, question) name = ui().input(question, name);
  GCONF_STR_FIELDS(ASK_STR_FIELDS)
#undef ASK_STR_FIELDS

  modified_ = true;
}

void GConf::default_config(const bool force)
{
  if (!force && fs::exists(file_) &&
      !ui().ask("A configuration already exists. Do you want to override it ?"))
    throw ZCException(ZCE_ABORTED, "Aborted.");

#define RESET_FIELDS(name, deflt, question) name = deflt;
  GCONF_BOOL_FIELDS(RESET_FIELDS)
  GCONF_STR_FIELDS(RESET_FIELDS)
#undef RESET_FIELDS

  modified_ = true;
}

void GConf::set(const std::string &key, const std::string &value)
{
#define CHECK_BOOL(name, deflt, question)                                                             \
  if (key == #name)                                                                                   \
  {                                                                                                   \
    if (value == "true" || value == "1")                                                              \
      (name) = true;                                                                                  \
    else if (value == "false" || value == "0")                                                        \
      (name) = false;                                                                                 \
    else                                                                                              \
      throw ZCException(ZCE_TYPE_ERROR, "Invalid value for key '" #name "': expected true or false"); \
    modified_ = true;                                                                                 \
    return;                                                                                           \
  }
  GCONF_BOOL_FIELDS(CHECK_BOOL)
#undef CHECK_BOOL

#define CHECK_STR(name, deflt, question) \
  if (key == #name)                      \
  {                                      \
    (name)    = value;                   \
    modified_ = true;                    \
    return;                              \
  }
  GCONF_STR_FIELDS(CHECK_STR)
#undef CHECK_STR

  throw ZCException(ZCE_NOT_FOUND, "Unknown configuration key: '" + key + "'");
}

void GConf::load()
{
  const json root = read_json(file_);

#define LOAD_BOOL_FIELD(name, deflt, question) get_key(root, #name, name, deflt);
  GCONF_BOOL_FIELDS(LOAD_BOOL_FIELD)
#undef LOAD_BOOL_FIELD
#define LOAD_STR_FIELD(name, deflt, question) get_key(root, #name, name, string(deflt));
  GCONF_STR_FIELDS(LOAD_STR_FIELD)
#undef LOAD_STR_FIELD

  get_key(root, "token", token, string());
  get_key(root, "username", username, string());

  if (root.contains("languages") && root["languages"].is_object())
  {
    languages.clear();
    for (const auto &[key, value] : root["languages"].items())
      languages.insert_or_assign(language_from_str(key), value.get<LanguageConf>());
  }
}

void GConf::write()
{
  json root;
#define WRITE_FIELD(name, deflt, question) root[#name] = name;
  GCONF_BOOL_FIELDS(WRITE_FIELD)
  GCONF_STR_FIELDS(WRITE_FIELD)
#undef WRITE_FIELD

  json lang_json = json::object();
  for (const auto &l : languages)
    lang_json[language_to_str(l.first)] = l.second;
  root["languages"] = lang_json;

  if (!token.empty())
    root["token"] = token;
  if (!username.empty())
    root["username"] = username;

  write_json(root, file_);
  chmod(file_.c_str(), S_IRUSR | S_IWUSR); // other users are not allowed to see the authentication token
}

GConf::~GConf()
{
  if (modified_)
    GConf::write();
}

GConf::GConf() : Conf(zc_root() / CONFIG_FILE)
{
  if (fs::exists(file_))
    GConf::load();
}

} // namespace zc
