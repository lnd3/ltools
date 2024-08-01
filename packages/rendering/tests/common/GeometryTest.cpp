#include "testing/Test.h"
#include "logging/Log.h"
#include "rendering/Geometry.h"
#include "logging/String.h"

TEST(GeometryTest, GeometryParsing) {
    l::rendering::Mesh mesh;
    TEST_TRUE(mesh.Load("./tests/data/modelformats/old_truck/old_truck_2.dae"), "");

    //mesh.AssimpImport("./tests/data/modelformats/old_truck/old_truck_2.dae");

    return 0;
}


