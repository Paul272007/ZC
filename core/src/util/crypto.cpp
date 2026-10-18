#include "core/util/crypto.hpp"

#include <fstream>
#include <openssl/evp.h>
#include <sstream>

#include "framework/Exception.hpp"
#include "framework/fs.hpp"

namespace zc::core::crypto
{

std::expected<std::string, Exception> sha256(const stdfs::path &path)
{
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return std::unexpected(Exception(FSError::ReadingError, "file {} could not be opened", path));

  EVP_MD_CTX   *md_ctx   = EVP_MD_CTX_new();
  const EVP_MD *md       = EVP_sha256();
  unsigned int  hash_len = 0;

  EVP_DigestInit_ex(md_ctx, md, nullptr);

  std::array<char, 4096>                     buffer{};
  std::array<unsigned char, EVP_MAX_MD_SIZE> hash{};

  while (file.read(buffer.data(), sizeof(buffer)))
    EVP_DigestUpdate(md_ctx, buffer.data(), static_cast<size_t>(file.gcount()));
  EVP_DigestUpdate(md_ctx, buffer.data(), static_cast<size_t>(file.gcount()));

  EVP_DigestFinal_ex(md_ctx, hash.data(), &hash_len);
  EVP_MD_CTX_free(md_ctx);

  std::stringstream ss;
  for (unsigned int i = 0; i < hash_len; i++)
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash.at(i));
  return ss.str();
}

} // namespace zc::core::crypto
