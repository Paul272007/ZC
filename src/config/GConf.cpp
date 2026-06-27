#include "GConf.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <thread>

#include "../excepts/ZCException.h"
#include "../helpers.h"
#include "../pkgs/Network.cpp"
#include "../ui/ui_utils.h"
#include "Conf.h"
#include "config/LanguageConf.h"
#include "excepts/ExitCode.h"
#include "Language.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

GConf &GConf::get()
{
  static GConf instance;
  return instance;
}

void GConf::login()
{
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

  if_.info("");
  if_.info("===============================================================");
  if_.info("1. Open your browser and go to: " U_BLUE + verification_uri + RESET);
  if_.info("2. Enter the following code:    " B_WHITE + user_code + RESET);
  if_.info("===============================================================");
  if_.info("");

  if_.info("Waiting for authorization (press Ctrl+C to abort)...");
  if_.flush_screen();

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
      {
        if_.flush_screen();
      }
      else if (err == "slow_down")
      {
        interval += 5;
      }
      else if (err == "expired_token")
      {
        throw ZCException(ZCE_AUTHENTICATION_ERROR, "The code has expired. Please run `zc login` again.");
      }
      else
      {
        throw ZCException(
          ZCE_NETWORK_ERROR, "Authorization failed: " + poll_json["error_description"].get<string>()
        );
      }
    }
    else if (poll_json.contains("access_token"))
    {
      if_.new_line();
      token     = poll_json["access_token"];
      modified_ = true;

      if_.success("Successfully authenticated with GitHub!");
      if_.success("Your credentials have been securely saved to " + file_.string());
      break;
    }
  }
}

void GConf::logout()
{
  if (token.empty())
  {
    if_.info("Already logged out.");
  }
  else
  {
    token     = "";
    modified_ = true;
    if_.success("Successfully logged out.");
  }
}

void GConf::load()
{
  const json root = read_json(file_);
  get_key(root, "always_keep", always_keep, false);
  get_key(root, "always_add_std", always_add_std, false);
  get_key(root, "open_after_init", open_after_init, false);
  get_key(root, "open_after_create", open_after_create, false);
  get_key(root, "clear_before_run", clear_before_run, false);
  get_key(root, "move_bin_to_current_path", move_bin_to_current_path, false);
  get_key(root, "editor", editor, editor);
  get_key(root, "token", token, string());
  get_key(root, "username", username, string());
  get_key(root, "archive", archive, archive);

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
  root["always_keep"]              = always_keep;
  root["always_add_std"]           = always_add_std;
  root["open_after_init"]          = open_after_init;
  root["open_after_create"]        = open_after_create;
  root["clear_before_run"]         = clear_before_run;
  root["move_bin_to_current_path"] = move_bin_to_current_path;
  root["editor"]                   = editor;
  root["archive"]                  = archive;

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
