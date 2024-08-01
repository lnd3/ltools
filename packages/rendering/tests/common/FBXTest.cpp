#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"
#include "filesystem/File.h"

#define OFBX_DEFAULT_DELETER
#include <openfbx/src/ofbx.h>

#include <memory>

TEST(FBXTest, SimpleLoadFBXFile) {
    
    l::filesystem::File f("./tests/data/modelformats/old_truck/old_truck_2.fbx");
    auto count = f.fileSize();
    f.modeReadPreload().modeBinary();

    TEST_TRUE(f.open(), "");

    std::unique_ptr<ofbx::IScene> oldTruckScene;

    {
        ofbx::u8* buffer = new ofbx::u8[count];
        f.read(buffer, count);
        oldTruckScene = std::unique_ptr<ofbx::IScene>(ofbx::load(buffer, static_cast<int>(count), 0));
        delete[] buffer;
    }

    TEST_TRUE(oldTruckScene->getGeometryCount() == 4, "");

    oldTruckScene.reset();

    return 0;
}

