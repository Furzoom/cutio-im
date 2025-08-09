// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include <getopt.h>
#include "gtest/gtest.h"
#include "test_utils/global_env.h"

static bool clean_clutter = true;

static void Usage(const char* exe);
static bool ParseOptions(int argc, char** argv);

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  if (!ParseOptions(argc, argv)) {
    exit(1);
  }

  testing::AddGlobalTestEnvironment(cutio::test::GlobalEnv::Instance());
  cutio::test::GlobalEnv::Instance()->SetCleanClutter(clean_clutter);

  return RUN_ALL_TESTS();
}

static void Usage(const char* exe) {
  printf("Usage:\n");
  printf(" %s [options]\n", exe);
  printf("\n");
  printf("Options:\n");
  printf(" -h --help                display this help and exit\n");
  printf(" --keep_test_file         keep test temporary files\n");
  printf("\n");
}

static bool ParseOptions(int argc, char** argv) {
  int c;

  opterr = 0;
  while (true) {
    int option_index = 0;
    static struct option long_options[] = {
        {"help",            no_argument,        nullptr,  'h'},
        {"keep_test_file",  no_argument,        nullptr,  0},
        {nullptr,           0,                  nullptr,  0}
    };

    c = getopt_long(argc, argv, ":h", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 0:
      {
        std::string opt = long_options[option_index].name;
        if (opt == "keep_test_file") {
          clean_clutter = false;
        } else {
          printf("[error] invalid option: --%s\n", opt.c_str());
          return false;
        }
      }
        break;
      case 'h':
        Usage(argv[0]);
        exit(0);
      case ':':
        if (optopt == 0) {
          printf("[error] missing argument %s\n", argv[optind-1]);
        } else {
          printf("[error] missing argument -%c\n", optopt);
        }
        return false;

      case '?':
        printf("[error] invalid option: %s\n", argv[optind-1]);
        return false;

      default:
        printf("[error] uncaught option 0%o\n", c);
        return false;
    }
  }

  if (optind < argc) {
    printf("[error] unsupported positional options: ");
    while (optind < argc) {
      printf("%s ", argv[optind]);
      optind++;
    }
    printf("\n");
    return false;
  }

  return true;
}
