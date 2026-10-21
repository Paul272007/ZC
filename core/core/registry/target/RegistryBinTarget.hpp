#pragma once
#include "RegistryTarget.hpp"

namespace zc::core
{

struct RegistryBinTarget final : RegistryTarget {
  /// @brief Name of the binary executable
  std::string output;

  RegistryBinTarget()                                     = default;
  RegistryBinTarget(RegistryBinTarget &&)                 = default;
  RegistryBinTarget(const RegistryBinTarget &)            = default;
  RegistryBinTarget &operator=(RegistryBinTarget &&)      = default;
  RegistryBinTarget &operator=(const RegistryBinTarget &) = default;
  ~RegistryBinTarget() override                           = default;

  [[nodiscard]] std::string_view type() const override { return "bin"; }

  [[nodiscard]] std::expected<void, Exception> uninstall() override
  {
    // TODO: implement
    return {};
  }
};

} // namespace zc::core
