add_library(zc-core STATIC
  core/global-config/GlobalConfig.hpp

  core/project/target/ProjectBinTarget.hpp
  core/project/target/ProjectCompiledTarget.hpp
  core/project/target/ProjectHeaderTarget.hpp
  core/project/target/ProjectLibTarget.hpp
  core/project/target/ProjectTarget.hpp
  core/project/target/ProjectTargetDependency.hpp
  core/project/target/ProjectTargetResolvedFlags.hpp

  core/project/BuildMode.hpp
  core/project/Language.hpp
  core/project/LanguageConf.hpp
  core/project/Link.hpp
  core/project/Project.hpp

  core/registry/target/RegistryBinTarget.hpp
  core/registry/target/RegistryHeaderTarget.hpp
  core/registry/target/RegistryLibTarget.hpp
  core/registry/target/RegistryTarget.hpp
  core/registry/target/RegistryTargetDependency.hpp
  core/registry/target/RegistryTargetType.hpp

  core/registry/package_index.hpp
  core/registry/RegistryPkg.hpp
  core/registry/RegistryProject.hpp
  core/registry/Registry.hpp
  core/registry/RemotePkg.hpp

  core/util/archive.hpp
  core/util/crypto.hpp
  core/util/directories.hpp
  core/util/GithubClient.hpp
  core/util/helpers.hpp
  core/util/json_helpers.hpp
  core/util/platform.hpp
  core/util/ZCError.hpp

  core/Conf.hpp

  src/global-config/GlobalConfig.cpp

  src/project/target/ProjectBinTarget.cpp
  src/project/target/ProjectCompiledTarget.cpp
  src/project/target/ProjectHeaderTarget.cpp
  src/project/target/ProjectLibTarget.cpp
  src/project/target/ProjectTarget.cpp

  src/project/Project.cpp

  src/registry/target/RegistryTarget.cpp

  src/registry/package_index.cpp
  src/registry/Registry.cpp

  src/util/archive.cpp
  src/util/crypto.cpp
  src/util/directories.cpp
  src/util/helpers.cpp
  src/util/json_helpers.cpp
)
target_include_directories(zc-core PUBLIC .)
target_link_libraries(zc-core PUBLIC
  zc-engine
  zc-framework
  nlohmann_json::nlohmann_json
  CURL::libcurl
  LibArchive::LibArchive
  OpenSSL::Crypto
)
