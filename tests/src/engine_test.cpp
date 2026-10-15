#include "engine/engine.hpp"

#include <gtest/gtest.h>

#include "engine/Ctx.hpp"
#include "framework/fs.hpp"
#include "framework/Version.hpp"

namespace zc::tests
{

TEST(EngineTest, CtxScalarsAndFormat)
{
  engine::Ctx ctx;
  ctx.set("name", "Antigravity");
  ctx.set("count", 42);
  ctx.set("ratio", 3.14);
  ctx.set("version", Version(1, 2, 3));

  EXPECT_EQ(ctx.get("name"), "Antigravity");
  EXPECT_EQ(ctx.get("count"), "42");
  EXPECT_EQ(ctx.get("ratio"), "3.14");
  EXPECT_EQ(ctx.get("version"), "1.2.3");
  EXPECT_EQ(ctx.get("missing"), "");
}

TEST(EngineTest, CtxInitializerList)
{
  engine::Ctx ctx({ { "app", "ZC" }, { "lang", "C++26" } });
  EXPECT_EQ(ctx.get("app"), "ZC");
  EXPECT_EQ(ctx.get("lang"), "C++26");
}

TEST(EngineTest, CtxLists)
{
  engine::Ctx            ctx;
  const std::vector<int> numbers = { 10, 20, 30 };
  ctx.setList("numbers", numbers);

  const auto *list = ctx.getList("numbers");
  ASSERT_NE(list, nullptr);
  ASSERT_EQ(list->size(), 3);
  EXPECT_EQ(list->at(0), "10");
  EXPECT_EQ(list->at(1), "20");
  EXPECT_EQ(list->at(2), "30");

  EXPECT_EQ(ctx.getList("unknown"), nullptr);
}

TEST(EngineTest, CtxTruthy)
{
  engine::Ctx ctx;
  ctx.set("true_str", "true");
  ctx.set("yes_str", "yes");
  ctx.set("val_num", 100);
  ctx.set("false_str", "false");
  ctx.set("zero_num", 0);
  ctx.set("empty_str", "");

  EXPECT_TRUE(ctx.is_truthy("true_str"));
  EXPECT_TRUE(ctx.is_truthy("yes_str"));
  EXPECT_TRUE(ctx.is_truthy("val_num"));

  EXPECT_FALSE(ctx.is_truthy("false_str"));
  EXPECT_FALSE(ctx.is_truthy("zero_num"));
  EXPECT_FALSE(ctx.is_truthy("empty_str"));
  EXPECT_FALSE(ctx.is_truthy("non_existent"));

  const std::vector<std::string> empty_list;
  const std::vector<std::string> non_empty_list = { "item" };
  ctx.setList("empty_list", empty_list);
  ctx.setList("non_empty_list", non_empty_list);

  EXPECT_FALSE(ctx.is_truthy("empty_list"));
  EXPECT_TRUE(ctx.is_truthy("non_empty_list"));
}

TEST(EngineTest, RenderVariables)
{
  engine::Ctx ctx;
  ctx.set("user", "Paul");
  ctx.set("role", "Developer");

  std::string template_str = "Hello {{ user }}, role: {{ role }}! Missing: {{ unknown }}.";
  std::string rendered     = engine::render(template_str, ctx);

  EXPECT_EQ(rendered, "Hello Paul, role: Developer! Missing: .");
}

TEST(EngineTest, RenderIfElse)
{
  engine::Ctx ctx;
  ctx.set("logged_in", true);
  ctx.set("is_admin", false);

  std::string template_str =
    "{% if logged_in %}Welcome back!{% else %}Please log in.{% endif %}" " " "{% if is_admin %}Admin panel{% else %}User view{% endif %}";

  std::string rendered = engine::render(template_str, ctx);
  EXPECT_EQ(rendered, "Welcome back! User view");
}

TEST(EngineTest, RenderNestedIf)
{
  engine::Ctx ctx;
  ctx.set("outer", true);
  ctx.set("inner", false);

  std::string template_str =
    "{% if outer %}OUTER_START({% if inner %}INNER_TRUE{% else %}INNER_FALSE{% endif %})OUTER_END{% endif %}";

  std::string rendered = engine::render(template_str, ctx);
  EXPECT_EQ(rendered, "OUTER_START(INNER_FALSE)OUTER_END");
}

TEST(EngineTest, RenderForLoop)
{
  engine::Ctx                    ctx;
  const std::vector<std::string> items = { "one", "two", "three" };
  ctx.setList("items", items);

  std::string template_str = "Items:{% for x in items %} [{{ x }}]{% endfor %}";
  std::string rendered     = engine::render(template_str, ctx);

  EXPECT_EQ(rendered, "Items: [one] [two] [three]");
}

TEST(EngineTest, RenderForLoopWithFormattable)
{
  engine::Ctx            ctx;
  const std::vector<int> ports = { 80, 443, 8080 };
  ctx.setList("ports", ports);

  std::string template_str = "{% for p in ports %}{{ p }};{% endfor %}";
  std::string rendered     = engine::render(template_str, ctx);

  EXPECT_EQ(rendered, "80;443;8080;");
}

TEST(EngineTest, RenderToFile)
{
  const stdfs::path temp_dir = stdfs::temp_directory_path() / "zc_engine_test";
  stdfs::create_directories(temp_dir);

  const stdfs::path input_file  = temp_dir / "template.txt";
  const stdfs::path output_file = temp_dir / "output.txt";

  (void)fs::write(input_file, "Generated for {{ name }} v{{ version }}");

  engine::Ctx ctx;
  ctx.set("name", "ZC");
  ctx.set("version", Version(2, 0, 0));

  auto res = engine::render_to(input_file, output_file, ctx);
  ASSERT_TRUE(res.has_value());

  auto out_content = fs::read(output_file);
  ASSERT_TRUE(out_content.has_value());
  EXPECT_EQ(*out_content, "Generated for ZC v2.0.0");

  const stdfs::path missing_file = temp_dir / "does_not_exist.txt";
  auto              fail_res     = engine::render_to(missing_file, output_file, ctx);
  EXPECT_FALSE(fail_res.has_value());

  std::error_code ec;
  stdfs::remove_all(temp_dir, ec);
}

} // namespace zc::tests
