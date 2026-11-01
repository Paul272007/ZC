#pragma once

#include <cstdio>
#include <string_view>

#include "framework/signals.hpp"
#include "framework/ui/DefaultUIStrategy.hpp"
#include "framework/ui/Style.hpp"
#include "framework/ui/UIStrategy.hpp"
#include "framework/util/raw_mode.hpp"
#include "framework/util/terminal.hpp"
#include "framework/util/types.hpp"

namespace zc
{

enum class UIMode : u8 {
  Quiet,   // No output at all
  Normal,  // Normal output
  Verbose, // Verbose output
};

// class RawModeGuard;

class UI
{
  class RawModeGuard
  {
  public:
    RawModeGuard(const RawModeGuard &)            = delete;
    RawModeGuard(RawModeGuard &&)                 = delete;
    RawModeGuard &operator=(const RawModeGuard &) = delete;
    RawModeGuard &operator=(RawModeGuard &&)      = delete;

    explicit RawModeGuard(const UI &guard_ui) : ui_(guard_ui) { ui_.enableRaw(); }

    ~RawModeGuard() { ui_.disableRaw(); }

  private:
    const UI &ui_;
  };

  static UI instance_;

  std::unique_ptr<UIStrategy> strategy_;

  UIMode     mode_ = UIMode::Normal;
  std::FILE *in_   = stdin;
  std::FILE *out_  = stdout;
  std::FILE *err_  = stderr;

  UI() : strategy_(std::make_unique<DefaultUIStrategy>(*this)) {}

  [[nodiscard]] bool quiet() const noexcept { return mode_ == UIMode::Quiet; }

  bool readLine(std::string &line) const
  {
    line.clear();
    if (!in_)
      return false;

    setStyle(Style::Bold | Style::Blue);
    flush();
    int c = std::fgetc(in_);
    if (c == EOF) {
      resetStyle();
      return false;
    }

    while (c != EOF && c != '\n') {
      if (c != '\r')
        line.push_back(static_cast<char>(c));
      c = std::fgetc(in_);
    }
    resetStyle();
    return true;
  }

  void closeStreams() const noexcept
  {
    if (in_ && in_ != stdin)
      (void)std::fclose(in_);
    if (out_ && out_ != stdout)
      (void)std::fclose(out_);
    if (err_ && err_ != stderr)
      (void)std::fclose(err_);
  }

  template<Formattable T>
  void displayCheckboxes(
    const std::vector<T> &options, const std::vector<bool> &selected, const u64 cursor
  ) const
  {
    for (u64 i = 0; i < options.size(); i++) {
      clearLine();
      println("{}", strategy_->checkbox(std::format("{}", options.at(i)), selected.at(i), i == cursor));
    }
  }

  template<Formattable T>
  void displayRadios(const std::vector<T> &options, const u64 cursor) const
  {
    for (u64 i = 0; i < options.size(); i++) {
      clearLine();
      println("{}", strategy_->radio(std::format("{}", options.at(i)), i == cursor));
    }
  }

public:
  UI(UI &&)                 = delete;
  UI &operator=(UI &&)      = delete;
  UI(const UI &)            = delete;
  UI &operator=(const UI &) = delete;

  ~UI() { closeStreams(); }

  static void init(
    FILE *in = stdin, FILE *out = stdout, FILE *err = stderr, const UIMode mode = UIMode::Normal,
    std::unique_ptr<UIStrategy> strategy = std::make_unique<DefaultUIStrategy>(instance_)
  )
  {
    instance_.in_       = in;
    instance_.out_      = out;
    instance_.err_      = err;
    instance_.mode_     = mode;
    instance_.strategy_ = std::move(strategy);

    signals::init();
    static bool registered_atexit = false;
    if (!registered_atexit) {
      registered_atexit = true;
      (void)std::atexit([] {
        set_raw_mode(false);
        write(STDOUT_FILENO, show_cursor.data(), show_cursor.size());
      });
    }
  }

  [[nodiscard]] static UI &get() noexcept { return instance_; }

  // ——— Utilities

  void clear() const
  {
    if (!quiet()) {
      print("{}{}", clear_screen, cursor_top_left);
      flush();
    }
  }

  void flush() const
  {
    if (!quiet())
      (void)std::fflush(out_); // Or log error ?
  }

  void ln() const
  {
    if (!quiet())
      std::println(out_);
  }

