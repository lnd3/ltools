#include "testing/Test.h"
#include "logging/LoggingAll.h"
#include "rendering/Geometry.h"

TEST(GeometryTest, GeometryParsing) {
    l::rendering::Mesh mesh;
    TEST_TRUE(mesh.Load("./tests/data/modelformats/old_truck/old_truck_2.dae"), "");

    return 0;
}


