// Copyright (C) 2019-2021 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include "../src/Config.hpp"
#include "../src/Fd.hpp"
#include "../src/Util.hpp"
#include "../src/fmtmacros.hpp"
#include "TestUtil.hpp"

#include <core/exceptions.hpp>
#include <core/wincompat.hpp>

#include "third_party/doctest.h"
#include "third_party/nonstd/optional.hpp"

#include <fcntl.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <algorithm>

using doctest::Approx;
using nonstd::nullopt;
using TestUtil::TestContext;

TEST_SUITE_BEGIN("Util");

TEST_CASE("Util::base_name")
{
  CHECK(Util::base_name("") == "");
  CHECK(Util::base_name(".") == ".");
  CHECK(Util::base_name("foo") == "foo");
  CHECK(Util::base_name("/") == "");
  CHECK(Util::base_name("/foo") == "foo");
  CHECK(Util::base_name("/foo/bar/f.txt") == "f.txt");
}

TEST_CASE("Util::big_endian_to_int")
{
  uint8_t bytes[8] = {0x70, 0x9e, 0x9a, 0xbc, 0xd6, 0x54, 0x4b, 0xca};

  uint8_t uint8;
  Util::big_endian_to_int(bytes, uint8);
  CHECK(uint8 == 0x70);

  int8_t int8;
  Util::big_endian_to_int(bytes, int8);
  CHECK(int8 == 0x70);

  uint16_t uint16;
  Util::big_endian_to_int(bytes, uint16);
  CHECK(uint16 == 0x709e);

  int16_t int16;
  Util::big_endian_to_int(bytes, int16);
  CHECK(int16 == 0x709e);

  uint32_t uint32;
  Util::big_endian_to_int(bytes, uint32);
  CHECK(uint32 == 0x709e9abc);

  int32_t int32;
  Util::big_endian_to_int(bytes, int32);
  CHECK(int32 == 0x709e9abc);

  uint64_t uint64;
  Util::big_endian_to_int(bytes, uint64);
  CHECK(uint64 == 0x709e9abcd6544bca);

  int64_t int64;
  Util::big_endian_to_int(bytes, int64);
  CHECK(int64 == 0x709e9abcd6544bca);
}

TEST_CASE("Util::change_extension")
{
  CHECK(Util::change_extension("", "") == "");
  CHECK(Util::change_extension("x", "") == "x");
  CHECK(Util::change_extension("", "x") == "x");
  CHECK(Util::change_extension("", ".") == ".");
  CHECK(Util::change_extension(".", "") == "");
  CHECK(Util::change_extension("...", "x") == "..x");
  CHECK(Util::change_extension("abc", "def") == "abcdef");
  CHECK(Util::change_extension("dot.", ".dot") == "dot.dot");
  CHECK(Util::change_extension("foo.ext", "e2") == "fooe2");
  CHECK(Util::change_extension("bar.txt", ".o") == "bar.o");
  CHECK(Util::change_extension("foo.bar.txt", ".o") == "foo.bar.o");
}

TEST_CASE("Util::clamp")
{
  CHECK(Util::clamp(0, 1, 2) == 1);
  CHECK(Util::clamp(1, 1, 2) == 1);
  CHECK(Util::clamp(2, 1, 2) == 2);
  CHECK(Util::clamp(3, 1, 2) == 2);

  CHECK(Util::clamp(7.0, 7.7, 8.8) == Approx(7.7));
  CHECK(Util::clamp(8.0, 7.7, 8.8) == Approx(8.0));
  CHECK(Util::clamp(9.0, 7.7, 8.8) == Approx(8.8));
}