  void disableRaw() const
  {
    if (!quiet()) {
      print(show_cursor);
      flush();
      set_raw_mode(false);
    }
  }

  void enableRaw() const
  {
    if (!quiet()) {
      print(hide_cursor);
      flush();
      set_raw_mode(true);
    }
  }

  void cursorUp(const u64 value) const { print("\r{}", cursor_up(value)); }

  void cursorDown(const u64 value) const { print("\r{}", cursor_down(value)); }

  void clearLine() const { print("\r{}", clear_line); }

  void clearUnderCursor() const { print("\r{}", clear_under_cursor); }

  // ——— Output style

  void setStyle(const std::string_view style) const { print("{}", style); }

  void setStyle(const Style s) const { print("{}", detail::ansi_seq(s)); }

  void resetStyle() const { print("{}", reset); }

  // ——— User interaction

  template<typename... Args>
  [[nodiscard]] bool ask(std::format_string<Args...> fmt, Args &&...args) const
  {
    return ask(false, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]] bool ask(const bool default_ans, std::format_string<Args...> fmt, Args &&...args) const
  {
    std::vector<std::string> options{ "Yes", "No" };
    return radios(options, default_ans ? 0 : 1, fmt, std::forward<Args>(args)...) == 0;
  }

  template<typename... Args>
  [[nodiscard]] std::string
  input(const std::string &default_ans, std::format_string<Args...> fmt, Args &&...args) const
  {
    const std::string o_default = default_ans.empty() ? "none" : default_ans;
    promptWithDefault(o_default, fmt, std::forward<Args>(args)...);
    std::string line;

    if (!readLine(line)) {
      if (std::feof(in_))
        return default_ans;
      std::clearerr(in_);
      return default_ans;
    }
    cursorUp(1);
    clearLine();
    resolvedPrompt(line.empty() ? o_default : line, fmt, std::forward<Args>(args)...);
    return line.empty() ? default_ans : line;
  }

  template<typename... Args>
  [[nodiscard]] std::vector<std::string> inputList(
    const std::vector<std::string> &default_ans, std::format_string<Args...> fmt, Args &&...args
  ) const
  {
    std::vector<std::string> answers;
    prompt(fmt, std::forward<Args>(args)...);
    info("Hit {}Enter{} to accept the default options:", blue, reset);
    bulletList(default_ans, ULStyle::Dot);
    info("Type a value and hit {}Enter{} to add this value.", blue, reset);
    info(
      "Type {}exit{} or hit {}Ctrl + D{} to confirm the given values and exit.", blue, reset, blue, reset
    );

    while (true) {
      std::string answer;
      print("❯ ");
      flush();

      if (!readLine(answer)) {
        if (!std::feof(in_))
          std::clearerr(in_);
        return answers; // Ctrl + D -> exit
      }

      // Empty answer but enter was typed
      if (answer.empty()) {
        if (answers.empty()) // if no values given return default
          return default_ans;
        return answers;      // if already some values return them
      }
      if (answer == "exit")
        return answers;
      answers.push_back(answer);
    }
  }

  template<Formattable T, typename... Args>
  void checkboxes(
    const std::vector<T> &options, std::vector<bool> &selected, std::format_string<Args...> fmt,
    Args &&...args
  ) const
  {
    if (quiet() || options.empty())
      return;

    if (selected.size() < options.size())
      selected.resize(options.size(), false);

    u64 cursor = 0;
    prompt(fmt, std::forward<Args>(args)...);
    ln();
    RawModeGuard guard(*this);

    while (true) {
      displayCheckboxes(options, selected, cursor);
      flush();

      switch (read_key()) {
      case Key::Enter:
        goto loop_end;
      case Key::Space:
        selected.at(cursor) = !selected.at(cursor);
        break;
      case Key::Up:
        cursor = (cursor == 0) ? options.size() - 1 : cursor - 1;
        break;
      case Key::Down:
        cursor = (cursor == options.size() - 1) ? 0 : cursor + 1;
        break;
      default:
        break;
      }
      cursorUp(options.size());
    }

  loop_end:
    std::string result;
    bool        first = true;
    for (u32 i = 0; i < options.size(); i++) {
      if (selected.at(i)) {
        if (!first)
          result += ", ";
        std::format_to(std::back_inserter(result), "{}", options[i]);
        first = false;
      }
    }
    cursorUp(options.size() + 1); // Also erase the question
    clearUnderCursor();
    resolvedPrompt(result, fmt, std::forward<Args>(args)...);
  }

