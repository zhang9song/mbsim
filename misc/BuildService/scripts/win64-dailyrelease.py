#!/usr/bin/python

from __future__ import print_function # to enable the print function for backward compatiblity with python2
import os
import time
import email
import subprocess
import simplesandbox
import shutil
import buildSystemState

SRCDIR="/home/mbsim/win64-dailyrelease"
OUTDIR="/var/www/html/mbsim/win64-dailyrelease"
URL="http://www.mbsim-env.de/mbsim/win64-dailyrelease"
SCRIPTDIR=os.path.dirname(os.path.realpath(__file__))


os.environ["PKG_CONFIG_PATH"]=SRCDIR+"/local/lib/pkgconfig:/home/mbsim/3rdparty/casadi-local-win64/lib/pkgconfig:"+ \
  "/home/mbsim/3rdparty/coin-local-win64/lib/pkgconfig:/usr/x86_64-w64-mingw32/sys-root/mingw/lib/pkgconfig:"+ \
  "/home/mbsim/3rdparty/xerces-c-local-win64/lib/pkgconfig:/home/mbsim/3rdparty/libarchive-local-win64/lib/pkgconfig:"+\
  "/usr/x86_64-w64-mingw32/sys-root/mingw/lib/pkgconfig"
os.environ["WINEPATH"]="/usr/x86_64-w64-mingw32/sys-root/mingw/bin;/home/mbsim/3rdparty/lapack-local-win64/bin;"+ \
  "/home/mbsim/3rdparty/xerces-c-local-win64/bin;/home/mbsim/3rdparty/casadi-local-win64/lib;"+ \
  "/home/mbsim/win64-dailyrelease/local/bin;/home/mbsim/3rdparty/octave-local-win64/bin;"+ \
  "/home/mbsim/3rdparty/hdf5-local-win64/bin;/home/mbsim/3rdparty/libarchive-local-win64/bin"
os.environ["CXXFLAGS"]="-g -O2"
os.environ["CFLAGS"]="-g -O2"
os.environ["FFLAGS"]="-g -O2"
os.environ["MOC"]="/usr/x86_64-w64-mingw32/bin/moc"
os.environ["UIC"]="/usr/x86_64-w64-mingw32/bin/uic"
os.environ["RCC"]="/usr/x86_64-w64-mingw32/bin/rcc"
os.environ["PLATFORM"]="Windows" # required for source code examples
os.environ["CXX"]="x86_64-w64-mingw32-g++" # required for source code examples

if subprocess.call([SCRIPTDIR+"/build.py", "--buildSystemRun", "--rotate", "14", "-j", "2", "--sourceDir", SRCDIR, "--prefix",
  SRCDIR+"/local", "--reportOutDir", OUTDIR+"/report", "--url", URL+"/report", "--buildType", "win64-dailyrelease",
  "--passToConfigure", "--enable-shared", "--disable-static", "--build=x86_64-redhat-linux", "--host=x86_64-w64-mingw32",
  "--with-javajnicflags=-I/usr/lib/jvm/java-1.6.0-openjdk-1.6.0.37.x86_64/include -I"+SCRIPTDIR+"/buildPreparation/windows",
  "--with-mkoctfile=/home/mbsim/3rdparty/octave-local-win64/bin/mkoctfile.exe",
  "--with-hdf5-prefix=/home/mbsim/3rdparty/hdf5-local-win64", "--with-windres=x86_64-w64-mingw32-windres",
  "--with-lapack-lib-prefix=/home/mbsim/3rdparty/lapack-local-win64/lib",
  "--with-qwt-inc-prefix=/home/mbsim/3rdparty/qwt-6.1.1/src", "--with-qwt-lib-prefix=/home/mbsim/3rdparty/qwt-6.1.1/lib",
  "--with-swigpath=/home/mbsim/3rdparty/swig-local-linux64/bin",
  "PYTHON_CFLAGS=-I/home/mbsim/3rdparty/python-win64/include -DMS_WIN64",
  "PYTHON_LIBS=-L/home/mbsim/3rdparty/python-win64 -lpython27",
  "PYTHON_BIN=/home/mbsim/3rdparty/python-win64/python.exe",
  "--passToRunexamples", "--disableCompare",
  "--disableValidate", "--exeExt", ".exe", "xmlflat/hierachical_modelling", "xml/hierachical_modelling",
  "xml/time_dependent_kinematics", "xml/hydraulics_ballcheckvalve", "fmi/simple_test", "fmi/hierachical_modelling",
  "fmi/sphere_on_plane", "mechanics/basics/hierachical_modelling", "mechanics/basics/time_dependent_kinematics"])!=0:
  print("win64-dailyrelease failed.")

f=open(OUTDIR+"/report_distribute/distribute.out", "w")
ret=simplesandbox.call([SCRIPTDIR+"/distribute.py", "/home/mbsim/win64-dailyrelease/local", "--distFile",
                       OUTDIR+"/download/mbsim-env-win64-shared-build-xxx"],
                       shareddir=[OUTDIR+"/download"],
                       buildSystemRun=True,
                       stderr=subprocess.STDOUT, stdout=f)
buildSystemState.update("win64-dailyrelease-distribution", "Distribution Failed: win64-dailyrelease",
                        "Unable to create the binary distribution file.", URL+"/report_distribute/distribute.out",
                        0 if ret==0 else 1, 1)
f.close()
