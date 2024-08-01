#include "testing/Test.h"
#include "logging/Log.h"
#include "rendering/ColladaLoader.h"
#include "logging/String.h"

TEST(ColladaLoaderTest, LoadAndParseMultiple) {
    auto alienPlanet = l::rendering::LoadColladaAsset("./tests/data/modelformats/AlienPlanet.dae");
    TEST_TRUE(alienPlanet.mGeometryNodes.size() == 1, "");
    auto marsPlanet = l::rendering::LoadColladaAsset("./tests/data/modelformats/MarsPlanet.dae");
    TEST_TRUE(marsPlanet.mGeometryNodes.size() == 1, "");
    auto oildrum = l::rendering::LoadColladaAsset("./tests/data/modelformats/oildrum.dae");
    TEST_TRUE(oildrum.mGeometryNodes.size() == 1, "");
    auto oldTruck = l::rendering::LoadColladaAsset("./tests/data/modelformats/old_truck/old_truck_2.dae");
    TEST_TRUE(oldTruck.mGeometryNodes.size() == 3, "");
    auto rock1 = l::rendering::LoadColladaAsset("./tests/data/modelformats/Rock1.dae");
    TEST_TRUE(rock1.mGeometryNodes.size() == 1, "");
    auto bullet = l::rendering::LoadColladaAsset("./tests/data/modelformats/shareablebullet.dae");
    TEST_TRUE(bullet.mGeometryNodes.size() == 1, "");
    auto sword = l::rendering::LoadColladaAsset("./tests/data/modelformats/sword.dae");
    TEST_TRUE(sword.mGeometryNodes.size() == 1, "");
    auto ussEnterprise = l::rendering::LoadColladaAsset("./tests/data/modelformats/enterprise/USSEnterprise.dae");
    TEST_TRUE(ussEnterprise.mGeometryNodes.size() == 1, "");

    return 0;
}

TEST(ColladaLoaderTest, LoadAndVerify) {

    auto colladaNodes = l::rendering::LoadColladaAsset("./tests/data/modelformats/old_truck/old_truck_2.dae");

    TEST_TRUE(l::string::cstring_equal(colladaNodes.mGeometryNodes[0].mId.c_str(), "TruckMat2-mesh"), "");
    TEST_TRUE(l::string::cstring_equal(colladaNodes.mGeometryNodes[0].mSourceNodes[0].mId.c_str(), "TruckMat2-mesh-positions"), "");
    TEST_TRUE(colladaNodes.mGeometryNodes[0].mSourceNodes[0].mFloatArray.mFloatArray[1] == -4.67387009f, "");

    return 0;
}

