#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/xuzhongping/Desktop/Dev/myGithub/ccache
  make -f /Users/xuzhongping/Desktop/Dev/myGithub/ccache/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/xuzhongping/Desktop/Dev/myGithub/ccache
  make -f /Users/xuzhongping/Desktop/Dev/myGithub/ccache/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/xuzhongping/Desktop/Dev/myGithub/ccache
  make -f /Users/xuzhongping/Desktop/Dev/myGithub/ccache/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/xuzhongping/Desktop/Dev/myGithub/ccache
  make -f /Users/xuzhongping/Desktop/Dev/myGithub/ccache/CMakeScripts/ReRunCMake.make
fi