TEST_CASE("Util::common_dir_prefix_length")
{
  CHECK(Util::common_dir_prefix_length("", "") == 0);
  CHECK(Util::common_dir_prefix_length("/", "") == 0);
  CHECK(Util::common_dir_prefix_length("", "/") == 0);
  CHECK(Util::common_dir_prefix_length("/", "/") == 0);
  CHECK(Util::common_dir_prefix_length("/", "/b") == 0);
  CHECK(Util::common_dir_prefix_length("/a", "/") == 0);
  CHECK(Util::common_dir_prefix_length("/a", "/b") == 0);
  CHECK(Util::common_dir_prefix_length("/a", "/a") == 2);
  CHECK(Util::common_dir_prefix_length("/a", "/a/b") == 2);
  CHECK(Util::common_dir_prefix_length("/a/b", "/a") == 2);
  CHECK(Util::common_dir_prefix_length("/a/b", "/a/c") == 2);
  CHECK(Util::common_dir_prefix_length("/a/b", "/a/b") == 4);
  CHECK(Util::common_dir_prefix_length("/a/bc", "/a/b") == 2);
  CHECK(Util::common_dir_prefix_length("/a/b", "/a/bc") == 2);
}

TEST_CASE("Util::create_dir")
{
  TestContext test_context;

  CHECK(Util::create_dir("/"));

  CHECK(Util::create_dir("create/dir"));
  CHECK(Stat::stat("create/dir").is_directory());

  Util::write_file("create/dir/file", "");
  CHECK(!Util::create_dir("create/dir/file"));
}

TEST_CASE("Util::dir_name")
{
  CHECK(Util::dir_name("") == ".");
  CHECK(Util::dir_name(".") == ".");
  CHECK(Util::dir_name("foo") == ".");
  CHECK(Util::dir_name("/") == "/");
  CHECK(Util::dir_name("/foo") == "/");
  CHECK(Util::dir_name("/foo/bar/f.txt") == "/foo/bar");

#ifdef _WIN32
  CHECK(Util::dir_name("C:/x/y") == "C:/x");
  CHECK(Util::dir_name("X:/x/y") == "X:/x");
  CHECK(Util::dir_name("C:\\x\\y") == "C:\\x");
  CHECK(Util::dir_name("C:/x") == "C:/");
  CHECK(Util::dir_name("C:\\x") == "C:\\");
  CHECK(Util::dir_name("C:/") == "C:/");
  CHECK(Util::dir_name("C:\\") == "C:\\");
#endif
}

TEST_CASE("Util::strip_ansi_csi_seqs")
{
  const char input[] =
    "Normal,"
    " \x1B[K\x1B[1mbold\x1B[m,"
    " \x1B[31mred\x1B[m,"
    " \x1B[1;32mbold green\x1B[m.\n";

  CHECK(Util::strip_ansi_csi_seqs(input) == "Normal, bold, red, bold green.\n");
}

TEST_CASE("Util::ensure_dir_exists")
{
  TestContext test_context;

  CHECK_NOTHROW(Util::ensure_dir_exists("/"));

  CHECK_NOTHROW(Util::ensure_dir_exists("create/dir"));
  CHECK(Stat::stat("create/dir").is_directory());

  Util::write_file("create/dir/file", "");
  CHECK_THROWS_WITH(
    Util::ensure_dir_exists("create/dir/file"),
    "Failed to create directory create/dir/file: Not a directory");
}

TEST_CASE("Util::expand_environment_variables")
{
  Util::setenv("FOO", "bar");

  CHECK(Util::expand_environment_variables("") == "");
  CHECK(Util::expand_environment_variables("$FOO") == "bar");
  CHECK(Util::expand_environment_variables("$") == "$");
  CHECK(Util::expand_environment_variables("$FOO $FOO:$FOO") == "bar bar:bar");
  CHECK(Util::expand_environment_variables("x$FOO") == "xbar");
  CHECK(Util::expand_environment_variables("${FOO}x") == "barx");

  DOCTEST_GCC_SUPPRESS_WARNING_PUSH
  DOCTEST_GCC_SUPPRESS_WARNING("-Wunused-result")
  CHECK_THROWS_WITH(
    (void)Util::expand_environment_variables("$surelydoesntexist"),
    "environment variable \"surelydoesntexist\" not set");
  CHECK_THROWS_WITH((void)Util::expand_environment_variables("${FOO"),
                    "syntax error: missing '}' after \"FOO\"");
  DOCTEST_GCC_SUPPRESS_WARNING_POP
}

