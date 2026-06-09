#include <filesystem>

#define ZC_DEV_CONFIG                                                                                        \
  using namespace std;                                                                                       \
  namespace fs = std::filesystem;

const std::filesystem::path get_project_root(const std::filesystem::path &base);
const std::filesystem::path get_zc_root();
