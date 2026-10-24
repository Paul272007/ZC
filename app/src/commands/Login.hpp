#pragma once

#include <chrono>
#include <thread>

#include "commands/Command.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "core/util/GithubClient.hpp"
#include "framework/network.hpp"

namespace zc::app
{

class Login final : public Command
{
  [[nodiscard]] static json initiateLogin()
  {
    auto resp = net::post(
      std::string(core::zc_device_code_url),
      std::format("client_id={}&scope=public_repo", core::zc_client_id),
      { { .name = "Accept", .value = "application/json" } }
    );
    if (!resp)
      throw_or_terminate(resp.error());

    json json_resp = json::parse(resp->body);

    if (json_resp.contains("error"))
      throw_or_terminate(Exception(
        OSError::NetworkError, "Failed to initiate login: {}", json_resp["error"].get<std::string_view>()
      ));

    return json_resp;
  }

public:
  explicit Login(const CommandCtx &cmd_ctx) : Command(cmd_ctx, false) {}

  void execute() override
  {
    if (!force_ && !core::gc().token().empty()) {
      const bool res = ui.ask("An account is already logged in. Do you want to change the account ?");
      if (!res)
        throw_or_terminate(Exception(CliError::Interrupted, "interrupted"));
    }

    json resp = initiateLogin();

    const std::string device_code      = resp["device_code"];
    const std::string user_code        = resp["user_code"];
    const std::string verification_uri = resp["verification_uri"];

    i32 interval = resp["interval"];

    ui.ln();
    ui.info("===============================================================");
    ui.info("1. Open your browser and go to: {}{}{}", blue, verification_uri, reset);
    ui.info("2. Enter the following code:    {}{}{}", b_white, user_code, reset);
    ui.info("===============================================================");
    ui.ln();

    ui.info("Waiting for authorization (press Ctrl+C to abort)...");

    const std::string token_payload = std::format(
      "client_id={}&device_code={}&grant_type=urn:ietf:params:oauth:grant-type:device_code",
      core::zc_client_id, device_code
    );

    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(interval));
      auto expected_poll = net::post(
        std::string(core::zc_token_url), token_payload,
        { { .name = "Accept", .value = "application/json" } }
      );
      if (!expected_poll)
        throw_or_terminate(expected_poll.error());

      auto poll = json::parse(expected_poll->body);

      if (poll.contains("error")) {
        std::string err = poll["error"];

        if (err == "authorization_pending")
          ui.flush();
        else if (err == "slow_down")
          interval += 5;
        else if (err == "expired_token")
          throw_or_terminate(Exception(
            core::ZCError::AuthenticationError, "The code has expired. Please run `zc login` again."
          ));
        else
          throw_or_terminate(Exception(
            OSError::NetworkError, "Authorization failed: {}", poll["error_description"].get<std::string>()
          ));
      } else if (poll.contains("access_token")) {
        const std::string access_token = poll["access_token"];
        core::gc().setToken(access_token);

        core::GithubClient gh(access_token);
        if (auto user_info = gh.get("https://api.github.com/user");
            user_info && user_info->contains("login"))
          core::gc().setUsername((*user_info)["login"]);

        ui.ln();
        ui.success("Successfully authenticated with GitHub!");
        return;
      }
    }
  }
};

} // namespace zc::app