TEST_CASE("Util::fallocate")
{
  TestContext test_context;

  const char filename[] = "test-file";

  CHECK(Util::fallocate(Fd(creat(filename, S_IRUSR | S_IWUSR)).get(), 10000)
        == 0);
  CHECK(Stat::stat(filename).size() == 10000);

  CHECK(
    Util::fallocate(Fd(open(filename, O_RDWR, S_IRUSR | S_IWUSR)).get(), 5000)
    == 0);
  CHECK(Stat::stat(filename).size() == 10000);

  CHECK(
    Util::fallocate(Fd(open(filename, O_RDWR, S_IRUSR | S_IWUSR)).get(), 20000)
    == 0);
  CHECK(Stat::stat(filename).size() == 20000);
}

TEST_CASE("Util::format_argv_for_logging")
{
  const char* argv_0[] = {nullptr};
  CHECK(Util::format_argv_for_logging(argv_0) == "");

  const char* argv_2[] = {"foo", "bar", nullptr};
  CHECK(Util::format_argv_for_logging(argv_2) == "foo bar");
}

TEST_CASE("Util::format_base16")
{
  uint8_t none[] = "";
  uint8_t text[4] = "foo"; // incl. NUL
  uint8_t data[4] = {0, 1, 2, 3};

  CHECK(Util::format_base16(none, 0) == "");
  CHECK(Util::format_base16(text, sizeof(text)) == "666f6f00");
  CHECK(Util::format_base16(data, sizeof(data)) == "00010203");
}

TEST_CASE("Util::format_base32hex")
{
  // Test vectors (without padding) from RFC 4648.
  const uint8_t input[] = {'f', 'o', 'o', 'b', 'a', 'r'};
  CHECK(Util::format_base32hex(input, 0) == "");
  CHECK(Util::format_base32hex(input, 1) == "co");
  CHECK(Util::format_base32hex(input, 2) == "cpng");
  CHECK(Util::format_base32hex(input, 3) == "cpnmu");
  CHECK(Util::format_base32hex(input, 4) == "cpnmuog");
  CHECK(Util::format_base32hex(input, 5) == "cpnmuoj1");
  CHECK(Util::format_base32hex(input, 6) == "cpnmuoj1e8");
}

TEST_CASE("Util::format_human_readable_size")
{
  CHECK(Util::format_human_readable_size(0) == "0.0 kB");
  CHECK(Util::format_human_readable_size(1) == "0.0 kB");
  CHECK(Util::format_human_readable_size(49) == "0.0 kB");
  CHECK(Util::format_human_readable_size(51) == "0.1 kB");
  CHECK(Util::format_human_readable_size(949) == "0.9 kB");
  CHECK(Util::format_human_readable_size(951) == "1.0 kB");
  CHECK(Util::format_human_readable_size(499.7 * 1000) == "499.7 kB");
  CHECK(Util::format_human_readable_size(1000 * 1000) == "1.0 MB");
  CHECK(Util::format_human_readable_size(1234 * 1000) == "1.2 MB");
  CHECK(Util::format_human_readable_size(438.5 * 1000 * 1000) == "438.5 MB");
  CHECK(Util::format_human_readable_size(1000 * 1000 * 1000) == "1.0 GB");
  CHECK(Util::format_human_readable_size(17.11 * 1000 * 1000 * 1000)
        == "17.1 GB");
}

TEST_CASE("Util::format_parsable_size_with_suffix")
{
  CHECK(Util::format_parsable_size_with_suffix(0) == "0");
  CHECK(Util::format_parsable_size_with_suffix(42 * 1000) == "42000");
  CHECK(Util::format_parsable_size_with_suffix(1000 * 1000) == "1.0M");
  CHECK(Util::format_parsable_size_with_suffix(1234 * 1000) == "1.2M");
  CHECK(Util::format_parsable_size_with_suffix(438.5 * 1000 * 1000)
        == "438.5M");
  CHECK(Util::format_parsable_size_with_suffix(1000 * 1000 * 1000) == "1.0G");
  CHECK(Util::format_parsable_size_with_suffix(17.11 * 1000 * 1000 * 1000)
        == "17.1G");
}

