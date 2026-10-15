#include "engine/engine.hpp"

#include "engine/Ctx.hpp"
#include "framework/fs.hpp"
#include "framework/util/string.hpp"

namespace zc::engine
{

namespace
{

void render_internal(std::string_view text, const Ctx &ctx, std::string &out);

void process_variable(std::string_view text, u64 &cursor, const Ctx &ctx, std::string &out)
{
  const u64 end_pos = text.find("}}", cursor + 2);
  if (end_pos == std::string_view::npos) {
    out.append(text.substr(cursor));
    cursor = text.size();
    return;
  }
  const std::string_view raw_var  = text.substr(cursor + 2, end_pos - (cursor + 2));
  const std::string_view var_name = trim(raw_var);

  out.append(ctx.get(var_name));
  cursor = end_pos + 2;
}

bool find_block_extent(
  std::string_view text, const u64 start_cursor, const std::string_view open_prefix,
  const std::string_view close_tag, const std::string_view else_tag, u64 &body_end, u64 &else_pos,
  u64 &next_cursor
)
{
  u64 scan  = start_cursor;
  u64 depth = 1;
  else_pos  = std::string_view::npos;

  while (scan < text.size()) {
    const u64 tag_open = text.find("{%", scan);
    if (tag_open == std::string_view::npos)
      return false;

    const u64 tag_close = text.find("%}", tag_open + 2);
    if (tag_close == std::string_view::npos)
      return false;

    if (const std::string_view inner = zc::trim(text.substr(tag_open + 2, tag_close - (tag_open + 2)));
        inner.starts_with(open_prefix)) {
      depth++;
    } else if (!else_tag.empty() && inner == else_tag && depth == 1) {
      else_pos = tag_open;
    } else if (inner == close_tag) {
      depth--;
      if (depth == 0) {
        body_end    = tag_open;
        next_cursor = tag_close + 2;
        return true;
      }
    }
    scan = tag_close + 2;
  }
  return false;
}

void process_if(
  const std::string_view tag_content, const std::string_view text, u64 &cursor, const Ctx &ctx,
  std::string &out
)
{
  const std::string_view condition = trim(tag_content.substr(3));

  const u64 body_start  = cursor;
  u64       body_end    = 0;
  u64       else_pos    = std::string_view::npos;
  u64       next_cursor = 0;

  if (!find_block_extent(text, body_start, "if ", "endif", "else", body_end, else_pos, next_cursor)) {
    out.append(text.substr(body_start));
    cursor = text.size();
    return;
  }

  cursor = next_cursor;

  std::string_view then_body;
  std::string_view else_body;

  if (else_pos != std::string_view::npos) {
    then_body              = text.substr(body_start, else_pos - body_start);
    const u64 else_tag_end = text.find("%}", else_pos) + 2;
    else_body              = text.substr(else_tag_end, body_end - else_tag_end);
  } else {
    then_body = text.substr(body_start, body_end - body_start);
  }

  if (ctx.is_truthy(condition))
    render_internal(then_body, ctx, out);
  else if (!else_body.empty())
    render_internal(else_body, ctx, out);
}

void process_for(
  std::string_view tag_content, const std::string_view text, u64 &cursor, const Ctx &ctx, std::string &out
)
{
  const u64 in_pos = tag_content.find(" in ");
  if (in_pos == std::string_view::npos || in_pos <= 4)
    return;

  const std::string_view item_name = zc::trim(tag_content.substr(4, in_pos - 4));
  const std::string_view list_name = zc::trim(tag_content.substr(in_pos + 4));

  const u64 body_start = cursor;

  u64 body_end    = 0;
  u64 unused_else = std::string_view::npos;
  u64 next_cursor = 0;

  if (!find_block_extent(text, body_start, "for ", "endfor", "", body_end, unused_else, next_cursor)) {
    out.append(text.substr(body_start));
    cursor = text.size();
    return;
  }

  cursor                          = next_cursor;
  const std::string_view for_body = text.substr(body_start, body_end - body_start);

  if (const auto *list_ptr = ctx.getList(list_name)) {
    for (const auto &elem : *list_ptr) {
      Ctx sub_ctx = ctx;
      sub_ctx.set(std::string(item_name), elem);
      render_internal(for_body, sub_ctx, out);
    }
  }
}

void render_internal(std::string_view text, const Ctx &ctx, std::string &out)
{
  u64 cursor = 0;

  while (cursor < text.size()) {
    const u64 open_pos = text.find('{', cursor);

    if (open_pos == std::string_view::npos || open_pos + 1 >= text.size()) {
      out.append(text.substr(cursor));
      break;
    }

    out.append(text.substr(cursor, open_pos - cursor));
    cursor = open_pos;

    if (const char tag_type = text.at(cursor + 1); tag_type == '{') {
      process_variable(text, cursor, ctx, out);
    } else if (tag_type == '%') {
      const u64 tag_end = text.find("%}", cursor + 2);
      if (tag_end == std::string_view::npos) {
        out.append(text.substr(cursor));
        break;
      }

      const std::string_view tag_content = trim(text.substr(cursor + 2, tag_end - (cursor + 2)));

      cursor = tag_end + 2;

      if (tag_content.starts_with("if "))
        process_if(tag_content, text, cursor, ctx, out);
      else if (tag_content.starts_with("for "))
        process_for(tag_content, text, cursor, ctx, out);
    } else {
      out.push_back('{');
      cursor++;
    }
  }
}

} // namespace

[[nodiscard]] std::string render(const std::string &text, const Ctx &ctx)
{
  std::string result;
  result.reserve(text.size());
  render_internal(text, ctx, result);
  return result;
}

std::expected<void, Exception>
render_to(const std::filesystem::path &i_file, const std::filesystem::path &o_file, const Ctx &ctx)
{
  const auto content = fs::read(i_file);
  if (!content)
    return std::unexpected(content.error());
  const auto result = render(*content, ctx);
  return fs::write(o_file, result);
}

} // namespace zc::engine
