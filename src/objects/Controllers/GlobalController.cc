#include <filesystem>
#include <string>
#include <thread>

#include "helpers.hh"
#include "interface.hh"
#include "nlohmann/json.hpp"
#include "objects/Configs/GlobalConfig.hh"
#include "objects/Controllers/Controller.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Registries/GlobalRegistry.hh"
#include "objects/ZCError.hh"

namespace fs = std::filesystem;

GlobalController::GlobalController(Logger log, bool force) : Controller(log, force, getZCRootDir())
{
  bin_dir_ = root_dir_ / BIN_DIR;
  lib_dir_ = root_dir_ / LIB_DIR;
  include_dir_ = root_dir_ / INCLUDE_DIR;

  auto global_config = std::make_unique<GlobalConfig>(root_dir_ / CONFIG);
  gc_ = global_config.get();
  c_ = std::move(global_config);
  r_ = std::make_unique<GlobalRegistry>(root_dir_ / REGISTRY);
}

void GlobalController::initializeWithTemplate(
    const std::filesystem::path &root, const std::string &template_to_use
) const
{
  const fs::path t_path = p_templates_dir_ / template_to_use;
  if (!fs::exists(t_path))
    throw ZCError(ZC_NOT_FOUND, "The following template was not found: " + template_to_use);

  for (const auto &entry : fs::recursive_directory_iterator(t_path))
  {
    const fs::path &src_path = entry.path();
    fs::path rel_path = fs::relative(src_path, t_path);

    if (rel_path.empty() || rel_path == ".")
      continue;

    fs::path dest_path = root / rel_path;
    if (fs::exists(dest_path) && !force_)
      if (!ask("The entry " + dest_path.string() + " already exists. Do you want to overwrite it ?"))
        continue;

    // Copy stuff
    if (fs::is_symlink(src_path))
    {
      fs::path target = fs::read_symlink(src_path);

      if (fs::is_directory(src_path))
        fs::create_directory_symlink(target, dest_path);
      else
        fs::create_symlink(target, dest_path);
    }
    else if (fs::is_directory(src_path))
    {
      fs::create_directories(dest_path);
    }
    else if (fs::is_regular_file(src_path))
    {
      fs::create_directories(dest_path.parent_path());
      fs::copy_file(src_path, dest_path, fs::copy_options::overwrite_existing);
    }
  }
}

std::vector<fs::path> GlobalController::getProjectTemplates() const
{
  std::vector<fs::path> templates_list;
  try
  {
    if (fs::exists(p_templates_dir_) && fs::is_directory(p_templates_dir_))
      for (const auto &entry : fs::directory_iterator(p_templates_dir_))
        if (entry.is_directory())
          templates_list.push_back(entry.path());
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return templates_list;
}

std::vector<fs::path> GlobalController::getTemplates() const
{
  std::vector<fs::path> file_list;
  try
  {
    if (fs::exists(templates_dir_) && fs::is_directory(templates_dir_))
      for (const auto &entry : fs::directory_iterator(templates_dir_))
        if (entry.is_regular_file())
          file_list.emplace_back(entry.path());
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return file_list;
}

Table GlobalController::projectTemplatesTable() const
{
  std::vector<std::vector<std::string>> str_p_t = {{"Name"}};
  const auto templates = getProjectTemplates();
  for (const auto &t_path : templates) str_p_t.push_back({t_path.filename().string()});

  return {static_cast<int>(str_p_t.size()), 1, false, true, str_p_t};
}

Table GlobalController::templatesTable() const
{
  std::vector<std::vector<std::string>> str_t = {{"Template"}};
  const auto templates = getTemplates();
  for (const auto &t_path : templates) str_t.push_back({t_path.filename().string()});

  return {static_cast<int>(str_t.size()), 1, false, true, str_t};
}

void GlobalController::logout()
{
  gc_->token_ = "";
  gc_->write();
  log_(LogLevel::SUCCESS, "Successfully logged out.");
}

void GlobalController::login()
{
  log_(LogLevel::INFO, "Initiating GitHub authentication...");

  std::string payload = "client_id=" CLIENT_ID "&scope=public_repo";

  std::string response = net_.post(DEVICE_CODE_URL, payload);
  auto json_resp = nlohmann::json::parse(response);

  if (json_resp.contains("error"))
  {
    throw ZCError(
        ZC_NETWORK_ERROR, "Failed to initiate login: " + json_resp["error_description"].get<std::string>()
    );
  }

  std::string device_code = json_resp["device_code"];
  std::string user_code = json_resp["user_code"];
  std::string verification_uri = json_resp["verification_uri"];
  int interval = json_resp["interval"];

  log_(LogLevel::INFO, "");
  log_(LogLevel::INFO, "===============================================================");
  log_(LogLevel::INFO, "1. Open your browser and go to: " U_BLUE + verification_uri + COLOR_RESET);
  log_(LogLevel::INFO, "2. Enter the following code:    " B_WHITE + user_code + COLOR_RESET);
  log_(LogLevel::INFO, "===============================================================");
  log_(LogLevel::INFO, "");

  log_(LogLevel::INFO, "Waiting for authorization (press Ctrl+C to abort)...");
  fl();

  std::string token_payload = "client_id=" CLIENT_ID "&device_code=" + device_code +
                              "&grant_type=urn:ietf:params:oauth:grant-type:device_code";

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(interval));

    std::string poll_resp = net_.post(TOKEN_URL, token_payload);
    auto poll_json = nlohmann::json::parse(poll_resp);

    if (poll_json.contains("error"))
    {
      std::string err = poll_json["error"];

      if (err == "authorization_pending")
      {
        fl();
        continue;
      }
      else if (err == "slow_down")
      {
        interval += 5;
        continue;
      }
      else if (err == "expired_token")
      {
        throw ZCError(ZC_AUTHENTICATION_ERROR, "The code has expired. Please run 'zc login' again.");
      }
      else
      {
        throw ZCError(
            ZC_NETWORK_ERROR, "Authorization failed: " + poll_json["error_description"].get<std::string>()
        );
      }
    }
    else if (poll_json.contains("access_token"))
    {
      nl();
      std::string access_token = poll_json["access_token"];

      gc_->token_ = access_token;
      gc_->write();

      log_(LogLevel::SUCCESS, "Successfully authenticated with GitHub!");
      log_(LogLevel::INFO, "Your credentials have been securely saved to ~/.zc/zc.json");
      break;
    }
  }
}