TEST_CASE("Util::get_extension")
{
  CHECK(Util::get_extension("") == "");
  CHECK(Util::get_extension(".") == ".");
  CHECK(Util::get_extension("...") == ".");
  CHECK(Util::get_extension("foo") == "");
  CHECK(Util::get_extension("/") == "");
  CHECK(Util::get_extension("/foo") == "");
  CHECK(Util::get_extension("/foo/bar/f") == "");
  CHECK(Util::get_extension("f.txt") == ".txt");
  CHECK(Util::get_extension("f.abc.txt") == ".txt");
  CHECK(Util::get_extension("/foo/bar/f.txt") == ".txt");
  CHECK(Util::get_extension("/foo/bar/f.abc.txt") == ".txt");
}

static inline std::string
os_path(std::string path)
{
#if defined(_WIN32) && !defined(HAVE_DIRENT_H)
  std::replace(path.begin(), path.end(), '/', '\\');
#endif

  return path;
}

TEST_CASE("Util::get_relative_path")
{
#ifdef _WIN32
  CHECK(Util::get_relative_path("C:/a", "C:/a") == ".");
  CHECK(Util::get_relative_path("C:/a", "Z:/a") == "Z:/a");
  CHECK(Util::get_relative_path("C:/a/b", "C:/a") == "..");
  CHECK(Util::get_relative_path("C:/a", "C:/a/b") == "b");
  CHECK(Util::get_relative_path("C:/a", "C:/a/b/c") == "b/c");
  CHECK(Util::get_relative_path("C:/a/b", "C:/a/c") == "../c");
  CHECK(Util::get_relative_path("C:/a/b", "C:/a/c/d") == "../c/d");
  CHECK(Util::get_relative_path("C:/a/b/c", "C:/a/c/d") == "../../c/d");
  CHECK(Util::get_relative_path("C:/a/b", "C:/") == "../..");
  CHECK(Util::get_relative_path("C:/a/b", "C:/c") == "../../c");
  CHECK(Util::get_relative_path("C:/", "C:/a/b") == "a/b");
  CHECK(Util::get_relative_path("C:/a", "D:/a/b") == "D:/a/b");
#else
  CHECK(Util::get_relative_path("/a", "/a") == ".");
  CHECK(Util::get_relative_path("/a/b", "/a") == "..");
  CHECK(Util::get_relative_path("/a", "/a/b") == "b");
  CHECK(Util::get_relative_path("/a", "/a/b/c") == "b/c");
  CHECK(Util::get_relative_path("/a/b", "/a/c") == "../c");
  CHECK(Util::get_relative_path("/a/b", "/a/c/d") == "../c/d");
  CHECK(Util::get_relative_path("/a/b/c", "/a/c/d") == "../../c/d");
  CHECK(Util::get_relative_path("/a/b", "/") == "../..");
  CHECK(Util::get_relative_path("/a/b", "/c") == "../../c");
  CHECK(Util::get_relative_path("/", "/a/b") == "a/b");
#endif
}

TEST_CASE("Util::hard_link")
{
  TestContext test_context;

  SUBCASE("Link file to nonexistent destination")
  {
    Util::write_file("old", "content");
    CHECK_NOTHROW(Util::hard_link("old", "new"));
    CHECK(Util::read_file("new") == "content");
  }

  SUBCASE("Link file to existing destination")
  {
    Util::write_file("old", "content");
    Util::write_file("new", "other content");
    CHECK_NOTHROW(Util::hard_link("old", "new"));
    CHECK(Util::read_file("new") == "content");
  }

  SUBCASE("Link nonexistent file")
  {
    CHECK_THROWS_AS(Util::hard_link("old", "new"), core::Error);
  }
}

