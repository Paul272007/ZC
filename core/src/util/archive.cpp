#include "core/util/archive.hpp"

#include <archive.h>
#include <archive_entry.h>

#include "core/util/ZCError.hpp"
#include "framework/fs.hpp"

namespace zc::core::archive
{

std::expected<void, Exception> extract(const stdfs::path &archive, const stdfs::path &dest)
{
  archive_entry *entry = nullptr;
  constexpr int  flags =
    ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

  struct archive *a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_filter_all(a);
  struct archive *ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

  if (archive_read_open_filename(a, archive.c_str(), 10240))
    return std::unexpected(Exception(FSError::ReadingError, "cannot open archive: {}", archive));

  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    const stdfs::path full_path = dest / archive_entry_pathname(entry);
    archive_entry_set_pathname(entry, full_path.c_str());

    if (archive_read_extract(a, entry, flags) != ARCHIVE_OK)
      return std::unexpected(Exception(ZCError::ArchiveError, "cannot extract {}", archive));
  }

  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);
  return {};
}

} // namespace zc::core::archive
