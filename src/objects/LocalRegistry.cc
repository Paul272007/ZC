#include <filesystem>

#include "objects/LocalRegistry.hh"
#include "objects/Registry.hh"

using namespace std;
namespace fs = std::filesystem;

LocalRegistry::LocalRegistry(const std::filesystem::path &file) : Registry(file)
{
  if (fs::exists(file_))
    load();
}