TEST_CASE("Util::int_to_big_endian")
{
  uint8_t bytes[8];

  uint8_t uint8 = 0x70;
  Util::int_to_big_endian(uint8, bytes);
  CHECK(bytes[0] == 0x70);

  int8_t int8 = 0x70;
  Util::int_to_big_endian(int8, bytes);
  CHECK(bytes[0] == 0x70);

  uint16_t uint16 = 0x709e;
  Util::int_to_big_endian(uint16, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);

  int16_t int16 = 0x709e;
  Util::int_to_big_endian(int16, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);

  uint32_t uint32 = 0x709e9abc;
  Util::int_to_big_endian(uint32, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);
  CHECK(bytes[2] == 0x9a);
  CHECK(bytes[3] == 0xbc);

  int32_t int32 = 0x709e9abc;
  Util::int_to_big_endian(int32, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);
  CHECK(bytes[2] == 0x9a);
  CHECK(bytes[3] == 0xbc);

  uint64_t uint64 = 0x709e9abcd6544bca;
  Util::int_to_big_endian(uint64, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);
  CHECK(bytes[2] == 0x9a);
  CHECK(bytes[3] == 0xbc);
  CHECK(bytes[4] == 0xd6);
  CHECK(bytes[5] == 0x54);
  CHECK(bytes[6] == 0x4b);
  CHECK(bytes[7] == 0xca);

  int64_t int64 = 0x709e9abcd6544bca;
  Util::int_to_big_endian(int64, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);
  CHECK(bytes[2] == 0x9a);
  CHECK(bytes[3] == 0xbc);
  CHECK(bytes[4] == 0xd6);
  CHECK(bytes[5] == 0x54);
  CHECK(bytes[6] == 0x4b);
  CHECK(bytes[7] == 0xca);
}

TEST_CASE("Util::is_dir_separator")
{
  CHECK(!Util::is_dir_separator('x'));
  CHECK(Util::is_dir_separator('/'));
#ifdef _WIN32
  CHECK(Util::is_dir_separator('\\'));
#else
  CHECK(!Util::is_dir_separator('\\'));
#endif
}

TEST_CASE("Util::make_relative_path")
{
  using Util::make_relative_path;

  const TestContext test_context;

  const std::string cwd = Util::get_actual_cwd();
  const std::string actual_cwd = FMT("{}/d", cwd);
#ifdef _WIN32
  const std::string apparent_cwd = actual_cwd;
#else
  const std::string apparent_cwd = FMT("{}/s", cwd);
#endif

  REQUIRE(Util::create_dir("d"));
#ifndef _WIN32
  REQUIRE(symlink("d", "s") == 0);
#endif
  REQUIRE(chdir("d") == 0);
  Util::setenv("PWD", apparent_cwd);

  SUBCASE("No base directory")
  {
    CHECK(make_relative_path("", "/a", "/a", "/a/x") == "/a/x");
  }

  SUBCASE("Path matches neither actual nor apparent CWD")
  {
#ifdef _WIN32
    CHECK(make_relative_path("C:/", "C:/a", "C:/b", "C:/x") == "C:/x");
#else
    CHECK(make_relative_path("/", "/a", "/b", "/x") == "/x");
#endif
  }

  SUBCASE("Match of actual CWD")
  {
#ifdef _WIN32
    CHECK(
      make_relative_path(
        actual_cwd.substr(0, 3), actual_cwd, apparent_cwd, actual_cwd + "/x")
      == "./x");
#else
    CHECK(make_relative_path("/", actual_cwd, apparent_cwd, actual_cwd + "/x")
          == "./x");
#endif
  }

#ifndef _WIN32
  SUBCASE("Match of apparent CWD")
  {
    CHECK(make_relative_path("/", actual_cwd, apparent_cwd, apparent_cwd + "/x")
          == "./x");
  }

  SUBCASE("Match if using resolved (using realpath(3)) path")
  {
    CHECK(make_relative_path("/", actual_cwd, actual_cwd, apparent_cwd + "/x")
          == "./x");
  }
#endif
}

