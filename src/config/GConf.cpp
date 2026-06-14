/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <filesystem>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <thread>

#include "../excepts/ZCException.h"
#include "../helpers.h"
#include "../pkgs/Network.cpp"
#include "Conf.h"
#include "GConf.h"

ZC_DEV_CONFIG_JSON

/**
 * GConf implementation
 *
 * ZC Global Configuration
 */

/**
 * @return GConf
 */
GConf &GConf::get()
{
  static GConf instance;
  return instance;
}

/**
 * @return void
 */
void GConf::login()
{
  Network &net = Network::get();
  std::string response = net.post(DEVICE_CODE_URL, "client_id=" CLIENT_ID "&scope=public_repo");
  auto json_resp = nlohmann::json::parse(response);

  if (json_resp.contains("error"))
  {
    throw ZCException(
        ZCE_NETWORK_ERROR, "Failed to initiate login: " + json_resp["error_description"].get<std::string>()
    );
  }

  const std::string device_code = json_resp["device_code"];
  const std::string user_code = json_resp["user_code"];
  const std::string verification_uri = json_resp["verification_uri"];
  int interval = json_resp["interval"];

  if_.info("");
  if_.info("===============================================================");
  if_.info("1. Open your browser and go to: " U_BLUE + verification_uri + RESET);
  if_.info("2. Enter the following code:    " B_WHITE + user_code + RESET);
  if_.info("===============================================================");
  if_.info("");

  if_.info("Waiting for authorization (press Ctrl+C to abort)...");
  if_.flush();

  const std::string token_payload = "client_id=" CLIENT_ID "&device_code=" + device_code +
                              "&grant_type=urn:ietf:params:oauth:grant-type:device_code";

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(interval));

    std::string poll_resp = net.post(TOKEN_URL, token_payload);
    auto poll_json = nlohmann::json::parse(poll_resp);

    if (poll_json.contains("error"))
    {
      std::string err = poll_json["error"];

      if (err == "authorization_pending")
      {
        if_.flush();
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
            ZCE_NETWORK_ERROR, "Authorization failed: " + poll_json["error_description"].get<std::string>()
        );
      }
    }
    else if (poll_json.contains("access_token"))
    {
      if_.new_line();
      std::string access_token = poll_json["access_token"];

      token = access_token;
      modified_ = true;

      if_.success("Successfully authenticated with GitHub!");
      if_.success("Your credentials have been securely saved to " + file_.string());
      break;
    }
  }
}

/**
 * @return void
 */
void GConf::logout()
{
  token = "";
  modified_ = true;
  if_.success("Successfully logged out.");
}

void GConf::load()
{
  const json root = if_.read_json(file_);
  get_key(root, "alway_keep", alway_keep);
  get_key(root, "always_add_std", always_add_std);
  get_key(root, "open_after_init", open_after_init);
  get_key(root, "open_after_create", open_after_create);
  get_key(root, "clear_before_run", clear_before_run);
  get_key(root, "move_bin_to_current_path", move_bin_to_current_path);
  get_key(root, "c_compiler", c_compiler);
  get_key(root, "cxx_compiler", cxx_compiler);
  get_key(root, "c_std", c_std);
  get_key(root, "cxx_std", cxx_std);
  get_key(root, "editor", editor);
  get_key(root, "token", token);
  get_key(root, "username", username);
  get_key(root, "flags", flags);
}

void GConf::write()
{
  json root;
  root["alway_keep"] = alway_keep;
  root["always_add_std"] = always_add_std;
  root["open_after_init"] = open_after_init;
  root["open_after_create"] = open_after_create;
  root["clear_before_run"] = clear_before_run;
  root["move_bin_to_current_path"] = move_bin_to_current_path;
  root["c_compiler"] = c_compiler;
  root["cxx_compiler"] = cxx_compiler;
  root["c_std"] = c_std;
  root["cxx_std"] = cxx_std;
  root["editor"] = editor;
  root["flags"] = flags;

  if (!token.empty())
    root["token"] = token;
  if (!username.empty())
    root["username"] = username;

  if_.write_json(root, file_);
  chmod(file_.c_str(), S_IRUSR | S_IWUSR);
}

GConf::~GConf()
{
  if (modified_)
    GConf::write();
}

GConf::GConf() : Conf(get_zc_root() / CONFIG_FILE)
{
  if (fs::exists(file_))
    GConf::load();
}