  template<Formattable T, typename... Args>
  [[nodiscard]] u64 radios(
    const std::vector<T> &options, u32 default_ans, std::format_string<Args...> fmt, Args &&...args
  ) const
  {
    if (quiet() || options.empty())
      return 0;

    u64 cursor = (default_ans < options.size()) ? default_ans : 0;
    prompt(fmt, std::forward<Args>(args)...);
    ln();
    RawModeGuard guard(*this);

    while (true) {
      displayRadios(options, cursor);
      flush();

      switch (read_key()) {
      case Key::Enter:
        goto loop_end;
      case Key::Up:
        cursor = (cursor == 0) ? options.size() - 1 : cursor - 1;
        break;
      case Key::Down:
        cursor = (cursor == options.size() - 1) ? 0 : cursor + 1;
        break;
      default:
        break;
      }
      cursorUp(options.size());
    }

  loop_end:
    cursorUp(options.size() + 1);
    clearUnderCursor();
    resolvedPrompt(options[cursor], fmt, std::forward<Args>(args)...);
    return cursor;
  }

  // ——— Text display

  template<typename... Args>
  void print(std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet())
      std::print(out_, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void print(Style s, std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      setStyle(s);
      std::print(out_, fmt, std::forward<Args>(args)...);
      resetStyle();
    }
  }

  template<typename... Args>
  void println(std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet())
      std::println(out_, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void println(Style s, std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      setStyle(s);
      std::println(out_, fmt, std::forward<Args>(args)...);
      resetStyle();
    }
  }

  template<typename... Args>
  void println_err(std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet())
      std::println(err_, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void info(std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      const auto message = std::format(fmt, std::forward<Args>(args)...);
      println("{}", strategy_->info(message));
    }
  }

  template<typename... Args>
  void success(std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      const auto message = std::format(fmt, std::forward<Args>(args)...);
      println("{}", strategy_->success(message));
    }
  }

  template<typename... Args>
  void warning(std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      const auto message = std::format(fmt, std::forward<Args>(args)...);
      println("{}", strategy_->warning(message));
    }
  }

  template<typename... Args>
  void debug(std::format_string<Args...> fmt, Args &&...args) const
  {
#ifdef ZC_DEBUG
    if (!quiet()) {
      const auto message = std::format(fmt, std::forward<Args>(args)...);
      println("{}", strategy_->debug(message));
    }
#endif
  }

  template<typename... Args>
  void error(std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      const auto message = std::format(fmt, std::forward<Args>(args)...);
      println_err("{}", strategy_->error(message));
    }
  }

  void printException(const Exception &e) const
  {
    if (!quiet())
      println_err("{}", strategy_->exception(e));
  }

  template<typename... Args>
  void prompt(std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      const auto message = std::format(fmt, std::forward<Args>(args)...);
      print("{}", strategy_->prompt(message));
      flush();
    }
  }

  template<typename... Args>
  void
  promptWithDefault(std::string_view default_ans, std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      const auto message = std::format(fmt, std::forward<Args>(args)...);
      print("{}", strategy_->prompt(message, default_ans));
      flush();
    }
  }

  template<typename... Args>
  void resolvedPrompt(std::string_view ans, std::format_string<Args...> fmt, Args &&...args) const
  {
    if (!quiet()) {
      const auto message = std::format(fmt, std::forward<Args>(args)...);
      println("{}", strategy_->resolved_prompt(message, ans));
    }
  }

  template<Formattable T>
  void bulletList(const std::vector<T> &list, const ULStyle style) const
  {
    if (!quiet()) {
      for (const auto &el : list) {
        const std::string formatted_el = std::format("{}", el);
        println("{}", strategy_->bullet_list_el(formatted_el, style));
      }
    }
  }

  template<Formattable T>
  void orderedList(const std::vector<T> &list, const OLStyle style) const
  {
    if (!quiet()) {
      for (u32 i = 0; i < list.size(); i++) {
        const std::string formatted_el = std::format("{}", list[i]);
        println("{}", strategy_->ordered_list_el(formatted_el, i, style));
      }
    }
  }
};

inline UI UI::instance_;

inline const UI &ui{ UI::get() };

} // namespace zc