TEST_CASE("Util::matches_dir_prefix_or_file")
{
  CHECK(!Util::matches_dir_prefix_or_file("", ""));
  CHECK(!Util::matches_dir_prefix_or_file("/", ""));
  CHECK(!Util::matches_dir_prefix_or_file("", "/"));

  CHECK(Util::matches_dir_prefix_or_file("aa", "aa"));
  CHECK(!Util::matches_dir_prefix_or_file("aaa", "aa"));
  CHECK(!Util::matches_dir_prefix_or_file("aa", "aaa"));
  CHECK(!Util::matches_dir_prefix_or_file("aa/", "aa"));

  CHECK(Util::matches_dir_prefix_or_file("/aa/bb", "/aa/bb"));
  CHECK(!Util::matches_dir_prefix_or_file("/aa/b", "/aa/bb"));
  CHECK(!Util::matches_dir_prefix_or_file("/aa/bbb", "/aa/bb"));

  CHECK(Util::matches_dir_prefix_or_file("/aa", "/aa/bb"));
  CHECK(Util::matches_dir_prefix_or_file("/aa/", "/aa/bb"));
  CHECK(!Util::matches_dir_prefix_or_file("/aa/bb", "/aa"));
  CHECK(!Util::matches_dir_prefix_or_file("/aa/bb", "/aa/"));

#ifdef _WIN32
  CHECK(Util::matches_dir_prefix_or_file("\\aa", "\\aa\\bb"));
  CHECK(Util::matches_dir_prefix_or_file("\\aa\\", "\\aa\\bb"));
#else
  CHECK(!Util::matches_dir_prefix_or_file("\\aa", "\\aa\\bb"));
  CHECK(!Util::matches_dir_prefix_or_file("\\aa\\", "\\aa\\bb"));
#endif
}

TEST_CASE("Util::normalize_absolute_path")
{
  CHECK(Util::normalize_absolute_path("") == "");
  CHECK(Util::normalize_absolute_path(".") == ".");
  CHECK(Util::normalize_absolute_path("..") == "..");
  CHECK(Util::normalize_absolute_path("...") == "...");
  CHECK(Util::normalize_absolute_path("x/./") == "x/./");

#ifdef _WIN32
  CHECK(Util::normalize_absolute_path("c:/") == "c:/");
  CHECK(Util::normalize_absolute_path("c:\\") == "c:/");
  CHECK(Util::normalize_absolute_path("c:/.") == "c:/");
  CHECK(Util::normalize_absolute_path("c:\\..") == "c:/");
  CHECK(Util::normalize_absolute_path("c:\\x/..") == "c:/");
  CHECK(Util::normalize_absolute_path("c:\\x/./y\\..\\\\z") == "c:/x/z");
#else
  CHECK(Util::normalize_absolute_path("/") == "/");
  CHECK(Util::normalize_absolute_path("/.") == "/");
  CHECK(Util::normalize_absolute_path("/..") == "/");
  CHECK(Util::normalize_absolute_path("/./") == "/");
  CHECK(Util::normalize_absolute_path("//") == "/");
  CHECK(Util::normalize_absolute_path("/../x") == "/x");
  CHECK(Util::normalize_absolute_path("/x/./y/z") == "/x/y/z");
  CHECK(Util::normalize_absolute_path("/x/../y/z/") == "/y/z");
  CHECK(Util::normalize_absolute_path("/x/.../y/z") == "/x/.../y/z");
  CHECK(Util::normalize_absolute_path("/x/yyy/../zz") == "/x/zz");
  CHECK(Util::normalize_absolute_path("//x/yyy///.././zz") == "/x/zz");
#endif
}

TEST_CASE("Util::parse_duration")
{
  CHECK(Util::parse_duration("0s") == 0);
  CHECK(Util::parse_duration("2s") == 2);
  CHECK(Util::parse_duration("1d") == 3600 * 24);
  CHECK(Util::parse_duration("2d") == 2 * 3600 * 24);
  CHECK_THROWS_WITH(
    Util::parse_duration("-2"),
    "invalid suffix (supported: d (day) and s (second)): \"-2\"");
  CHECK_THROWS_WITH(
    Util::parse_duration("2x"),
    "invalid suffix (supported: d (day) and s (second)): \"2x\"");
  CHECK_THROWS_WITH(
    Util::parse_duration("2"),
    "invalid suffix (supported: d (day) and s (second)): \"2\"");
}

