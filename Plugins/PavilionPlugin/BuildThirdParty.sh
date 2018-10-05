#!/bin/bash

if ! [[ -e "./ThirdParty/Release" ]]; then
    echo "Please run this script from the plugin folder!"
    exit 1
fi

TOPDIR=`pwd`
TP_DIR=$TOPDIR/ThirdParty
TP_REL_DIR=$TOPDIR/ThirdParty/Release

UE_TP_DIR=$TOPDIR/Source/ThirdParty

echo "Updating dependencies"

git submodule update --init

echo "Building dependencies"

#-------------
# Cap'n Proto
#-------------

(
    cd "$TP_DIR/Build"
    mkdir -p capnproto
    cd capnproto
    cmake $TP_DIR/Source/capnproto -DCMAKE_CXX_FLAGS="-O3 -fno-rtti -fno-exceptions" -DCMAKE_INSTALL_PREFIX="$TP_REL_DIR/capnproto" -DBUILD_TESTING=0
    make -j install
)

#-------------
# cROS
#-------------

(
    cd "$TP_DIR/Build"
    mkdir -p cros
    cd cros
    cmake $TP_DIR/Source/cros -DCMAKE_CXX_FLAGS="-O3 -fno-rtti -fno-exceptions" -DCMAKE_INSTALL_PREFIX="$TP_REL_DIR/cros" -DBUILD_TESTING=0
    make -j install
)

#-------------
# assimp
#-------------

(
    cd "$TP_DIR/Build"
    mkdir -p assimp
    cd assimp
    cmake $TP_DIR/Source/assimp -DCMAKE_C_FLAGS="-fno-rtti" -DCMAKE_CXX_FLAGS="-O3 -fno-rtti" -DCMAKE_INSTALL_PREFIX="$TP_REL_DIR/assimp" -DASSIMP_BUILD_COLLADA_IMPORTER=ON -DASSIMP_BUILD_DAE_IMPORTER=ON -DASSIMP_BUILD_FBX_IMPORTER=ON -DASSIMP_BUILD_STL_IMPORTER=ON -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=FALSE -DASSIMP_BUILD_IFC_IMPORTER=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_ZLIB=ON -DCMAKE_BUILD_TYPE=Release
    make -j install
)

#-------------
# ignition-math
#-------------

(
    cd "$TP_DIR/Build"
    mkdir -p ignition-math
    cd ignition-math
    cmake $TP_DIR/Source/ignition-math -DCMAKE_C_FLAGS="-fno-rtti" -DCMAKE_CXX_FLAGS="-O3 -fno-rtti" -DCMAKE_INSTALL_PREFIX="$TP_REL_DIR/ignition-math" -DCMAKE_BUILD_TYPE=Release
    make -j install
    install_name_tool -add_rpath "@loader_path/../../ThirdParty/Release/ignition-math/lib" "$TP_REL_DIR/ignition-math/lib/libignition-math4.4.dylib"
)

#-------------
# sdformat
#-------------

(
    cd "$TP_DIR/Build"
    mkdir -p sdformat
    cd sdformat
    cmake $TP_DIR/Source/sdformat -DCMAKE_C_FLAGS_ALL="-fno-rtti -fno-exceptions" -DCMAKE_CXX_FLAGS="-O3 -fno-rtti -fno-exceptions" -DCMAKE_INSTALL_PREFIX="$TP_REL_DIR/sdformat" -DCMAKE_BUILD_TYPE=Release
    make -j install
    install_name_tool -add_rpath "$TP_REL_DIR/ignition-math/lib" "$TP_REL_DIR/sdformat/lib/libsdformat.dylib"
    install_name_tool -id "@rpath/libsdformat.6.dylib" "$TP_REL_DIR/sdformat/lib/libsdformat.dylib"
)

#----------------------
# Finalize UBT Modules
#----------------------

(
    cd $TP_REL_DIR/capnproto/include
    for i in $(find . -depth 1); do
        cp -Rf $i/. $UE_TP_DIR/capnproto/Public/$i
    done
)

(
    cd $TP_REL_DIR/cros/include
    for i in $(find . -depth 1); do
        cp -Rf $i/. $UE_TP_DIR/cros/Public/$i
    done
)

(
    cd $TP_REL_DIR/assimp/include
    for i in $(find . -depth 1); do
        cp -Rf $i/. $UE_TP_DIR/assimp/Public/$i
    done
)

(
    cd $TP_REL_DIR/sdformat/include/sdformat-6.0
    for i in $(find . -depth 1); do
        cp -Rf $i/. $UE_TP_DIR/sdformat/Public/$i
    done
)

(
    cd /usr/local/opt/boost/include/boost
    mkdir -p $UE_TP_DIR/sdformat/Public/boost/type_traits
    cp -Rf ./type_traits/. $UE_TP_DIR/sdformat/Public/boost/type_traits
    cp -f ./any.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./variant.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./version.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./config.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./throw_exception.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./blank_fwd.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./static_assert.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./limits.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./current_function.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./assert.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./cstdint.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./call_traits.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -Rf ./utility/. $UE_TP_DIR/sdformat/Public/boost/utility
    cp -Rf ./exception/. $UE_TP_DIR/sdformat/Public/boost/exception
    cp -Rf ./variant/. $UE_TP_DIR/sdformat/Public/boost/variant
    cp -Rf ./preprocessor/. $UE_TP_DIR/sdformat/Public/boost/preprocessor
    cp -Rf ./integer/. $UE_TP_DIR/sdformat/Public/boost/integer
    cp -f ./integer_fwd.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./aligned_storage.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./checked_delete.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -f ./blank.hpp $UE_TP_DIR/sdformat/Public/boost/
    cp -Rf ./core/. $UE_TP_DIR/sdformat/Public/boost/core
    cp -Rf ./mpl/. $UE_TP_DIR/sdformat/Public/boost/mpl
    cp -Rf ./detail/. $UE_TP_DIR/sdformat/Public/boost/detail
    cp -Rf ./move/. $UE_TP_DIR/sdformat/Public/boost/move
    cp -Rf ./functional/. $UE_TP_DIR/sdformat/Public/boost/functional
    cp -Rf ./type_index/. $UE_TP_DIR/sdformat/Public/boost/type_index
    cp -Rf ./math/. $UE_TP_DIR/sdformat/Public/boost/math
    cp -f ./type_index.hpp $UE_TP_DIR/sdformat/Public/boost/
    # mkdir -p $UE_TP_DIR/sdformat/Public/boost/config
    # cp -Rf ./config/. $UE_TP_DIR/sdformat/Public/boost/config
)

(
    cd $TP_REL_DIR/ignition-math/include/ignition/math4
    for i in $(find . -depth 1); do
        cp -Rf $i/. $UE_TP_DIR/ignition/Public/$i
    done
)