TEST_CASE("Util::parse_size")
{
  CHECK(Util::parse_size("0") == 0);
  CHECK(Util::parse_size("42") // Default suffix: G
        == static_cast<uint64_t>(42) * 1000 * 1000 * 1000);
  CHECK(Util::parse_size("78k") == 78 * 1000);
  CHECK(Util::parse_size("78K") == 78 * 1000);
  CHECK(Util::parse_size("1.1 M") == (int64_t(1.1 * 1000 * 1000)));
  CHECK(Util::parse_size("438.55M") == (int64_t(438.55 * 1000 * 1000)));
  CHECK(Util::parse_size("1 G") == 1 * 1000 * 1000 * 1000);
  CHECK(Util::parse_size("2T")
        == static_cast<uint64_t>(2) * 1000 * 1000 * 1000 * 1000);
  CHECK(Util::parse_size("78 Ki") == 78 * 1024);
  CHECK(Util::parse_size("1.1Mi") == (int64_t(1.1 * 1024 * 1024)));
  CHECK(Util::parse_size("438.55 Mi") == (int64_t(438.55 * 1024 * 1024)));
  CHECK(Util::parse_size("1Gi") == 1 * 1024 * 1024 * 1024);
  CHECK(Util::parse_size("2 Ti")
        == static_cast<uint64_t>(2) * 1024 * 1024 * 1024 * 1024);

  CHECK_THROWS_WITH(Util::parse_size(""), "invalid size: \"\"");
  CHECK_THROWS_WITH(Util::parse_size("x"), "invalid size: \"x\"");
  CHECK_THROWS_WITH(Util::parse_size("10x"), "invalid size: \"10x\"");
}

TEST_CASE("Util::read_file and Util::write_file")
{
  TestContext test_context;

  Util::write_file("test", "foo\nbar\n");
  std::string data = Util::read_file("test");
  CHECK(data == "foo\nbar\n");

  Util::write_file("test", "car");
  data = Util::read_file("test");
  CHECK(data == "car");

  Util::write_file("test", "pet", std::ios::app);
  data = Util::read_file("test");
  CHECK(data == "carpet");

  Util::write_file("test", "\n", std::ios::app | std::ios::binary);
  data = Util::read_file("test");
  CHECK(data == "carpet\n");

  Util::write_file("test", "\n", std::ios::app); // text mode
  data = Util::read_file("test");
#ifdef _WIN32
  CHECK(data == "carpet\n\r\n");
#else
  CHECK(data == "carpet\n\n");
#endif

  Util::write_file("size_hint_test", std::string(8192, '\0'));
  CHECK(Util::read_file("size_hint_test", 4096 /*size_hint*/).size() == 8192);

  CHECK_THROWS_WITH(Util::read_file("does/not/exist"),
                    "No such file or directory");

  CHECK_THROWS_WITH(Util::write_file("", "does/not/exist"),
                    "No such file or directory");

  CHECK_THROWS_WITH(Util::write_file("does/not/exist", "does/not/exist"),
                    "No such file or directory");
}

TEST_CASE("Util::{read,write,copy}_file with binary files")
{
  TestContext test_context;

  std::string data;
  for (size_t i = 0; i < 512; ++i) {
    data.push_back(static_cast<char>((32 + i) % 256));
  }

  Util::write_file("test", data);
  CHECK(Util::read_file("test") == data);

  Util::copy_file("test", "copy");
  CHECK(Util::read_file("copy") == data);
}

TEST_CASE("Util::remove_extension")
{
  CHECK(Util::remove_extension("") == "");
  CHECK(Util::remove_extension(".") == "");
  CHECK(Util::remove_extension("...") == "..");
  CHECK(Util::remove_extension("foo") == "foo");
  CHECK(Util::remove_extension("/") == "/");
  CHECK(Util::remove_extension("/foo") == "/foo");
  CHECK(Util::remove_extension("/foo/bar/f") == "/foo/bar/f");
  CHECK(Util::remove_extension("f.txt") == "f");
  CHECK(Util::remove_extension("f.abc.txt") == "f.abc");
  CHECK(Util::remove_extension("/foo/bar/f.txt") == "/foo/bar/f");
  CHECK(Util::remove_extension("/foo/bar/f.abc.txt") == "/foo/bar/f.abc");
}

TEST_CASE("Util::same_program_name")
{
  CHECK(Util::same_program_name("foo", "foo"));
#ifdef _WIN32
  CHECK(Util::same_program_name("FOO", "foo"));
  CHECK(Util::same_program_name("FOO.exe", "foo"));
#else
  CHECK(!Util::same_program_name("FOO", "foo"));
  CHECK(!Util::same_program_name("FOO.exe", "foo"));
#endif
}

// Util::split_into_strings and Util::split_into_views are tested implicitly in
// test_util_Tokenizer.cpp.

TEST_CASE("Util::to_lowercase")
{
  CHECK(Util::to_lowercase("") == "");
  CHECK(Util::to_lowercase("x") == "x");
  CHECK(Util::to_lowercase("X") == "x");
  CHECK(Util::to_lowercase(" x_X@") == " x_x@");
}

TEST_CASE("Util::traverse")
{
  TestContext test_context;

  REQUIRE(Util::create_dir("dir-with-subdir-and-file/subdir"));
  Util::write_file("dir-with-subdir-and-file/subdir/f", "");
  REQUIRE(Util::create_dir("dir-with-files"));
  Util::write_file("dir-with-files/f1", "");
  Util::write_file("dir-with-files/f2", "");
  REQUIRE(Util::create_dir("empty-dir"));

  std::vector<std::string> visited;
  auto visitor = [&visited](const std::string& path, bool is_dir) {
    visited.push_back(FMT("[{}] {}", is_dir ? 'd' : 'f', path));
  };

  SUBCASE("traverse nonexistent path")
  {
    CHECK_THROWS_WITH(
      Util::traverse("nonexistent", visitor),
      "failed to open directory nonexistent: No such file or directory");
  }

  SUBCASE("traverse file")
  {
    CHECK_NOTHROW(Util::traverse("dir-with-subdir-and-file/subdir/f", visitor));
    REQUIRE(visited.size() == 1);
    CHECK(visited[0] == "[f] dir-with-subdir-and-file/subdir/f");
  }

  SUBCASE("traverse empty directory")
  {
    CHECK_NOTHROW(Util::traverse("empty-dir", visitor));
    REQUIRE(visited.size() == 1);
    CHECK(visited[0] == "[d] empty-dir");
  }

  SUBCASE("traverse directory with files")
  {
    CHECK_NOTHROW(Util::traverse("dir-with-files", visitor));
    REQUIRE(visited.size() == 3);
    std::string f1 = os_path("[f] dir-with-files/f1");
    std::string f2 = os_path("[f] dir-with-files/f2");
    CHECK(((visited[0] == f1 && visited[1] == f2)
           || (visited[0] == f2 && visited[1] == f1)));
    CHECK(visited[2] == "[d] dir-with-files");
  }

  SUBCASE("traverse directory hierarchy")
  {
    CHECK_NOTHROW(Util::traverse("dir-with-subdir-and-file", visitor));
    REQUIRE(visited.size() == 3);
    CHECK(visited[0] == os_path("[f] dir-with-subdir-and-file/subdir/f"));
    CHECK(visited[1] == os_path("[d] dir-with-subdir-and-file/subdir"));
    CHECK(visited[2] == "[d] dir-with-subdir-and-file");
  }
}

TEST_CASE("Util::wipe_path")
{
  TestContext test_context;

  SUBCASE("Wipe nonexistent path")
  {
    CHECK_NOTHROW(Util::wipe_path("a"));
  }

  SUBCASE("Wipe file")
  {
    Util::write_file("a", "");
    CHECK_NOTHROW(Util::wipe_path("a"));
    CHECK(!Stat::stat("a"));
  }

  SUBCASE("Wipe directory")
  {
    REQUIRE(Util::create_dir("a/b"));
    Util::write_file("a/1", "");
    Util::write_file("a/b/1", "");
    CHECK_NOTHROW(Util::wipe_path("a"));
    CHECK(!Stat::stat("a"));
  }

  SUBCASE("Wipe bad path")
  {
#ifdef _WIN32
    const char error[] = "failed to rmdir .: Permission denied";
#elif defined(_AIX)
    const char error[] = "failed to rmdir .: Device busy";
#else
    const char error[] = "failed to rmdir .: Invalid argument";
#endif
    CHECK_THROWS_WITH(Util::wipe_path("."), error);
  }
}

TEST_SUITE_END();
